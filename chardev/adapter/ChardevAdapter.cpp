// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "ChardevAdapter.hpp"

#include "../../util/FileHelper.hpp"
#include "../../util/Exceptions.hpp"

#include "silkit/util/serdes/Deserializer.hpp"
#include "silkit/util/serdes/Serializer.hpp"

#include "asio/read.hpp"

using namespace SilKit::Services::PubSub;
using namespace adapters;

ChardevAdapter::ChardevAdapter(SilKit::IParticipant* participant,
                               const std::string& publisherName, 
                               const std::string& subscriberName, 
                               PubSubSpec* pubDataSpec, 
                               PubSubSpec* subDataSpec,
                               const std::string& pathToCharDev,
                               int inotifyFd) :
    _pathToCharDev(pathToCharDev),
    _isRecvValue(false)
{
    _logger = participant->GetLogger();
    
    _wd = inotify_add_watch(inotifyFd, _pathToCharDev.c_str(), IN_CLOSE_WRITE);
    if (_wd == -1) {
        throw InotifyError("inotify add watch error (" + std::to_string(errno) +") on: " + _pathToCharDev);
    }

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

void ChardevAdapter::Publish()
{
    if (!_publishTopic.empty())
    {
        if (const auto& bytes = Serialize(); !bytes.empty())
        {
            _publisher->Publish(bytes);
        }
    }
}

auto ChardevAdapter::Serialize() -> std::vector<uint8_t>
{    
    // Check if the file read is empty
    if(isBufferFromChardevEmpty())
    {
        _logger->Info(_pathToCharDev + " file is empty or resource is temporarily unavailable");
        return std::vector<uint8_t>{};
    }

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

auto ChardevAdapter::isBufferFromChardevEmpty() const -> bool
{
    const std::string str(_bufferFromChardev.begin(), _bufferFromChardev.end());

    // Check if the file read is empty
    if ((str == "") || (str[0] == '\n'))
    {
        return true;
    }
    return false;
}
