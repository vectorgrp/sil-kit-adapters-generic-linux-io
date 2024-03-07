// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "AdManager.hpp"

#include <algorithm>

#include "AdAdapter.hpp"
#include "../../util/Exceptions.hpp"

#include "yaml-cpp/yaml.h"

#include "silkit/util/serdes/Serialization.hpp"

using namespace adapters;
using namespace SilKit::Services::PubSub;

static const std::vector<std::string> attributes
    {"advalues", "path", "files", "name", "topic_subscribe", "topic_publish"};

static const std::vector<std::string> types
    {"int8_t", "uint8_t", "int16_t", "uint16_t", "int32_t", "uint32_t", "int64_t", "uint64_t", "float", "double"};

AdManager::AdManager(const YAML::Node& configFile,
                     std::vector<std::shared_ptr<IOAdapter>>& ioAdapters,
                     SilKit::Services::Logging::ILogger* logger,
                     SilKit::IParticipant* participant) :
    _logger(logger)
{
    // Initialize values from config file
    InitAdaptersFromConfigFile(configFile, ioAdapters, participant);
}

AdManager::~AdManager()
{
    Stop();
}

void AdManager::Stop() 
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

void AdManager::InitAdaptersFromConfigFile(const YAML::Node& configFile,
                                           std::vector<std::shared_ptr<IOAdapter>>& ioAdapters, 
                                           SilKit::IParticipant* participant)
{
    std::vector<std::vector<Util::DataYAMLConfig>> advaluesYAMLConfigs;
    GetYamlConfig(configFile, advaluesYAMLConfigs);

    for (const auto& advaluesYaml : advaluesYAMLConfigs)
    {
        for (const auto& fileValuesYaml : advaluesYaml)
        {
            std::unique_ptr<PubSubSpec> subDataSpec, pubDataSpec;
            std::string publisherName, subscriberName;

            // Manage topic_subscribe
            if (!fileValuesYaml.topic_subscribe.empty())
            {
                subDataSpec = std::make_unique<PubSubSpec>(fileValuesYaml.topic_subscribe, SilKit::Util::SerDes::MediaTypeData());
                subDataSpec->AddLabel("KeyA", "ValueA", SilKit::Services::MatchingLabel::Kind::Optional);
                subscriberName = "sub" + fileValuesYaml.fileName;
            }

            // Manage topic_publish
            if (!fileValuesYaml.topic_publish.empty())
            {
                pubDataSpec = std::make_unique<PubSubSpec>(fileValuesYaml.topic_publish, SilKit::Util::SerDes::MediaTypeData());
                pubDataSpec->AddLabel("KeyA", "ValueA", SilKit::Services::MatchingLabel::Kind::Optional);
                publisherName = "pub" + fileValuesYaml.fileName;
            }

            auto newAdapter = std::make_shared<AdAdapter>(participant, 
                                                         publisherName, 
                                                         subscriberName, 
                                                         std::move(pubDataSpec), 
                                                         std::move(subDataSpec), 
                                                         fileValuesYaml.path + fileValuesYaml.fileName, 
                                                         &_ioc,
                                                         fileValuesYaml.dataType);
            
            newAdapter->Initialize();

            ioAdapters.push_back(newAdapter);
        }
    }

    _thread = std::thread([&]() -> void {
        _ioc.run();
    });
}

void AdManager::GetYamlConfig(const YAML::Node& doc, std::vector<std::vector<Util::DataYAMLConfig>>& dataYAMLConfigs)
{
    const auto nodeAdvalues = doc["advalues"];

    _logger->Debug("Advalues chips found in the YAML configuration file.");

    std::vector<std::string> allAttr = attributes;
    std::for_each(types.begin(), types.end(), [&allAttr](const std::string& str){allAttr.push_back(str);});

    for (std::size_t i = 0; i < nodeAdvalues.size(); ++i)
    {    
        // Check unknown attribute in path, and all data types
        if (Util::FindUnknownAttribute(allAttr, nodeAdvalues[i], _logger))
        {
            throw YamlError("Unknown attribute found in item " + std::to_string(i));
        }

        std::vector<Util::DataYAMLConfig> advaluesYaml;

        for (const auto& dataType : types)
        {
            if (const auto& typeNodes = nodeAdvalues[i][dataType])
            {
                FillAdvaluesYAML(typeNodes, advaluesYaml, dataType);
            }
        }
        
        // Set the right chip path to each file
        std::string path;
        Util::GetYAMLValue(nodeAdvalues[i], "path", path, true, _logger);
        for (auto&& file : advaluesYaml)
        {
            file.path = path;
        }

        dataYAMLConfigs.push_back(advaluesYaml);
    }
}

void AdManager::FillAdvaluesYAML(const YAML::Node& mainNode, std::vector<Util::DataYAMLConfig>& advaluesYaml, const std::string& dataType)
{
    for (const auto& subNode : mainNode)
    {
        // Check unknown attribute in file
        if (Util::FindUnknownAttribute(attributes, subNode, _logger))
        {
            throw YamlError("Unknown attribute found.");
        }
             
        // Manage mandatory files attribute
        if (const auto& nodeFiles = subNode["files"])
        {
            for (const auto& node : nodeFiles)
            {
                // Check unknown attribute in name and topics
                if (Util::FindUnknownAttribute(attributes, node, _logger))
                {
                    throw YamlError("Unknown attribute found.");
                }
            
                Util::DataYAMLConfig fileValuesYaml;
                // Manage data type
                fileValuesYaml.dataType = dataType;

                // Manage mandatory name attribute
                Util::GetYAMLValue(node, "name", fileValuesYaml.fileName, true, _logger);

                // Manage optional topic_subscribe attribute
                Util::GetYAMLValue(node, "topic_subscribe", fileValuesYaml.topic_subscribe, false, _logger);

                // Manage optional topic_publish attribute
                Util::GetYAMLValue(node, "topic_publish", fileValuesYaml.topic_publish, false, _logger);

                advaluesYaml.push_back(fileValuesYaml);
            }
        }
        else
        {
            throw YamlError("Missing mandatory 'files' attribute.");
        }
    }
}