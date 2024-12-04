// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <string>
#include <vector>
#include <array>
#include <unistd.h>

#include "silkit/services/logging/all.hpp"

namespace adapters {
namespace Util {

static constexpr std::size_t BUF_LEN = 4096;

auto ReadFileStr(const std::string& path, SilKit::Services::Logging::ILogger* logger) -> std::string;
auto ReadFile(const std::string& path, SilKit::Services::Logging::ILogger* logger, std::array<uint8_t, BUF_LEN>& buffer) -> std::size_t;
void WriteFile(const std::string& path, const std::vector<uint8_t>& dataToWrite, SilKit::Services::Logging::ILogger* logger);

inline auto FileExists(const std::string& path) -> bool {
    return (access(path.c_str(), F_OK) != -1);
}

} // namespace Util
} // namespace adapters