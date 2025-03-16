#pragma once

#include "CharacterStats.hpp"
#include "Inventory.hpp"
#include "WorldState.hpp"

#include <map>
#include <string>

// Context for all systems
struct GameContext {
    CharacterStats playerStats;
    WorldState worldState;
    Inventory playerInventory;

    std::map<std::string, std::string> questJournal;
    std::map<std::string, std::string> dialogueHistory;
};

// TODO :
// - Merge Disease/Health

/*
#include "../systems/health/DiseaseManager.hpp"
#include "../systems/health/HealthContext.hpp"
#include "CharacterStats.hpp"
#include "Inventory.hpp"
#include "WorldState.hpp"

// Other includes...

struct GameContext {
    WorldState worldState;
    Inventory playerInventory;
    CharacterStats playerStats;
    HealthContext healthContext;
    DiseaseManager diseaseManager;
    // Other game state...
};
*/