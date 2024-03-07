// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <vector>
#include <memory>

#include "./../adapter/IOAdapter.hpp"
#include "../../adapter/IOManager.hpp"

#include "../../util/YamlHelper.hpp"

#include "silkit/SilKit.hpp"
#include "silkit/services/logging/all.hpp"

#include <asio/posix/stream_descriptor.hpp>

// Manage all advalues adapters (initialize, events identification)
class AdManager : public IOManager
{
public:
	AdManager() = delete;
	AdManager(const YAML::Node& configFile,
              std::vector<std::shared_ptr<IOAdapter>>& ioAdapters, 
              SilKit::Services::Logging::ILogger* logger, 
              SilKit::IParticipant* participant);
	~AdManager();

    void Stop() override;

    // Get chip config from YAML file and initialize all chardev adapters
	void InitAdaptersFromConfigFile(const YAML::Node& configFile,
                                    std::vector<std::shared_ptr<IOAdapter>>& ioAdapters, 
                                    SilKit::IParticipant* participant) override;

private:
    // Ioc to handle every character devices 
    asio::io_context _ioc;
    std::thread _thread;

    SilKit::Services::Logging::ILogger* _logger; 

    // Get all informations from YAML configuration file
    void GetYamlConfig(const YAML::Node& doc, std::vector<std::vector<adapters::Util::DataYAMLConfig>>& dataYAMLConfigs);
    void FillAdvaluesYAML(const YAML::Node& mainNode, std::vector<adapters::Util::DataYAMLConfig>& advaluesYAMLConfigs, const std::string& dataType);
};