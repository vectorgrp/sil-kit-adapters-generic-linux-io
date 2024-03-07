// Copyright (c) Vector Informatik GmbH. All rights reserved.

#pragma once

#include <string>
#include <array>

#include "Exceptions.hpp"

namespace adapters::Parsing 
{

// All arguments handled by the adapter
const std::string regUriArg = "--registry-uri";
const std::string adapterConfigurationArg = "--adapter-configuration";
const std::string logLevelArg = "--log";
const std::string participantNameArg = "--name";
const std::string helpArg = "--help";
const std::string configurationArg = "--configuration";

// Arguments only available for demos
const std::string pubTopicArg = "--pub-topic";
const std::string subTopicArg = "--sub-topic";

const std::array<std::string, 7> switchesWithArgument = 
    {regUriArg, adapterConfigurationArg, logLevelArg, participantNameArg, configurationArg, pubTopicArg, subTopicArg};

const std::array<std::string, 1> switchesWithoutArguments = {helpArg};

// Returns wether or not there are unknown arguments in the provided command line.
auto ThereAreUnknownArguments(int argc, char** argv) -> bool;

// Prints the help message containing all switches and arguments.
void PrintHelp(bool userRequested = false);

// Searches [argv,argv+argc[ for a string matching argument, starting at args.
auto FindArg(int argc, char** argv, const std::string& argument, char** args) -> char**;

// Searches [argv,argv+argc[ for a string following a string matching argument, starting at args.
auto FindArgOf(int argc, char** argv, const std::string& argument, char** args) -> char**;

// Searches [argv,argv+argc[ for a string following a string matching argument.
auto GetArgDefault(int argc, char** argv, const std::string& argument, const std::string& defaultValue) -> std::string;

// Small utility function to quickly check if "it" is not "cont.end()"
template <typename iterator, typename container>
void AssertAdditionalIterator(const iterator& it, const container& cont)
{
    throwInvalidCliIf(it == cont.end());
}

// Searches [argv,argv+argc[ for all strings following a string matching argument, and calls action(those strings)
template<typename Action>
void ForeachArgDo(int argc, char** argv, const std::string& argument, const Action& action)
{
    for (char** arg = FindArgOf(argc, argv, argument, argv); 
                arg != NULL; 
                arg = FindArgOf(argc, argv, argument, arg + 1))
    {
        action(*arg);
    }
}

} // namespace adapters::Parsing