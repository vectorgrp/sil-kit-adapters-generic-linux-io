// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "FileHelper.hpp"
#include "Exceptions.hpp"

#include "silkit/services/pubsub/all.hpp"
#include "silkit/util/serdes/Serialization.hpp"

#include "yaml-cpp/yaml.h"

namespace adapters {
namespace Util {

using PubSubSpec = SilKit::Services::PubSub::PubSubSpec;

// data to get from YAML config file
struct DataYAMLConfig
{
    std::string path = "";
    std::string topic_subscribe = "";
    std::string topic_publish = "";

    // optional data
    std::string fileName = ""; 
    std::string dataType = "";
    std::string offset = "";
};

// load configFile into a YAML::Node
void LoadYAMLConfigFile(YAML::Node& doc, const std::string& configFile, SilKit::Services::Logging::ILogger* logger);

// get the given value in the given node, and set it to the corresponding value to update
void GetYAMLValue(const YAML::Node& node, const std::string& valueToGet, std::string& valueToUpdate, const bool isMandatory, SilKit::Services::Logging::ILogger* logger);

// check if there is an unknown attribute in the YAML file for the given node
template<std::size_t N>
inline auto FindUnknownAttribute(const std::array<const char*, N>& attributes, const YAML::Node& node, SilKit::Services::Logging::ILogger* logger) -> bool
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

} // namespace Util
} // namespace adapters