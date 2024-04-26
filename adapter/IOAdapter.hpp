// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <vector>

#include "silkit/SilKit.hpp"
#include "silkit/services/logging/all.hpp"
#include "silkit/services/pubsub/all.hpp"

// Interface for handling one data
class IOAdapter
{
public:
    // Sil Kit logger
    SilKit::Services::Logging::ILogger* _logger;

    // Sil Kit pub/sub
    SilKit::Services::PubSub::IDataPublisher* _publisher;
    SilKit::Services::PubSub::IDataSubscriber* _subscriber;

    // Publisher and subscriber topics
    std::string _publishTopic{};
    std::string _subscribeTopic{};

    virtual ~IOAdapter() = default;

    virtual void Publish() = 0;
    // Serialize internal values
    virtual auto Serialize() -> std::vector<uint8_t> = 0;
    // Deserialize all received values
    virtual void Deserialize(const std::vector<uint8_t>& bytes) = 0;

    // Create an associate data subscriber can be done in a specific method
    virtual void CreateDataSubscriber() {};
};