// SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
// SPDX-License-Identifier: MIT

#include <string>
#include <thread>
#include <vector>
#include <chrono>
#include <sys/inotify.h>

#include "IOAdapter.hpp"
#include "InotifyHandler.hpp"
#include "AdapterFactory.hpp"

#include "chardev/adapter/ChardevAdapter.hpp"
#include "advalues/adapter/AdAdapter.hpp"

#ifndef QNX_BUILD
#include "gpio/adapter/GpioAdapter.hpp"
#endif

#include "util/Exceptions.hpp"
#include "util/YamlHelper.hpp"
#include "util/Parsing.hpp"
#include "util/SignalHandler.hpp"

#include "silkit/SilKit.hpp"
#include "silkit/config/all.hpp"
#include "silkit/services/pubsub/all.hpp"
#include "silkit/services/logging/all.hpp"
#include "silkit/util/serdes/Serialization.hpp"

using namespace std::chrono_literals;
using namespace adapters;

int main(int argc, char** argv)
{
    // handle arguments
    if (Parsing::FindArg(argc, argv, Parsing::helpArg, argv) != NULL)
    {
        Parsing::PrintHelp(true);
        return NO_ERROR;
    }

    const std::string participantName = Parsing::GetArgDefault(argc, argv, Parsing::participantNameArg, "SilKitAdapterGenericLinuxIO");

    const std::string registryURI = Parsing::GetArgDefault(argc, argv, Parsing::regUriArg, "silkit://localhost:8501");

    const std::string adapterConfiguration = Parsing::GetArgDefault(argc, argv, Parsing::adapterConfigurationArg, "");

    const std::string configurationFile = Parsing::GetArgDefault(argc, argv, Parsing::configurationArg, "");

    try
    {
        throwInvalidCliIf(Parsing::ThereAreUnknownArguments(argc, argv));

        if (adapterConfiguration.empty())
        {
            std::cout << "[error] Missing mandatory argument \"--adapter-configuration\"\n" << std::endl;
            Parsing::PrintHelp(false);
            return CLI_ERROR;
        }

        // configure the participant
        std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfiguration;
        if (!configurationFile.empty())
        {
            participantConfiguration = SilKit::Config::ParticipantConfigurationFromFile(configurationFile);
            static const auto conflictualArguments = {
                &Parsing::logLevelArg,
                /* &participantNameArg, &regUriArg are correctly handled by SilKit if one is overwritten.*/};
            for (const auto* conflictualArgument : conflictualArguments)
            {
                if (Parsing::FindArg(argc, argv, *conflictualArgument, argv) != NULL)
                {
                    auto configFileName = configurationFile;
                    if (configurationFile.find_last_of("/\\") != std::string::npos)
                    {
                        configFileName = configurationFile.substr(configurationFile.find_last_of("/\\") + 1);
                    }
                    std::cout << "[info] Be aware that argument given with " << *conflictualArgument 
                              << " can be overwritten by a different value defined in the given configuration file "
                              << configFileName << std::endl;
                }
            }
        }
        else
        {
            const std::string loglevel = Parsing::GetArgDefault(argc, argv, Parsing::logLevelArg, "Info");
            const std::string participantConfigurationString =
                R"({ "Logging": { "Sinks": [ { "Type": "Stdout", "Level": ")" + loglevel + R"("} ] } })";
            participantConfiguration =
                SilKit::Config::ParticipantConfigurationFromString(participantConfigurationString);
        }

        auto participant =
            SilKit::CreateParticipant(std::move(participantConfiguration), participantName, registryURI);

        auto logger = participant->GetLogger();

        // handle Sil Kit life cycle
        auto* lifecycleService = participant->CreateLifecycleService({ SilKit::Services::Orchestration::OperationMode::Autonomous });
        auto* systemMonitor = participant->CreateSystemMonitor();
        std::promise<void> runningStatePromise;

        systemMonitor->AddParticipantStatusHandler(
            [&runningStatePromise, participantName](const SilKit::Services::Orchestration::ParticipantStatus& status) {
                if (participantName == status.participantName)
                {
                    if (status.state == SilKit::Services::Orchestration::ParticipantState::Running)
                    {
                        runningStatePromise.set_value();
                    }
                }
            });
        
        // handle the YAML config file 
        YAML::Node configFile;
        Util::LoadYAMLConfigFile(configFile, adapterConfiguration, logger);

        if (configFile.IsNull())
        {
            throw YamlError("Loading YAML configuration file failed.");
        }

        asio::io_context ioc;

        // initialize chip and values
        std::vector<std::unique_ptr<IOAdapter>> ioAdapters;

        InotifyHandler::SetLogger(logger);

        AdapterFactory::ConstructAdAdapters(configFile, ioAdapters, participant.get(), ioc);
        AdapterFactory::ConstructChardevAdapters(configFile, ioAdapters, participant.get(), ioc);

#ifndef QNX_BUILD
        std::vector<std::unique_ptr<GpioWrapper::Chip>> chips;
        AdapterFactory::ConstructGpioAdapters(configFile, ioAdapters, chips, participant.get(), ioc);
#endif

        std::thread iocThread([&]() -> void {
            ioc.run();
        });

        auto finalStateFuture = lifecycleService->StartLifecycle();

        promptForExit();

        ioc.stop();

        if (iocThread.joinable())
        {
            iocThread.join();
        }

        auto runningStateFuture = runningStatePromise.get_future();
        auto futureStatus = runningStateFuture.wait_for(15s);
        if (futureStatus != std::future_status::ready)
        {
            logger->Debug("Lifecycle Service Stopping: timed out while checking if the participant is currently running.");
        }
        lifecycleService->Stop("Adapter stopped by the user.");

        auto finalState = finalStateFuture.wait_for(15s);
        if (finalState != std::future_status::ready)
        {
            logger->Debug("Lifecycle service stopping: timed out");
        }

        logger->Debug("Simulation stopped. Final State: " + std::to_string(static_cast<std::int16_t>(finalState)));
    }
    catch (const SilKit::ConfigurationError& error)
    {
        std::cerr << "Invalid configuration: " << error.what() << std::endl;
        return CONFIGURATION_ERROR;
    }
    catch (const InvalidCli&)
    {
        std::cerr << "Invalid command line arguments." << std::endl;
        return CLI_ERROR;
    }
    catch (const SilKit::SilKitError& error)
    {
        std::cerr << "SIL Kit runtime error: " << error.what() << std::endl;
        return SILKIT_ERROR;
    }
    catch (const YamlError& error)
    {
        std::cerr << "YAML configuration file error: " << error.what() << std::endl;
        return YAML_ERROR;
    }
    catch (const InotifyError& error)
    {
        std::cerr << "Inotify error: " << error.what() << std::endl;
        return INOTIFY_ERROR;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Something went wrong: " << error.what() << std::endl;
        return OTHER_ERROR;
    }

    return NO_ERROR;
}
