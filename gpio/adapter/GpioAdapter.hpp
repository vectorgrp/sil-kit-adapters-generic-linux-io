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
class GpioAdapter : public IOAdapter, public std::enable_shared_from_this<GpioAdapter>
{
using PubSubSpec = SilKit::Services::PubSub::PubSubSpec;
using offset_t = uint8_t;

public:
    GpioAdapter(SilKit::IParticipant* participant,
                const std::string& publisherName, 
                const std::string& subscriberName, 
                std::unique_ptr<PubSubSpec> pubDataSpec, 
                std::unique_ptr<PubSubSpec> subDataSpec,
                GpioWrapper::Chip* gpiochip,
                GpioWrapper::Ioc* ioc,
                const offset_t offset);
    ~GpioAdapter() = default;

    // Called after the constructor in order to use shared_from_this()
    void Initialize();

    // Access publish method in main function
    void Publish() override;
    // Deserialize received value and direction 
    void Deserialize(const std::vector<uint8_t>& bytes) override;
    // Serialize value and direction
    auto Serialize() -> std::vector<uint8_t> override;

    // Create data subscriber after the end of the initialization
    void CreateDataSubscriber() override;

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
    std::shared_ptr<GpioWrapper::EventHandle> _eh;
    std::shared_ptr<GpioWrapper::LineHandle> _lh;   

    // Publisher and subscriber name and topics
    std::string _publishTopic{};
    std::string _subscribeTopic{};
    std::string _subscriberName{};

    // Sil Kit participant and pub/sub 
    SilKit::IParticipant* _participant;
    SilKit::Services::Logging::ILogger* _logger;
    SilKit::Services::PubSub::IDataPublisher* _publisher;
    SilKit::Services::PubSub::IDataSubscriber* _subscriber;
    std::unique_ptr<PubSubSpec> _subDataSpec;

    // Receive and handle events
    void ReceiveEvent() override;
    // Close the current request
    void CloseRequests();

    inline auto to_string(const offset_t offset) const -> std::string;
};

// Inline implementations
auto GpioAdapter::to_string(const offset_t offset) const -> std::string
{
    return std::to_string(static_cast<int>(offset));
}