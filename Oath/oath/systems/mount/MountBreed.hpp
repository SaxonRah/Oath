#pragma once

#include <map>
#include <string>

#include <nlohmann/json.hpp>

struct MountStats;

// Mount breed information
struct MountBreed {
    std::string id;
    std::string name;
    std::string description;

    // Base stats for this breed
    int baseSpeed;
    int baseStamina;
    int baseCarryCapacity;
    int baseTrainability;

    // Special characteristics
    bool naturalSwimmer;
    bool naturalJumper;
    bool naturalClimber;
    std::string specialAbility;

    // For breeding purposes
    std::map<std::string, float> traitProbabilities;

    MountBreed(const std::string& breedId, const std::string& breedName);
    static MountBreed* createFromJson(const nlohmann::json& j);
    void initializeMountStats(MountStats& stats) const;
};