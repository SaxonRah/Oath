#include "SpellModifier.hpp"

SpellModifier::SpellModifier(const nlohmann::json& modifierJson)
    : id(modifierJson["id"])
    , name(modifierJson["name"])
    , description(modifierJson.contains("description") ? modifierJson["description"].get<std::string>() : "")
    , powerMultiplier(modifierJson["power_multiplier"])
    , rangeMultiplier(modifierJson["range_multiplier"])
    , durationMultiplier(modifierJson["duration_multiplier"])
    , areaMultiplier(modifierJson["area_multiplier"])
    , castingTimeMultiplier(modifierJson["casting_time_multiplier"])
    , manaCostMultiplier(modifierJson["mana_cost_multiplier"])
    , requiredSchool(modifierJson["required_school"])
    , requiredLevel(modifierJson["required_level"])
{
}

bool SpellModifier::canApply(const GameContext& context) const
{
    if (requiredSchool.empty() || requiredLevel <= 0) {
        return true;
    }

    return context.playerStats.hasSkill(requiredSchool, requiredLevel);
}

nlohmann::json SpellModifier::toJson() const
{
    nlohmann::json j;
    j["id"] = id;
    j["name"] = name;
    j["description"] = description;
    j["power_multiplier"] = powerMultiplier;
    j["range_multiplier"] = rangeMultiplier;
    j["duration_multiplier"] = durationMultiplier;
    j["area_multiplier"] = areaMultiplier;
    j["casting_time_multiplier"] = castingTimeMultiplier;
    j["mana_cost_multiplier"] = manaCostMultiplier;
    j["required_school"] = requiredSchool;
    j["required_level"] = requiredLevel;
    return j;
}