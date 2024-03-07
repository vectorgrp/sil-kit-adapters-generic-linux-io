// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "ChardevManager.hpp"

#include <sys/inotify.h>

#include "ChardevAdapter.hpp"

#include "../../util/FileHelper.hpp"
#include "../../util/Exceptions.hpp"

#include "silkit/util/serdes/Serialization.hpp"

using namespace adapters;
using namespace SilKit::Services::PubSub;

static constexpr std::size_t MAX_EVENTS = 4096;

ChardevManager::ChardevManager(const YAML::Node& configFile,
                               std::vector<std::shared_ptr<IOAdapter>>& ioAdapters, 
                               SilKit::Services::Logging::ILogger* logger, 
                               SilKit::IParticipant* participant) :
    _logger(logger)
{
    // Initialize values from config file
    InitAdaptersFromConfigFile(configFile, ioAdapters, participant);
}

ChardevManager::~ChardevManager()
{
    Stop();
}

void ChardevManager::Stop() 
{ 
    if (!_ioc.stopped())
    {
        _ioc.stop();
    }
    if (_thread.joinable())
    {
        _thread.join();
    }
}

void ChardevManager::InitAdaptersFromConfigFile(const YAML::Node& configFile, 
                                                std::vector<std::shared_ptr<IOAdapter>>& ioAdapters, 
                                                SilKit::IParticipant* participant)
{
    std::vector<Util::DataYAMLConfig> chardevYAMLConfigs;
    GetYamlConfig(configFile, chardevYAMLConfigs);
    
    for (auto chardevYaml : chardevYAMLConfigs)
    {
        const std::string pathToFile = chardevYaml.path;
        
        std::string pathToFile_ = pathToFile;
        for(std::size_t i = 0; i < pathToFile_.size(); ++i)
        {
            if (pathToFile_[i] == '/')
            {
                pathToFile_.replace(pathToFile_.begin() + i, pathToFile_.begin() + i + 1, "_");
            }
        }

        const auto pos = pathToFile.find_last_of('/');
        const auto fileName = pathToFile.substr(pos + 1);

        std::unique_ptr<PubSubSpec> subDataSpec, pubDataSpec;
        std::string subscriberName, publisherName;
        // Manage topic_subscribe
        if (!chardevYaml.topic_subscribe.empty())
        {
            subDataSpec = std::make_unique<PubSubSpec>(chardevYaml.topic_subscribe, SilKit::Util::SerDes::MediaTypeData());
            subDataSpec->AddLabel("KeyA", "ValueA", SilKit::Services::MatchingLabel::Kind::Optional);
            subscriberName = "sub" + pathToFile_;
        }

        // Manage topic_publish
        if (!chardevYaml.topic_publish.empty())
        {
            pubDataSpec = std::make_unique<PubSubSpec>(chardevYaml.topic_publish, SilKit::Util::SerDes::MediaTypeData());
            pubDataSpec->AddLabel("KeyA", "ValueB", SilKit::Services::MatchingLabel::Kind::Optional);
            publisherName = "pub" + pathToFile_;
        }

        auto newAdapter = std::make_shared<ChardevAdapter>(participant, 
                                                           publisherName, 
                                                           subscriberName, 
                                                           std::move(pubDataSpec), 
                                                           std::move(subDataSpec), 
                                                           pathToFile, 
                                                           &_ioc);

        newAdapter->Initialize();

        ioAdapters.push_back(newAdapter);
    }

    _thread = std::thread([&]() -> void {
        _ioc.run();
    });
}

void ChardevManager::GetYamlConfig(const YAML::Node& doc, std::vector<Util::DataYAMLConfig>& dataYAMLConfigs)
{
    const std::vector<std::string> attributes{"path", "topic_subscribe", "topic_publish"};

    const auto nodeChardevs = doc["chardevs"];

    _logger->Debug("Character devices found in the YAML configuration file.");
    
    for (std::size_t i = 0; i < nodeChardevs.size(); ++i)
    {    
        // Check if there is any unknown attribute
        if (Util::FindUnknownAttribute(attributes, nodeChardevs[i], _logger))
        {
            throw YamlError("Unknown attribute found.");
        }

        Util::DataYAMLConfig chardevYaml;
        // Manage path attribute
        Util::GetYAMLValue(nodeChardevs[i], "path", chardevYaml.path, true, _logger);

        // Manage topic_subscribe attribute
        Util::GetYAMLValue(nodeChardevs[i], "topic_subscribe", chardevYaml.topic_subscribe, false, _logger);

        // Manage topic_publish attribute
        Util::GetYAMLValue(nodeChardevs[i], "topic_publish", chardevYaml.topic_publish, false, _logger);

        dataYAMLConfigs.push_back(chardevYaml);
    }
}