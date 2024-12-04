// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <vector>
#include <string>

#include "../../adapter/IOAdapter.hpp"

#include "silkit/SilKit.hpp"
#include "silkit/services/pubsub/all.hpp"

#include "asio/posix/stream_descriptor.hpp"

// each file has a specific ChardevAdapter
class ChardevAdapter : public IOAdapter
{
using PubSubSpec = SilKit::Services::PubSub::PubSubSpec;

public:
    // access and manage the chardev
    std::string _pathToFile;
    std::array<uint8_t, 4096> _bufferToPublisher = {};

    ChardevAdapter() = delete;
    ChardevAdapter(SilKit::IParticipant* participant, 
                   const std::string& publisherName, 
                   const std::string& subscriberName, 
                   PubSubSpec* pubDataSpec, 
                   PubSubSpec* subDataSpec,
                   const std::string& pathToCharDev,
                   asio::io_context& ioc);
    ~ChardevAdapter();

    // publish n bytes of the chardev buffer
    void Publish(const std::size_t n);
private:
    // handle error code introduced by _fd.cancel()
    bool _isCancelled;
    std::unique_ptr<asio::posix::stream_descriptor> _fd;

    std::vector<uint8_t> _bufferFromSubscriber = {};

    // deserialize received values
    void Deserialize(const std::vector<uint8_t>& bytes) override;
    void ReceiveEvent();    
};