// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <utility>
#include <atomic>

#include "silkit/SilKit.hpp"
#include "silkit/config/all.hpp"
#include "silkit/services/pubsub/all.hpp"
#include "silkit/util/serdes/Serialization.hpp"


using namespace SilKit::Services::PubSub;

using namespace std::chrono_literals;

std::condition_variable cv;
std::atomic<bool> demoIsWorking;
std::mutex m;
bool newValuesReceived;

// Current state of the chip pins
struct GpioChip {
    std::vector<std::uint8_t> pinsValues;
    std::vector<std::uint8_t> pinsIO;
}gpioChip;

std::vector<uint8_t> serialize(const GpioChip& pinsToSerialize)
{
    SilKit::Util::SerDes::Serializer serializer;
    serializer.BeginStruct();
    serializer.Serialize(pinsToSerialize.pinsValues);
    serializer.Serialize(pinsToSerialize.pinsIO);
    serializer.EndStruct();

    return serializer.ReleaseBuffer();
}

GpioChip deserialize(const std::vector<uint8_t>& data)
{
    GpioChip deserializedPins;

    SilKit::Util::SerDes::Deserializer deserializer(data);
    deserializer.BeginStruct();
    deserializedPins.pinsValues = deserializer.Deserialize<std::vector<std::uint8_t>>();
    deserializedPins.pinsIO = deserializer.Deserialize<std::vector<std::uint8_t>>();
    deserializer.EndStruct();

    return deserializedPins;
}

void printPinsValues() {
    std::unique_lock<std::mutex> lk(m);
    std::cout << '\n';
    std::cout << "  gpiochip 0" << std::endl;
    std::cout << "     pins values : ";
    std::for_each(gpioChip.pinsValues.begin(), gpioChip.pinsValues.end(), [](int i) {
        std::cout << i;
        });
    std::cout << "\n     pins in/out : ";
    std::for_each(gpioChip.pinsIO.begin(), gpioChip.pinsIO.end(), [](int i) {
        std::cout << i;
        });

    std::cout << '\n';
    //std::cout << "Enter a new value (x:yz) : " << std::endl;
}

int main(int argc, char** argv)
{
    // Participant configuration
    const std::string loglevel = "Info"; 
    const std::string participantName = "DisplayModifyDevice"; 
    const std::string registryURI = "silkit://localhost:8501";
    const std::string participantConfigurationString =
        R"({ "Logging": { "Sinks": [ { "Type": "Stdout", "Level": ")" + loglevel + R"("} ] } })";

    // Publisher - Subscriber specifications
    const auto create_pubsubspec = [](const std::string& topic_name,
        SilKit::Services::MatchingLabel::Kind matching_mode) {
            PubSubSpec r(topic_name, SilKit::Util::SerDes::MediaTypeData());
            r.AddLabel("VirtualNetwork", "Default", matching_mode);
            r.AddLabel("Namespace", "Namespace", matching_mode);
            return r;
    };

    PubSubSpec subDataSpec = create_pubsubspec("fromChardev", SilKit::Services::MatchingLabel::Kind::Mandatory);
    subDataSpec.AddLabel("Instance", "Adapter", SilKit::Services::MatchingLabel::Kind::Mandatory);

    PubSubSpec pubDataSpec = create_pubsubspec("toChardev", SilKit::Services::MatchingLabel::Kind::Optional);
    pubDataSpec.AddLabel("Instance", participantName, SilKit::Services::MatchingLabel::Kind::Optional);

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

        auto dataPublisher = participant->CreateDataPublisher(participantName + "_pub", pubDataSpec);

        auto dataSubscriber = participant->CreateDataSubscriber(
            participantName + "_sub", subDataSpec,
            [&](SilKit::Services::PubSub::IDataSubscriber* subscriber, const DataMessageEvent& dataMessageEvent) {
                // add error management for received datas

                // pins' state is updating with received values from the gpiochip 
                gpioChip = deserialize(SilKit::Util::ToStdVector(dataMessageEvent.data));
                newValuesReceived = true;
            });

        // lifeCycle handling
        bool isStopped = false;
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
                        printPinsValues();
                        newValuesReceived = false;
                    }
                    std::this_thread::sleep_for(1s);
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
