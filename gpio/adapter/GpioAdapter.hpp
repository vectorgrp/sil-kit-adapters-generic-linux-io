// SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
// SPDX-License-Identifier: MIT

#pragma once

#include <vector>
#include <memory>
#include <thread>

#include "GpioWrapper.hpp"

#include "../../adapter/IOAdapter.hpp"

#include "silkit/SilKit.hpp"
#include "silkit/services/logging/all.hpp"
#include "silkit/services/pubsub/all.hpp"

namespace adapters
{
enum Direction : uint8_t
{
    INPUT = 0,
    OUTPUT = 1
};

enum Value : uint8_t
{
    VALUE_LOW = 0,
    VALUE_HIGH = 1,
    VALUE_UNKNOWN = 2
};

// each gpiochip line has a specific GpioAdapter
class GpioAdapter : public IOAdapter
{
using PubSubSpec = SilKit::Services::PubSub::PubSubSpec;
using offset_t = uint8_t;

public:

    GpioAdapter(SilKit::IParticipant* participant,
                const std::string& publisherName, 
                const std::string& subscriberName, 
                PubSubSpec* pubDataSpec, 
                std::unique_ptr<PubSubSpec> subDataSpec,
                GpioWrapper::Chip* gpiochip,
                GpioWrapper::Ioc& ioc,
                const offset_t offset);
    ~GpioAdapter();

private:
    // information about the line
    offset_t _offset;
    Value _value;
    Direction _direction;

    // handle error code introduced by fd.cancel()
    bool _isCancelled;

    // chip and io context linked to the line
    GpioWrapper::Chip* _chip;
    GpioWrapper::Ioc* _ioc;
    // requests on the line
    std::unique_ptr<GpioWrapper::EventHandle> _eh;
    std::unique_ptr<GpioWrapper::LineHandle> _lh;

    SilKit::IParticipant* _participant;
    std::unique_ptr<PubSubSpec> _subDataSpec;
    std::string _subscriberName{};

    // publish internal data
    void Publish();

    void Initialize();

    // deserialize received value and direction 
    void Deserialize(const std::vector<uint8_t>& bytes) override;

    // create data subscriber after the end of the initialization
    void CreateDataSubscriber() override;

    // receive and handle events
    void ReceiveEvent();
    // close the current request
    void CloseRequests();

    inline auto to_string(const offset_t offset) const -> std::string;
};

////////////////////////////
// Inline implementations //
////////////////////////////

auto GpioAdapter::to_string(const offset_t offset) const -> std::string
{
    return std::to_string(static_cast<int>(offset));
}

} // namespace adapters
