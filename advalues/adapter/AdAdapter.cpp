// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "AdAdapter.hpp"

#include <unordered_map>

#include "silkit/util/serdes/Deserializer.hpp"
#include "silkit/util/serdes/Serializer.hpp"

AdAdapter::AdAdapter(SilKit::IParticipant* participant,
                     const std::string& publisherName,
                     const std::string& subscriberName,
                     PubSubSpec* pubDataSpec,
                     PubSubSpec* subDataSpec,
                     const std::string& pathToCharDev,
                     const std::string& dataType,
                     int inotifyFd) :
    ChardevAdapter(participant, publisherName, subscriberName, std::move(pubDataSpec), std::move(subDataSpec), pathToCharDev, inotifyFd),
    _strDataType(dataType)
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
    // Check if the file read is empty
    if(isBufferFromChardevEmpty())
    {
        _logger->Info(_pathToCharDev + " file is empty or resource is temporarily unavailable");
        return std::vector<uint8_t>{};
    }

    SilKit::Util::SerDes::Serializer serializer;
    const std::string str(_bufferFromChardev.begin(), _bufferFromChardev.end());

    try
    {
        switch (_dataType)
        {
        case enum_int8_t:
        {
            int8_t value = isValidData<int8_t, int64_t>(str);
            serializer.Serialize(bufferFromChardevTo<int16_t>(), 8);
            break;
        }
        case enum_uint8_t:
        {
            uint8_t value = isValidData<uint8_t, uint64_t>(str);
            serializer.Serialize(bufferFromChardevTo<uint16_t>(), 8);
            break;
        }
        case enum_float:
        {
            float value = isValidData<float, float>(str);
            serializer.Serialize(value);
            break;
        }
        case enum_double:
        {
            double value = isValidData<double, double>(str);
            serializer.Serialize(value);
            break;
        }
#define CASE(typename,width) \
        case enum_##typename:\
        {\
            if (std::is_signed_v<typename>) {\
                typename value = isValidData<typename, int64_t>(str);\
                serializer.Serialize(value, width);\
            } else {\
                typename value = isValidData<typename, uint64_t>(str);\
                serializer.Serialize(value, width);\
            }\
            break;\
        }
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

        _logger->Debug("Serializing data and publishing on topic: " + _publishTopic);

        return serializer.ReleaseBuffer();
    }
    catch (const std::out_of_range& e)
    {
        _logger->Error("Invalid value for topic " + _publishTopic + ": " + strWithoutNewLine(str) + " value is out of min or max boundaries for data type " + _strDataType);
    }
    catch (const std::invalid_argument& e)
    {
        _logger->Error("Invalid value for topic " + _publishTopic + ": '" + strWithoutNewLine(str) + "' value contains characters which are not allowed for data type " + _strDataType);
    }
    catch (const std::exception& e)
    {
        _logger->Error("Something went wrong when trying to serialize data on " + _publishTopic + ": " + e.what());
    }

    return std::vector<uint8_t>{};
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
        ss << static_cast<int>(deserializer.Deserialize<uint16_t>(8));
        str = ss.str();
        break;
    }
    case enum_int8_t:
    {
        std::stringstream ss;
        ss << static_cast<int>(deserializer.Deserialize<int16_t>(8));
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

void AdAdapter::strContainsOnly(const std::string& str, const std::string& allowedChars, bool isFloatingNumber, bool isSigned)
{
    // if isFloatingNumber, allowedChars contains '.', verify if there is one at maximum
    if (isFloatingNumber)
    {
        if (std::count(str.begin(), str.end(), '.') > 1)
        {
            throw std::invalid_argument("The value contains more than one '.' character");
        }
    }

    // if isSigned, allowedChars contains '-', verify if there is one at maximum
    if (isSigned)
    {
        if (std::count(str.begin(), str.end(), '-') > 1)
        {
            throw std::invalid_argument("The value contains more than one '-' character");
        }
    }

    if (strWithoutNewLine(str).find_first_not_of(allowedChars) != std::string::npos)
    {
        throw std::invalid_argument("The value contains unexpected characters");
    }
}

auto AdAdapter::strWithoutNewLine(const std::string& str) -> std::string
{
    if (str.back() == '\n')
    {
        return str.substr(0, str.size() - 1);
    }
    return str;
}
