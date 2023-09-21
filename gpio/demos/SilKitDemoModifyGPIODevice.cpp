// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <iostream>
#include <string>
#include <vector>
#include <chrono>

#include "silkit/SilKit.hpp"
#include "silkit/config/all.hpp"
#include "silkit/services/pubsub/all.hpp"
#include "silkit/util/serdes/Serialization.hpp"

#include "SilKitDemoGPIODevice.hpp"

using namespace std::chrono_literals;

int main(int argc, char** argv)
{
    // direction is useless when it is INPUT (== 0)
    if (argc < 2) {
        ::std::cerr << "usage: " << argv[0] <<
            " <line_offset0>=<direction0>:<value0> ..." << ::std::endl;
        return EXIT_FAILURE;
    }

    std::vector<std::uint8_t> offsets;
    std::vector<std::uint8_t> values;
    std::vector<std::uint8_t> directions;

    for (int i = 1; i < argc; i++) {
        ::std::string arg(argv[i]);

        size_t pos = arg.find('=');

        ::std::string offset(arg.substr(0, pos));
        std::string dirVal = arg.substr(pos + 1, ::std::string::npos);

        pos = dirVal.find(':');
        ::std::string direction(dirVal.substr(0, pos));
        ::std::string value(dirVal.substr(pos + 1, ::std::string::npos));

        if (offset.empty() || value.empty() || direction.empty())
            throw ::std::invalid_argument("invalid offset=direction:value mapping: " +
                ::std::string(argv[i]));
        std::cout << "offset is : " << offset << std::endl;
        std::cout << "direction is : " << direction << std::endl;
        std::cout << "value is : " << value << std::endl;
        offsets.push_back(::std::stoul(offset));
        values.push_back(::std::stoul(value));
        directions.push_back(::std::stoul(direction));
    }

    // Participant configuration
    const std::string loglevel = "Info";
    const std::string deviceName = "gpiochip0";
    const std::string participantName = "ModifyDevice";
    const std::string registryURI = "silkit://localhost:8501";
    const std::string participantConfigurationString =
        R"({ "Logging": { "Sinks": [ { "Type": "Stdout", "Level": ")" + loglevel + R"("} ] } })";

    // Publisher specifications
    SilKit::Services::PubSub::PubSubSpec pubDataSpec{"Topic2", SilKit::Util::SerDes::MediaTypeData()};
    pubDataSpec.AddLabel("KeyB", "ValB", SilKit::Services::MatchingLabel::Kind::Optional);

    // Subscriber specifications to get the last gpiochip state
    SilKit::Services::PubSub::PubSubSpec subDataSpec{"Topic1", SilKit::Util::SerDes::MediaTypeData()};
    subDataSpec.AddLabel("KeyA", "ValA", SilKit::Services::MatchingLabel::Kind::Optional);


    try
    {
        // Current state of the chip pins
        GpioChip gpioChip;

        auto participantConfiguration =
            SilKit::Config::ParticipantConfigurationFromString(participantConfigurationString);

        std::cout << "Creating participant " << participantName << " with registry " << registryURI << std::endl;

        auto participant = SilKit::CreateParticipant(participantConfiguration, participantName, registryURI);

        auto dataSubscriber = participant->CreateDataSubscriber(
            participantName + "_sub", subDataSpec,
            [&](SilKit::Services::PubSub::IDataSubscriber* subscriber, 
                const SilKit::Services::PubSub::DataMessageEvent& dataMessageEvent) {
                // add error management for received datas

                // getting last gpiochip state from the registry
                std::cout << "deserializing" << std::endl;
                gpioChip = deserialize(SilKit::Util::ToStdVector(dataMessageEvent.data));
            });

        auto dataPublisher = participant->CreateDataPublisher(participantName + "_pub", pubDataSpec);

        // waiting for getting the last published message on the topic
        std::this_thread::sleep_for(1s);

        // A modifier pour afficher les valeurs reçues
        std::cout << "Values received are : ";
        for (int i = 0; i < gpioChip.pinsIO.size(); ++i) {
            std::cout << (gpioChip.pinsIO[i] == 0 ? '0' : '1');
            std::cout << ':';
            std::cout << (gpioChip.pinsValues[i] == 0 ? '0' : '1');
            std::cout << ' ';
        }
        std::cout << '\n';

        // Handling values from args
        // Modifying values => OUTPUT mode
        for (int i = 0; i < offsets.size(); ++i) {
            gpioChip.pinsIO[offsets[i]] = directions[i];
            gpioChip.pinsValues[offsets[i]] = values[i];
        }

        // Ajouter les valeurs envoyées io + values
        std::cout << "Values sent are     : ";
        for (int i = 0; i < gpioChip.pinsIO.size(); ++i) {
            std::cout << (gpioChip.pinsIO[i] == 0 ? "0" : "1");
            std::cout << ':';
            std::cout << (gpioChip.pinsValues[i] == 0 ? "0" : "1");
            std::cout << ' ';
        }
        std::cout << '\n';

        std::vector<uint8_t> serializedDatas = serialize(gpioChip);
        dataPublisher->Publish(serializedDatas);

        std::cout << "Press enter to stop the process..." << std::endl;
        std::cin.ignore();

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
