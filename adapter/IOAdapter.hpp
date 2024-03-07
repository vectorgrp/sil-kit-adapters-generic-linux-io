// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <vector>

// Interface for handling one data
class IOAdapter
{
public:
    virtual ~IOAdapter() = default;

    virtual void Publish() = 0;
    // Serialize internal values
    virtual auto Serialize() -> std::vector<uint8_t> = 0;
    // Deserialize all received values
    virtual void Deserialize(const std::vector<uint8_t>& bytes) = 0;
    // Handler to receive events
    virtual void ReceiveEvent() = 0;

    // Create an associate data subscriber can be done in a specific method
    virtual void CreateDataSubscriber() {};
};