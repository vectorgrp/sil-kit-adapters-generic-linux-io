// SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
// SPDX-License-Identifier: MIT

#include "YamlHelper.hpp"

// using namespace adapters;
namespace adapters {
namespace Util {
    
void LoadYAMLConfigFile(YAML::Node& doc, const std::string& configFile, SilKit::Services::Logging::ILogger* logger)
{
    logger->Debug("Loading YAML configuration file: " + configFile);
    const std::string buffer = Util::ReadFileStr(configFile, logger);
    if (buffer.empty())
    {
        logger->Error("Buffer read from " + configFile + " is empty");
        return;
    }

    doc = YAML::Load(buffer);
}

void GetYAMLValue(const YAML::Node& node, const std::string& valueToGet, std::string& valueToUpdate, const bool isMandatory, SilKit::Services::Logging::ILogger* logger)
{
    if (const auto value = node[valueToGet])
    {
        valueToUpdate = value.as<std::string>();
    }
    else
    {
        if (isMandatory)
        {
            throw YamlError("Missing " + valueToGet + " attribute.");
        }
    }
}

} // namespace Util
} // namespace adapters