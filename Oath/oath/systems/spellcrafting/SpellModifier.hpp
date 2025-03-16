#pragma once

#include "../../data/GameContext.hpp"
#include <nlohmann/json.hpp>
#include <string>


class SpellModifier {
public:
    std::string id;
    std::string name;
    std::string description;

    // Effect on spell attributes
    float powerMultiplier;
    float rangeMultiplier;
    float durationMultiplier;
    float areaMultiplier;
    float castingTimeMultiplier;
    float manaCostMultiplier;

    // Required skill to use
    std::string requiredSchool;
    int requiredLevel;

    // Constructor from JSON
    SpellModifier(const nlohmann::json& modifierJson);

    bool canApply(const GameContext& context) const;

    // Convert to JSON
    nlohmann::json toJson() const;
};