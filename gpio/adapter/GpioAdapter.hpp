// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <vector>
#include <memory>
#include <thread>

#include "GpioWrapper.hpp"

#include "../../adapter/IOAdapter.hpp"

#include "silkit/SilKit.hpp"
#include "silkit/services/logging/all.hpp"
#include "silkit/services/pubsub/all.hpp"

// Each gpiochip line has a specific GpioAdapter
class GpioAdapter : public IOAdapter
{
using PubSubSpec = SilKit::Services::PubSub::PubSubSpec;
using offset_t = uint8_t;

public:
    friend class GpioManager;

    GpioAdapter(SilKit::IParticipant* participant,
                const std::string& publisherName, 
                const std::string& subscriberName, 
                PubSubSpec* pubDataSpec, 
                std::unique_ptr<PubSubSpec> subDataSpec,
                GpioWrapper::Chip* gpiochip,
                GpioWrapper::Ioc* ioc,
                const offset_t offset);
    ~GpioAdapter()
    {
        if (_eh->IsFdOpen())
        {
            _eh->Cancel();
            _eh->Close();
        }
        if (_lh->IsFdOpen())
        {
            _lh->Close();
        }
    }

private:
    // Specific enum to gpios
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

    // Informations about the line
    offset_t _offset;
    Value _value;
    Direction _direction;

    // Handle error code introduced by fd.cancel()
    bool _isCancelled;

    // Chip and io context linked to the line
    GpioWrapper::Chip* _chip;
    GpioWrapper::Ioc* _ioc;
    // Requests on the line
    std::unique_ptr<GpioWrapper::EventHandle> _eh;
    std::unique_ptr<GpioWrapper::LineHandle> _lh;

    // Subscriber name
    std::string _subscriberName{};

    // Sil Kit participant
    SilKit::IParticipant* _participant;
    std::unique_ptr<PubSubSpec> _subDataSpec;

    // Publish internal data
    void Publish() override;

    // Called after the constructor in order to use shared_from_this()
    void Initialize();

    // Deserialize received value and direction 
    void Deserialize(const std::vector<uint8_t>& bytes) override;
    // Serialize value and direction
    auto Serialize() -> std::vector<uint8_t> override;

    // Create data subscriber after the end of the initialization
    void CreateDataSubscriber() override;

    // Receive and handle events
    void ReceiveEvent();
    // Close the current request
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