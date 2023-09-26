// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "SilKitAdapterGPIO.hpp"

#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>

#include "Exceptions.hpp"
#include "ChipDatas.hpp"
#include "GpioChip.hpp"

#include "silkit/SilKit.hpp"
#include "silkit/config/all.hpp"
#include "silkit/services/pubsub/all.hpp"
#include "silkit/services/logging/all.hpp"
#include "silkit/util/serdes/Serialization.hpp"

#include "gpiod.hpp"

using namespace std::chrono_literals;
using namespace adapters;

std::atomic<bool> newMsg;
std::atomic<bool> isWorking;

int main(int argc, char** argv)
{
    const std::string gpiochipName = "/dev/gpiochip0";

    const std::string participantName = "SilKitAdapterGPIO";

    const std::string registryURI = "silkit://localhost:8501";

    const std::string topicPublisher = "Topic1";

    const std::string topicSubscriber = "Topic2";

    const std::string loglevel = "Info";

    try
    {
        newMsg = false;
        isWorking = true;

        // Configure the participant
        std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfiguration;
        const std::string participantConfigurationString =
            R"({ "Logging": { "Sinks": [ { "Type": "Stdout", "Level": ")" + loglevel + R"("} ] } })";
        participantConfiguration =
            SilKit::Config::ParticipantConfigurationFromString(participantConfigurationString);

        auto participant =
            SilKit::CreateParticipant(std::move(participantConfiguration), participantName, registryURI);

        auto logger = participant->GetLogger();
        logger->Info("Creating participant " + participantName + " with registry " + registryURI);

        // Configure Publisher/Subscriber
        SilKit::Services::PubSub::PubSubSpec pubDataSpec{topicPublisher, SilKit::Util::SerDes::MediaTypeData()};
        pubDataSpec.AddLabel("KeyA", "ValA", SilKit::Services::MatchingLabel::Kind::Optional);

        SilKit::Services::PubSub::PubSubSpec subDataSpec{topicSubscriber, SilKit::Util::SerDes::MediaTypeData()};
        subDataSpec.AddLabel("KeyB", "ValB", SilKit::Services::MatchingLabel::Kind::Optional);

        // Handling gpio chip
        GpioChip gpioChip(gpiochipName);

        const auto directions = gpioChip.GetGpioOffsetsDirection();
        const auto values = gpioChip.GetGpioOffsetsValues();

        // Initializing internal gpio chip object
        ChipDatas chipDatas(ConvertGpiodValuesToBits(values), ConvertGpiodDirectionsToBits(directions));

        // Set history to 1 to keep last data published on the topic, and publishing initial values
        auto dataPublisher = participant->CreateDataPublisher(participantName + "_pub", pubDataSpec, 1);
        logger->Info("Serializing initial datas and publishing on " + topicPublisher);
        dataPublisher->Publish(chipDatas.Serialize());

        auto dataSubscriber = participant->CreateDataSubscriber(
            participantName + "_sub", subDataSpec,
            [&](SilKit::Services::PubSub::IDataSubscriber* subscriber, 
                const SilKit::Services::PubSub::DataMessageEvent& dataMessageEvent)
            {
                // If new values are received for the gpio chip
                logger->Info("New values received on " + topicSubscriber);
                newMsg = true;

                // Updating internal chip datas
                chipDatas.SpecificDeserialize(SilKit::Util::ToStdVector(dataMessageEvent.data));

                // Updating gpio chip
                logger->Info("Updating " + gpiochipName);
                gpioChip.SetGpioValues(chipDatas);
                
                // Sending new gpio values on topicPublisher
                logger->Info("Serializing datas and publishing on " + topicPublisher);
                dataPublisher->Publish(chipDatas.Serialize());

                newMsg = false;
            });

        auto* lifecycleService = participant->CreateLifecycleService({ SilKit::Services::Orchestration::OperationMode::Autonomous });

        lifecycleService->SetStopHandler([logger]() {
            logger->Debug("Stop handler called");
            });

        lifecycleService->SetShutdownHandler([logger]() {
            logger->Debug("Shutdown handler called");
            });

        // Gpiochip events handling
        std::thread workerThread;
        std::promise<void> startHandlerPromise;
        auto startHandlerFuture = startHandlerPromise.get_future();
        lifecycleService->SetCommunicationReadyHandler([&]() {
            logger->Debug("Communication ready handler called for " + participantName);
            workerThread = std::thread{ [&]() {
                startHandlerFuture.get();

                while (isWorking) 
                {
                    if (!newMsg) 
                    {
                        ::gpiod::edge_event_buffer buffer;
                        const bool newEvent = gpioChip.ReadGpioEvents(chipDatas, buffer);

                        if (newEvent) 
                        {
                            for (const auto& event : buffer) 
                            {
                                auto line = event.line_offset();

                                if (event.type() == ::gpiod::edge_event::event_type::RISING_EDGE) 
                                {
                                    logger->Info(gpiochipName + " : RISING_EDGE line " + std::to_string(static_cast<unsigned int>(line)));
                                    chipDatas.SetLineValue(::gpiod::line::offset(line), 1);
                                    chipDatas.SetLineDirection(::gpiod::line::offset(line), 0);
                                }
                                else 
                                {
                                    logger->Info(gpiochipName + " : FALLING_EDGE line " + std::to_string(static_cast<unsigned int>(line)));
                                    chipDatas.SetLineValue(::gpiod::line::offset(line), 0);
                                    chipDatas.SetLineDirection(::gpiod::line::offset(line), 0);
                                }
                            }

                            logger->Info("Serializing datas and publishing on " + topicPublisher);
                            dataPublisher->Publish(chipDatas.Serialize());
                        }
                    }
                }
                lifecycleService->Stop("User Requested to Stop");
            } };
            });

        lifecycleService->SetStartingHandler([&]() {
            startHandlerPromise.set_value();
            });

        auto finalStateFuture = lifecycleService->StartLifecycle();
        logger->Info("Press enter to stop the process...");
        std::cin.ignore();

        // Exit while loop in thread
        newMsg = true;
        isWorking = false;
        
        if (workerThread.joinable())
        {
            workerThread.join();
        }
        auto finalState = finalStateFuture.get();
        logger->Debug("Simulation stopped. Final State: " + static_cast<int16_t>(finalState));

    }
    catch (const SilKit::ConfigurationError& error)
    {
        std::cerr << "Invalid configuration: " << error.what() << std::endl;
        promptForExit();
        return CONFIGURATION_ERROR;
    }
    catch (const InvalidCli&)
    {
        std::cerr << std::endl << "Invalid command line arguments." << std::endl;
        promptForExit();
        return CLI_ERROR;
    }
    catch (const SilKit::SilKitError& error)
    {
        std::cerr << "SIL Kit runtime error: " << error.what() << std::endl;
        promptForExit();
        return OTHER_ERROR;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Something went wrong: " << error.what() << std::endl;
        promptForExit();
        return OTHER_ERROR;
    }

    return NO_ERROR;
}
