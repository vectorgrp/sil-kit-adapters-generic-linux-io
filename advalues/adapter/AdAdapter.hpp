// SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
// SPDX-License-Identifier: MIT

#pragma once

#include <sstream>
#include <stdexcept>
#include <limits>
#include <array>

#include "../../chardev/adapter/ChardevAdapter.hpp"

#include "silkit/SilKit.hpp"
#include "silkit/services/pubsub/all.hpp"

#include "asio/io_context.hpp"

// each file has a specific AdAdapter
class AdAdapter : public IOAdapter
{
using PubSubSpec = SilKit::Services::PubSub::PubSubSpec;

enum EnumTypes {enum_int8_t, enum_uint8_t, enum_int16_t, enum_uint16_t, 
    enum_int32_t, enum_uint32_t, enum_int64_t, enum_uint64_t, enum_float, enum_double};

public:
    // access and manage the file
    std::string _pathToFile;
    std::array<uint8_t, 4096> _bufferToPublisher = {};

    AdAdapter() = delete;
    AdAdapter(SilKit::IParticipant* participant,
              const std::string& publisherName,
              const std::string& subscriberName,
              PubSubSpec* pubDataSpec,
              PubSubSpec* subDataSpec,
              const std::string& pathToFile,
              const std::string& dataType,
              asio::io_context& ioc);

    // serialize chip values
    void Publish(const std::size_t n);

private:
    EnumTypes _dataType;
    std::string _strDataType;

    // access and manage the file
    std::vector<uint8_t> _bufferFromSubscriber = {};
    
    // deserialize received values 
    void Deserialize(const std::vector<uint8_t>& bytes) override;

    // converting and checking values
    template<typename T>
    inline auto BufferFromFileTo() -> T;

    template<typename T, typename U>
    inline void ThrowIfInvalid(const T max, const T lowest, const U value);
    
    template<typename T, typename U>
    inline auto IsValidData(const std::string& str) -> T;

    void StrContainsOnly(const std::string& str, const std::string& allowedChars, bool isFloatingNumber = false, bool isSigned = false);
    auto StrWithoutNewLine(const std::string& str) -> std::string;
};

////////////////////////////
// Inline implementations //
////////////////////////////

template<typename T>
auto AdAdapter::BufferFromFileTo() -> T
{
    std::string str(_bufferToPublisher.begin(), _bufferToPublisher.end());
    std::stringstream val(str);
    T out;
    val >> out;
    return out;
}

template<typename T, typename U>
void AdAdapter::ThrowIfInvalid(const T max, const T lowest, const U value)
{
    if (value < static_cast<U>(lowest) || value > static_cast<U>(max))
    {
        throw std::out_of_range("value < lowest || value > max");
    }
}

template<typename T, typename U>
auto AdAdapter::IsValidData(const std::string& str) -> T
{
    static const std::string strNum{"0123456789"};

    // Check hexadecimal value
    bool isHexa = false;
    if (str.substr(0,2) == "0x")
    {
        isHexa = true;
        std::string hex = str.substr(2, str.size());
        StrContainsOnly(hex, strNum + "ABCDEFabcdef");
    }

    T max = std::numeric_limits<T>::max();
    T lowest = std::numeric_limits<T>::lowest();

    U value;
    memset(&value, 0, sizeof(value));

    if (_dataType == enum_float)
    {
        if (!isHexa) StrContainsOnly(str, strNum + ".-", true, true);
        value = std::stof(str);
        return value;
    }
    else if (_dataType == enum_double)
    {
        if (!isHexa) StrContainsOnly(str, strNum + ".-", true, true);
        value = std::stod(str);
        return value;
    }
    else if (std::is_signed<T>::value)
    {
        if (isHexa)
        {
            value = std::stoll(str, nullptr, 0);
        }
        else
        {
            StrContainsOnly(str, strNum + "-", false, true);
            value = std::stoll(str);
        }
        ThrowIfInvalid(max, lowest, value);
        return static_cast<T>(value);
    }
    else if (std::is_unsigned<T>::value)
    {
        if (isHexa)
        {
            value = std::stoull(str, nullptr, 0);
        }
        else
        {
            StrContainsOnly(str, strNum);
            value = std::stoull(str);
        }
        ThrowIfInvalid(max, lowest, value);
        return static_cast<T>(value);
    }
}
