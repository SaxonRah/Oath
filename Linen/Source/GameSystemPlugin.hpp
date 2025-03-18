// GameSystemPlugin.hpp
#pragma once

#include "TACore.hpp" // Include core TA system
#include <memory>
#include <string>

class GameContext;

class GameSystemPlugin {
public:
    virtual ~GameSystemPlugin() = default;

    // Core functionality
    virtual std::string getSystemName() const = 0;
    virtual std::string getDescription() const = 0;
    virtual TANode* getRootNode() const = 0;

    // Lifecycle hooks
    virtual void initialize(TAController* controller, GameContext* context) = 0;
    virtual void shutdown() = 0;

    // System update (called every game frame/tick)
    virtual void update(float deltaTime) = 0;

    // Save/load system data
    virtual json saveState() const = 0;
    virtual bool loadState(const json& data) = 0;

    // Event handling (allows cross-system communication)
    virtual bool handleEvent(const std::string& eventType, const json& eventData) = 0;
};

// Helper for creating plugin instances
template <typename T>
std::unique_ptr<GameSystemPlugin> createPlugin()
{
    return std::make_unique<T>();
}
