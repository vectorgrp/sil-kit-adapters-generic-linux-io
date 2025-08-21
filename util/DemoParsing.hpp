// SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
// SPDX-License-Identifier: MIT

#pragma once

#include <iostream>
#include <array>

#include "common/Cli.hpp"
#include "common/Parsing.hpp"

namespace adapters {
const std::array<const std::string, 3> demoSwitchesWithArgument = {participantNameArg, regUriArg, logLevelArg};
const std::array<const std::string, 1> demoSwitchesWithoutArgument = {helpArg};

inline void PrintDemoHelp(const std::string& mode, bool userRequested = false)
{
    std::cout << "Usage (defaults in curly braces if you omit the switch):\n"
                 "sil-kit-demo-glio-"
              << std::tolower(mode[0], std::locale()) << mode.substr(1, mode.size()) << "-forward-device ["
              << adapters::participantNameArg << " <participant's name{" << mode
              << "ForwardDevice}>]\n"
                 "  ["
              << adapters::regUriArg
              << " silkit://<host{localhost}>:<port{8501}>]\n"
                 "  ["
              << adapters::logLevelArg << " <Trace|Debug|Warn|{Info}|Error|Critical|Off>]\n";

    if (!userRequested)
    {
        std::cout << '\n';
        std::cout << "Pass " << adapters::helpArg << " to get this message" << '\n';
    }
}

template <class elementType>
bool ThereAreUnknownArgumentsDemo(int argc, char** argv, std::initializer_list<elementType>&& switchesWithArgument,
                                  std::initializer_list<elementType>&& switchesWithoutArguments,
                                  const std::string& mode)
{
    if (util::thereAreUnknownArguments(argc, argv, switchesWithArgument, switchesWithoutArguments))
    {
        PrintDemoHelp(mode);
        return true;
    }
    return false;
}

} // namespace adapters