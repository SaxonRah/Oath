// GameSystemManager.cpp
#include "GameSystemManager.hpp"
#include <iostream>

GameSystemManager::GameSystemManager(TAController* ctrl, GameContext* ctx)
    : controller(ctrl)
    , gameContext(ctx)
{
}

void GameSystemManager::registerPlugin(std::unique_ptr<GameSystemPlugin> plugin)
{
    std::string name = plugin->getSystemName();
    std::cout << "Registering plugin: " << name << " - " << plugin->getDescription() << std::endl;
    plugins[name] = std::move(plugin);
}

GameSystemPlugin* GameSystemManager::getPlugin(const std::string& name)
{
    auto it = plugins.find(name);
    if (it != plugins.end()) {
        return it->second.get();
    }
    return nullptr;
}

const std::map<std::string, std::unique_ptr<GameSystemPlugin>>& GameSystemManager::getPlugins() const
{
    return plugins;
}

void GameSystemManager::initializeAll()
{
    for (auto& [name, plugin] : plugins) {
        std::cout << "Initializing system: " << name << "..." << std::endl;
        plugin->initialize(controller, gameContext);
    }
}

void GameSystemManager::updateAll(float deltaTime)
{
    for (auto& [name, plugin] : plugins) {
        plugin->update(deltaTime);
    }
}

void GameSystemManager::shutdownAll()
{
    for (auto& [name, plugin] : plugins) {
        std::cout << "Shutting down system: " << name << "..." << std::endl;
        plugin->shutdown();
    }
}

json GameSystemManager::saveAllSystems() const
{
    json saveData;

    // Save global game context
    saveData["worldState"] = {
        { "daysPassed", gameContext->worldState.daysPassed },
        { "currentSeason", gameContext->worldState.currentSeason },
        { "locationStates", gameContext->worldState.locationStates },
        { "factionStates", gameContext->worldState.factionStates },
        { "worldFlags", gameContext->worldState.worldFlags }
    };

    // Save player stats
    saveData["playerStats"] = {
        { "strength", gameContext->playerStats.strength },
        { "dexterity", gameContext->playerStats.dexterity },
        { "constitution", gameContext->playerStats.constitution },
        { "intelligence", gameContext->playerStats.intelligence },
        { "wisdom", gameContext->playerStats.wisdom },
        { "charisma", gameContext->playerStats.charisma },
        { "factionReputation", gameContext->playerStats.factionReputation },
        { "skills", gameContext->playerStats.skills }
    };

    // Save individual systems
    json systemsData;
    for (const auto& [name, plugin] : plugins) {
        systemsData[name] = plugin->saveState();
    }
    saveData["systems"] = systemsData;

    return saveData;
}

bool GameSystemManager::loadAllSystems(const json& data)
{
    try {
        // Load global game context
        if (data.contains("worldState")) {
            gameContext->worldState.daysPassed = data["worldState"]["daysPassed"];
            gameContext->worldState.currentSeason = data["worldState"]["currentSeason"];
            gameContext->worldState.locationStates = data["worldState"]["locationStates"].get<std::map<std::string, std::string>>();
            gameContext->worldState.factionStates = data["worldState"]["factionStates"].get<std::map<std::string, std::string>>();
            gameContext->worldState.worldFlags = data["worldState"]["worldFlags"].get<std::map<std::string, bool>>();
        }

        // Load player stats
        if (data.contains("playerStats")) {
            gameContext->playerStats.strength = data["playerStats"]["strength"];
            gameContext->playerStats.dexterity = data["playerStats"]["dexterity"];
            gameContext->playerStats.constitution = data["playerStats"]["constitution"];
            gameContext->playerStats.intelligence = data["playerStats"]["intelligence"];
            gameContext->playerStats.wisdom = data["playerStats"]["wisdom"];
            gameContext->playerStats.charisma = data["playerStats"]["charisma"];
            gameContext->playerStats.factionReputation = data["playerStats"]["factionReputation"].get<std::map<std::string, int>>();
            gameContext->playerStats.skills = data["playerStats"]["skills"].get<std::map<std::string, int>>();
        }

        // Load individual systems
        if (data.contains("systems")) {
            for (auto& [name, plugin] : plugins) {
                if (data["systems"].contains(name)) {
                    if (!plugin->loadState(data["systems"][name])) {
                        std::cerr << "Error loading state for system: " << name << std::endl;
                    }
                }
            }
        }

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading systems: " << e.what() << std::endl;
        return false;
    }
}

bool GameSystemManager::dispatchEvent(const std::string& eventType, const json& eventData)
{
    bool handled = false;

    // Send event to all plugins
    for (auto& [name, plugin] : plugins) {
        if (plugin->handleEvent(eventType, eventData)) {
            handled = true;
        }
    }

    return handle
}