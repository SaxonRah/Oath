#include "Mount.hpp"
#include "MountBreed.hpp"
#include "MountEquipment.hpp"
#include "MountStats.hpp"
#include "MountSystemConfig.hpp"
#include <cstdlib>
#include <nlohmann/json.hpp>
#include <sstream>


Mount::Mount(const std::string& mountId, const std::string& mountName, MountBreed* mountBreed)
    : id(mountId)
    , name(mountName)
    , breed(mountBreed)
    , age(36) // 3 years old by default
    , isOwned(false)
    , isStabled(false)
    , isSummoned(false)
    , isMounted(false)
{
    if (breed) {
        breed->initializeMountStats(stats);
    }
}

Mount* Mount::createFromTemplate(const nlohmann::json& templateJson, MountBreed* breed, MountSystemConfig& config)
{
    if (!breed) {
        std::cerr << "Cannot create mount: no breed specified" << std::endl;
        return nullptr;
    }

    std::string name = templateJson["name"];
    std::string mountId = breed->id + "_" + name + "_" + std::to_string(std::rand());

    Mount* mount = new Mount(mountId, name, breed);

    // Set color
    if (templateJson.contains("color")) {
        mount->color = templateJson["color"];
    } else {
        mount->color = config.getRandomColor();
    }

    // Set age
    if (templateJson.contains("age")) {
        mount->age = templateJson["age"];
    }

    // Initialize stats
    if (templateJson.contains("trainingLevel")) {
        mount->stats.training = templateJson["trainingLevel"];
    }

    // Apply specific training if specified
    if (templateJson.contains("specialTraining") && templateJson["specialTraining"].is_object()) {
        for (auto& [type, level] : templateJson["specialTraining"].items()) {
            mount->stats.specialTraining[type] = level;
        }
    }

    // Enable special abilities if specified
    if (templateJson.contains("specialAbilities") && templateJson["specialAbilities"].is_array()) {
        for (const auto& ability : templateJson["specialAbilities"]) {
            std::string abilityId = ability;
            if (abilityId == "jump")
                mount->stats.canJump = true;
            else if (abilityId == "swim")
                mount->stats.canSwim = true;
            else if (abilityId == "climb")
                mount->stats.canClimb = true;
        }
    }

    return mount;
}

MountStats Mount::getEffectiveStats() const
{
    MountStats effectiveStats = stats;

    // Apply equipment modifiers
    for (const auto& [slot, equipment] : equippedItems) {
        if (equipment) {
            equipment->applyModifiers(effectiveStats);
        }
    }

    return effectiveStats;
}

bool Mount::equipItem(MountEquipment* equipment)
{
    if (!equipment)
        return false;

    // Replace any existing equipment in this slot
    equippedItems[equipment->slot] = equipment;
    return true;
}

MountEquipment* Mount::unequipItem(MountEquipmentSlot slot)
{
    if (equippedItems.find(slot) != equippedItems.end()) {
        MountEquipment* removed = equippedItems[slot];
        equippedItems.erase(slot);
        return removed;
    }
    return nullptr;
}

void Mount::update(int minutes)
{
    // If stabled, recover faster and consume less
    if (isStabled) {
        stats.rest(minutes * 2); // Twice the rest effectiveness
        stats.hunger += minutes / 240; // Half the hunger increase
    }
    // If active, consume resources faster
    else if (isSummoned || isMounted) {
        stats.hunger += minutes / 60; // Normal hunger while active

        // Mounted state uses more stamina
        if (isMounted) {
            stats.useStamina(minutes / 30); // Light stamina use just from being ridden
        }

        // Natural stamina regeneration if not exhausted
        if (!stats.isExhausted() && stats.stamina < stats.maxStamina) {
            stats.stamina += (stats.staminaRegen * minutes) / 120; // Half rate when active
            if (stats.stamina > stats.maxStamina)
                stats.stamina = stats.maxStamina;
        }
    }
    // Idle state (owned but not summoned or stabled)
    else if (isOwned) {
        stats.rest(minutes);
        stats.hunger += minutes / 180; // Lower hunger while idle
    }

    // Cap values
    if (stats.hunger > 100)
        stats.hunger = 100;
    if (stats.fatigue > 100)
        stats.fatigue = 100;

    // Effects of starvation
    if (stats.isStarving()) {
        stats.health -= minutes / 60; // Lose health if starving
        if (stats.health < 0)
            stats.health = 0;
    }

    // Natural health recovery if fed and rested
    if (!stats.isStarving() && !stats.isExhausted() && stats.health < stats.maxHealth) {
        stats.health += minutes / 240; // Slow natural healing
        if (stats.health > stats.maxHealth)
            stats.health = stats.maxHealth;
    }

    // Damage equipment if mounted
    if (isMounted) {
        for (auto& [slot, equipment] : equippedItems) {
            if (equipment) {
                equipment->use(minutes / 120); // Light wear and tear over time
            }
        }
    }
}

float Mount::getTravelTimeModifier() const
{
    if (!isMounted)
        return 1.0f; // No change if not mounted

    MountStats effectiveStats = getEffectiveStats();
    int effectiveSpeed = effectiveStats.getEffectiveSpeed();

    // 100 is standard human walking speed
    return 100.0f / static_cast<float>(effectiveSpeed);
}

bool Mount::useSpecialAbility(const std::string& ability, const MountSystemConfig& config)
{
    // Check if ability exists in the system
    if (config.specialAbilities.find(ability) == config.specialAbilities.end()) {
        return false;
    }

    // Get ability info
    const SpecialAbilityInfo& abilityInfo = config.specialAbilities.at(ability);

    // Check if mount can use this ability
    bool canPerform = config.canUseAbility(stats, ability);

    if (!canPerform) {
        return false;
    }

    // Use stamina
    return stats.useStamina(abilityInfo.staminaCost);
}

std::string Mount::getStateDescription() const
{
    std::stringstream ss;

    ss << name << " (" << breed->name << "): ";

    // Health and condition
    if (stats.health <= 0) {
        ss << "Dead";
        return ss.str();
    }

    if (stats.health < 20)
        ss << "Severely injured, ";
    else if (stats.health < 50)
        ss << "Injured, ";

    if (stats.isStarving())
        ss << "Starving, ";
    else if (stats.hunger > 70)
        ss << "Very hungry, ";
    else if (stats.hunger > 40)
        ss << "Hungry, ";

    if (stats.isExhausted())
        ss << "Exhausted, ";
    else if (stats.fatigue > 70)
        ss << "Very tired, ";
    else if (stats.fatigue > 40)
        ss << "Tired, ";

    // Location/status
    if (isStabled)
        ss << "Stabled";
    else if (isMounted)
        ss << "Being ridden";
    else if (isSummoned)
        ss << "Following you";
    else if (isOwned)
        ss << "Waiting at camp";
    else
        ss << "Wild";

    return ss.str();
}