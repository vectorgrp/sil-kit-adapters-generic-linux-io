// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include <string>
#include <vector>
#include <thread>
#include <chrono>

#include "../../../util/Parsing.hpp"
#include "../../../util/SignalHandler.hpp"

#include "silkit/SilKit.hpp"
#include "silkit/config/all.hpp"
#include "silkit/services/pubsub/all.hpp"
#include "silkit/util/serdes/Serialization.hpp"

using namespace adapters;
using namespace adapters::Parsing;
using namespace SilKit::Services::PubSub;

const std::array<const std::string, 3> demoSwitchesWithArgument = {participantNameArg, regUriArg, logLevelArg};
const std::array<const std::string, 1> demoSwitchesWithoutArgument = {helpArg};

int main(int argc, char** argv)
{
    if (FindArg(argc, argv, "--help", argv) != nullptr)
    {
        PrintDemoHelp("Chardev", true);
        return NO_ERROR;
    }

    const std::string loglevel = GetArgDefault(argc, argv, logLevelArg, "Info");
    const std::string participantName = GetArgDefault(argc, argv, participantNameArg, "ChardevForwardDevice");
    const std::string registryURI = GetArgDefault(argc, argv, regUriArg, "silkit://localhost:8501");

    try
    {
        throwInvalidCliIf(ThereAreUnknownArgumentsDemo(argc, argv, "Chardev"));

        const std::string participantConfigurationString =
        R"({ "Logging": { "Sinks": [ { "Type": "Stdout", "Level": ")" + loglevel + R"("} ] } })";

        const std::string pubTopic = "toFifo2";
        const std::string subTopic = "fromFifo1";
        PubSubSpec pubDataSpec(pubTopic, SilKit::Util::SerDes::MediaTypeData());
        PubSubSpec subDataSpec(subTopic, SilKit::Util::SerDes::MediaTypeData());

        auto participantConfiguration =
            SilKit::Config::ParticipantConfigurationFromString(participantConfigurationString);

        std::cout << "Creating participant '" << participantName << "' at " << registryURI << std::endl;
        auto participant = SilKit::CreateParticipant(participantConfiguration, participantName, registryURI);

        auto dataPublisher = participant->CreateDataPublisher(participantName + "_pub", pubDataSpec);

        // sleep to be sure that the publisher is created before the subscriber
        std::this_thread::sleep_for(std::chrono::seconds(1));

        std::string line_buffer;
        
        auto dataSubscriber = participant->CreateDataSubscriber(
            participantName + "_sub", subDataSpec,
            [&](SilKit::Services::PubSub::IDataSubscriber* subscriber, const DataMessageEvent& dataMessageEvent) {
                if (dataMessageEvent.data.size() <= 4)
                {
                    std::cerr << "warning: message received probably wasn't following SAB format."<<std::endl;
                    line_buffer += std::string(reinterpret_cast<const char*>(dataMessageEvent.data.data()),
                                               dataMessageEvent.data.size());
                }
                else
                {
                    
                    line_buffer += std::string(reinterpret_cast<const char*>(dataMessageEvent.data.data() + 4),
                                               dataMessageEvent.data.size() - 4);
                }
                std::string::size_type newline_pos;
                while ( (newline_pos = line_buffer.find_first_of('\n')) != std::string::npos)
                {
                    std::string tmpStr = line_buffer.substr(0,newline_pos);
                    std::cout << "GLIO Adapter  >> ForwardDevice: " << tmpStr << std::endl;
                    std::cout << "ForwardDevice >> GLIO Adapter : " << tmpStr << std::endl;
                    line_buffer.erase(0, newline_pos + 1);
                }
                dataPublisher->Publish(dataMessageEvent.data);
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
