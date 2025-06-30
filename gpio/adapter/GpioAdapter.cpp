// SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
// SPDX-License-Identifier: MIT

#include "GpioAdapter.hpp"

#include <linux/gpio.h>

#include "silkit/util/serdes/Deserializer.hpp"
#include "silkit/util/serdes/Serializer.hpp"

#include "asio/posix/stream_descriptor.hpp"
#include "asio/read.hpp"

using namespace SilKit::Services::PubSub;
using namespace GpioWrapper;

namespace adapters
{
GpioAdapter::GpioAdapter(SilKit::IParticipant* participant,
                         const std::string& publisherName, 
                         const std::string& subscriberName, 
                         PubSubSpec* pubDataSpec, 
                         std::unique_ptr<PubSubSpec> subDataSpec,
                         Chip* gpiochip,
                         Ioc& ioc,
                         const offset_t offset) :
    _offset(offset),
    _isCancelled(false),
    _chip(gpiochip),
    _ioc(&ioc),
    _participant(participant)
{
    _logger = participant->GetLogger();

    Initialize();

    if (subDataSpec)
    {
        _subscribeTopic = subDataSpec->Topic();
        _subscriberName = subscriberName;
        _subDataSpec = std::move(subDataSpec);
    }

    if (pubDataSpec)
    {
        _publishTopic = pubDataSpec->Topic();
        _publisher = participant->CreateDataPublisher(publisherName, *pubDataSpec, 1);

        // publish initial values
        Publish();
    }
}

GpioAdapter::~GpioAdapter()
{
    _isCancelled = true;

    if (_eh && _eh->IsFdOpen())
    {
        _eh->Cancel();
        _eh->Close();
    }
    if (_lh && _lh->IsFdOpen())
    {
        _lh->Close();
    }
}

void GpioAdapter::Initialize()
{
    // handle chip values for each line 
    // get the line direction
    if (_chip->GetLineInfo(_offset).GetDirection() == GpioWrapper::In)
    {
        _direction = INPUT;

        // get the initial value for INPUT mode
        _lh.reset(new LineHandle(*_ioc, *_chip, {_offset}, GpioWrapper::In));
        _value = static_cast<Value>(_lh->GetValue());
        CloseRequests();

        // new request to catch events
        _eh.reset(new EventHandle(*_ioc, *_chip, _offset, GpioWrapper::BothEdges));
        ReceiveEvent();
    }
    else if (_chip->GetLineInfo(_offset).GetDirection() == GpioWrapper::Out)
    {
        _direction = OUTPUT;

        // don't handle output direction at initialization (to keep chip config as is)
        // set value as unknown 
        _value = VALUE_UNKNOWN;
    }
}

void GpioAdapter::Publish()
{
    if (_publishTopic.empty())
        return;

    _logger->Debug("Serializing data and publishing on topic: " + _publishTopic);
    SilKit::Util::SerDes::Serializer serializer;
    serializer.Serialize(static_cast<uint8_t>(_value), 8);
    serializer.Serialize(static_cast<uint8_t>(_direction), 8);

    _publisher->Publish(serializer.ReleaseBuffer());
}

void GpioAdapter::Deserialize(const std::vector<uint8_t> &bytes)
{
    _logger->Debug("Deserializing data from topic: " + _subscribeTopic);
    SilKit::Util::SerDes::Deserializer deserializer(bytes);
    
    auto receivedValue = deserializer.Deserialize<uint8_t>(8);
    _direction = static_cast<Direction>(deserializer.Deserialize<uint8_t>(8));
    
    // if received direction is OUTPUT update the value
    if (_direction == OUTPUT)
    {
        _value = static_cast<Value>(receivedValue);
    }
}

 void GpioAdapter::CreateDataSubscriber()
 {
    // init data subscriber only if sub topic has been defined
    if (_subDataSpec)
    {
        // the initialization of the subscriber is done after the whole chip initialization
        // to avoid data receiving during this step (the gpio mode needs all adapters to update data)
        _subscriber = _participant->CreateDataSubscriber(_subscriberName, *_subDataSpec,
            [&](SilKit::Services::PubSub::IDataSubscriber* /*subscriber*/, const DataMessageEvent& dataMessageEvent) {
                // if new values are received for the io device
                _logger->Debug("New values received on " + _subscribeTopic);
                
                // updating internal chip values
                Deserialize(SilKit::Util::ToStdVector(dataMessageEvent.data));

                // updating io device
                // close the previous request on the line
                CloseRequests();

                _logger->Debug("Updating " + std::string(_chip->GetInfo().GetName()) + " line " + to_string(_offset));

                if (_direction == INPUT)
                {
                    // when switching to INPUT, value goes back to its initial state
                    // internal value has to be updated
                    _lh.reset(new LineHandle(*_ioc, *_chip, {_offset}, GpioWrapper::In));
                    _value = static_cast<Value>(_lh->GetValue());
                    CloseRequests();
                    
                    // set the request to catch events
                    _eh.reset(new EventHandle(*_ioc, *_chip, _offset, GpioWrapper::BothEdges));
                    ReceiveEvent();
                }
                else
                {
                    _lh.reset(new LineHandle(*_ioc, *_chip, {_offset}, Out, _value));
                }

                Publish();
            });
    }
 }
 
void GpioAdapter::ReceiveEvent()
{
    gpioevent_data data;
    asio::async_read(*(_eh->GetFd()), asio::buffer(&data, sizeof(data)),
    [this, &data](const std::error_code ec, const long unsigned int /*size*/){
        if (ec)
        {
            if(_isCancelled && (ec == asio::error::operation_aborted))
            {
                // an error code comes right after calling fd.cancel() in order to close all asynchronous reads
                _isCancelled = false;
            }
            else
            {
                // if the error does not happened after fd.cancel(), handle it
                _logger->Error("Unable to handle event on " + std::string(_chip->GetInfo().GetName()) + " line " + to_string(_offset) + ". " +
                                "Error code: " + std::to_string(ec.value()) + " (" + ec.message()+ "). " +
                                "Error category: " + ec.category().name());
            }
        }
        else
        {
            if (data.id == GPIOEVENT_EVENT_RISING_EDGE)
            {
                _logger->Debug("Event rising edge on " + std::string(_chip->GetInfo().GetName()) + " line " + to_string(_offset));
                _value = VALUE_HIGH;
            }
            else
            {
                _logger->Debug("Event falling edge on " + std::string(_chip->GetInfo().GetName()) + " line " + to_string(_offset));
                _value = VALUE_LOW;
            }

            Publish();

            ReceiveEvent();
        }
    });
}

void GpioAdapter::CloseRequests()
{
    // close if any request
    if (_eh != nullptr && _eh->IsFdOpen())
    {
        _isCancelled = true;
        _logger->Trace("Cancel _eh for " + std::string(_chip->GetInfo().GetName()) + " line " + to_string(_offset));
        _eh->Cancel();
        _logger->Trace("Close _eh for " + std::string(_chip->GetInfo().GetName()) + " line " + to_string(_offset));
        _eh->Close();
    }
    
    if (_lh != nullptr && _lh->IsFdOpen())
    {
        _isCancelled = true;
        _logger->Trace("Close _lh for " + std::string(_chip->GetInfo().GetName()) + " line " + to_string(_offset));
        _lh->Close();
    }
}

} // namespace adapters