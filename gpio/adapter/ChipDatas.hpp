// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <string>
#include <vector>

#include "silkit/services/pubsub/all.hpp"

class ChipDatas
{
public:
    ChipDatas() = default;
    ChipDatas(std::vector<std::uint8_t> initPinsValues, std::vector<std::uint8_t> initPinsIO);

    void Deserialize(const std::vector<uint8_t>& data);
    auto Serialize() -> std::vector<uint8_t>;

    auto GetLinesDirection() const -> const std::pair<std::vector<std::uint8_t>, std::vector<std::uint8_t>>;
    auto GetOutputLinesValues() const -> const std::vector<std::uint8_t>;
    auto GetDatasSize() const -> const  std::size_t;
    auto GetPinValue(const int offset) const -> const std::uint8_t;
    auto GetPinDirection(const int offset) const -> const std::uint8_t;

    void SetPinValue(const int offset, const std::uint8_t value);
    void SetIOValue(const int offset, const std::uint8_t value);

private:
    std::vector<std::uint8_t> _pinsValues;
    std::vector<std::uint8_t> _pinsIO;
};

