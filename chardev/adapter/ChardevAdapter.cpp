// SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
// SPDX-License-Identifier: MIT

#include "ChardevAdapter.hpp"

#include <sys/stat.h>

#include "../../adapter/InotifyHandler.hpp"
#include "../../util/FileHelper.hpp"

#include "silkit/util/serdes/Deserializer.hpp"
#include "silkit/util/serdes/Serializer.hpp"

using namespace SilKit::Services::PubSub;
using namespace adapters;

ChardevAdapter::ChardevAdapter(SilKit::IParticipant* participant,
                               const std::string& publisherName, 
                               const std::string& subscriberName, 
                               PubSubSpec* pubDataSpec, 
                               PubSubSpec* subDataSpec,
                               const std::string& pathToCharDev,
                               asio::io_context& ioc) :
    _pathToFile(pathToCharDev),
    _isCancelled(false)    
{
    _logger = participant->GetLogger();

    if (pubDataSpec)
    {
        struct stat sb;
        if (stat(_pathToFile.c_str(), &sb) == 0 && S_ISREG(sb.st_mode))
        {
            // file exists and it's a regular file
            // use inotify to handle the events
            InotifyHandler& eHandler = InotifyHandler::GetInstance(ioc);
            eHandler.AddAdapterCallBack(this, _pathToFile);
        }
        else
        {
            int fd = open(_pathToFile.c_str(), O_RDONLY | O_NONBLOCK);
            if (fd == -1) {
                _logger->Error("Error while openning " + _pathToFile);
                throw std::runtime_error("file " + _pathToFile + " can not be opened");
            }

            _fd = std::make_unique<asio::posix::stream_descriptor>(ioc, fd);
        }

        _publishTopic = pubDataSpec->Topic();
        _publisher = participant->CreateDataPublisher(publisherName, *pubDataSpec, 1);

        // read initial data from character devices
        auto n = Util::ReadFile(_pathToFile, _logger, _bufferToPublisher);

        // publish initial value
        Publish(n);

        if (_fd)
        {
            ReceiveEvent();
        }
    }

    if (subDataSpec)
    {
        _subscribeTopic = subDataSpec->Topic();
        _subscriber = participant->CreateDataSubscriber(subscriberName, *subDataSpec,
            [&](SilKit::Services::PubSub::IDataSubscriber* subscriber, const DataMessageEvent& dataMessageEvent) {
                Deserialize(SilKit::Util::ToStdVector(dataMessageEvent.data));
                _logger->Debug("New value received on " + _subscribeTopic + ". Updating " + _pathToFile);
                _logger->Trace("Value received: " + std::string(_bufferFromSubscriber.begin(), _bufferFromSubscriber.end()));
                Util::WriteFile(_pathToFile, _bufferFromSubscriber, _logger);
            });
    }
}

ChardevAdapter::~ChardevAdapter()
{
    _isCancelled = true;
    
    if (_fd && _fd->is_open())
    {
        _logger->Trace("Cancel operations on asio stream_descriptor.");
        _fd->cancel();
        _logger->Trace("Close asio stream_descriptor.");
        _fd->close();
    }
}

void ChardevAdapter::Publish(const std::size_t n)
{
    if (_publishTopic.empty())
        return;

    // check if the file read is empty
    if(_bufferToPublisher.empty() || (n == 0))
    {
        _logger->Info(_pathToFile + " file is empty or resource is temporarily unavailable");
        return;
    }

    SilKit::Util::SerDes::Serializer serializer;
    
    serializer.BeginArray(n);
    auto publishBuffer = serializer.ReleaseBuffer();
    publishBuffer.reserve(publishBuffer.size() + n);
    publishBuffer.insert(publishBuffer.end(), _bufferToPublisher.begin(),
                        _bufferToPublisher.begin() + n);

    _logger->Debug("Serializing data and publishing on topic: " + _publishTopic);

    _publisher->Publish(publishBuffer);
}

void ChardevAdapter::Deserialize(const std::vector<uint8_t>& bytes)
{
    _logger->Debug("Deserializing data from topic: " + _subscribeTopic);
    SilKit::Util::SerDes::Deserializer deserializer(bytes);
    _bufferFromSubscriber = deserializer.Deserialize<std::vector<uint8_t>>();
}

void ChardevAdapter::ReceiveEvent()
{
    _fd->async_read_some(asio::buffer(_bufferToPublisher, _bufferToPublisher.size()),
        [this](const std::error_code ec, const std::size_t bytes_transferred){
            if (ec)
            {
                if (_isCancelled && (ec == asio::error::operation_aborted))
                {
                    // an error code comes right after calling fd.cancel() in order to close all asynchronous reads
                    _isCancelled = false;
                }
                else
                {
                    // if the error does not happened after fd.cancel(), handle it
                    _logger->Error("Unable to handle event. "
                                "Error code: " + std::to_string(ec.value()) + " (" + ec.message()+ "). " +
                                "Error category: " + ec.category().name());
                }
            }
            else
            {
                _logger->Debug(_pathToFile + " has been updated");
                Publish(bytes_transferred);

                ReceiveEvent();
            }
        });
}
