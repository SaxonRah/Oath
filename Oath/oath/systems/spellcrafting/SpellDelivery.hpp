#pragma once

#include <nlohmann/json.hpp>
#include <string>


#include "../../data/GameContext.hpp"
#include "SpellEnums.hpp"


class SpellDelivery {
public:
    std::string id;
    std::string name;
    std::string description;
    SpellDeliveryMethod method;

    // Base attributes
    int manaCostModifier;
    float castingTimeModifier;
    float rangeBase;

    // Required skill to use
    std::string requiredSchool;
    int requiredLevel;

    // Constructor from JSON
    SpellDelivery(const nlohmann::json& deliveryJson);

    bool canUse(const GameContext& context) const;

    float getAdjustedRange(const GameContext& context) const;

    // Convert to JSON
    nlohmann::json toJson() const;
};