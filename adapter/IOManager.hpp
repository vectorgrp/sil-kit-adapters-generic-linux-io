// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <vector>

#include "IOAdapter.hpp"

#include "silkit/SilKit.hpp"
#include "yaml-cpp/yaml.h"

// Creates the adapters
class IOManager
{
public:
    // Sil Kit logger
    SilKit::Services::Logging::ILogger* _logger;

	virtual ~IOManager() = default;

    // Initialize the adapters from YAML configuration file
    virtual void InitAdaptersFromConfigFile(const YAML::Node& configFile,
                                            std::vector<std::unique_ptr<IOAdapter>>& ioAdapters,
                                            SilKit::IParticipant* participant) = 0;

    virtual void Stop() = 0;
};