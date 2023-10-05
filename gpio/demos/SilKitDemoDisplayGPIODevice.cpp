// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <utility>
#include <atomic>

#include "adapter/ChipDatas.hpp"

#include "silkit/SilKit.hpp"
#include "silkit/config/all.hpp"
#include "silkit/services/pubsub/all.hpp"
#include "silkit/util/serdes/Serialization.hpp"

using namespace std::chrono_literals;

std::atomic<bool> isWorking;
std::atomic<bool> newValuesReceived;
static ChipDatas chipDatas;

void PrintPinsValues(const std::string &deviceName) {
    auto numLines = chipDatas.GetDatasSize();
    ::std::cout << ' ' << deviceName << " - " << numLines << " lines:" << ::std::endl;

    for (std::size_t offset = 0; offset < numLines; ++offset) 
    {
        ::std::cout << "\tline ";
        ::std::cout.width(3);
        ::std::cout << offset << ": ";

        ::std::cout.width(12);
        ::std::cout << "SilKitDemo ";

        ::std::cout.width(8);
        if (chipDatas.GetLineDirection(offset) == 0) 
        {
            std::cout << "input   " << (chipDatas.GetLineValue(offset) == 0 ? '0': '1') << std::endl;
        }
        else 
        {
            std::cout << "output  " << '-' << std::endl;
        }
    }
    std::cout << '\n';
}

int main(int argc, char** argv)
{
    // Participant configuration
    const std::string loglevel = "Info";

    const std::string deviceName = "gpiochip0";

    const std::string participantName = "DisplayDevice";

    const std::string registryURI = "silkit://localhost:8501";

    const std::string participantConfigurationString =
        R"({ "Logging": { "Sinks": [ { "Type": "Stdout", "Level": ")" + loglevel + R"("} ] } })";

    // Publisher specifications
    SilKit::Services::PubSub::PubSubSpec subDataSpec{"Topic1", SilKit::Util::SerDes::MediaTypeData()};
    subDataSpec.AddLabel("KeyA", "ValA", SilKit::Services::MatchingLabel::Kind::Optional);

    try
    {
        isWorking = true;
        newValuesReceived = false;

        // Configure the participant
        auto participantConfiguration =
            SilKit::Config::ParticipantConfigurationFromString(participantConfigurationString);

        std::cout << "Creating participant " << participantName << " with registry " << registryURI << std::endl;
        auto participant = SilKit::CreateParticipant(std::move(participantConfiguration), participantName, registryURI);

        auto dataSubscriber = participant->CreateDataSubscriber(
            participantName + "_sub", subDataSpec,
            [&](SilKit::Services::PubSub::IDataSubscriber* subscriber, 
                const SilKit::Services::PubSub::DataMessageEvent& dataMessageEvent)
            {
                // TODO: Add error management for received datas
                std::cout << "Deserializing new values received from GPIO Adapter" << std::endl;

                // pins' state is updating with received values from the gpiochip 
                chipDatas.Deserialize(SilKit::Util::ToStdVector(dataMessageEvent.data));

                newValuesReceived = true;
            });

        auto* lifecycleService = participant->CreateLifecycleService({ SilKit::Services::Orchestration::OperationMode::Autonomous });

        lifecycleService->SetStopHandler([]() {
            std::cout << "Stop handler called" << std::endl;
            });

        lifecycleService->SetShutdownHandler([]() {
            std::cout << "Shutdown handler called" << std::endl;
            });

        // Print thread handling
        std::thread workerThread;
        std::promise<void> startHandlerPromise;
        auto startHandlerFuture = startHandlerPromise.get_future();
        lifecycleService->SetCommunicationReadyHandler([&]() {
            std::cout << "Communication ready handler called for " << participantName << std::endl;
            workerThread = std::thread{ [&]() {
                startHandlerFuture.get();

                while (isWorking)
                {
                    if (newValuesReceived) 
                    {
                        PrintPinsValues(deviceName);

                        newValuesReceived = false;
                    }
                }
                lifecycleService->Stop("User Requested to Stop");
            } };
            });

        lifecycleService->SetStartingHandler([&]() {
            startHandlerPromise.set_value();
            });

        auto finalStateFuture = lifecycleService->StartLifecycle();

        std::cout << "Press enter to stop the process..." << std::endl;
        std::cin.ignore();

        isWorking = false;

        if (workerThread.joinable())
        {
            workerThread.join();
        }

        auto finalState = finalStateFuture.get();
        std::cout << "Simulation stopped. Final State: " << static_cast<int16_t>(finalState) << std::endl;
    }
    catch (const SilKit::ConfigurationError& error)
    {
        std::cerr << "Invalid configuration: " << error.what() << std::endl;
        std::cout << "Press enter to stop the process..." << std::endl;
        std::cin.ignore();
        return -2;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Something went wrong: " << error.what() << std::endl;
        std::cout << "Press enter to stop the process..." << std::endl;
        std::cin.ignore();
        return -3;
    }

    return 0;
}
