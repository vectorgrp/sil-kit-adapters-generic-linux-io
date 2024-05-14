// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <vector>
#include <string>
#include <memory>
#include <sys/inotify.h>

#include "../../adapter/IOAdapter.hpp"

#include "silkit/SilKit.hpp"
#include "silkit/services/pubsub/all.hpp"

#include "asio/posix/stream_descriptor.hpp"

// Each file has a specific ChardevAdapter
class ChardevAdapter : public IOAdapter
{
using PubSubSpec = SilKit::Services::PubSub::PubSubSpec;

public:
    friend class ChardevManager;

    ChardevAdapter() = delete;
    ChardevAdapter(SilKit::IParticipant* participant, 
                   const std::string& publisherName, 
                   const std::string& subscriberName, 
                   PubSubSpec* pubDataSpec, 
                   PubSubSpec* subDataSpec,
                   const std::string& pathToCharDev,
                   int inotifyFd);
    ~ChardevAdapter() = default;

protected:
    static constexpr uint32_t BUF_LEN = 4096;
    static constexpr uint32_t EVENT_SIZE = sizeof(inotify_event);

    // Access and manage the chardev
    std::vector<uint8_t> _bufferToChardev = {};
    std::vector<uint8_t> _bufferFromChardev = {};
    std::string _pathToCharDev;

    // Avoid to read several times the same received value
    bool _isRecvValue;

    // Inotify watch descriptor
    int _wd;

    auto isBufferFromChardevEmpty() const -> bool;

private:
    // Publish chardev values
    void Publish() override;

    // Serialize chip values
    auto Serialize() -> std::vector<uint8_t> override;
    // Deserialize received values 
    void Deserialize(const std::vector<uint8_t>& bytes) override;
};