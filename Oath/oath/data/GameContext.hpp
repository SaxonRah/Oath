#pragma once

#include "CharacterStats.hpp"
#include "Inventory.hpp"
#include "WorldState.hpp"

// #include "../core/TAController.hpp"
// #include "../systems/health/DiseaseManager.hpp"
// #include "../systems/health/HealthContext.hpp"

#include <map>
#include <string>

// class TAController;
struct CharacterStats;
class Inventory;

// struct HealthContext;
// class DiseaseManager;
struct WorldState;

// Context for all systems
struct GameContext {
    // TAController controller;
    CharacterStats playerStats;
    Inventory playerInventory;

    // HealthContext healthContext;
    // DiseaseManager diseaseManager;

    WorldState worldState;

    std::map<std::string, std::string> questJournal;
    std::map<std::string, std::string> dialogueHistory;
};
