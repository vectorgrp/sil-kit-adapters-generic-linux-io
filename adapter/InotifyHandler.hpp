// SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
// SPDX-License-Identifier: MIT

#pragma once

#include <vector>
#include <array>
#include <sys/inotify.h>

#include "advalues/adapter/AdAdapter.hpp"
#include "util/Exceptions.hpp"
#include "util/FileHelper.hpp"

#include "silkit/services/logging/all.hpp"
#include "asio/posix/stream_descriptor.hpp"

// class to handle events on files using inotify
class InotifyHandler
{
public:
    // delete copy constructor and assignment operator
    InotifyHandler(const InotifyHandler&) = delete;
    InotifyHandler& operator=(const InotifyHandler&) = delete;

    template<typename T>
    inline void AddAdapterCallBack(T* adapter, const std::string& path)
    {
        const auto wd = inotify_add_watch(_inotifyFd, path.c_str(), IN_CLOSE_WRITE);
        if (wd == -1) {
            throw adapters::InotifyError("inotify add watch error (" + std::to_string(errno) +") on: " + path);
        }
        _logger->Trace("New watcher (wd: " + std::to_string(wd) + ") added on " + path);

        _callbacks.push_back([this, adapter](){
            _logger->Debug(adapter->_pathToFile + " has been updated");
            auto n = adapters::Util::ReadFile(adapter->_pathToFile, _logger, adapter->_bufferToPublisher);
            adapter->Publish(n);
        });
    }

    // stop and close the stream_descriptor 
    static void Stop();
    static void SetLogger(SilKit::Services::Logging::ILogger* logger) { _logger = logger; }

    static auto GetInstance(asio::io_context& ioc) -> InotifyHandler& {
        if ( _instance == nullptr ) {
            _instance = new InotifyHandler();
            _instance->InitInotify(ioc);
            ReceiveEvent();
        }
        return *_instance;
    }

private:
    static InotifyHandler* _instance;
    static SilKit::Services::Logging::ILogger* _logger;

    // functions that will be called when receiving an event
    std::vector<std::function<void()>> _callbacks;

    // handle error code introduced by fd.cancel()
    bool _isCancelled;

    int _inotifyFd;
    std::unique_ptr<asio::posix::stream_descriptor> _fd;
    // eventBuffer contains a inotify event which has an optional name attribute unused for now
    // the buffer should be increased if name is used
    std::array<uint8_t, 1000> _eventBuffer = {};
    // handle potential uncomplete event read with async_read_some
    std::size_t remainingBytes = 0;

    InotifyHandler();
    ~InotifyHandler();

    void InitInotify(asio::io_context& ioc);
    static void ReceiveEvent();
};
