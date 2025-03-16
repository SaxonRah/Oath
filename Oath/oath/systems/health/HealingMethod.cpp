// /oath/systems/health/HealingMethod.cpp
#include "HealingMethod.hpp"
#include "HealthState.hpp"
#include <algorithm>
#include <iostream>


HealingMethod::HealingMethod(const std::string& methodId, const std::string& methodName)
    : id(methodId)
    , name(methodName)
    , healAmount(0.0f)
    , requiresItem(false)
    , requiredAmount(0)
    , requiresLocation(false)
{
}

HealingMethod::HealingMethod(const nlohmann::json& methodJson)
    : id(methodJson["id"])
    , name(methodJson["name"])
    , description(methodJson["description"])
    , healAmount(methodJson["healAmount"])
    , requiresItem(methodJson["requiresItem"])
    , requiredAmount(0)
    , requiresLocation(methodJson.value("requiresLocation", false))
{
    // Load effectiveness
    for (auto& [disease, effect] : methodJson["effectiveness"].items()) {
        effectivenessAgainstDisease[disease] = effect;
    }

    // Load item requirements if needed
    if (requiresItem) {
        requiredItem = methodJson["requiredItem"];
        requiredAmount = methodJson["requiredAmount"];
    }

    // Load location requirements if needed
    if (requiresLocation) {
        requiredLocationType = methodJson["requiredLocationType"];
    }

    // Load costs if present
    if (methodJson.contains("costs")) {
        for (auto& [disease, cost] : methodJson["costs"].items()) {
            costs[disease] = cost;
        }
    }

    // Set up requirement check and apply effect
    setupRequirementCheck();
    setupApplyEffect();
}

void HealingMethod::setupRequirementCheck()
{
    requirementCheck = [this](GameContext* context) {
        if (!context)
            return false;

        // Check item requirements
        if (requiresItem) {
            if (!context->playerInventory.hasItem(requiredItem, requiredAmount)) {
                std::cout << "You need " << requiredAmount << " " << requiredItem << " to use this treatment." << std::endl;
                return false;
            }
        }

        // Check location requirements
        if (requiresLocation) {
            // This would check if player is in the right location type
            // For demo purposes, always return true
            return true;
        }

        return true;
    };
}

void HealingMethod::setupApplyEffect()
{
    applyEffect = [this](GameContext* context, const std::string& diseaseId) {
        if (!context)
            return;

        HealthState* health = &context->healthContext.playerHealth;
        if (!health)
            return;

        // Consume required items
        if (requiresItem) {
            context->playerInventory.removeItem(requiredItem, requiredAmount);
            std::cout << "Used " << requiredAmount << " " << requiredItem << " for treatment." << std::endl;
        }

        // Handle cost if applicable
        if (costs.find(diseaseId) != costs.end()) {
            int cost = costs[diseaseId];
            // Would deduct gold here
            std::cout << "Paid " << cost << " gold for treatment." << std::endl;
        }

        // Apply healing
        health->heal(healAmount);
        std::cout << "Recovered " << healAmount << " health points." << std::endl;

        // Chance to recover based on effectiveness
        float recoveryChance = getEffectivenessAgainst(diseaseId);
        float roll = static_cast<float>(rand()) / RAND_MAX;

        if (roll < recoveryChance) {
            health->recoverFromDisease(diseaseId, context->worldState.daysPassed, true);
            std::cout << "The treatment was successful! You have recovered from " << diseaseId << "." << std::endl;
        } else {
            std::cout << "The treatment provided some relief, but hasn't cured you completely." << std::endl;
        }
    };
}

void HealingMethod::setEffectiveness(const std::string& diseaseId, float effectiveness)
{
    effectivenessAgainstDisease[diseaseId] = std::clamp(effectiveness, 0.0f, 1.0f);
}

float HealingMethod::getEffectivenessAgainst(const std::string& diseaseId) const
{
    auto it = effectivenessAgainstDisease.find(diseaseId);
    return (it != effectivenessAgainstDisease.end()) ? it->second : 0.0f;
}

bool HealingMethod::canBeUsed(GameContext* context) const
{
    return requirementCheck ? requirementCheck(context) : true;
}

void HealingMethod::apply(GameContext* context, const std::string& diseaseId)
{
    if (applyEffect && canBeUsed(context)) {
        applyEffect(context, diseaseId);
    }
}