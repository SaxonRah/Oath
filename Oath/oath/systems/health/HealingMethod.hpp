// /oath/systems/health/HealingMethod.hpp
#pragma once

#include "../../data/GameContext.hpp"
#include <functional>
#include <map>
#include <nlohmann/json.hpp>
#include <string>

// Class to represent a healing method
class HealingMethod {
public:
    std::string id;
    std::string name;
    std::string description;
    std::map<std::string, float> effectivenessAgainstDisease; // Disease ID to effectiveness (0.0-1.0)
    std::function<bool(GameContext*)> requirementCheck; // Check if the method can be used
    std::function<void(GameContext*, const std::string&)> applyEffect; // Apply healing effects
    float healAmount;
    bool requiresItem;
    std::string requiredItem;
    int requiredAmount;
    bool requiresLocation;
    std::string requiredLocationType;
    std::map<std::string, int> costs; // Disease ID to cost

    HealingMethod(const std::string& methodId, const std::string& methodName);

    // Constructor from JSON
    HealingMethod(const nlohmann::json& methodJson);

    void setupRequirementCheck();
    void setupApplyEffect();
    void setEffectiveness(const std::string& diseaseId, float effectiveness);
    float getEffectivenessAgainst(const std::string& diseaseId) const;
    bool canBeUsed(GameContext* context) const;
    void apply(GameContext* context, const std::string& diseaseId);
};