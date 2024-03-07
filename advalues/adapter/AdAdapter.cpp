// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "AdAdapter.hpp"

#include <unordered_map>

#include "silkit/util/serdes/Deserializer.hpp"
#include "silkit/util/serdes/Serializer.hpp"

AdAdapter::AdAdapter(SilKit::IParticipant* participant,
                     const std::string& publisherName,
                     const std::string& subscriberName,
                     std::unique_ptr<PubSubSpec> pubDataSpec,
                     std::unique_ptr<PubSubSpec> subDataSpec,
                     const std::string& pathToCharDev,
                     asio::io_context* ioc,
                     const std::string& dataType) :
    ChardevAdapter(participant, publisherName, subscriberName, std::move(pubDataSpec), std::move(subDataSpec), pathToCharDev, ioc)
{
    static std::unordered_map<std::string, EnumTypes> map {
#define CREATE(typename) {#typename, enum_##typename}
        CREATE(int8_t),
        CREATE(uint8_t),
        CREATE(int16_t),
        CREATE(uint16_t),
        CREATE(int32_t),
        CREATE(uint32_t),
        CREATE(int64_t),
        CREATE(uint64_t),
        CREATE(float),
        CREATE(double)
#undef CREATE
    };

    _dataType = map[dataType];
}

// Serialize chip values
auto AdAdapter::Serialize() -> std::vector<uint8_t>
{
    SilKit::Util::SerDes::Serializer serializer;
    switch (_dataType)
    {
    case enum_int8_t:
        serializer.Serialize(bufferFromChardevTo<int16_t>(), 8);
        break;
    case enum_uint8_t:
        serializer.Serialize(bufferFromChardevTo<uint16_t>(), 8);
        break;
    case enum_float:
        serializer.Serialize(bufferFromChardevTo<float>());
        break;
    case enum_double:
        serializer.Serialize(bufferFromChardevTo<double>());
        break;
#define CASE(typename,width) \
    case enum_##typename:\
        serializer.Serialize(bufferFromChardevTo<typename>(), width);\
        break;
    CASE(int16_t, 16);
    CASE(uint16_t, 16);
    CASE(int32_t, 32);
    CASE(uint32_t, 32);
    CASE(int64_t, 64);
    CASE(uint64_t, 64);
#undef CASE
    default:
        break;
    }

    return serializer.ReleaseBuffer();
}

// Deserialize received values 
void AdAdapter::Deserialize(const std::vector<uint8_t>& bytes)
{
    SilKit::Util::SerDes::Deserializer deserializer(bytes);

    std::string str;

    switch (_dataType)
    {
    case enum_uint8_t:
    {
        std::stringstream ss;
        ss << static_cast<int>(deserializer.Deserialize<int16_t>(8));
        str = ss.str();
        break;
    }
    case enum_int8_t:
    {
        std::stringstream ss;
        ss << static_cast<int>(deserializer.Deserialize<uint16_t>(8));
        str = ss.str();
        break;
    }
    case enum_uint16_t:
    {
        std::stringstream ss;
        ss << static_cast<int>(deserializer.Deserialize<uint16_t>(16));
        str = ss.str();
        break;
    }
    case enum_float:
        str = std::to_string(deserializer.Deserialize<float>());
        break;
    case enum_double:
        str = std::to_string(deserializer.Deserialize<double>());
        break;
#define CASE(typename, width)\
    case enum_##typename:\
        str = std::to_string(deserializer.Deserialize<typename>(width));\
        break;
    CASE(int16_t, 16);
    CASE(int32_t, 32);
    CASE(uint32_t, 32);
    CASE(int64_t, 64);
    CASE(uint64_t, 64);
#undef CASE
    default:
        break;
    }

    if (!str.empty()) _bufferToChardev = std::vector<uint8_t>(str.begin(), str.end());
}