// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include "../../chardev/adapter/ChardevAdapter.hpp"

#include <sstream>
#include <stdexcept>
#include <limits>

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
    std::string _strDataType;

    template<typename T>
    inline T bufferFromChardevTo();

    template<typename T, typename U>
    inline void throwIfInvalid(const T max, const T lowest, const U value);
    
    template<typename T, typename U>
    inline T isValidData(const std::string& str);

    void strContainsOnly(const std::string& str, const std::string& allowedChars, bool isFloatingNumber = false, bool isSigned = false);
    auto strWithoutNewLine(const std::string& str) -> std::string;
};

// Inline implementations
template<typename T>
T AdAdapter::bufferFromChardevTo()
{
    std::string str(_bufferFromChardev.begin(), _bufferFromChardev.end());
    std::stringstream val(str);
    T out;
    val >> out;
    return out;
}

template<typename T, typename U>
void AdAdapter::throwIfInvalid(const T max, const T lowest, const U value)
{
    if (value < static_cast<U>(lowest) || value > static_cast<U>(max))
    {
        throw std::out_of_range("value < lowest || value > max");
    }
}

template<typename T, typename U>
T AdAdapter::isValidData(const std::string& str)
{
    static const std::string strNum{"0123456789"};

    // Check hexadecimal value
    bool isHexa = false;
    if (str.substr(0,2) == "0x")
    {
        isHexa = true;
        std::string hex = str.substr(2, str.size());
        strContainsOnly(hex, strNum + "ABCDEFabcdef");
    }

    T max = std::numeric_limits<T>::max();
    T lowest = std::numeric_limits<T>::lowest();

    U value;
    memset(&value, 0, sizeof(value));

    if (_dataType == enum_float)
    {
        if (!isHexa) strContainsOnly(str, strNum + ".-", true, true);
        value = std::stof(str);
        return value;
    }
    else if (_dataType == enum_double)
    {
        if (!isHexa) strContainsOnly(str, strNum + ".-", true, true);
        value = std::stod(str);
        return value;
    }
    else if (std::is_signed_v<T>)
    {
        if (!isHexa) strContainsOnly(str, strNum + "-", false, true);
        value = std::stoll(str, nullptr, 0);
        throwIfInvalid(max, lowest, value);
        return static_cast<T>(value);
    }
    else if (std::is_unsigned_v<T>)
    {
        if (!isHexa) strContainsOnly(str, strNum);
        value = std::stoull(str, nullptr, 0);
        throwIfInvalid(max, lowest, value);
        return static_cast<T>(value);
    }
}