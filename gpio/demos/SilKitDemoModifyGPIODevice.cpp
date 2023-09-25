// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>

#include "ChipDatas.hpp"

#include "silkit/SilKit.hpp"
#include "silkit/config/all.hpp"
#include "silkit/services/pubsub/all.hpp"
#include "silkit/util/serdes/Serialization.hpp"

using namespace std::chrono_literals;

void PrintDirectionsValues(const ChipDatas& chipDatas)
{
    for (std::size_t i = 0; i < chipDatas.GetDatasSize(); ++i) 
    {
        std::cout << (chipDatas.GetPinDirection(i) == 0 ? '0' : '1');
        std::cout << ':';
        std::cout << (chipDatas.GetPinValue(i) == 0 ? '0' : '1');
        std::cout << ' ';
    }
    std::cout << '\n';
}

int main(int argc, char** argv)
{
    // Value is useless when it is INPUT (== 0)
    if (argc < 2) 
    {
        ::std::cerr << "usage: " << argv[0] <<
            " <line_offset0>=<direction0>:<value0> ..." << ::std::endl;
        return EXIT_FAILURE;
    }

    // Handle arg values <line_offset0>=<direction0>:<value0> ...
    std::vector<std::uint8_t> offsets, values, directions;

    for (int i = 1; i < argc; i++) 
    {
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

        offsets.push_back(::std::stoul(offset));
        values.push_back(::std::stoul(value));
        directions.push_back(::std::stoul(direction));
    }

    // Configure the participant
    const std::string loglevel = "Info";

    const std::string deviceName = "gpiochip0";

    const std::string participantName = "ModifyDevice";

    const std::string registryURI = "silkit://localhost:8501";

    const std::string participantConfigurationString =
        R"({ "Logging": { "Sinks": [ { "Type": "Stdout", "Level": ")" + loglevel + R"("} ] } })";

    // Publisher specifications
    SilKit::Services::PubSub::PubSubSpec pubDataSpec{"Topic2", SilKit::Util::SerDes::MediaTypeData()};
    pubDataSpec.AddLabel("KeyB", "ValB", SilKit::Services::MatchingLabel::Kind::Optional);

    // Subscriber specifications to get the last gpio chip state
    SilKit::Services::PubSub::PubSubSpec subDataSpec{"Topic1", SilKit::Util::SerDes::MediaTypeData()};
    subDataSpec.AddLabel("KeyA", "ValA", SilKit::Services::MatchingLabel::Kind::Optional);

    try
    {
        // Current state of the chip pins
        ChipDatas chipDatas;

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

                // Get the last gpiochip state from the registry
                std::cout << "Deserializing new values received from GPIO Adapter" << std::endl;
                chipDatas.Deserialize(SilKit::Util::ToStdVector(dataMessageEvent.data));
            });

        auto dataPublisher = participant->CreateDataPublisher(participantName + "_pub", pubDataSpec);

        // Wait for getting the last published message on the topic
        std::this_thread::sleep_for(1s);

        std::cout << "Values received are : ";
        PrintDirectionsValues(chipDatas);

        // Modify chip datas received from the adapter with arg datas
        for (std::size_t i = 0; i < offsets.size(); ++i) 
        {
            chipDatas.SetIOValue(offsets[i], directions[i]);
            chipDatas.SetPinValue(offsets[i], values[i]);
        }

        std::cout << "Values sent are     : ";
        PrintDirectionsValues(chipDatas);

        std::cout << "Serializing datas and publishing" << std::endl;
        dataPublisher->Publish(chipDatas.Serialize());

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
