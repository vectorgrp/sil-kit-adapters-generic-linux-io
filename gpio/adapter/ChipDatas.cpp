// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "ChipDatas.hpp"

#include "silkit/SilKit.hpp"
#include "silkit/util/serdes/Serialization.hpp"

ChipDatas::ChipDatas(std::vector<std::uint8_t> initPinsValues, std::vector<std::uint8_t> initPinsIO) :
    _pinsValues(initPinsValues),
    _pinsIO(initPinsIO)
{
}

void ChipDatas::Deserialize(const std::vector<uint8_t>& data)
{
    SilKit::Util::SerDes::Deserializer deserializer(data);
    deserializer.BeginStruct();
    _pinsValues = deserializer.Deserialize<std::vector<std::uint8_t>>();
    _pinsIO = deserializer.Deserialize<std::vector<std::uint8_t>>();
    deserializer.EndStruct();
}

auto ChipDatas::Serialize() -> std::vector<uint8_t>
{
    SilKit::Util::SerDes::Serializer serializer;
    serializer.BeginStruct();
    serializer.Serialize(_pinsValues);
    serializer.Serialize(_pinsIO);
    serializer.EndStruct();

    return serializer.ReleaseBuffer();
}

auto ChipDatas::GetLinesDirection() const -> const std::pair<std::vector<std::uint8_t>, std::vector<std::uint8_t>>
{
    std::vector<std::uint8_t> offsetsIN, offsetsOUT;
    for (std::size_t i = 0; i < _pinsIO.size(); ++i) 
    {
        // Assume that input == 0 / output == 1
        _pinsIO[i] == 0 ? offsetsIN.push_back(i) : offsetsOUT.push_back(i);
    }

    return std::make_pair(offsetsIN, offsetsOUT);
}

auto ChipDatas::GetOutputLinesValues() const -> const std::vector<std::uint8_t>
{
    std::vector<std::uint8_t> values;
    const auto offsetsOUT = GetLinesDirection().second;
    for (const auto offset : offsetsOUT) 
    {
        values.push_back(_pinsValues[offset] == 0 ? 0 : 1);
    }

    return values;
}

auto ChipDatas::GetDatasSize() const -> const std::size_t
{
    return _pinsValues.size();
}

auto ChipDatas::GetPinValue(const int offset) const -> const std::uint8_t
{
    return _pinsValues[offset];
}

auto ChipDatas::GetPinDirection(const int offset) const -> const std::uint8_t
{
    return _pinsIO[offset];
}

void ChipDatas::SetPinValue(const int offset, const std::uint8_t value)
{
    _pinsValues[offset] = value;
}

void ChipDatas::SetIOValue(const int offset, const std::uint8_t value)
{
    _pinsIO[offset] = value;
}