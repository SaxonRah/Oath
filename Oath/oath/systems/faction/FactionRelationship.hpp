
// systems/faction/FactionRelationship.hpp
#pragma once

#include <map>
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

// Faction relationship structure
struct FactionRelationship {
    std::string factionA;
    std::string factionB;
    int relationValue; // -100 to 100
    std::string relationState; // "war", "hostile", "unfriendly", "neutral", "friendly", "allied"
    std::map<std::string, int> relationThresholds; // Thresholds for different relation states

    FactionRelationship(const std::string& a, const std::string& b, int value = 0);
    void loadThresholds(const json& thresholds);
    void updateState();
    void changeRelation(int amount);
};
