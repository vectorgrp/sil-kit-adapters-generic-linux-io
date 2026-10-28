// SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
// SPDX-License-Identifier: MIT

#include "YamlParserUtils.hpp"

#include <cstdlib>
#include <sstream>

namespace {

auto RapidyamlAllocate(size_t length, void* hint, void* userData) -> void*;
void RapidyamlFree(void* ptr, size_t length, void* userData);
void RapidyamlError(const char* message, size_t length, ryml::Location location, void* userData);

} // namespace

namespace adapters {
namespace Util {

auto ToStdString(ryml::csubstr str) -> std::string
{
    if (str.empty())
    {
        return {};
    }
    return std::string{str.str, str.len};
}

auto MakeYamlError(const ryml::Location& location, ryml::csubstr message) -> YamlError
{
    std::ostringstream s;

    s << "Error in YAML config file";
    if (!location.name.empty())
    {
        s << " " << ToStdString(location.name);
    }

    // rapidyaml terminates its messages with newlines, which would break up the reported error
    s << ": line " << (location.line + 1) << " column " << (location.col + 1) << ": "
      << ToStdString(message.trimr(" \t\r\n"));

    return YamlError{s.str()};
}

auto GetRapidyamlCallbacks() -> ryml::Callbacks
{
    return ryml::Callbacks{nullptr, RapidyamlAllocate, RapidyamlFree, RapidyamlError};
}

void EnsureSingleDocument(ryml::ConstNodeRef root)
{
    if (root.readable() && root.is_stream() && root.num_children() > 1)
    {
        throw YamlError("Error in YAML config file: the configuration must contain exactly one YAML document, but "
                        "multiple \"---\"-separated documents were found.");
    }
}

} // namespace Util
} // namespace adapters

namespace {

auto RapidyamlAllocate(const size_t length, void* /*hint*/, void* /*userData*/) -> void*
{
    return std::malloc(length);
}

void RapidyamlFree(void* ptr, size_t /*length*/, void* /*userData*/)
{
    std::free(ptr);
}

void RapidyamlError(const char* message, const size_t length, ryml::Location location, void* /*userData*/)
{
    location.line = location.line > 0 ? location.line - 1 : 0;
    location.col = location.col > 0 ? location.col - 1 : 0;
    throw adapters::Util::MakeYamlError(location, ryml::csubstr{message, length});
}

} // namespace
