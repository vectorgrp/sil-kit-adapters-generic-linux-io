// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <vector>
#include <memory>

#include "silkit/SilKit.hpp"
#include "yaml-cpp/yaml.h"
#include "asio/io_context.hpp"

class IOAdapter;
namespace GpioWrapper{
    class Chip;
}

namespace AdapterFactory
{
void ConstructAdAdapters(const YAML::Node& configFile,
                         std::vector<std::unique_ptr<IOAdapter>>& ioAdapters,
                         SilKit::IParticipant* participant,
                         asio::io_context& ioc);

void ConstructChardevAdapters(const YAML::Node& configFile,
                              std::vector<std::unique_ptr<IOAdapter>>& ioAdapters,
                              SilKit::IParticipant* participant,
                              asio::io_context& ioc);

#ifndef QNX_BUILD
void ConstructGpioAdapters(const YAML::Node& configFile,
                           std::vector<std::unique_ptr<IOAdapter>>& ioAdapters,
                           std::vector<std::unique_ptr<GpioWrapper::Chip>>& chips,
                           SilKit::IParticipant* participant,
                           asio::io_context& ioc);
#endif
} // namespace AdapterFactory
