#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <vector>


#include "../../data/GameContext.hpp"
#include "SpellComponent.hpp"
#include "SpellDelivery.hpp"
#include "SpellEnums.hpp"
#include "SpellModifier.hpp"


class SpellDesign {
public:
    std::string id;
    std::string name;
    std::string description;
    std::vector<SpellComponent*> components;
    std::vector<SpellModifier*> modifiers;
    SpellDelivery* delivery;
    SpellTargetType targetType;

    // Visual effects
    std::string castingVisual;
    std::string impactVisual;
    std::string spellIcon;

    // Calculated attributes
    int totalManaCost;
    float castingTime;
    int power;
    float duration;
    float range;
    float area;

    // Difficulty and learning
    int complexityRating;
    bool isLearned;
    bool isFavorite;

    // Basic constructor
    SpellDesign(const std::string& spellId, const std::string& spellName);

    // Constructor from JSON - special version that takes component/modifier/delivery maps
    SpellDesign(const nlohmann::json& spellJson,
        const std::map<std::string, SpellComponent*>& componentMap,
        const std::map<std::string, SpellModifier*>& modifierMap,
        const std::map<std::string, SpellDelivery*>& deliveryMap);

    // Calculate the total mana cost and other attributes
    void calculateAttributes(const GameContext& context);

    // Check if the player can cast this spell
    bool canCast(const GameContext& context) const;

    // Check if the player can learn this spell
    bool canLearn(const GameContext& context) const;

    // Try to cast the spell and return success/failure
    bool cast(GameContext* context);

    // Create a string representation of the spell for display
    std::string getDescription() const;

    // Convert to JSON
    nlohmann::json toJson() const;
};