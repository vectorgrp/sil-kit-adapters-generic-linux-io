// SPDX-FileCopyrightText: Copyright 2025 Vector Informatik GmbH
// SPDX-License-Identifier: MIT

#include "AdapterFactory.hpp"

#include "util/YamlHelper.hpp"

#include "advalues/adapter/AdAdapter.hpp"
#include "chardev/adapter/ChardevAdapter.hpp"

#ifndef QNX_BUILD
#include "gpio/adapter/GpioAdapter.hpp"
#include "gpio/adapter/GpioWrapper.hpp"
#endif

using namespace adapters;
using namespace adapters::Util;
using namespace SilKit::Services::PubSub;

void HandleTopics(const DataYAMLConfig& chardevYaml, std::unique_ptr<PubSubSpec>& subDataSpec,
                  std::unique_ptr<PubSubSpec>& pubDataSpec, std::string& subscriberName, std::string& publisherName)
{
    if (!FileExists(chardevYaml.path))
    {
        throw std::runtime_error("file " + chardevYaml.path + " does not exist");
    }

    std::string pathToFile_ = chardevYaml.path;
    for (std::size_t i = 0; i < pathToFile_.size(); ++i)
    {
        if (pathToFile_[i] == '/')
        {
            pathToFile_.replace(pathToFile_.begin() + i, pathToFile_.begin() + i + 1, "_");
        }
    }

    // manage topic_subscribe
    if (!chardevYaml.topic_subscribe.empty())
    {
        subDataSpec = std::make_unique<PubSubSpec>(chardevYaml.topic_subscribe, SilKit::Util::SerDes::MediaTypeData());
        subDataSpec->AddLabel("KeyA", "ValueA", SilKit::Services::MatchingLabel::Kind::Optional);
        subscriberName = "sub" + pathToFile_;
    }

    // manage topic_publish
    if (!chardevYaml.topic_publish.empty())
    {
        pubDataSpec = std::make_unique<PubSubSpec>(chardevYaml.topic_publish, SilKit::Util::SerDes::MediaTypeData());
        pubDataSpec->AddLabel("KeyA", "ValueB", SilKit::Services::MatchingLabel::Kind::Optional);
        publisherName = "pub" + pathToFile_;
    }
}

void GetYamlConfig(const YAML::Node& node, std::vector<DataYAMLConfig>& dataYAMLConfigs,
                   SilKit::Services::Logging::ILogger* logger, const std::string& chipPath = "",
                   const std::string& type = "")
{
    static constexpr std::array<const char*, 5> attributes{"path", "name", "offset", "topic_subscribe",
                                                           "topic_publish"};

    for (const auto& subNode : node)
    {
        // check if there is any unknown attribute
        if (FindUnknownAttribute(attributes, subNode, logger))
        {
            throw adapters::YamlError("Unknown attribute found.");
        }

        DataYAMLConfig chardevYaml;
        if (chipPath != "")
        {
            // handle the path differently if it is gpio or advalues mode
            if (subNode["offset"])
            {
                // gpio mode
                GetYAMLValue(subNode, "offset", chardevYaml.offset, true, logger);
                chardevYaml.path = chipPath;
            }
            else
            {
                // advalues mode
                GetYAMLValue(subNode, "name", chardevYaml.fileName, true, logger);
                chardevYaml.path = chipPath + chardevYaml.fileName;

                if (type != "")
                {
                    chardevYaml.dataType = type;
                }
            }
        }
        else
        {
            // manage path attribute for chardev mode
            GetYAMLValue(subNode, "path", chardevYaml.path, true, logger);
        }

        // manage topic_subscribe attribute
        GetYAMLValue(subNode, "topic_subscribe", chardevYaml.topic_subscribe, false, logger);

        // manage topic_publish attribute
        GetYAMLValue(subNode, "topic_publish", chardevYaml.topic_publish, false, logger);

        dataYAMLConfigs.push_back(chardevYaml);
    }
}

void GetAdValuesYamlConfig(const YAML::Node& node, std::vector<DataYAMLConfig>& dataYAMLConfigs,
                           SilKit::Services::Logging::ILogger* logger)
{
    static constexpr std::array<const char*, 11> attributes{"path",     "int8_t",  "uint8_t",  "int16_t",
                                                            "uint16_t", "int32_t", "uint32_t", "int64_t",
                                                            "uint64_t", "float",   "double"};

    // iterate trough each chip
    for (const auto& nodeChip : node)
    {
        // check if there is any unknown attribute
        if (FindUnknownAttribute(attributes, nodeChip, logger))
        {
            throw adapters::YamlError("Unknown attribute found.");
        }

        std::string chipPath;
        for (const auto& keyVal : nodeChip)
        {
            if (keyVal.first.as<std::string>() == "path")
            {
                chipPath = keyVal.second.as<std::string>();
            }
            else
            {
                // if it is not the path, it is the data type
                const auto dataType = keyVal.first.as<std::string>();
                // keyVal.second[0] because of "- files" which creates an array
                const auto nodeFiles = keyVal.second[0]["files"];
                if (!nodeFiles)
                {
                    logger->Error("Error in YAML config file. Missing mandatory attribute \"files\" for " + chipPath);
                    throw adapters::YamlError("Unknown attribute found.");
                }
                GetYamlConfig(nodeFiles, dataYAMLConfigs, logger, chipPath, dataType);
            }
        }
    }
}

#ifndef QNX_BUILD
void GetGpioYamlConfig(const YAML::Node& chipNode, std::vector<DataYAMLConfig>& dataYAMLConfigs, std::string& chipPath,
                       SilKit::Services::Logging::ILogger* logger)
{
    static constexpr std::array<const char*, 2> attributes{"path", "lines"};

    // check if there is any unknown attribute
    if (FindUnknownAttribute(attributes, chipNode, logger))
    {
        throw adapters::YamlError("Unknown attribute found.");
    }

    for (const auto& keyVal : chipNode)
    {
        if (keyVal.first.as<std::string>() == "path")
        {
            chipPath = keyVal.second.as<std::string>();

            if (!FileExists(chipPath))
            {
                throw std::runtime_error("GPIO chip " + chipPath + " does not exist");
            }
        }
        else
        {
            // if it is not the path, it is the "lines" key, so pass its value
            GetYamlConfig(keyVal.second, dataYAMLConfigs, logger, chipPath);
        }
    }
}
#endif

void AdapterFactory::ConstructAdAdapters(const YAML::Node& configFile,
                                         std::vector<std::unique_ptr<IOAdapter>>& ioAdapters,
                                         SilKit::IParticipant* participant, asio::io_context& ioc)
{
    auto logger = participant->GetLogger();

    const auto nodeAdvalues = configFile["advalues"];
    if (!nodeAdvalues)
    {
        logger->Debug("No advalues chips found in the YAML configuration file.");
        return;
    }

    logger->Debug("Advalues chips found in the YAML configuration file.");

    std::vector<DataYAMLConfig> advaluesYAMLConfigs;

    GetAdValuesYamlConfig(nodeAdvalues, advaluesYAMLConfigs, logger);

    for (const auto& advaluesYaml : advaluesYAMLConfigs)
    {
        std::unique_ptr<PubSubSpec> subDataSpec, pubDataSpec;
        std::string publisherName, subscriberName;

        HandleTopics(advaluesYaml, subDataSpec, pubDataSpec, subscriberName, publisherName);

        auto newAdapter = std::make_unique<AdAdapter>(participant, publisherName, subscriberName, pubDataSpec.get(),
                                                      subDataSpec.get(), advaluesYaml.path, advaluesYaml.dataType, ioc);

        ioAdapters.push_back(std::move(newAdapter));
    }
}

void AdapterFactory::ConstructChardevAdapters(const YAML::Node& configFile,
                                              std::vector<std::unique_ptr<IOAdapter>>& ioAdapters,
                                              SilKit::IParticipant* participant, asio::io_context& ioc)
{
    auto logger = participant->GetLogger();

    const auto nodeChardev = configFile["chardevs"];
    if (!nodeChardev)
    {
        logger->Debug("No character devices found in the YAML configuration file.");
        return;
    }

    logger->Debug("Character devices found in the YAML configuration file.");

    std::vector<DataYAMLConfig> chardevYAMLConfigs;
    GetYamlConfig(nodeChardev, chardevYAMLConfigs, logger);

    for (auto chardevYaml : chardevYAMLConfigs)
    {
        std::unique_ptr<PubSubSpec> subDataSpec, pubDataSpec;
        std::string subscriberName, publisherName;

        HandleTopics(chardevYaml, subDataSpec, pubDataSpec, subscriberName, publisherName);

        auto newAdapter = std::make_unique<ChardevAdapter>(participant, publisherName, subscriberName,
                                                           pubDataSpec.get(), subDataSpec.get(), chardevYaml.path, ioc);

        ioAdapters.push_back(std::move(newAdapter));
    }
}

#ifndef QNX_BUILD
void AdapterFactory::ConstructGpioAdapters(const YAML::Node& configFile,
                                           std::vector<std::unique_ptr<IOAdapter>>& ioAdapters,
                                           std::vector<std::unique_ptr<GpioWrapper::Chip>>& chips,
                                           SilKit::IParticipant* participant, asio::io_context& ioc)
{
    auto logger = participant->GetLogger();

    const auto nodeGpio = configFile["gpiochips"];
    if (!nodeGpio)
    {
        logger->Debug("No GPIO chips found in the YAML configuration file.");
        return;
    }

    logger->Debug("GPIO chips found in the YAML configuration file.");
    // for each gpio chip
    for (const auto& chipNode : nodeGpio)
    {
        // get attributes from YAML config file
        std::string chipPath;
        std::vector<DataYAMLConfig> gpioYAMLConfigs;
        GetGpioYamlConfig(chipNode, gpioYAMLConfigs, chipPath, logger);

        auto chip = std::make_unique<GpioWrapper::Chip>(ioc, chipPath);

        // manage each line specified in the YAML config file
        for (const auto& gpioYaml : gpioYAMLConfigs)
        {
            std::unique_ptr<PubSubSpec> pubDataSpec, subDataSpec;
            std::string publisherName, subscriberName;

            HandleTopics(gpioYaml, subDataSpec, pubDataSpec, subscriberName, publisherName);

            const std::string offset = gpioYaml.offset;
            subscriberName += offset;
            publisherName += offset;

            auto newAdapter =
                std::make_unique<GpioAdapter>(participant, publisherName, subscriberName, pubDataSpec.get(),
                                              std::move(subDataSpec), chip.get(), ioc, atoi(offset.c_str()));

            ioAdapters.push_back(std::move(newAdapter));
        }

        chips.push_back(std::move(chip));
    }

    // after initialization the data subscribers can be created
    for (const auto& adapter : ioAdapters)
    {
        adapter->CreateDataSubscriber();
    }
}
#endif
