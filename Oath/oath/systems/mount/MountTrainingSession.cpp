#include "MountTrainingSession.hpp"
#include "Mount.hpp"
#include "MountStats.hpp"
#include "MountSystemConfig.hpp"
#include <iostream>
#include <random>

MountTrainingSession::MountTrainingSession(Mount* targetMount, const std::string& type, int sessionDuration, int sessionDifficulty, MountSystemConfig& cfg)
    : mount(targetMount)
    , trainingType(type)
    , duration(sessionDuration)
    , difficulty(sessionDifficulty)
    , successChance(70)
    , experienceGain(5)
    , config(cfg)
{
    if (mount) {
        // Adjust success chance based on mount's current training and condition
        MountStats effectiveStats = mount->getEffectiveStats();

        // Higher training in this area increases success chance
        if (effectiveStats.specialTraining.find(trainingType) != effectiveStats.specialTraining.end()) {
            int currentTraining = effectiveStats.specialTraining.at(trainingType);
            successChance += (currentTraining / 10);
        }

        // Exhaustion and hunger decrease success chance
        if (effectiveStats.isExhausted())
            successChance -= 20;
        if (effectiveStats.isStarving())
            successChance -= 30;
        if (effectiveStats.isInjured())
            successChance -= 15;

        // Difficulty reduces success chance
        successChance -= difficulty / 5;

        // Clamp values
        if (successChance < 10)
            successChance = 10;
        if (successChance > 95)
            successChance = 95;

        // Experience gain based on difficulty
        experienceGain = 3 + (difficulty / 10);
    }
}

bool MountTrainingSession::conductTraining()
{
    if (!mount)
        return false;

    // Random success check
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 100);
    bool success = dis(gen) <= successChance;

    // Use mount stamina and increase fatigue
    mount->stats.useStamina(duration / 5);
    mount->stats.fatigue += duration / 15;
    if (mount->stats.fatigue > 100)
        mount->stats.fatigue = 100;

    // Increase hunger from exercise
    mount->stats.hunger += duration / 60;
    if (mount->stats.hunger > 100)
        mount->stats.hunger = 100;

    // If successful, improve training in this area
    if (success) {
        mount->stats.train(trainingType, experienceGain);

        // Check for ability unlocks based on the config system
        for (const auto& [abilityId, abilityInfo] : config.specialAbilities) {
            // Only check abilities related to this training type
            if (abilityInfo.trainingType == trainingType) {
                // Check if ability should be unlocked
                bool canUnlock = config.canUnlockAbility(mount->stats, abilityId);
                bool alreadyUnlocked = false;

                // Check if ability is already unlocked
                if (abilityId == "jump")
                    alreadyUnlocked = mount->stats.canJump;
                else if (abilityId == "swim")
                    alreadyUnlocked = mount->stats.canSwim;
                else if (abilityId == "climb")
                    alreadyUnlocked = mount->stats.canClimb;

                // If can unlock and not already unlocked, random chance to unlock
                if (canUnlock && !alreadyUnlocked && dis(gen) <= 10) {
                    if (abilityId == "jump") {
                        mount->stats.canJump = true;
                        std::cout << mount->name << " has learned to jump obstacles!" << std::endl;
                    } else if (abilityId == "swim") {
                        mount->stats.canSwim = true;
                        std::cout << mount->name << " has learned to swim across water!" << std::endl;
                    } else if (abilityId == "climb") {
                        mount->stats.canClimb = true;
                        std::cout << mount->name << " has learned to climb steep slopes!" << std::endl;
                    }
                }
            }
        }
    }

    return success;
}