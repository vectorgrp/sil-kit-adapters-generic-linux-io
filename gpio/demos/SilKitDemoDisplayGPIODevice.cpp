// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <utility>
#include <atomic>

#include "SilKitDemoGPIODevice.hpp"
#include "silkit/SilKit.hpp"
#include "silkit/config/all.hpp"
#include "silkit/services/pubsub/all.hpp"
#include "silkit/util/serdes/Serialization.hpp"


using namespace SilKit::Services::PubSub;

using namespace std::chrono_literals;

std::condition_variable cv;
std::atomic<bool> isStopped;
std::mutex m;
bool newValuesReceived;

GpioChip gpioChip;

void printPinsValues(const std::string &deviceName) {
    //std::cout << "Enter a new value (x:yz) : " << std::endl;

    ::std::cout << ' ' << deviceName << " - " << gpioChip.pinsValues.size() << " lines:" << ::std::endl;

    for (unsigned int offset = 0; offset < gpioChip.pinsIO.size(); ++offset) {
        ::std::cout << "\tline ";
        ::std::cout.width(3);
        ::std::cout << offset << ": ";

        ::std::cout.width(12);
        ::std::cout << "SilKitDemo ";

        ::std::cout.width(8);
        if (gpioChip.pinsIO[offset] == 0) {
            std::cout << "input    " << (gpioChip.pinsValues[offset] == 0 ? '0': '1') << std::endl;
        }
        else {
            std::cout << "output   " << '-' << std::endl;
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

    // Publishers specifications
    SilKit::Services::PubSub::PubSubSpec subDataSpec{"Topic1", SilKit::Util::SerDes::MediaTypeData()};
    subDataSpec.AddLabel("KeyA", "ValA", SilKit::Services::MatchingLabel::Kind::Optional);

    //SilKit::Services::PubSub::PubSubSpec subDataSpec2{"Topic2", SilKit::Util::SerDes::MediaTypeData()};
    //subDataSpec.AddLabel("KeyB", "ValB", SilKit::Services::MatchingLabel::Kind::Optional);

    try
    {
        auto participantConfiguration =
            SilKit::Config::ParticipantConfigurationFromString(participantConfigurationString);

        std::cout << "Creating participant " << participantName << " with registry " << registryURI << std::endl;

        auto participant = SilKit::CreateParticipant(participantConfiguration, participantName, registryURI);

        auto* lifecycleService = participant->CreateLifecycleService({SilKit::Services::Orchestration::OperationMode::Autonomous});

        lifecycleService->SetStopHandler([]() {
            std::cout << "Stop handler called" << std::endl;
            });

        lifecycleService->SetShutdownHandler([]() {
            std::cout << "Shutdown handler called" << std::endl;
            });

        auto dataSubscriber = participant->CreateDataSubscriber(
            participantName + "_sub", subDataSpec,
            [&](SilKit::Services::PubSub::IDataSubscriber* subscriber, const DataMessageEvent& dataMessageEvent) {
                // add error management for received datas
                std::cout << "New values received from GPIO Adapter \n" << std::endl;
                // pins' state is updating with received values from the gpiochip 
                gpioChip = deserialize(SilKit::Util::ToStdVector(dataMessageEvent.data));
                newValuesReceived = true;
            });

        // lifeCycle handling
        isStopped = false;
        std::thread workerThread;
        std::promise<void> startHandlerPromise;
        auto startHandlerFuture = startHandlerPromise.get_future();
        lifecycleService->SetCommunicationReadyHandler([&]() {
            std::cout << "Communication ready handler called for " << participantName << std::endl;
            workerThread = std::thread{ [&]() {
                startHandlerFuture.get();
                while (!isStopped)
                {
                    if (newValuesReceived) {
                        printPinsValues(deviceName);
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

        isStopped = true;
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
