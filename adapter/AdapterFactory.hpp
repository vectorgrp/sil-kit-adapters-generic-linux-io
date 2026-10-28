// SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
// SPDX-License-Identifier: MIT

#pragma once

#include <vector>
#include <memory>

#include "silkit/SilKit.hpp"
#include "util/YamlHelper.hpp"
#include "asio/io_context.hpp"

class IOAdapter;
namespace GpioWrapper {
class Chip;
}

namespace AdapterFactory {
using YamlNode = adapters::Util::YamlNode;

void ConstructAdAdapters(const YamlNode& configFile, std::vector<std::unique_ptr<IOAdapter>>& ioAdapters,
                         SilKit::IParticipant* participant, asio::io_context& ioc);

void ConstructChardevAdapters(const YamlNode& configFile, std::vector<std::unique_ptr<IOAdapter>>& ioAdapters,
                              SilKit::IParticipant* participant, asio::io_context& ioc);

#ifndef QNX_BUILD
void ConstructGpioAdapters(const YamlNode& configFile, std::vector<std::unique_ptr<IOAdapter>>& ioAdapters,
                           std::vector<std::unique_ptr<GpioWrapper::Chip>>& chips, SilKit::IParticipant* participant,
                           asio::io_context& ioc);
#endif
} // namespace AdapterFactory
