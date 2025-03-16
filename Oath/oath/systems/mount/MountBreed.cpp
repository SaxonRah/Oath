#include "MountBreed.hpp"
#include "MountStats.hpp"
#include <nlohmann/json.hpp>

MountBreed::MountBreed(const std::string& breedId, const std::string& breedName)
    : id(breedId)
    , name(breedName)
    , baseSpeed(100)
    , baseStamina(100)
    , baseCarryCapacity(50)
    , baseTrainability(50)
    , naturalSwimmer(false)
    , naturalJumper(false)
    , naturalClimber(false)
{
}

MountBreed* MountBreed::createFromJson(const nlohmann::json& j)
{
    std::string id = j["id"];
    std::string name = j["name"];

    MountBreed* breed = new MountBreed(id, name);
    breed->description = j.value("description", "");
    breed->baseSpeed = j.value("baseSpeed", 100);
    breed->baseStamina = j.value("baseStamina", 100);
    breed->baseCarryCapacity = j.value("baseCarryCapacity", 50);
    breed->baseTrainability = j.value("baseTrainability", 50);
    breed->naturalSwimmer = j.value("naturalSwimmer", false);
    breed->naturalJumper = j.value("naturalJumper", false);
    breed->naturalClimber = j.value("naturalClimber", false);
    breed->specialAbility = j.value("specialAbility", "");

    // Load trait probabilities if present
    if (j.contains("traitProbabilities") && j["traitProbabilities"].is_object()) {
        for (auto& [trait, probability] : j["traitProbabilities"].items()) {
            breed->traitProbabilities[trait] = probability;
        }
    }

    return breed;
}

void MountBreed::initializeMountStats(MountStats& stats) const
{
    stats.speed = baseSpeed;
    stats.maxStamina = baseStamina;
    stats.stamina = baseStamina;
    stats.carryCapacity = baseCarryCapacity;

    // Set natural abilities
    stats.canSwim = naturalSwimmer;
    stats.canJump = naturalJumper;
    stats.canClimb = naturalClimber;

    // Base trainability affects how quickly skills improve
    int trainablityBonus = (baseTrainability - 50) / 10;
    for (auto& [type, level] : stats.specialTraining) {
        level = 10 + trainablityBonus; // Start with slight bonus based on breed
    }
}