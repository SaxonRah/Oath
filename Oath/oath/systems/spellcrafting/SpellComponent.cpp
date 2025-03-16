#include "SpellComponent.hpp"
#include "SpellUtils.hpp"
#include <algorithm>

SpellComponent::SpellComponent(const nlohmann::json& componentJson)
    : id(componentJson["id"])
    , name(componentJson["name"])
    , description(componentJson["description"])
    , effectType(stringToEffectType(componentJson["effect_type"]))
    , manaCost(componentJson["mana_cost"])
    , basePower(componentJson["base_power"])
    , complexity(componentJson["complexity"])
{
    // Load school requirements
    if (componentJson.contains("school_requirements")) {
        for (auto& [school, level] : componentJson["school_requirements"].items()) {
            schoolRequirements[school] = level;
        }
    }

    // Load modifiers if present
    if (componentJson.contains("modifiers")) {
        for (auto& [modKey, modValue] : componentJson["modifiers"].items()) {
            modifiers[modKey] = modValue;
        }
    }

    // Load visual effects
    if (componentJson.contains("casting_effect")) {
        castingEffect = componentJson["casting_effect"];
    }

    if (componentJson.contains("impact_effect")) {
        impactEffect = componentJson["impact_effect"];
    }
}

int SpellComponent::getAdjustedPower(const GameContext& context) const
{
    int adjustedPower = basePower;

    // Apply skill bonuses
    for (const auto& [school, requirement] : schoolRequirements) {
        int actualSkill = context.playerStats.skills.count(school) ? context.playerStats.skills.at(school) : 0;
        if (actualSkill > requirement) {
            // Bonus for exceeding requirement
            adjustedPower += (actualSkill - requirement) * 2;
        }
    }

    // Apply intelligence bonus
    adjustedPower += (context.playerStats.intelligence - 10) / 2;

    return adjustedPower;
}

int SpellComponent::getAdjustedManaCost(const GameContext& context) const
{
    float costMultiplier = 1.0f;

    // Apply skill discounts
    for (const auto& [school, requirement] : schoolRequirements) {
        int actualSkill = context.playerStats.skills.count(school) ? context.playerStats.skills.at(school) : 0;
        if (actualSkill > requirement) {
            // Discount for higher skill
            costMultiplier -= std::min(0.5f, (actualSkill - requirement) * 0.02f);
        }
    }

    // Apply special abilities
    if (context.playerStats.hasAbility("mana_efficiency")) {
        costMultiplier -= 0.1f;
    }

    // Ensure minimum cost
    costMultiplier = std::max(0.5f, costMultiplier);

    return static_cast<int>(manaCost * costMultiplier);
}

nlohmann::json SpellComponent::toJson() const
{
    nlohmann::json j;
    j["id"] = id;
    j["name"] = name;
    j["description"] = description;
    j["effect_type"] = effectTypeToString(effectType);
    j["mana_cost"] = manaCost;
    j["base_power"] = basePower;
    j["complexity"] = complexity;
    j["school_requirements"] = schoolRequirements;
    j["modifiers"] = modifiers;
    j["casting_effect"] = castingEffect;
    j["impact_effect"] = impactEffect;
    return j;
}