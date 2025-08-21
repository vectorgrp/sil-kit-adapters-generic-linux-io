// SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
// SPDX-License-Identifier: MIT

#pragma once

#include <iostream>

#include "common/Cli.hpp"
#include "common/Parsing.hpp"

namespace adapters {

// Arguments handled by the adapter
const std::string adapterConfigurationArg = "--adapter-configuration";
const std::string versionArg = "--version";
const std::string defaultParticipantName = "SilKitAdapterGenericLinuxIO";

inline void PrintVersion()
{
    std::cout << "Vector SIL Kit Adapter for Generic Linux IO - version: " << SILKIT_ADAPTER_VERSION << std::endl;
}

inline void PrintHelp(bool userRequested)
{
    PrintVersion();
    std::cout << "Usage (defaults in curly braces if you omit the switch):\n"
                 "sil-kit-adapter-generic-linux-io "
              << adapterConfigurationArg
              << " <path to .yaml devices configuration file>\n"
                 "  ["
              << adapters::configurationArg
              << " <path to .silkit.yaml or .json configuration file>]\n"
                 "  ["
              << adapters::participantNameArg
              << " <participant name{SilKitAdapterGenericLinuxIO}>]\n"
                 "  ["
              << adapters::regUriArg
              << " silkit://<host{localhost}>:<port{8501}>]\n"
                 "  ["
              << adapters::logLevelArg
              << " <Trace|Debug|Warn|{Info}|Error|Critical|Off>]\n"
                 "\n"
                 "SIL Kit-specific CLI arguments will be overwritten by the config file passed by "
              << adapters::configurationArg << ".\n";
    std::cout << "\n"
                 "Example:\n"
                 "sil-kit-adapter-generic-linux-io "
              << adapterConfigurationArg << " ./adapter/demos/DevicesConfig.yaml " << adapters::participantNameArg
              << " GLIO_Participant" << '\n';

    std::cout << "Pass " << versionArg << " to get the version of the Adapter.\n";

    if (!userRequested)
    {
        std::cout << '\n';
        std::cout << "Pass " << adapters::helpArg << " to get this message" << '\n';
    }
}
} // namespace adapters