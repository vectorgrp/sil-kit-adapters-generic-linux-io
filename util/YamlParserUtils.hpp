// SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

#include "Exceptions.hpp"

#include "rapidyaml.hpp"

namespace adapters {
namespace Util {

// copy a rapidyaml string view, which does not own the characters it points at, into a std::string
auto ToStdString(ryml::csubstr str) -> std::string;

// build a YamlError which points at the position the problem was found at
auto MakeYamlError(const ryml::Location& location, ryml::csubstr message) -> YamlError;

// callbacks which turn rapidyaml errors into YamlError exceptions instead of aborting the process
auto GetRapidyamlCallbacks() -> ryml::Callbacks;

// throws unless the configuration consists of exactly one YAML document
void EnsureSingleDocument(ryml::ConstNodeRef root);

} // namespace Util
} // namespace adapters
