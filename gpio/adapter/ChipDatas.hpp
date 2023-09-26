// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <string>
#include <vector>

#include "silkit/services/pubsub/all.hpp"

/* Handling chip datas with bits
* A line in a chip is represented as an offset
*/ 
class ChipDatas
{
public:
    ChipDatas() = default;
    ChipDatas(const std::vector<std::uint8_t>& initLinesValues, const std::vector<std::uint8_t>& initLinesDirections);

    // Deserialize all received datas in _linesValues and _linesDirections
    void Deserialize(const std::vector<uint8_t>& data);
    // Deserialize received datas in _linesValues and _linesDirections without
    // Taking into account the value when direction is INPUT
    void SpecificDeserialize(const std::vector<uint8_t>& data);
    // Serialize internal chip datas _linesValues and _linesDirections
    auto Serialize() -> std::vector<uint8_t>;

    // Get a vector of input lines and a second one of output lines
    auto GetLinesDirection() const -> const std::pair<std::vector<std::size_t>, std::vector<std::size_t>>;
    // Get the value of each output line
    auto GetOutputLinesValues() const -> const std::vector<std::uint8_t>;
    // Get the number of lines
    auto GetDatasSize() const -> std::size_t;
    // Get the value of a given offset
    auto GetLineValue(const std::size_t offset) const -> std::uint8_t;
    // Get the direction of a given offset
    auto GetLineDirection(const std::size_t offset) const -> std::uint8_t;

    // Set the value of a given offset
    void SetLineValue(const std::size_t offset, const std::uint8_t value);
    // Set the direction of a given offset
    void SetLineDirection(const std::size_t offset, const std::uint8_t value);
    // Set all lines values
    void SetLinesValues(const std::vector<std::uint8_t>& values);
    // Set all lines directions
    void SetLinesDirections(const std::vector<std::uint8_t>& directions);

private:
    std::vector<std::uint8_t> _linesValues;
    std::vector<std::uint8_t> _linesDirections;
};

