// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "../../chardev/adapter/ChardevAdapter.hpp"

#include <sstream>

#include "silkit/SilKit.hpp"
#include "silkit/services/pubsub/all.hpp"

#include <asio/posix/stream_descriptor.hpp>

// Each file has a specific AdAdapter
class AdAdapter : public ChardevAdapter
{
using PubSubSpec = SilKit::Services::PubSub::PubSubSpec;

enum EnumTypes {enum_int8_t, enum_uint8_t, enum_int16_t, enum_uint16_t, 
    enum_int32_t, enum_uint32_t, enum_int64_t, enum_uint64_t, enum_float, enum_double};

public:
    AdAdapter() = delete;
    AdAdapter(SilKit::IParticipant* participant,
              const std::string& publisherName,
              const std::string& subscriberName,
              std::unique_ptr<PubSubSpec> pubDataSpec,
              std::unique_ptr<PubSubSpec> subDataSpec,
              const std::string& pathToCharDev,
              asio::io_context* ioc,
              const std::string& dataType);

    // Serialize chip values
    auto Serialize() -> std::vector<uint8_t> override;
    // Deserialize received values 
    void Deserialize(const std::vector<uint8_t>& bytes) override;

private:
    EnumTypes _dataType;

    template<typename T>
    inline T bufferFromChardevTo()
    {
        std::string str(_bufferFromChardev.begin(), _bufferFromChardev.end());
        std::stringstream val(str);
        T out;
        val >> out;
        return out;
    }
};