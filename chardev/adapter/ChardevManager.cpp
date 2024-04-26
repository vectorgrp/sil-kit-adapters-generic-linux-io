// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "ChardevManager.hpp"

#include "../../util/FileHelper.hpp"
#include "../../util/Exceptions.hpp"

#include "silkit/util/serdes/Serialization.hpp"

#include "asio/read.hpp"

using namespace adapters;
using namespace SilKit::Services::PubSub;

ChardevManager::ChardevManager(const YAML::Node& configFile,
                               std::vector<std::unique_ptr<IOAdapter>>& ioAdapters,
                               SilKit::Services::Logging::ILogger* logger, 
                               SilKit::IParticipant* participant) :
    ChardevManager(logger)
{
    // Initialize values from config file
    InitAdaptersFromConfigFile(configFile, ioAdapters, participant);
}

ChardevManager::ChardevManager(SilKit::Services::Logging::ILogger* logger) :
    _isCancelled(false)
{
    _logger = logger;
    
    _inotifyFd = inotify_init1( IN_NONBLOCK );
    if (_inotifyFd == -1) {
        throw InotifyError("inotify initialization error (" + std::to_string(errno) +")");
    }

    _fd = std::make_unique<asio::posix::stream_descriptor>(_ioc, _inotifyFd);
}

ChardevManager::~ChardevManager()
{
    Stop();
}

void ChardevManager::Stop() 
{
    _isCancelled = true;

    if (_fd->is_open())
    {
        _logger->Debug("Cancel operation on asio stream_descriptor (with inotify fd: " + std::to_string(_inotifyFd) + ") and close it.");
        _fd->cancel();
        _fd->close();
    }
    if (!_ioc.stopped())
    {
        _logger->Debug("Stop the associated asio io_context.");
        _ioc.stop();
    }
    if (_thread.joinable())
    {
        _thread.join();
    }
}

void ChardevManager::InitAdaptersFromConfigFile(const YAML::Node& configFile,
                                                std::vector<std::unique_ptr<IOAdapter>>& ioAdapters,
                                                SilKit::IParticipant* participant)
{
    std::vector<Util::DataYAMLConfig> chardevYAMLConfigs;
    GetYamlConfig(configFile, chardevYAMLConfigs);
    
    for (auto chardevYaml : chardevYAMLConfigs)
    {
        const std::string pathToFile = chardevYaml.path;
        
        std::string pathToFile_ = pathToFile;
        for(std::size_t i = 0; i < pathToFile_.size(); ++i)
        {
            if (pathToFile_[i] == '/')
            {
                pathToFile_.replace(pathToFile_.begin() + i, pathToFile_.begin() + i + 1, "_");
            }
        }

        const auto pos = pathToFile.find_last_of('/');
        const auto fileName = pathToFile.substr(pos + 1);

        std::unique_ptr<PubSubSpec> subDataSpec, pubDataSpec;
        std::string subscriberName, publisherName;
        // Manage topic_subscribe
        if (!chardevYaml.topic_subscribe.empty())
        {
            subDataSpec = std::make_unique<PubSubSpec>(chardevYaml.topic_subscribe, SilKit::Util::SerDes::MediaTypeData());
            subDataSpec->AddLabel("KeyA", "ValueA", SilKit::Services::MatchingLabel::Kind::Optional);
            subscriberName = "sub" + pathToFile_;
        }

        // Manage topic_publish
        if (!chardevYaml.topic_publish.empty())
        {
            pubDataSpec = std::make_unique<PubSubSpec>(chardevYaml.topic_publish, SilKit::Util::SerDes::MediaTypeData());
            pubDataSpec->AddLabel("KeyA", "ValueB", SilKit::Services::MatchingLabel::Kind::Optional);
            publisherName = "pub" + pathToFile_;
        }

        auto newAdapter = std::make_unique<ChardevAdapter>(participant, 
                                                           publisherName, 
                                                           subscriberName, 
                                                           pubDataSpec.get(), 
                                                           subDataSpec.get(), 
                                                           pathToFile,
                                                           _inotifyFd);

        // _wdAdapter[newAdapter->_wd] = std::move(newAdapter);
        _wdAdapter[newAdapter->_wd] = newAdapter.get();

        ioAdapters.push_back(std::move(newAdapter));
    }

    ReceiveEvent();

    _thread = std::thread([&]() -> void {
        _ioc.run();
    });
}

void ChardevManager::GetYamlConfig(const YAML::Node& doc, std::vector<Util::DataYAMLConfig>& dataYAMLConfigs)
{
    const std::vector<std::string> attributes{"path", "topic_subscribe", "topic_publish"};

    const auto nodeChardevs = doc["chardevs"];

    _logger->Debug("Character devices found in the YAML configuration file.");
    
    for (std::size_t i = 0; i < nodeChardevs.size(); ++i)
    {    
        // Check if there is any unknown attribute
        if (Util::FindUnknownAttribute(attributes, nodeChardevs[i], _logger))
        {
            throw YamlError("Unknown attribute found.");
        }

        Util::DataYAMLConfig chardevYaml;
        // Manage path attribute
        Util::GetYAMLValue(nodeChardevs[i], "path", chardevYaml.path, true, _logger);

        // Manage topic_subscribe attribute
        Util::GetYAMLValue(nodeChardevs[i], "topic_subscribe", chardevYaml.topic_subscribe, false, _logger);

        // Manage topic_publish attribute
        Util::GetYAMLValue(nodeChardevs[i], "topic_publish", chardevYaml.topic_publish, false, _logger);

        dataYAMLConfigs.push_back(chardevYaml);
    }
}

void ChardevManager::ReceiveEvent()
{
    async_read(*_fd, asio::buffer(_eventBuffer, sizeof(inotify_event)),
    [this](const std::error_code ec, const std::size_t bytes_transferred){
        if (ec) 
        {
            if(_isCancelled && (ec == asio::error::operation_aborted))
            {
                // An error code comes right after calling fd.cancel() in order to close all asynchronous reads
                _isCancelled = false;
            }
            else
            {
                // If the error does not happened after fd.cancel(), handle it
                _logger->Error("Unable to handle event. "
                               "Error code: " + std::to_string(ec.value()) + " (" + ec.message()+ "). " +
                               "Error category: " + ec.category().name());
            }
        }
        else
        {
            auto event = reinterpret_cast<const struct inotify_event *>(_eventBuffer.data());

            ChardevAdapter* adapterOnEvent;

            if (auto search = _wdAdapter.find(event->wd); search != _wdAdapter.end())
            {
                adapterOnEvent = search->second;

                if (adapterOnEvent->_isRecvValue)
                {
                    adapterOnEvent->_isRecvValue = false;
                }
                else // The file has been modified and does not come from deserialization
                {
                    _logger->Debug(adapterOnEvent->_pathToCharDev + " has been updated");
                    // Read the value only if it has to be sent
                    if (!adapterOnEvent->_publishTopic.empty())
                    {
                        adapterOnEvent->_bufferFromChardev = Util::ReadFile(adapterOnEvent->_pathToCharDev, _logger, ChardevAdapter::BUF_LEN);
                        adapterOnEvent->Publish();
                    }
                }
            }
            else
            {
                _logger->Error("Event error on watch descriptor " + std::to_string(event->wd));
            }

            ReceiveEvent();
        }
    });
}
