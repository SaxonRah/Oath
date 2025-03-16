#pragma once

#include <map>
#include <string>
#include <vector>

struct MountStats;
struct SpecialAbilityInfo;

// Struct to hold global mount system configuration
struct MountSystemConfig {
    // Map of all special abilities by ID
    std::map<std::string, SpecialAbilityInfo> specialAbilities;

    // Training types
    std::vector<std::pair<std::string, std::string>> trainingTypes;

    // Available mount colors
    std::vector<std::string> colors;

    bool canUnlockAbility(const MountStats& stats, const std::string& abilityId);
    bool canUseAbility(const MountStats& stats, const std::string& abilityId);
    std::string getRandomColor();
    int getAbilityStaminaCost(const std::string& abilityId);
};

// Structure to hold special ability information
struct SpecialAbilityInfo {
    std::string id;
    std::string name;
    std::string description;
    int staminaCost;
    int skillRequired;
    std::string trainingType;
    int unlockThreshold;

    static SpecialAbilityInfo fromJson(const nlohmann::json& j);
};