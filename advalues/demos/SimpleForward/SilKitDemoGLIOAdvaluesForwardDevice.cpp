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
                     "SilKitDemoGLIOAdvaluesForwardDevice [" << participantNameArg << " <participant's name{AdvaluesForwardDevice}>]\n"
                     "  [" << regUriArg << " silkit://<host{localhost}>:<port{8501}>]\n"
                     "  [" << logLevelArg << " <Trace|Debug|Warn|{Info}|Error|Critical|off>]\n";
        
        return NO_ERROR;
    }

    const std::string loglevel = GetArgDefault(argc, argv, logLevelArg, "Info");
    const std::string participantName = GetArgDefault(argc, argv, participantNameArg, "AdvaluesForwardDevice");
    const std::string registryURI = GetArgDefault(argc, argv, regUriArg, "silkit://localhost:8501");

    const std::string participantConfigurationString =
        R"({ "Logging": { "Sinks": [ { "Type": "Stdout", "Level": ")" + loglevel + R"("} ] } })";

    const std::string pubTopic = "toVoltage103";
    const std::string subTopic = "fromVoltage32";
    PubSubSpec pubDataSpec(pubTopic, SilKit::Util::SerDes::MediaTypeData());
    PubSubSpec subDataSpec(subTopic, SilKit::Util::SerDes::MediaTypeData());

    try
    {
        auto participantConfiguration =
            SilKit::Config::ParticipantConfigurationFromString(participantConfigurationString);

        std::cout << "Creating participant '" << participantName << "' at " << registryURI << std::endl;
        auto participant = SilKit::CreateParticipant(participantConfiguration, participantName, registryURI);

        auto dataPublisher = participant->CreateDataPublisher(participantName + "_pub", pubDataSpec);

        int16_t recvValue;
        
        auto dataSubscriber = participant->CreateDataSubscriber(
            participantName + "_sub", subDataSpec,
            [&](SilKit::Services::PubSub::IDataSubscriber* subscriber, const DataMessageEvent& dataMessageEvent) {
                SilKit::Util::SerDes::Deserializer deserializer(SilKit::Util::ToStdVector(dataMessageEvent.data));
                // Deserialize the received value from out_voltage32
                recvValue = deserializer.Deserialize<int16_t>(16);
                std::cout << "GLIO Adapter  >> ForwardDevice: " << std::to_string(recvValue) << std::endl;

                // Serialize the received value to in_voltage103
                SilKit::Util::SerDes::Serializer serializer;
                serializer.Serialize(recvValue, 16);
                dataPublisher->Publish(serializer.ReleaseBuffer());
                std::cout << "ForwardDevice >> GLIO Adapter : " << std::to_string(recvValue) << std::endl;
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
