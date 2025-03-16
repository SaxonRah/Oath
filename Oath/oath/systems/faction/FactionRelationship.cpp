
// systems/faction/FactionRelationship.cpp
#include "FactionRelationship.hpp"
#include <algorithm>

namespace oath {

FactionRelationship::FactionRelationship(const std::string& a, const std::string& b, int value)
    : factionA(a)
    , factionB(b)
    , relationValue(value)
{
    // Default thresholds if not loaded from JSON
    relationThresholds = {
        { "war", -75 },
        { "hostile", -50 },
        { "unfriendly", -20 },
        { "neutral", 20 },
        { "friendly", 50 },
        { "cordial", 75 },
        { "allied", 100 }
    };
    updateState();
}

void FactionRelationship::loadThresholds(const json& thresholds)
{
    if (thresholds.is_object()) {
        for (auto& [state, value] : thresholds.items()) {
            relationThresholds[state] = value;
        }
    }
    // After loading, update the state based on new thresholds
    updateState();
}

void FactionRelationship::updateState()
{
    // Sort thresholds by value
    std::vector<std::pair<std::string, int>> sortedThresholds;
    for (const auto& [state, value] : relationThresholds) {
        sortedThresholds.push_back({ state, value });
    }

    std::sort(sortedThresholds.begin(), sortedThresholds.end(),
        [](const auto& a, const auto& b) { return a.second < b.second; });

    // Find appropriate state based on current relation value
    for (auto it = sortedThresholds.rbegin(); it != sortedThresholds.rend(); ++it) {
        if (relationValue >= it->second) {
            relationState = it->first;
            return;
        }
    }

    // Default to the lowest state if no match found
    if (!sortedThresholds.empty()) {
        relationState = sortedThresholds[0].first;
    } else {
        relationState = "neutral"; // Fallback
    }
}

void FactionRelationship::changeRelation(int amount)
{
    relationValue = std::max(-100, std::min(100, relationValue + amount));
    updateState();
}

} // namespace oath