// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <string>
#include <vector>

#include "silkit/services/logging/all.hpp"

namespace adapters::Util
{

auto ReadFileStr(const std::string& path, SilKit::Services::Logging::ILogger* logger) -> std::string;
auto ReadFile(const std::string& path, SilKit::Services::Logging::ILogger* logger, const std::size_t valueSizeMax) -> std::vector<uint8_t>;
void WriteFile(const std::string& path, const std::vector<uint8_t>& dataToWrite, SilKit::Services::Logging::ILogger* logger);

// Return a string without '\n' if any
inline auto RemoveReturn(const std::string& str) -> std::string
{
    if(auto pos = str.find('\n'); pos != std::string::npos)
        return str.substr(0, pos);
    
    return str;
}

} // namespace adapters::Util