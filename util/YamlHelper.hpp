// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "FileHelper.hpp"
#include "Exceptions.hpp"

#include "silkit/services/pubsub/all.hpp"

#include "yaml-cpp/yaml.h"

namespace adapters::Util
{

// Data to get from YAML config file
struct DataYAMLConfig
{
    std::string path = "";
    std::string fileName = "";    
    std::string topic_subscribe = "";
    std::string topic_publish = "";

    // Optional data
    std::string dataType = "";
    std::string offset = "";
};

// Load configFile into a YAML::Node
void LoadYAMLConfigFile(YAML::Node& doc, const std::string& configFile, SilKit::Services::Logging::ILogger* logger);

// Get the given value in the given node, and set it to the corresponding value to update
void GetYAMLValue(const YAML::Node& node, const std::string& valueToGet, std::string& valueToUpdate, const bool isMandatory, SilKit::Services::Logging::ILogger* logger);

// Check if there is an unknown attribute in the YAML file for the given node
inline auto FindUnknownAttribute(std::vector<std::string> attributes, const YAML::Node& node, SilKit::Services::Logging::ILogger* logger) -> bool
{
    for (YAML::const_iterator it = node.begin(); it != node.end(); ++it)
    {
        if (std::find(attributes.begin(), attributes.end(), it->first.as<std::string>()) == std::end(attributes))
        {
            logger->Error("Error in YAML config file. Unknown attribute: '" + it->first.as<std::string>() + "'");
            return true;
        }
    }
    return false;
}

} // namespace adapters::Util