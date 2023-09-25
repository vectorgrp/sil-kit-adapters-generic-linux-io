// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <string>
#include <vector>
#include <cstdint>

#include "gpiod.hpp"

namespace adapters 
{
    enum ReturnCode
    {
        NO_ERROR = 0,
        CLI_ERROR,
        CONFIGURATION_ERROR,
        OTHER_ERROR
    };
}

void promptForExit()
{
    std::cout << "Press enter to stop the process..." << std::endl;
    std::cin.ignore();
}

auto ConvertGpioValueToBit(const ::gpiod::line::value value) -> const std::uint8_t
{
    return (value == gpiod::line::value::ACTIVE ? 1 : 0);
};

auto ConvertGpioDirectionToBit(const ::gpiod::line::direction dir) -> const std::uint8_t
{
    return (dir == ::gpiod::line::direction::OUTPUT ? 1 : 0);
};

auto ConvertGpiodValuesToBits(const ::gpiod::line::values& values) -> const std::vector<std::uint8_t>
{
    std::vector<std::uint8_t> bits;
    for (auto val : values)
        bits.push_back(ConvertGpioValueToBit(val));

    return bits;
};

auto ConvertGpiodDirectionsToBits(const std::vector<::gpiod::line::direction>& directions) -> const std::vector<std::uint8_t>
{
    std::vector<std::uint8_t> bits;
    for (auto dir : directions)
        bits.push_back(ConvertGpioDirectionToBit(dir));

    return bits;
};
