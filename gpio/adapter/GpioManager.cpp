// Copyright (c) Vector Informatik GmbH. All rights reserved.

#include "GpioManager.hpp"

#include <thread>
#include <sys/ioctl.h>
#include <linux/gpio.h>

#include "GpioAdapter.hpp"

#include "../../util/FileHelper.hpp"
#include "../../util/Exceptions.hpp"

#include "silkit/util/serdes/Serialization.hpp"

using namespace adapters;
using namespace GpioWrapper;
using namespace std::chrono_literals;
using namespace SilKit::Services::PubSub;

GpioManager::GpioManager(const YAML::Node& configFile,
                         std::vector<std::shared_ptr<IOAdapter>>& ioAdapters, 
                         SilKit::Services::Logging::ILogger* logger, 
                         SilKit::IParticipant* participant) :
    _logger(logger)
{
    InitAdaptersFromConfigFile(configFile, ioAdapters, participant);
}

GpioManager::~GpioManager()
{
    Stop();
}

void GpioManager::Stop() 
{ 
    std::size_t i = 0;
    for (const auto& chipContext : _chipContexts)
    {
        if (!chipContext.second->stopped())
        {
            chipContext.second->stop();
        }
        if (threadPool[i].joinable())
        {
            threadPool[i].join();
        }

        chipContext.first->Close();
        ++i;
    }
}

void GpioManager::InitAdaptersFromConfigFile(const YAML::Node& configFile,
                                             std::vector<std::shared_ptr<IOAdapter>>& ioAdapters, 
                                             SilKit::IParticipant* participant)
{
    // Root attribute gpiochips
    const auto gpiochipNodes = configFile["gpiochips"];

    _logger->Debug("Gpio chips found in the YAML configuration file.");

    // for each gpio chip
    for (const auto& chipNode : gpiochipNodes)
    {
        // Get attributes from YAML config file
        std::string chipPath;
        std::vector<Util::DataYAMLConfig> gpioYAMLConfigs;
        GetYamlConfig(chipNode, gpioYAMLConfigs, chipPath);

        const auto chipName = chipPath.substr(chipPath.find_last_of('/') + 1);

        auto ioc = std::make_unique<Ioc>();
        auto chip = std::make_unique<Chip>(*ioc, chipPath);
        
        // Manage each line specified in the YAML config file
        for (std::size_t j = 0; j < gpioYAMLConfigs.size(); ++j)
        {
            std::unique_ptr<PubSubSpec> pubDataSpec, subDataSpec;
            std::string publisherName, subscriberName;

            if (!gpioYAMLConfigs[j].topic_publish.empty())
            {
                pubDataSpec = std::make_unique<PubSubSpec>(gpioYAMLConfigs[j].topic_publish, SilKit::Util::SerDes::MediaTypeData());
                pubDataSpec->AddLabel("KeyA", "ValueA", SilKit::Services::MatchingLabel::Kind::Optional);
                publisherName = chipName + "PubLine" + gpioYAMLConfigs[j].offset;
            }
            if (!gpioYAMLConfigs[j].topic_subscribe.empty())
            {
                subDataSpec = std::make_unique<PubSubSpec>(gpioYAMLConfigs[j].topic_subscribe, SilKit::Util::SerDes::MediaTypeData());
                subDataSpec->AddLabel("KeyA", "ValueB", SilKit::Services::MatchingLabel::Kind::Optional);
                subscriberName = chipName + "SubLine" + gpioYAMLConfigs[j].offset;
            }

            auto newAdapter = std::make_shared<GpioAdapter>(participant, 
                                                            publisherName, 
                                                            subscriberName, 
                                                            std::move(pubDataSpec), 
                                                            std::move(subDataSpec), 
                                                            chip.get(), 
                                                            ioc.get(), 
                                                            atoi(gpioYAMLConfigs[j].offset.c_str()));

            newAdapter->Initialize();

            ioAdapters.push_back(newAdapter);
        }

        _chipContexts.insert(std::make_pair(std::move(chip), std::move(ioc)));
    }
    
    // After initialization the data subscribers can be created
    for (const auto& adapter : ioAdapters)
    {
        adapter->CreateDataSubscriber();
    }

    for (const auto& chipContext : _chipContexts)
    {
        threadPool.emplace_back(std::thread([&]() -> void {
            chipContext.second->run();
        }));
    }
}

void GpioManager::GetYamlConfig(const YAML::Node& chipNode, std::vector<Util::DataYAMLConfig>& dataYAMLConfigs, std::string& chipPath)
{
    const std::vector<std::string> attributes{"path", "lines", "offset", "topic_subscribe", "topic_publish"};

    // Check unknown attribute in path and lines
    if (Util::FindUnknownAttribute(attributes, chipNode, _logger))
    {
        throw YamlError("Unknown attribute found.");
    }
    
    // Manage mandatory path attribute
    Util::GetYAMLValue(chipNode, "path", chipPath, true, _logger);

    // Manage mandatory lines attribute
    if (const auto& lines = chipNode["lines"])
    {
        for (const auto& lineNode : lines)
        {
            // Check unknown attribute in offset and topics
            if (Util::FindUnknownAttribute(attributes, lineNode, _logger))
            {
                throw YamlError("Unknown attribute found.");
            }

            Util::DataYAMLConfig gpioYAML;
            // Manage mandatory offset attribute
            Util::GetYAMLValue(lineNode, "offset", gpioYAML.offset, true, _logger);

            // Manage optional topic_subscribe attribute
            Util::GetYAMLValue(lineNode, "topic_subscribe", gpioYAML.topic_subscribe, false, _logger);

            // Manage optional topic_publish attribute
            Util::GetYAMLValue(lineNode, "topic_publish", gpioYAML.topic_publish, false, _logger);

            dataYAMLConfigs.push_back(gpioYAML);
        }
    }
    else
    {
        throw YamlError("Missing mandatory 'lines' attribute.");
    }

}