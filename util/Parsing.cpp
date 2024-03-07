// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "Parsing.hpp"

#include <iostream>
#include <algorithm>
#include <cstring>

namespace adapters::Parsing 
{

auto ThereAreUnknownArguments(int argc, char** argv) ->bool
{
    //skip the executable calling:
    argc -= 1;
    argv += 1;
    while (argc)
    {
        if (strncmp(*argv, "--", 2) != 0)
            return true;
        if (std::find(switchesWithArgument.begin(), switchesWithArgument.end(), *argv) != switchesWithArgument.end())
        {
            //switches with argument have an argument to ignore, so skip "2"
            argc -= 2;
            argv += 2;
        }
        else if (std::find(switchesWithoutArguments.begin(), switchesWithoutArguments.end(), *argv)
                 != switchesWithoutArguments.end())
        {
            //switches without argument don't have an argument to ignore, so skip "1"
            argc -= 1;
            argv += 1;
        }
        else
        {
            PrintHelp(false);
            return true;
        }
    }
    return false;
}

void PrintHelp(bool userRequested)
{
    std::cout << "Usage (defaults in curly braces if you omit the switch):\n"
                "SilKitAdapterGenericLinuxIO " << adapterConfigurationArg << " <path to .yaml devices configuration file>\n"
                "  [" << configurationArg << " <path to .silkit.yaml or .json configuration file>]\n"
                "  [" << participantNameArg << " <participant name{SilKitAdapterGenericLinuxIO}>]\n"
                "  [" << regUriArg << " silkit://<host{localhost}>:<port{8501}>]\n"
                "  [" << logLevelArg << " <Trace|Debug|Warn|{Info}|Error|Critical|Off>]\n"
                "\n"
                "Example:\n"
                "SilKitAdapterGenericLinuxIO " << adapterConfigurationArg << " ./adapter/demos/ChipConfig.yaml " << participantNameArg << " GLIO_Participant" << '\n';

    if (!userRequested)
    {
        std::cout << '\n';
        std::cout << "Pass " << helpArg << " to get this message" << '\n';
    }
}

auto FindArg(int argc, char** argv, const std::string& argument, char** args) -> char**
{
    auto found = std::find_if(args, argv + argc, [argument](const char* arg) -> bool {
        return argument == arg;
    });
    if (found < argv + argc)
        return found;

    return NULL;
}

auto FindArgOf(int argc, char** argv, const std::string& argument, char** args) -> char**
{
    auto found = FindArg(argc, argv, argument, args);
    if (found != NULL && found + 1 < argv + argc)
        return found + 1;

    return NULL;
}

auto GetArgDefault(int argc, char** argv, const std::string& argument, [[maybe_unused]] const std::string& defaultValue) -> std::string
{
    auto found = FindArgOf(argc, argv, argument, argv);
    if (found != NULL)
        return *(found);

    return defaultValue;
}

} // namespace adapters::Parsing