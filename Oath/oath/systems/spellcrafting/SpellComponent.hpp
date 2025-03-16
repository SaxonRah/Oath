#pragma once

#include <map>
#include <nlohmann/json.hpp>
#include <string>


#include "../../data/GameContext.hpp"
#include "SpellEnums.hpp"


class SpellComponent {
public:
    std::string id;
    std::string name;
    std::string description;
    SpellEffectType effectType;

    // Base attributes
    int manaCost;
    int basePower;
    int complexity; // Difficulty to learn/use

    // Skill requirements
    std::map<std::string, int> schoolRequirements;

    // Special modifiers
    std::map<std::string, float> modifiers;

    // Visual effects
    std::string castingEffect;
    std::string impactEffect;

    // Constructor from JSON
    SpellComponent(const nlohmann::json& componentJson);

    // Get the adjusted power based on caster stats and skills
    int getAdjustedPower(const GameContext& context) const;

    // Get mana cost adjusted for skills and abilities
    int getAdjustedManaCost(const GameContext& context) const;

    // Convert to JSON
    nlohmann::json toJson() const;
};