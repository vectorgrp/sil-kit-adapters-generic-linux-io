// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "ChipDatas.hpp"

#include "silkit/SilKit.hpp"
#include "silkit/util/serdes/Serialization.hpp"

ChipDatas::ChipDatas(const std::vector<std::uint8_t>& initLinesValues, const std::vector<std::uint8_t>& initLinesDirections) :
    _linesValues(initLinesValues),
    _linesDirections(initLinesDirections)
{
}

void ChipDatas::SpecificDeserialize(const std::vector<uint8_t>& data)
{
    SilKit::Util::SerDes::Deserializer deserializer(data);
    deserializer.BeginStruct();
    
    const auto receivedValues = deserializer.Deserialize<std::vector<std::uint8_t>>();
    _linesDirections = deserializer.Deserialize<std::vector<std::uint8_t>>();

    for (std::size_t i = 0; i < receivedValues.size(); ++i)
    {
        // If received direction is INPUT, the associated value is ignored
        if (_linesDirections[i] == 1)
            _linesValues[i] = receivedValues[i];
    }

    deserializer.EndStruct();
}

void ChipDatas::Deserialize(const std::vector<uint8_t>& data)
{
    SilKit::Util::SerDes::Deserializer deserializer(data);
    deserializer.BeginStruct();
    _linesValues =  deserializer.Deserialize<std::vector<std::uint8_t>>();
    _linesDirections = deserializer.Deserialize<std::vector<std::uint8_t>>();
    deserializer.EndStruct();
}

auto ChipDatas::Serialize() -> std::vector<uint8_t>
{
    SilKit::Util::SerDes::Serializer serializer;
    serializer.BeginStruct();
    serializer.Serialize(_linesValues);
    serializer.Serialize(_linesDirections);
    serializer.EndStruct();

    return serializer.ReleaseBuffer();
}

auto ChipDatas::GetLinesDirection() const -> const std::pair<std::vector<std::size_t>, std::vector<std::size_t>>
{
    std::vector<std::size_t> offsetsIN, offsetsOUT;
    for (std::size_t i = 0; i < GetDatasSize(); ++i)
    {
        // Assume that input == 0 / output == 1
        _linesDirections[i] == 0 ? offsetsIN.push_back(i) : offsetsOUT.push_back(i);
    }

    return std::make_pair(offsetsIN, offsetsOUT);
}

auto ChipDatas::GetOutputLinesValues() const -> const std::vector<std::uint8_t>
{
    std::vector<std::uint8_t> values;
    const auto offsetsOUT = GetLinesDirection().second;
    for (const auto offset : offsetsOUT) 
    {
        values.push_back(_linesValues[offset] == 0 ? 0 : 1);
    }

    return values;
}

auto ChipDatas::GetDatasSize() const -> std::size_t
{
    return _linesValues.size();
}

auto ChipDatas::GetLineValue(const std::size_t offset) const -> std::uint8_t
{
    return _linesValues[offset];
}

auto ChipDatas::GetLineDirection(const std::size_t offset) const -> std::uint8_t
{
    return _linesDirections[offset];
}

void ChipDatas::SetLineValue(const std::size_t offset, const std::uint8_t value)
{
    _linesValues[offset] = value;
}

void ChipDatas::SetLineDirection(const std::size_t offset, const std::uint8_t value)
{
    _linesDirections[offset] = value;
}

void ChipDatas::SetLinesValues(const std::vector<std::uint8_t>& values)
{
    _linesValues = values;
}

void ChipDatas::SetLinesDirections(const std::vector<std::uint8_t>& directions)
{
    _linesDirections = directions;
}