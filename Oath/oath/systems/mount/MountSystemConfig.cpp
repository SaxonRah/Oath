#include "MountSystemConfig.hpp"
#include "MountStats.hpp"
#include <nlohmann/json.hpp>
#include <random>

SpecialAbilityInfo SpecialAbilityInfo::fromJson(const nlohmann::json& j)
{
    SpecialAbilityInfo info;
    info.id = j["id"];
    info.name = j["name"];
    info.description = j["description"];
    info.staminaCost = j["staminaCost"];
    info.skillRequired = j["skillRequired"];
    info.trainingType = j["trainingType"];
    info.unlockThreshold = j["unlockThreshold"];
    return info;
}

bool MountSystemConfig::canUnlockAbility(const MountStats& stats, const std::string& abilityId)
{
    if (specialAbilities.find(abilityId) == specialAbilities.end()) {
        return false;
    }

    const SpecialAbilityInfo& ability = specialAbilities[abilityId];

    // Check if the mount has the required training level in the appropriate skill
    if (stats.specialTraining.find(ability.trainingType) != stats.specialTraining.end()) {
        int trainingLevel = stats.specialTraining.at(ability.trainingType);
        return trainingLevel >= ability.unlockThreshold;
    }

    return false;
}

bool MountSystemConfig::canUseAbility(const MountStats& stats, const std::string& abilityId)
{
    if (specialAbilities.find(abilityId) == specialAbilities.end()) {
        return false;
    }

    const SpecialAbilityInfo& ability = specialAbilities[abilityId];

    // Check specific ability flags
    if (abilityId == "jump" && !stats.canJump)
        return false;
    if (abilityId == "swim" && !stats.canSwim)
        return false;
    if (abilityId == "climb" && !stats.canClimb)
        return false;

    // Check stamina and skill requirements
    if (stats.stamina < ability.staminaCost)
        return false;

    if (stats.specialTraining.find(ability.trainingType) != stats.specialTraining.end()) {
        int skillLevel = stats.specialTraining.at(ability.trainingType);
        return skillLevel >= ability.skillRequired;
    }

    return false;
}

std::string MountSystemConfig::getRandomColor()
{
    if (colors.empty()) {
        return "Brown"; // Default fallback
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, colors.size() - 1);
    return colors[dist(gen)];
}

int MountSystemConfig::getAbilityStaminaCost(const std::string& abilityId)
{
    if (specialAbilities.find(abilityId) != specialAbilities.end()) {
        return specialAbilities[abilityId].staminaCost;
    }
    return 0; // Default if ability not found
}