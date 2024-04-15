// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "ChardevAdapter.hpp"

#include "../../util/FileHelper.hpp"
#include "../../util/Exceptions.hpp"

#include "silkit/util/serdes/Deserializer.hpp"
#include "silkit/util/serdes/Serializer.hpp"

#include <asio/read.hpp>

using namespace SilKit::Services::PubSub;
using namespace adapters;

ChardevAdapter::ChardevAdapter(SilKit::IParticipant* participant,
                               const std::string& publisherName, 
                               const std::string& subscriberName, 
                               std::unique_ptr<PubSubSpec> pubDataSpec, 
                               std::unique_ptr<PubSubSpec> subDataSpec,
                               const std::string& pathToCharDev,
                               asio::io_context* ioc) :
    _pathToCharDev(pathToCharDev),
    _logger(participant->GetLogger()),
    _isRecvValue(false),
    _isCancelled(false),
    _ioc(ioc)
{
    if (pubDataSpec)
    {
        _publishTopic = pubDataSpec->Topic();
        _publisher = participant->CreateDataPublisher(publisherName, *pubDataSpec, 1);

        // Read initial data from character devices
        _bufferFromChardev = Util::ReadFile(_pathToCharDev, _logger, BUF_LEN);
    }

    if (subDataSpec)
    {
        _subscribeTopic = subDataSpec->Topic();
        _subscriber = participant->CreateDataSubscriber(subscriberName, *subDataSpec,
            [&](SilKit::Services::PubSub::IDataSubscriber* subscriber, const DataMessageEvent& dataMessageEvent) {
                Deserialize(SilKit::Util::ToStdVector(dataMessageEvent.data));

                _logger->Debug("New value received on " + _subscribeTopic);
                _logger->Trace("Value received: " + std::string(_bufferToChardev.begin(), _bufferToChardev.end()));
                _isRecvValue = true;
                
                _logger->Debug("Updating " + _pathToCharDev);
                Util::WriteFile(_pathToCharDev, _bufferToChardev, _logger);

                // Copy the deserialized buffer into the one to publish for other participants
                _bufferFromChardev = _bufferToChardev;
                Publish();
            });
    }
}

ChardevAdapter::~ChardevAdapter()
{
    int ret = close(inotifyFd);
    if (ret == -1)
    {
        _logger->Error("Error while closing inotify file descriptor (" + std::to_string(inotifyFd) + ") for " + _pathToCharDev);
    }
}

void ChardevAdapter::Initialize()
{
    // Handle event only if there is a publisher topic
    // Initialize file event handler
    inotifyFd = inotify_init1( IN_NONBLOCK );
    if (inotifyFd == -1) {
        throw InotifyError("inotify initialization error");
    }

    fd = std::make_unique<asio::posix::stream_descriptor>(*_ioc, inotifyFd);
    auto wd = inotify_add_watch(inotifyFd, _pathToCharDev.c_str(), IN_CLOSE_WRITE);
    if (wd == -1) {
        throw InotifyError("inotify add watcher error");
    }

    ReceiveEvent();
}

void ChardevAdapter::Publish()
{
    if (!_publishTopic.empty())
    {
        _publisher->Publish(Serialize());
    }
}

auto ChardevAdapter::Serialize() -> std::vector<uint8_t>
{    
    SilKit::Util::SerDes::Serializer serializer;

    std::size_t size = _bufferFromChardev.size();
    
    serializer.BeginArray(size);
    auto publishBuffer = serializer.ReleaseBuffer();
    publishBuffer.reserve(publishBuffer.size() + size);
    publishBuffer.insert(publishBuffer.end(), _bufferFromChardev.begin(),
                            _bufferFromChardev.begin() + size);

    _logger->Debug("Serializing data and publishing on topic: " + _publishTopic);
    
    return publishBuffer;
}

void ChardevAdapter::Deserialize(const std::vector<uint8_t>& bytes)
{
    _logger->Debug("Deserializing data from topic: " + _subscribeTopic);
    SilKit::Util::SerDes::Deserializer deserializer(bytes);
    _bufferToChardev = deserializer.Deserialize<std::vector<uint8_t>>();
}

void ChardevAdapter::ReceiveEvent()
{
    async_read(*fd, asio::buffer(_eventBuffer, sizeof(inotify_event)),
    [that = shared_from_this(), this](const std::error_code ec, const std::size_t bytes_transferred){
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
                _logger->Error("Unable to handle event on " + _pathToCharDev + ". " +
                               "Error code: " + std::to_string(ec.value()) + " (" + ec.message()+ "). " +
                               "Error category: " + ec.category().name());
            }
        }
        else
        {
            if (_isRecvValue)
            {
                _isRecvValue = false;
            }
            // The file has been modified and does not come from deserialization
            else
            {
                _logger->Debug(_pathToCharDev + " has been updated");
                // Read the value only if it has to be sent
                if (!_publishTopic.empty())
                {
                    _bufferFromChardev = Util::ReadFile(_pathToCharDev, _logger, BUF_LEN);
                    Publish();
                }
            }

            ReceiveEvent();
        }
    });
}