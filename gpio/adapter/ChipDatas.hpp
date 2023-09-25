// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <string>
#include <vector>

#include "silkit/services/pubsub/all.hpp"

// Handling chip datas with bits
class ChipDatas
{
public:
    ChipDatas() = default;
    ChipDatas(std::vector<std::uint8_t> initPinsValues, std::vector<std::uint8_t> initPinsIO);

    // Deserialize datas received in _pinsValue and _pinsIO
    void Deserialize(const std::vector<uint8_t>& data);
    // Serialize internal chip datas _pinsValues and _pinsIO
    auto Serialize() -> std::vector<uint8_t>;

    // Get a vector of input lines and a second one of output lines
    auto GetLinesDirection() const -> const std::pair<std::vector<std::size_t>, std::vector<std::size_t>>;
    // Get the value of each output line
    auto GetOutputLinesValues() const -> const std::vector<std::uint8_t>;
    // Get the number of lines
    auto GetDatasSize() const -> std::size_t;
    // Get the value of a given offset
    auto GetPinValue(const std::size_t offset) const -> std::uint8_t;
    // Get the direction of a given offset
    auto GetPinDirection(const std::size_t offset) const -> std::uint8_t;

    // Set the value of a given offset
    void SetPinValue(const std::size_t offset, const std::uint8_t value);
    // Set the direction of a given offset
    void SetIOValue(const std::size_t offset, const std::uint8_t value);

private:
    std::vector<std::uint8_t> _pinsValues;
    std::vector<std::uint8_t> _pinsIO;
};

