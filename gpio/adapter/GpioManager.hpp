// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <string>
#include <memory>
#include <unordered_map>

#include "GpioWrapper.hpp"

#include "../../adapter/IOManager.hpp"
#include "../../adapter/IOAdapter.hpp"
#include "../../util/YamlHelper.hpp"

#include "silkit/SilKit.hpp"
#include "silkit/services/logging/all.hpp"
#include "silkit/services/pubsub/all.hpp"

#include "yaml-cpp/yaml.h"

// Manage all gpio adapters (initialize, events identification)
class GpioManager : public IOManager
{
public:
	GpioManager() = delete;
	GpioManager(const YAML::Node& configFile,
                std::vector<std::unique_ptr<IOAdapter>>& ioAdapters,
                SilKit::Services::Logging::ILogger* logger, 
                SilKit::IParticipant* participant);
	~GpioManager();

    void Stop() override;

private:
    // One ioc and thread per gpio chip
    std::unordered_map<std::unique_ptr<GpioWrapper::Chip>, std::unique_ptr<GpioWrapper::Ioc>> _chipContexts;
    std::vector<std::thread> threadPool;

    // Open the chip and create Adapters 
	void InitAdaptersFromConfigFile(const YAML::Node& configFile,
                                    std::vector<std::unique_ptr<IOAdapter>>& ioAdapters,
                                    SilKit::IParticipant* participant) override;

    // Get all informations from YAML configuration file
    void GetYamlConfig(const YAML::Node& chipNode, std::vector<adapters::Util::DataYAMLConfig>& dataYAMLConfigs, std::string& chipPath);
};