// SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
// SPDX-License-Identifier: MIT

#include "AdAdapter.hpp"

#include <unordered_map>

#include "../../adapter/InotifyHandler.hpp"
#include "../../util/FileHelper.hpp"

#include "silkit/util/serdes/Deserializer.hpp"
#include "silkit/util/serdes/Serializer.hpp"

using namespace adapters;
using namespace SilKit::Services::PubSub;

AdAdapter::AdAdapter(SilKit::IParticipant* participant, const std::string& publisherName,
                     const std::string& subscriberName, PubSubSpec* pubDataSpec, PubSubSpec* subDataSpec,
                     const std::string& pathToFile, const std::string& dataType, asio::io_context& ioc)
    : _pathToFile(pathToFile)
    , _strDataType(dataType)
{
    _logger = participant->GetLogger();

    static std::unordered_map<std::string, EnumTypes> map{
#define CREATE(typename) \
    { \
        #typename, enum_##typename \
    }
        CREATE(int8_t),  CREATE(uint8_t),  CREATE(int16_t), CREATE(uint16_t), CREATE(int32_t), CREATE(uint32_t),
        CREATE(int64_t), CREATE(uint64_t), CREATE(float),   CREATE(double)
#undef CREATE
    };

    _dataType = map[dataType];

    if (pubDataSpec)
    {
        InotifyHandler& eHandler = InotifyHandler::GetInstance(ioc);
        eHandler.AddAdapterCallBack(this, _pathToFile);

        _publishTopic = pubDataSpec->Topic();
        _publisher = participant->CreateDataPublisher(publisherName, *pubDataSpec, 1);

        // read initial data from the file
        auto n = Util::ReadFile(_pathToFile, _logger, _bufferToPublisher);

        Publish(n);
    }

    if (subDataSpec)
    {
        _subscribeTopic = subDataSpec->Topic();
        _subscriber = participant->CreateDataSubscriber(
            subscriberName, *subDataSpec,
            [&](SilKit::Services::PubSub::IDataSubscriber* /*subscriber*/, const DataMessageEvent& dataMessageEvent) {
            Deserialize(SilKit::Util::ToStdVector(dataMessageEvent.data));
            _logger->Debug("New value received on " + _subscribeTopic + ", updating " + _pathToFile);
            _logger->Trace("Value received: "
                           + std::string(_bufferFromSubscriber.begin(), _bufferFromSubscriber.end()));
            Util::WriteFile(_pathToFile, _bufferFromSubscriber, _logger);
        });
    }
}

void AdAdapter::Publish(const std::size_t n)
{
    if (_publishTopic.empty())
        return;

    // check if the file read is empty
    if (_bufferToPublisher.empty() || (n == 0))
    {
        _logger->Info(_pathToFile + " file is empty or resource is temporarily unavailable");
        return;
    }

    SilKit::Util::SerDes::Serializer serializer;
    const std::string str(_bufferToPublisher.begin(), _bufferToPublisher.begin() + n);

    try
    {
        switch (_dataType)
        {
        case enum_int8_t:
        {
            int8_t value = IsValidData<int8_t, int64_t>(str);
            serializer.Serialize(BufferFromFileTo<int16_t>(), 8);
            break;
        }
        case enum_uint8_t:
        {
            uint8_t value = IsValidData<uint8_t, uint64_t>(str);
            serializer.Serialize(BufferFromFileTo<uint16_t>(), 8);
            break;
        }
        case enum_float:
        {
            float value = IsValidData<float, float>(str);
            serializer.Serialize(value);
            break;
        }
        case enum_double:
        {
            double value = IsValidData<double, double>(str);
            serializer.Serialize(value);
            break;
        }
#define CASE(typename, width) \
    case enum_##typename: \
    { \
        if (std::is_signed<typename>::value) \
        { \
            typename value = IsValidData<typename, int64_t>(str); \
            serializer.Serialize(value, width); \
        } \
        else \
        { \
            typename value = IsValidData<typename, uint64_t>(str); \
            serializer.Serialize(value, width); \
        } \
        break; \
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

        _publisher->Publish(serializer.ReleaseBuffer());
    }
    catch (const std::out_of_range& e)
    {
        _logger->Error("Invalid value for topic " + _publishTopic + ": " + StrWithoutNewLine(str)
                       + " value is out of min or max boundaries for data type " + _strDataType);
    }
    catch (const std::invalid_argument& e)
    {
        _logger->Error("Invalid value for topic " + _publishTopic + ": '" + StrWithoutNewLine(str)
                       + "' value contains characters which are not allowed for data type " + _strDataType);
    }
    catch (const std::exception& e)
    {
        _logger->Error("Something went wrong when trying to serialize data on " + _publishTopic + ": " + e.what());
    }
}

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
#define CASE(typename, width) \
    case enum_##typename: \
        str = std::to_string(deserializer.Deserialize<typename>(width)); \
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

    _bufferFromSubscriber = std::vector<uint8_t>(str.begin(), str.end());
}

void AdAdapter::StrContainsOnly(const std::string& str, const std::string& allowedChars, bool isFloatingNumber,
                                bool isSigned)
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

    if (StrWithoutNewLine(str).find_first_not_of(allowedChars) != std::string::npos)
    {
        throw std::invalid_argument("The value contains unexpected characters");
    }
}

auto AdAdapter::StrWithoutNewLine(const std::string& str) -> std::string
{
    if (str.back() == '\n')
    {
        return str.substr(0, str.size() - 1);
    }
    return str;
}
