// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <utility>
#include <atomic>

#include "SilKitDemoGPIODevice.hpp"
#include "silkit/SilKit.hpp"
#include "silkit/config/all.hpp"
#include "silkit/services/pubsub/all.hpp"
#include "silkit/util/serdes/Serialization.hpp"

struct GpioChip {
    std::vector<std::uint8_t> pinsValues;
    std::vector<std::uint8_t> pinsIO;
};

std::vector<std::uint8_t> serialize(const GpioChip& pinsToSerialize)
{
    SilKit::Util::SerDes::Serializer serializer;
    serializer.BeginStruct();
    serializer.Serialize(pinsToSerialize.pinsValues);
    serializer.Serialize(pinsToSerialize.pinsIO);
    serializer.EndStruct();

    return serializer.ReleaseBuffer();
}

GpioChip deserialize(const std::vector<uint8_t>& data)
{
    GpioChip deserializedPins;

    SilKit::Util::SerDes::Deserializer deserializer(data);
    deserializer.BeginStruct();
    deserializedPins.pinsValues = deserializer.Deserialize<std::vector<std::uint8_t>>();
    deserializedPins.pinsIO = deserializer.Deserialize<std::vector<std::uint8_t>>();
    deserializer.EndStruct();

    return deserializedPins;
}