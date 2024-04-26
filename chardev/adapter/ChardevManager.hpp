// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <vector>
#include <memory>
#include <thread>
#include <unordered_map>
#include <sys/inotify.h>

#include "ChardevAdapter.hpp"

#include "./../adapter/IOAdapter.hpp"
#include "../../adapter/IOManager.hpp"
#include "../../util/YamlHelper.hpp"
#include "../../util/FileHelper.hpp"

#include "silkit/SilKit.hpp"
#include "silkit/services/logging/all.hpp"

#include "yaml-cpp/yaml.h"

#include "asio/posix/stream_descriptor.hpp"

// Manage all chardev adapters (initialize, events identification)
class ChardevManager : public IOManager
{
public:
	ChardevManager() = default;
    ChardevManager(SilKit::Services::Logging::ILogger* logger);
	ChardevManager(const YAML::Node& configFile,
                   std::vector<std::unique_ptr<IOAdapter>>& ioAdapters,
                   SilKit::Services::Logging::ILogger* logger, 
                   SilKit::IParticipant* participant);
	~ChardevManager();

    void Stop() override;

protected:
    // Inotify instance
    int _inotifyFd;

    // Perform asynchronous reads on the inotify instance
    std::unique_ptr<asio::posix::stream_descriptor> _fd;
    std::array<uint8_t, ChardevAdapter::EVENT_SIZE> _eventBuffer = {};

    // Handle error code introduced by fd.cancel()
    bool _isCancelled;

    // Ioc to handle every character devices 
    asio::io_context _ioc;
    std::thread _thread;

    // Pointer to each adapter instance with its associated watch descriptor
    std::unordered_map<int, ChardevAdapter*> _wdAdapter;

    // Handle the events on each chardev
    void ReceiveEvent();

private:
    // Get chip config from YAML file and initialize all chardev adapters
	void InitAdaptersFromConfigFile(const YAML::Node& configFile,
                                    std::vector<std::unique_ptr<IOAdapter>>& ioAdapters,
                                    SilKit::IParticipant* participant) override;

    // Get all informations from YAML configuration file
    void GetYamlConfig(const YAML::Node& doc, std::vector<adapters::Util::DataYAMLConfig>& dataYAMLConfigs);    
};
