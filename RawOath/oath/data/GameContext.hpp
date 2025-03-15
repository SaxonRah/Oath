#pragma once

#include "CharacterStats.hpp"
#include "Inventory.hpp"
#include "WorldState.hpp"


#include <map>
#include <string>

struct GameContext {
    CharacterStats playerStats;
    WorldState worldState;
    Inventory playerInventory;

    std::map<std::string, std::string> questJournal;
    std::map<std::string, std::string> dialogueHistory;
};