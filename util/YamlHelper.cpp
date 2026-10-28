// SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
// SPDX-License-Identifier: MIT

#include "YamlHelper.hpp"

namespace {

// number of source lines the parser preallocates location information for
constexpr unsigned int reservedLocations = 100u;

} // namespace

namespace adapters {
namespace Util {

auto YamlNode::Key() const -> std::string
{
    if (!IsValid() || !_node.has_key())
    {
        return {};
    }
    return ToStdString(_node.key());
}

auto YamlNode::Value() const -> std::string
{
    if (!IsValid())
    {
        return {};
    }
    if (!_node.has_val())
    {
        throw MakeError("expected a scalar value, but got a mapping or a sequence.");
    }
    return ToStdString(_node.val());
}

auto YamlNode::operator[](ryml::csubstr key) const -> YamlNode
{
    if (!IsValid())
    {
        return {};
    }

    if (_node.is_map())
    {
        return YamlNode{*_parser, _node.find_child(key)};
    }

    if (_node.is_stream())
    {
        for (const auto& document : _node.cchildren())
        {
            if (document.is_seq())
            {
                throw MakeError(std::string{"expected a mapping with key '"}
                                    .append(ToStdString(key))
                                    .append("', but got a sequence."));
            }

            if (document.is_map())
            {
                const auto found = document.find_child(key);
                if (!found.invalid())
                {
                    return YamlNode{*_parser, found};
                }
            }
        }
        return {};
    }

    if (_node.is_seq())
    {
        throw MakeError(
            std::string{"expected a mapping with key '"}.append(ToStdString(key)).append("', but got a sequence."));
    }

    // absent, null or empty node: treat the key as not present
    return {};
}

auto YamlNode::Child(std::size_t index) const -> YamlNode
{
    if (!IsValid() || index >= static_cast<std::size_t>(_node.num_children()))
    {
        return {};
    }
    return YamlNode{*_parser, _node.child(static_cast<ryml::id_type>(index))};
}

auto YamlNode::begin() const -> Iterator
{
    if (!IsValid())
    {
        return {};
    }

    const auto* tree = _node.tree();
    return Iterator{_parser, tree, tree->first_child(_node.id())};
}

auto YamlNode::end() const -> Iterator
{
    if (!IsValid())
    {
        return {};
    }
    return Iterator{_parser, _node.tree(), ryml::NONE};
}

auto YamlNode::MakeError(const std::string& message) const -> YamlError
{
    if (!IsValid())
    {
        return YamlError{"Error in YAML config file: " + message};
    }
    return MakeYamlError(_parser->location(_node), ryml::to_csubstr(message));
}

void YamlDocument::Parse(std::string input, std::string name)
{
    // both are referenced by the parser and therefore have to outlive it
    _buffer = std::move(input);
    _name = std::move(name);

    ryml::ParserOptions options{};
    options.locations(true);

    _parser = std::make_unique<ryml::Parser>(&_eventHandler, options);
    _parser->reserve_locations(reservedLocations);

    ryml::parse_in_arena(_parser.get(), ryml::to_csubstr(_name), ryml::to_csubstr(_buffer), &_tree);

    // install the error handling callbacks on the tree as well, so that errors raised while walking
    // it are reported as a YamlError instead of aborting the process
    _tree.callbacks(GetRapidyamlCallbacks());

    EnsureSingleDocument(_tree.crootref());
}

auto YamlDocument::IsNull() const -> bool
{
    if (_parser == nullptr || _tree.empty())
    {
        return true;
    }

    const auto root = _tree.crootref();
    return !root.readable() || (!root.is_map() && !root.is_seq() && !root.is_stream());
}

auto YamlDocument::Root() const -> YamlNode
{
    if (_parser == nullptr || _tree.empty())
    {
        return {};
    }
    return YamlNode{*_parser, _tree.crootref()};
}

void LoadYAMLConfigFile(YamlDocument& doc, const std::string& configFile, SilKit::Services::Logging::ILogger* logger)
{
    logger->Debug("Loading YAML configuration file: " + configFile);
    std::string buffer = Util::ReadFileStr(configFile, logger);
    if (buffer.empty())
    {
        logger->Error("Buffer read from " + configFile + " is empty");
        return;
    }

    doc.Parse(std::move(buffer), configFile);
}

void GetYAMLValue(const YamlNode& node, ryml::csubstr valueToGet, std::string& valueToUpdate, const bool isMandatory,
                  SilKit::Services::Logging::ILogger* /*logger*/)
{
    if (const auto value = node[valueToGet])
    {
        valueToUpdate = value.Value();
    }
    else
    {
        if (isMandatory)
        {
            throw node.MakeError(std::string{"Missing "}.append(ToStdString(valueToGet)).append(" attribute."));
        }
    }
}

} // namespace Util
} // namespace adapters
