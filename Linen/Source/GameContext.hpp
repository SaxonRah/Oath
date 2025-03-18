// GameContext.hpp
#pragma once

#include "CharacterStats.hpp"
#include "Inventory.hpp"
#include "WorldState.hpp"
#include <memory>

class GameSystemManager;

class GameContext {
public:
    // Core game state
    CharacterStats playerStats;
    WorldState worldState;
    Inventory playerInventory;
    std::map<std::string, std::string> questJournal;
    std::map<std::string, std::string> dialogueHistory;

    // Additional contexts for various systems
    // These would be initialized by respective plugins
    struct HealthContext {
        // ...health system state...
        HealthState playerHealth;
    } healthContext;

    struct EconomyContext {
        // ...economy system state...
        int playerGold = 1000;
    } economyContext;

    struct CrimeLawContext {
        // ...crime system state...
        CriminalRecord criminalRecord;
    } crimeLawContext;

    struct FactionContext {
        // ...faction system state...
        std::map<std::string, int> factionStanding;
    } factionContext;

    struct RelationshipContext {
        // ...NPC relationship state...
        std::map<std::string, int> npcRelationships;
    } relationshipContext;

    // Access to the system manager (set externally)
    GameSystemManager* systemManager = nullptr;

    std::string getCurrentRegion() const;
    int getTimeOfDay() const;

    // Helper methods for system interaction
    template <typename T>
    T* getSystem()
    {
        if (!systemManager)
            return nullptr;
        return dynamic_cast<T*>(systemManager->getPlugin(T::SystemName));
    }
};