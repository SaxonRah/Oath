#include "SpellDelivery.hpp"
#include "SpellUtils.hpp"

SpellDelivery::SpellDelivery(const nlohmann::json& deliveryJson)
    : id(deliveryJson["id"])
    , name(deliveryJson["name"])
    , description(deliveryJson.contains("description") ? deliveryJson["description"].get<std::string>() : "")
    , method(stringToDeliveryMethod(deliveryJson["method"]))
    , manaCostModifier(deliveryJson["mana_cost_modifier"])
    , castingTimeModifier(deliveryJson["casting_time_modifier"])
    , rangeBase(deliveryJson["range_base"])
    , requiredSchool(deliveryJson["required_school"])
    , requiredLevel(deliveryJson["required_level"])
{
}

bool SpellDelivery::canUse(const GameContext& context) const
{
    if (requiredSchool.empty() || requiredLevel <= 0) {
        return true;
    }

    return context.playerStats.hasSkill(requiredSchool, requiredLevel);
}

float SpellDelivery::getAdjustedRange(const GameContext& context) const
{
    float skillBonus = 1.0f;

    if (!requiredSchool.empty()) {
        int actualSkill = context.playerStats.skills.count(requiredSchool) ? context.playerStats.skills.at(requiredSchool) : 0;
        skillBonus += (actualSkill - requiredLevel) * 0.05f;
    }

    return rangeBase * skillBonus;
}

nlohmann::json SpellDelivery::toJson() const
{
    nlohmann::json j;
    j["id"] = id;
    j["name"] = name;
    j["description"] = description;
    j["method"] = deliveryMethodToString(method);
    j["mana_cost_modifier"] = manaCostModifier;
    j["casting_time_modifier"] = castingTimeModifier;
    j["range_base"] = rangeBase;
    j["required_school"] = requiredSchool;
    j["required_level"] = requiredLevel;
    return j;
}