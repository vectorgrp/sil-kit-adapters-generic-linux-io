// SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
// SPDX-License-Identifier: MIT

#include "SilKitAdapterGenericLinuxIO.hpp"

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

#include "common/ParticipantCreation.hpp"

#include "silkit/SilKit.hpp"
#include "silkit/config/all.hpp"
#include "silkit/services/pubsub/all.hpp"
#include "silkit/services/logging/all.hpp"
#include "silkit/util/serdes/Serialization.hpp"

using namespace util;
using namespace adapters;

int main(int argc, char** argv)
{
    // handle arguments
    if (findArg(argc, argv, helpArg, argv) != NULL)
    {
        PrintHelp(true);
        return NO_ERROR;
    }

    if (findArg(argc, argv, versionArg, argv) != NULL)
    {
        PrintVersion();
        return NO_ERROR;
    }

    try
    {
        throwInvalidCliIf(thereAreUnknownArguments(
            argc, argv, {&adapterConfigurationArg, &regUriArg, &logLevelArg, &participantNameArg, &configurationArg},
            {&helpArg, &versionArg}));

        const std::string adapterConfiguration = getArgDefault(argc, argv, adapterConfigurationArg, "");

        if (adapterConfiguration.empty())
        {
            std::cout << "[error] Missing mandatory argument \"--adapter-configuration\"\n" << std::endl;
            PrintHelp(false);
            return CLI_ERROR;
        }

        SilKit::Services::Logging::ILogger* logger;
        SilKit::Services::Orchestration::ILifecycleService* lifecycleService;
        std::promise<void> runningStatePromise;

        // configure the participant
        std::string participantName = defaultParticipantName;
        const auto participant =
            CreateParticipant(argc, argv, logger, &participantName, &lifecycleService, &runningStatePromise);

        // handle the YAML config file
        YAML::Node configFile;
        Util::LoadYAMLConfigFile(configFile, adapterConfiguration, logger);

        if (configFile.IsNull())
        {
            throw YamlError("Loading YAML configuration file failed.");
        }

        asio::io_context ioContext;

        // initialize chip and values
        std::vector<std::unique_ptr<IOAdapter>> ioAdapters;

        InotifyHandler::SetLogger(logger);

        AdapterFactory::ConstructAdAdapters(configFile, ioAdapters, participant.get(), ioContext);
        AdapterFactory::ConstructChardevAdapters(configFile, ioAdapters, participant.get(), ioContext);

#ifndef QNX_BUILD
        std::vector<std::unique_ptr<GpioWrapper::Chip>> chips;
        AdapterFactory::ConstructGpioAdapters(configFile, ioAdapters, chips, participant.get(), ioContext);
#endif

        std::thread ioContextThread([&]() -> void { ioContext.run(); });

        auto finalStateFuture = lifecycleService->StartLifecycle();

        promptForExit();

        Stop(ioContext, ioContextThread, *logger, &runningStatePromise, lifecycleService, &finalStateFuture);
    }
    catch (const SilKit::ConfigurationError& error)
    {
        std::cerr << "Invalid configuration: " << error.what() << std::endl;
        return CONFIGURATION_ERROR;
    }
    catch (const InvalidCli&)
    {
        PrintHelp(false);
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
