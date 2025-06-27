// SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
// SPDX-License-Identifier: MIT

#pragma once

#include <vector>
#include <string>

#include "silkit/services/logging/all.hpp"
#include "silkit/services/pubsub/all.hpp"

// interface for handling one data
class IOAdapter
{
public:
    // SIL Kit logger
    SilKit::Services::Logging::ILogger* _logger;

    // SIL Kit pub/sub
    SilKit::Services::PubSub::IDataPublisher* _publisher;
    SilKit::Services::PubSub::IDataSubscriber* _subscriber;

    // publisher and subscriber topics
    std::string _publishTopic{};
    std::string _subscribeTopic{};

    virtual ~IOAdapter() = default;

    // virtual void Publish() = 0;
    // deserialize all received values
    virtual void Deserialize(const std::vector<uint8_t>& bytes) = 0;

    // create an associate data subscriber can be done in a specific method
    virtual void CreateDataSubscriber() {};
};