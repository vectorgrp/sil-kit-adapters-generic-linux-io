// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "SilKitAdapterGPIO.hpp"

#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <set>
#include <algorithm>
#include <numeric>
#include <atomic>
#include <mutex>

#include "silkit/SilKit.hpp"
#include "silkit/config/all.hpp"
#include "silkit/services/pubsub/all.hpp"
#include "silkit/services/logging/all.hpp"
#include "silkit/util/serdes/Serialization.hpp"

#include "Exceptions.hpp"

#include "gpiod.hpp"

using namespace std::chrono_literals;
using namespace adapters;
using namespace SilKit::Services::PubSub;

std::atomic<bool> newMsg;
std::atomic<bool> isWorking;
std::mutex m;

struct GpioChip {
    std::vector<std::uint8_t> pinsValues;
    std::vector<std::uint8_t> pinsIO;
};

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

std::pair < ::gpiod::line::offsets, ::gpiod::line::offsets > get_lines_direction(GpioChip newValues) {
    ::gpiod::line::offsets offsets_IN, offsets_OUT;
    for (int i = 0; i < newValues.pinsIO.size(); ++i) {
        // Assume that input == 0 / output == 1
        newValues.pinsIO[i] == 0 ? offsets_IN.push_back(i) : offsets_OUT.push_back(i);
    }
    return std::make_pair(offsets_IN, offsets_OUT);
}

::gpiod::line::values get_output_line_values(GpioChip newValues) {
    ::gpiod::line::values values;
    auto offsets_OUT = get_lines_direction(newValues).second;
    for (::gpiod::line::offset offset : offsets_OUT) {
        values.push_back(newValues.pinsValues[::gpiod::line::offset(offset)] == 0 ? ::gpiod::line::value::INACTIVE : ::gpiod::line::value::ACTIVE);
    }
    return values;
}

void set_gpio_values(::gpiod::line_request &line_req, GpioChip newValues) {
    auto offsets_OUT = get_lines_direction(newValues).second;
    auto values_OUT = get_output_line_values(newValues);

    auto line_conf = ::gpiod::line_config();

    for (int i = 0; i < newValues.pinsIO.size(); ++i) {
        if (newValues.pinsIO[i] == 0) {
            line_conf.add_line_settings(
                ::gpiod::line::offset(i),
                    ::gpiod::line_settings()
                    .set_direction(::gpiod::line::direction::INPUT)
                );
        }
        else {
            line_conf.add_line_settings(
                ::gpiod::line::offset(i),
                ::gpiod::line_settings()
                .set_direction(::gpiod::line::direction::OUTPUT)
            );
        }
    }
    std::cout << "Reconfiguring ouput lines with their associated values" << std::endl;
    if (offsets_OUT.size() != 0) {
        line_req.reconfigure_lines(line_conf)
            .set_values(offsets_OUT, values_OUT);
    }
    else {
        line_req.reconfigure_lines(line_conf);
    }
}

void read_gpio_events(::gpiod::line_request& line_req, GpioChip & gpioLines, SilKit::Services::PubSub::IDataPublisher* publisher) {
    auto line_conf = ::gpiod::line_config();

    for (int i = 0; i < gpioLines.pinsIO.size(); ++i) {
        if (gpioLines.pinsIO[i] == 0) {
            line_conf.add_line_settings(
                ::gpiod::line::offset(i),
                ::gpiod::line_settings()
                .set_direction(::gpiod::line::direction::INPUT)
                .set_edge_detection(::gpiod::line::edge::BOTH)
            );
        }
        else {
            line_conf.add_line_settings(
                ::gpiod::line::offset(i),
                ::gpiod::line_settings()
                .set_direction(::gpiod::line::direction::OUTPUT)
                .set_output_value((gpioLines.pinsValues[i] == 0 ? ::gpiod::line::value::INACTIVE : ::gpiod::line::value::ACTIVE))
            );
        }
    }

    // refresh every 100000000 nanoseconds to check if a newMsg is received
    // Can be modified by adding a pipe to break
    bool modif = line_req.reconfigure_lines(line_conf)
        .wait_edge_events(::std::chrono::nanoseconds(1000000000));

    if (modif) {
        std::cout << "new event from the gpiochip" << std::endl;
        // Modify the internal current GpioChip state variable and then publish it
        ::gpiod::edge_event_buffer buffer;
        line_req.read_edge_events(buffer);

        for (const auto& event : buffer) {
            if (event.type() == ::gpiod::edge_event::event_type::RISING_EDGE) {
                std::cout << "    -> RISING_EDGE, line offset : " << ::gpiod::line::offset(event.line_offset()) << std::endl;
                gpioLines.pinsValues[::gpiod::line::offset(event.line_offset())] = 1;
                gpioLines.pinsIO[::gpiod::line::offset(event.line_offset())] = 0;
            }
            else {
                std::cout << "    -> FALLING_EDGE, line offset : " << ::gpiod::line::offset(event.line_offset()) << std::endl;
                gpioLines.pinsValues[::gpiod::line::offset(event.line_offset())] = 0;
                gpioLines.pinsIO[::gpiod::line::offset(event.line_offset())] = 0;
            } 
        }
               
        std::cout << "serializing data and publishing" << std::endl;
        std::vector<uint8_t> serializedDatas = serialize(gpioLines);
        publisher->Publish(serializedDatas);
    }
}

void promptForExit()
{
    std::cout << "Press enter to stop the process..." << std::endl;
    std::cin.ignore();
}

int main(int argc, char** argv)
{
    const std::string gpiochipName = "/dev/gpiochip0";

    const std::string participantName = "SilKitAdapterGPIO";

    const std::string registryURI = "silkit://localhost:8501";

    const std::string topicPublisher = "Topic1";

    try
    {
        newMsg = false;
        isWorking = true;
        std::shared_ptr<SilKit::Config::IParticipantConfiguration> participantConfiguration;
        const std::string loglevel = "Info";
        const std::string participantConfigurationString =
            R"({ "Logging": { "Sinks": [ { "Type": "Stdout", "Level": ")" + loglevel + R"("} ] } })";
        participantConfiguration =
            SilKit::Config::ParticipantConfigurationFromString(participantConfigurationString);

        std::cout << "Creating participant " << participantName << " with registry " << registryURI << std::endl;
        auto participant =
            SilKit::CreateParticipant(participantConfiguration, participantName, registryURI);

        auto logger = participant->GetLogger();

        // default publisher / subscriber name
        SilKit::Services::PubSub::PubSubSpec pubDataSpec{topicPublisher, SilKit::Util::SerDes::MediaTypeData()};
        pubDataSpec.AddLabel("KeyA", "ValA", SilKit::Services::MatchingLabel::Kind::Optional);

        SilKit::Services::PubSub::PubSubSpec subDataSpec{"Topic2", SilKit::Util::SerDes::MediaTypeData()};
        subDataSpec.AddLabel("KeyB", "ValB", SilKit::Services::MatchingLabel::Kind::Optional);

        // Handling gpiochip
        std::cout << "Opening gpiochip : " << gpiochipName << std::endl;
        ::gpiod::chip chip{gpiochipName};
        auto gpiochipInfo = chip.get_info();
        auto nbPins = gpiochipInfo.num_lines();

        // first, get all lines direction from the current gpiochip state
        // assume that all are INPUT
        ::gpiod::line::offsets offsets(nbPins);
        std::iota(std::begin(offsets), std::end(offsets), 0);

        std::cout << "Getting initial pins state" << std::endl;
        auto request = chip.prepare_request();
        ::gpiod::line_request line_req = request
            .set_consumer("SilKitAdapterGPIO")
            .add_line_settings(
                offsets,
                ::gpiod::line_settings()
                .set_direction(::gpiod::line::direction::INPUT)
            )
            .do_request();

        // Assume starting state is INPUT - 0
        std::vector<std::uint8_t> init(nbPins, 0);
        GpioChip currentGPIOState;
        currentGPIOState.pinsIO = init;
        currentGPIOState.pinsValues = init;

        SilKit::Util::SerDes::Serializer serializer;

        auto dataPublisher = participant->CreateDataPublisher(participantName + "_pub", pubDataSpec, 1); // removing 1 ?

        auto dataSubscriber = participant->CreateDataSubscriber(
            participantName + "_sub", subDataSpec,
            [&](SilKit::Services::PubSub::IDataSubscriber* subscriber, const DataMessageEvent& dataMessageEvent) {
                // Sending new message information to read events thread
                std::cout << "New values are received" << std::endl;
                newMsg = true;
                std::this_thread::sleep_for(1s);
                currentGPIOState = deserialize(SilKit::Util::ToStdVector(dataMessageEvent.data));

                std::cout << "Updating " << gpiochipName << " pins" << std::endl;
                set_gpio_values(line_req, currentGPIOState);
                
                std::cout << "Serializing data received and publishing on topic : " << topicPublisher << std::endl;
                std::vector<uint8_t> serializedDatas = serialize(currentGPIOState);
                dataPublisher->Publish(serializedDatas);

                newMsg = false;

                /*_logger->Debug("SIL Kit >> QEMU: "
                    + std::string((const char*)_data_buffer_toChardev.data(), _data_buffer_toChardev.size()));
                _socket.write_some(asio::buffer(_data_buffer_toChardev.data(), _data_buffer_toChardev.size()));*/
            });
    

        auto* lifecycleService = participant->CreateLifecycleService({ SilKit::Services::Orchestration::OperationMode::Autonomous });

        lifecycleService->SetStopHandler([]() {
            std::cout << "Stop handler called" << std::endl;
            });

        lifecycleService->SetShutdownHandler([]() {
            std::cout << "Shutdown handler called" << std::endl;
            });

        // lifeCycle handling
        std::thread workerThread;
        std::promise<void> startHandlerPromise;
        auto startHandlerFuture = startHandlerPromise.get_future();
        lifecycleService->SetCommunicationReadyHandler([&]() {
            std::cout << "Communication ready handler called for " << participantName << std::endl;
            workerThread = std::thread{ [&]() {
                startHandlerFuture.get();

                while (isWorking) {


                    if (!newMsg) {
                        read_gpio_events(line_req, currentGPIOState, dataPublisher);
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

        // Free gpiochip lines
        newMsg = true;
        isWorking = false;
        line_req.release();

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
