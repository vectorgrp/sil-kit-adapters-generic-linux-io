// SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
// SPDX-License-Identifier: MIT

#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <iostream>

#include "../../../util/DemoParsing.hpp"
#include "../../../util/Exceptions.hpp"

#include "common/Parsing.hpp"

#include "silkit/SilKit.hpp"
#include "silkit/config/all.hpp"
#include "silkit/services/pubsub/all.hpp"
#include "silkit/util/serdes/Serialization.hpp"

using namespace util;
using namespace adapters;
using namespace SilKit::Services::PubSub;

int main(int argc, char** argv)
{
    if (findArg(argc, argv, "--help", argv) != nullptr)
    {
        PrintDemoHelp("Advalues", true);
        return NO_ERROR;
    }

    try
    {
        throwInvalidCliIf(ThereAreUnknownArgumentsDemo(argc, argv, {&participantNameArg, &regUriArg, &logLevelArg},
                                                       {&helpArg}, "Advalues"));

        const std::string loglevel = getArgDefault(argc, argv, logLevelArg, "Info");
        const std::string participantName = getArgDefault(argc, argv, participantNameArg, "AdvaluesForwardDevice");
        const std::string registryURI = getArgDefault(argc, argv, regUriArg, "silkit://localhost:8501");

        const std::string participantConfigurationString =
            R"({ "Logging": { "Sinks": [ { "Type": "Stdout", "Level": ")" + loglevel + R"("} ] } })";

        const std::string pubTopic = "toVoltage103";
        const std::string subTopic = "fromVoltage32";
        PubSubSpec pubDataSpec(pubTopic, SilKit::Util::SerDes::MediaTypeData());
        PubSubSpec subDataSpec(subTopic, SilKit::Util::SerDes::MediaTypeData());

        auto participantConfiguration =
            SilKit::Config::ParticipantConfigurationFromString(participantConfigurationString);

        auto participant = SilKit::CreateParticipant(participantConfiguration, participantName, registryURI);
        auto logger = participant->GetLogger();
        auto dataPublisher = participant->CreateDataPublisher(participantName + "_pub", pubDataSpec);

        // sleep to be sure that the publisher is created before the subscriber
        std::this_thread::sleep_for(std::chrono::seconds(1));
        int16_t recvValue;

        auto dataSubscriber = participant->CreateDataSubscriber(
            participantName + "_sub", subDataSpec,
            [&](SilKit::Services::PubSub::IDataSubscriber* /*subscriber*/, const DataMessageEvent& dataMessageEvent) {
            SilKit::Util::SerDes::Deserializer deserializer(SilKit::Util::ToStdVector(dataMessageEvent.data));
            // Deserialize the received value from out_voltage32
            recvValue = deserializer.Deserialize<int16_t>(16);
            logger->Info("Adapter >> ForwardDevice: " + std::to_string(recvValue));

            // Serialize the received value to in_voltage103
            SilKit::Util::SerDes::Serializer serializer;
            serializer.Serialize(recvValue, 16);
            dataPublisher->Publish(serializer.ReleaseBuffer());
            logger->Info("ForwardDevice >> Adapter: " + std::to_string(recvValue));
        });

        promptForExit();
    }
    catch (const InvalidCli&)
    {
        std::cerr << "Invalid command line arguments." << std::endl;
        return CLI_ERROR;
    }
    catch (const SilKit::ConfigurationError& error)
    {
        std::cerr << "Invalid configuration: " << error.what() << std::endl;
        return SILKIT_ERROR;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Something went wrong: " << error.what() << std::endl;
        return OTHER_ERROR;
    }

    return 0;
}
