#pragma once

#include "../data/CharacterStats.hpp"
#include "../data/Inventory.hpp"
#include "../data/WorldState.hpp"

#include "nlohmann/json.hpp"
#include <string>

// JSON serialization functions
nlohmann::json serializeCharacterStats(const CharacterStats& stats);
void deserializeCharacterStats(const nlohmann::json& statsData, CharacterStats& stats);

nlohmann::json serializeWorldState(const WorldState& state);
void deserializeWorldState(const nlohmann::json& worldData, WorldState& state);

nlohmann::json serializeInventory(const Inventory& inventory);
void deserializeInventory(const nlohmann::json& inventoryData, Inventory& inventory);