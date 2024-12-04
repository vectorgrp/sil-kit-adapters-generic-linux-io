// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "InotifyHandler.hpp"

using namespace adapters;

InotifyHandler* InotifyHandler::_instance = nullptr;
SilKit::Services::Logging::ILogger* InotifyHandler::_logger = nullptr;

InotifyHandler::InotifyHandler()
{
    // add a default first element in the callbacks vector because the watcher index starts from
    // 1. This avoid doing an operation to get the right callback to call in ReceiveEvent().
    _callbacks.push_back([](){});
}

InotifyHandler::~InotifyHandler()
{
    Stop();
}

void InotifyHandler::Stop()
{
    if (!_instance)
        return;

    _instance->_isCancelled = true;
    
    if (_instance->_fd && _instance->_fd->is_open())
    {
        _instance->_logger->Debug("Cancel operations on asio stream_descriptor.");
        _instance->_fd->cancel();
        _instance->_logger->Debug("Close asio stream_descriptor.");
        _instance->_fd->close();
    }
}

void InotifyHandler::InitInotify(asio::io_context& ioc)
{

#ifdef QNX_BUILD
    _inotifyFd = inotify_init();
#else
    _inotifyFd = inotify_init1( IN_NONBLOCK );
#endif

    if (_inotifyFd == -1) {
        throw InotifyError("inotify initialization error (" + std::to_string(errno) +")");
    }

#ifdef QNX_BUILD
    // set the file descriptor to non-blocking mode
    int flags = fcntl(_inotifyFd, F_GETFL);
    if (flags == -1) {
        close(_inotifyFd);
        throw InotifyError("inotify initialization error while getting inotify fd flag (" + std::to_string(errno) +")");
    }

    flags |= O_NONBLOCK;
    if (fcntl(_inotifyFd, F_SETFL, flags) == -1) {
        close(_inotifyFd);
        throw InotifyError("inotify initialization error while setting the IN_NONBLOCK flag (" + std::to_string(errno) +")");
    }
#endif

    _fd = std::make_unique<asio::posix::stream_descriptor>(ioc, _inotifyFd);
}

void InotifyHandler::ReceiveEvent()
{
    _instance->_fd->async_read_some(asio::buffer(_instance->_eventBuffer.data() + _instance->remainingBytes, _instance->_eventBuffer.size() - _instance->remainingBytes),
    [](const std::error_code ec, const std::size_t bytes_transferred){
        if (ec)
        {
            if (_instance->_isCancelled && (ec == asio::error::operation_aborted))
            {
                // an error code comes right after calling fd.cancel() in order to close all asynchronous reads
                 _instance->_isCancelled = false;
            }
            else
            {
                // if the error does not happened after fd.cancel(), handle it
                 _instance->_logger->Error("Unable to handle event. "
                               "Error code: " + std::to_string(ec.value()) + " (" + ec.message()+ "). " +
                               "Error category: " + ec.category().name());
            }
        }
        else
        {
            // if there is not enough bytes while reading
            if (_instance->remainingBytes + bytes_transferred < sizeof(struct inotify_event))
            {
                _instance->remainingBytes += bytes_transferred;
                ReceiveEvent();
            }
            
            std::size_t processedBytes = 0;
            while (processedBytes + sizeof(struct inotify_event) <= bytes_transferred)
            {
                auto event = reinterpret_cast<const struct inotify_event *>( _instance->_eventBuffer.data() + processedBytes);

                _instance->_callbacks[event->wd]();

                processedBytes += sizeof(inotify_event);
            }

            // if there are remaining bytes
            if (processedBytes != bytes_transferred)
            {
                _instance->remainingBytes = bytes_transferred - processedBytes;
                std::memmove(_instance->_eventBuffer.data(), _instance->_eventBuffer.data() + bytes_transferred - _instance->remainingBytes, _instance->remainingBytes);
            }
            else
            {
                _instance->remainingBytes = 0;
            }

            ReceiveEvent();
        }
    });
}
