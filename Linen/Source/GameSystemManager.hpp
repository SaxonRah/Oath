// GameSystemManager.hpp
#pragma once

#include "GameSystemPlugin.hpp"
#include <map>
#include <memory>
#include <vector>

class TAController;
class GameContext;

class GameSystemManager {
private:
    TAController* controller;
    GameContext* gameContext;
    std::map<std::string, std::unique_ptr<GameSystemPlugin>> plugins;

public:
    GameSystemManager(TAController* ctrl, GameContext* ctx);

    // Plugin registration
    void registerPlugin(std::unique_ptr<GameSystemPlugin> plugin);

    // Plugin access
    GameSystemPlugin* getPlugin(const std::string& name);

    const std::map<std::string, std::unique_ptr<GameSystemPlugin>>& getPlugins() const;

    // System-wide operations
    void initializeAll();
    void updateAll(float deltaTime);
    void shutdownAll();

    // Save/load all systems
    json saveAllSystems() const;
    bool loadAllSystems(const json& data);

    // Global event dispatcher
    bool dispatchEvent(const std::string& eventType, const json& eventData);
};