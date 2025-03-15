#pragma once

#include "../core/TAController.hpp"
#include <nlohmann/json.hpp>

// Load game data from JSON files
bool loadGameData(TAController& controller);

// Helper functions for loading specific parts
void loadQuestsFromJSON(TAController& controller, const nlohmann::json& questData);
void loadNPCsFromJSON(TAController& controller, const nlohmann::json& npcData);
void loadSkillsFromJSON(TAController& controller, const nlohmann::json& skillsData);
void loadCraftingFromJSON(TAController& controller, const nlohmann::json& craftingData);
void loadWorldFromJSON(TAController& controller, const nlohmann::json& worldData);

// Create default JSON files
void createDefaultJSONFiles();