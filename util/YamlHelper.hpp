// SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
// SPDX-License-Identifier: MIT

#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <memory>
#include <string>

#include "FileHelper.hpp"
#include "Exceptions.hpp"
#include "YamlParserUtils.hpp"

#include "silkit/services/pubsub/all.hpp"
#include "silkit/util/serdes/Serialization.hpp"

#include "rapidyaml.hpp"

namespace adapters {
namespace Util {

using PubSubSpec = SilKit::Services::PubSub::PubSubSpec;

// data to get from YAML config file
struct DataYAMLConfig
{
    std::string path = "";
    std::string topic_subscribe = "";
    std::string topic_publish = "";

    // optional data
    std::string fileName = "";
    std::string dataType = "";
    std::string offset = "";
};

class YamlNode
{
public:
    class Iterator;

    YamlNode() = default;

    YamlNode(const ryml::Parser& parser, ryml::ConstNodeRef node)
        : _parser{&parser}
        , _node{node}
    {
    }

    // whether the node exists, i.e. whether the looked up key was present
    explicit operator bool() const
    {
        return IsValid();
    }

    auto IsValid() const -> bool
    {
        return _parser != nullptr && _node.readable();
    }

    auto IsMap() const -> bool
    {
        return IsValid() && _node.is_map();
    }

    auto IsSequence() const -> bool
    {
        return IsValid() && _node.is_seq();
    }

    auto IsScalar() const -> bool
    {
        return IsValid() && (_node.is_val() || _node.is_keyval());
    }

    // key of a mapping entry, empty if the node does not have one
    auto Key() const -> std::string;

    // scalar value of the node, empty if the node does not hold one
    auto Value() const -> std::string;

    // child of a mapping by key, an invalid node if it is absent or if this is not a mapping
    auto operator[](ryml::csubstr key) const -> YamlNode;

    // n-th child of a sequence or a mapping, an invalid node if the index is out of range
    auto Child(std::size_t index) const -> YamlNode;

    auto begin() const -> Iterator;
    auto end() const -> Iterator;

    // a YamlError pointing at the position of this node in the config file
    auto MakeError(const std::string& message) const -> YamlError;

private:
    const ryml::Parser* _parser = nullptr;
    ryml::ConstNodeRef _node{};
};


class YamlNode::Iterator
{
public:
    Iterator() = default;

    Iterator(const ryml::Parser* parser, const ryml::Tree* tree, ryml::id_type id)
        : _parser{parser}
        , _tree{tree}
        , _id{id}
    {
    }

    auto operator*() const -> YamlNode
    {
        return YamlNode{*_parser, _tree->cref(_id)};
    }

    auto operator++() -> Iterator&
    {
        _id = _tree->next_sibling(_id);
        return *this;
    }

    auto operator!=(const Iterator& other) const -> bool
    {
        return _id != other._id;
    }

private:
    const ryml::Parser* _parser = nullptr;
    const ryml::Tree* _tree = nullptr;
    ryml::id_type _id = ryml::NONE;
};


class YamlDocument
{
public:
    YamlDocument() = default;

    YamlDocument(const YamlDocument&) = delete;
    auto operator=(const YamlDocument&) -> YamlDocument& = delete;

    // parse input, name is only used to make error messages point at the source
    void Parse(std::string input, std::string name);

    // whether the document does not hold any usable configuration
    auto IsNull() const -> bool;

    auto Root() const -> YamlNode;

private:
    std::string _buffer;
    std::string _name;
    ryml::EventHandlerTree _eventHandler{GetRapidyamlCallbacks()};
    std::unique_ptr<ryml::Parser> _parser;
    ryml::Tree _tree;
};

// load configFile into a YamlDocument
void LoadYAMLConfigFile(YamlDocument& doc, const std::string& configFile, SilKit::Services::Logging::ILogger* logger);

// get the given value in the given node, and set it to the corresponding value to update
void GetYAMLValue(const YamlNode& node, ryml::csubstr valueToGet, std::string& valueToUpdate, const bool isMandatory,
                  SilKit::Services::Logging::ILogger* logger);

// check if there is an unknown attribute in the YAML file for the given node
template <std::size_t N>
inline auto FindUnknownAttribute(const std::array<const char*, N>& attributes, const YamlNode& node,
                                 SilKit::Services::Logging::ILogger* logger) -> bool
{
    for (const auto& child : node)
    {
        const auto key = child.Key();
        if (std::find(attributes.begin(), attributes.end(), key) == std::end(attributes))
        {
            logger->Error("Error in YAML config file. Unknown attribute: '" + key + "'");
            return true;
        }
    }
    return false;
}

} // namespace Util
} // namespace adapters
