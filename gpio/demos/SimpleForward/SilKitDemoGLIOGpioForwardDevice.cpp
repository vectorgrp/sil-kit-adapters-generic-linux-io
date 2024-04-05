// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <iostream>
#include <string>
#include <vector>

#include "../../../util/Parsing.hpp"
#include "../../../util/SignalHandler.hpp"

#include "silkit/SilKit.hpp"
#include "silkit/config/all.hpp"
#include "silkit/services/pubsub/all.hpp"
#include "silkit/util/serdes/Serialization.hpp"

using namespace adapters;
using namespace adapters::Parsing;
using namespace SilKit::Services::PubSub;

int main(int argc, char** argv)
{
    if (FindArg(argc, argv, "--help", argv) != nullptr)
    {
        std::cout << "Usage (defaults in curly braces if you omit the switch):\n"
                     "sil-kit-demo-glio-gpio-forward-device [" << participantNameArg << " <participant's name{GpioForwardDevice}>]\n"
                     "  [" << regUriArg << " silkit://<host{localhost}>:<port{8501}>]\n"
                     "  [" << logLevelArg << " <Trace|Debug|Warn|{Info}|Error|Critical|off>]\n";
        
        return NO_ERROR;
    }

    const std::string loglevel = GetArgDefault(argc, argv, logLevelArg, "Info");
    const std::string participantName = GetArgDefault(argc, argv, participantNameArg, "GpioForwardDevice");
    const std::string registryURI = GetArgDefault(argc, argv, regUriArg, "silkit://localhost:8501");

    const std::string participantConfigurationString =
        R"({ "Logging": { "Sinks": [ { "Type": "Stdout", "Level": ")" + loglevel + R"("} ] } })";


    const std::string pubTopic = "toGpiochip1Line2";
    const std::string subTopic = "fromGpiochip0Line4";
    PubSubSpec pubDataSpec(pubTopic, SilKit::Util::SerDes::MediaTypeData());
    PubSubSpec subDataSpec(subTopic, SilKit::Util::SerDes::MediaTypeData());

    try
    {
        auto participantConfiguration =
            SilKit::Config::ParticipantConfigurationFromString(participantConfigurationString);

        std::cout << "Creating participant '" << participantName << "' at " << registryURI << std::endl;
        auto participant = SilKit::CreateParticipant(participantConfiguration, participantName, registryURI);

        auto dataPublisher = participant->CreateDataPublisher(participantName + "_pub", pubDataSpec);

        std::string line_buffer;

        constexpr auto printData = [](std::string_view str, const uint8_t dir, const uint8_t val){
            std::cout << str << (dir == 0 ? "INPUT - " : "OUTPUT - ") 
                << (val == 0 ? "LOW" : "HIGH") << std::endl;
        };
        
        auto dataSubscriber = participant->CreateDataSubscriber(
            participantName + "_sub", subDataSpec,
            [&](SilKit::Services::PubSub::IDataSubscriber* subscriber, const DataMessageEvent& dataMessageEvent) {
                
                SilKit::Util::SerDes::Deserializer deserializer(SilKit::Util::ToStdVector(dataMessageEvent.data));
                
                auto receivedValue = deserializer.Deserialize<uint8_t>(8);
                auto receivedDirection = deserializer.Deserialize<uint8_t>(8);

                printData("GLIO Adapter  >> ForwardDevice: ", receivedDirection, receivedValue);

                // Set dir to OUT - forward received value
                uint8_t dir = 1;
                printData("ForwardDevice >> GLIO Adapter : ", dir, receivedValue);

                SilKit::Util::SerDes::Serializer serializer;
                serializer.Serialize(receivedValue, 8);
                serializer.Serialize(dir, 8);
                dataPublisher->Publish(serializer.ReleaseBuffer());
            });

        promptForExit();
    }
    catch (const SilKit::ConfigurationError& error)
    {
        std::cerr << "Invalid configuration: " << error.what() << std::endl;
        return -2;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Something went wrong: " << error.what() << std::endl;
        return -3;
    }

    return 0;
}
