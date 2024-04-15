// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <vector>
#include <string>
#include <memory>
#include <sys/inotify.h>

#include "../../adapter/IOAdapter.hpp"

#include "silkit/SilKit.hpp"
#include "silkit/services/pubsub/all.hpp"

#include <asio/posix/stream_descriptor.hpp>

// Each file has a specific ChardevAdapter
class ChardevAdapter : public IOAdapter, public std::enable_shared_from_this<ChardevAdapter>
{
using PubSubSpec = SilKit::Services::PubSub::PubSubSpec;
static constexpr uint32_t BUF_LEN = 4096;
static constexpr uint32_t EVENT_SIZE = sizeof(inotify_event);

public:    
    ChardevAdapter() = delete;
    ChardevAdapter(SilKit::IParticipant* participant, 
                   const std::string& publisherName, 
                   const std::string& subscriberName, 
                   std::unique_ptr<PubSubSpec> pubDataSpec, 
                   std::unique_ptr<PubSubSpec> subDataSpec,
                   const std::string& pathToCharDev,
                   asio::io_context* ioc);
    
    ~ChardevAdapter();

    // Called after the constructor in order to use shared_from_this()
    void Initialize();

    // Publish chip values
    void Publish() override;
    // Serialize chip values
    auto Serialize() -> std::vector<uint8_t> override;
    // Deserialize received values 
    void Deserialize(const std::vector<uint8_t>& bytes) override;

    std::vector<uint8_t> _bufferToChardev = {};
    std::vector<uint8_t> _bufferFromChardev = {};

    std::string _pathToCharDev;
    
    // Sil Kit logger
    SilKit::Services::Logging::ILogger* _logger;

    // Publisher and subscriber topics
    std::string _publishTopic = {};
    std::string _subscribeTopic = {};

private:
    // Buffers to handle received/sent values
    std::array<uint8_t, EVENT_SIZE> _eventBuffer = {};
    
    // Prevent catching an event after deserializing and updating the character devices
    bool _isRecvValue;
    // Handle error code introduced by fd.cancel() 
    bool _isCancelled;
    
    // Handlers for events on the character device
    int inotifyFd;
    std::unique_ptr<asio::posix::stream_descriptor> fd;
    asio::io_context* _ioc;

    // Sil Kit pub/sub
    SilKit::Services::PubSub::IDataPublisher* _publisher;
    SilKit::Services::PubSub::IDataSubscriber* _subscriber;

    // Receive and handle events
    void ReceiveEvent() override;
};