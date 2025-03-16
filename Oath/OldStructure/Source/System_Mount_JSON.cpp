// System_Mount_JSON.cpp

#include "System_Mount_JSON.h"

#include <algorithm>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <queue>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

// Forward declarations
class TANode;
class TAController;
class Inventory;
class GameContext;
class NodeID;
struct TAInput;
struct TAAction;
struct TATransitionRule;
struct WorldState;

//----------------------------------------
// MOUNT SYSTEM
//----------------------------------------

// Base mount stats structure
struct MountStats {
    int stamina = 100; // Base energy for running/special abilities
    int maxStamina = 100; // Maximum stamina capacity
    int staminaRegen = 5; // How much stamina regenerates per time unit
    int speed = 100; // Base movement speed (100 = normal human walking)
    int carryCapacity = 50; // Additional weight the mount can carry
    int loyalty = 50; // How loyal/responsive the mount is (affects control)
    int training = 0; // Training level for special abilities

    // Track the mount's condition
    int hunger = 0; // 0-100, increases over time
    int fatigue = 0; // 0-100, increases with activity
    int health = 100; // Current health points
    int maxHealth = 100; // Maximum health

    // Special ability flags
    bool canJump = false; // Can jump ravines/obstacles
    bool canSwim = false; // Can cross rivers/lakes
    bool canClimb = false; // Can climb steep slopes

    // Training specialties
    std::map<std::string, int> specialTraining; // Combat, racing, etc.

    MountStats()
    {
        // Initialize specialties from defaults - will be overridden by JSON
        specialTraining["combat"] = 0; // Combat training
        specialTraining["endurance"] = 0; // Long distance travel
        specialTraining["agility"] = 0; // Jumps and difficult terrain
        specialTraining["racing"] = 0; // Speed and bursts
    }

    // Load from JSON
    void loadFromJson(const json& j)
    {
        stamina = j.value("stamina", 100);
        maxStamina = j.value("maxStamina", 100);
        staminaRegen = j.value("staminaRegen", 5);
        speed = j.value("speed", 100);
        carryCapacity = j.value("carryCapacity", 50);
        loyalty = j.value("loyalty", 50);
        training = j.value("training", 0);
        hunger = j.value("hunger", 0);
        fatigue = j.value("fatigue", 0);
        health = j.value("health", 100);
        maxHealth = j.value("maxHealth", 100);
        canJump = j.value("canJump", false);
        canSwim = j.value("canSwim", false);
        canClimb = j.value("canClimb", false);

        // Load any specialty training defined in JSON
        if (j.contains("specialTraining") && j["specialTraining"].is_object()) {
            for (auto& [key, value] : j["specialTraining"].items()) {
                specialTraining[key] = value;
            }
        }
    }

    bool isExhausted() const
    {
        return stamina <= 10 || fatigue >= 90;
    }

    bool isStarving() const
    {
        return hunger >= 90;
    }

    bool isInjured() const
    {
        return health <= maxHealth / 2;
    }

    // Calculate effective speed based on condition
    int getEffectiveSpeed() const
    {
        float multiplier = 1.0f;

        // Reduce speed if tired or hungry
        if (fatigue > 50)
            multiplier -= (fatigue - 50) * 0.01f;
        if (hunger > 50)
            multiplier -= (hunger - 50) * 0.01f;
        if (health < maxHealth / 2)
            multiplier -= (1.0f - (float)health / maxHealth) * 0.5f;

        // Apply minimum multiplier
        if (multiplier < 0.3f)
            multiplier = 0.3f;

        return static_cast<int>(speed * multiplier);
    }

    // Consume stamina for an action
    bool useStamina(int amount)
    {
        if (stamina < amount)
            return false;

        stamina -= amount;
        fatigue += amount / 5; // Increase fatigue based on exertion
        if (fatigue > 100)
            fatigue = 100;

        return true;
    }

    // Rest the mount to recover stamina
    void rest(int minutes)
    {
        // Recover stamina
        stamina += (staminaRegen * minutes) / 60;
        if (stamina > maxStamina)
            stamina = maxStamina;

        // Reduce fatigue
        fatigue -= minutes / 15; // 4 fatigue points per hour of rest
        if (fatigue < 0)
            fatigue = 0;

        // Increase hunger during rest
        hunger += minutes / 120; // 0.5 hunger per hour of rest
        if (hunger > 100)
            hunger = 100;
    }

    // Feed the mount
    void feed(int nutritionValue)
    {
        hunger -= nutritionValue;
        if (hunger < 0)
            hunger = 0;

        // Slight health recovery from good feeding
        health += nutritionValue / 10;
        if (health > maxHealth)
            health = maxHealth;
    }

    // Apply training in a specific area
    void train(const std::string& area, int amount)
    {
        if (specialTraining.find(area) != specialTraining.end()) {
            specialTraining[area] += amount;
            if (specialTraining[area] > 100)
                specialTraining[area] = 100;
        }

        // Overall training level is average of all specialties
        int totalTraining = 0;
        for (const auto& [_, value] : specialTraining) {
            totalTraining += value;
        }
        training = totalTraining / specialTraining.size();

        // Unlocking abilities is now handled by SpecialAbilityInfo
    }
};

// Mount equipment slot types
enum class MountEquipmentSlot {
    Saddle,
    Armor,
    Bags,
    Shoes,
    Accessory
};

// Helper function to convert string to MountEquipmentSlot
MountEquipmentSlot stringToSlot(const std::string& slotStr)
{
    if (slotStr == "Saddle")
        return MountEquipmentSlot::Saddle;
    if (slotStr == "Armor")
        return MountEquipmentSlot::Armor;
    if (slotStr == "Bags")
        return MountEquipmentSlot::Bags;
    if (slotStr == "Shoes")
        return MountEquipmentSlot::Shoes;
    if (slotStr == "Accessory")
        return MountEquipmentSlot::Accessory;

    // Default
    std::cerr << "Unknown equipment slot: " << slotStr << std::endl;
    return MountEquipmentSlot::Saddle;
}

// Helper function to convert MountEquipmentSlot to string
std::string slotToString(MountEquipmentSlot slot)
{
    switch (slot) {
    case MountEquipmentSlot::Saddle:
        return "Saddle";
    case MountEquipmentSlot::Armor:
        return "Armor";
    case MountEquipmentSlot::Bags:
        return "Bags";
    case MountEquipmentSlot::Shoes:
        return "Shoes";
    case MountEquipmentSlot::Accessory:
        return "Accessory";
    default:
        return "Unknown";
    }
}

// Mount equipment item
struct MountEquipment {
    std::string id;
    std::string name;
    std::string description;
    MountEquipmentSlot slot;
    int quality; // 1-100
    int durability;
    int maxDurability;
    int price;
    std::map<std::string, int> statModifiers; // Speed, stamina, etc.

    MountEquipment(const std::string& itemId, const std::string& itemName, MountEquipmentSlot itemSlot)
        : id(itemId)
        , name(itemName)
        , slot(itemSlot)
        , quality(50)
        , durability(100)
        , maxDurability(100)
        , price(100)
    {
    }

    // Create from JSON
    static MountEquipment* createFromJson(const json& j)
    {
        std::string id = j["id"];
        std::string name = j["name"];
        MountEquipmentSlot slot = stringToSlot(j["slot"]);

        MountEquipment* equipment = new MountEquipment(id, name, slot);
        equipment->description = j.value("description", "");
        equipment->quality = j.value("quality", 50);
        equipment->durability = j.value("durability", 100);
        equipment->maxDurability = j.value("maxDurability", 100);
        equipment->price = j.value("price", 100);

        // Load stat modifiers
        if (j.contains("statModifiers") && j["statModifiers"].is_object()) {
            for (auto& [stat, modifier] : j["statModifiers"].items()) {
                equipment->statModifiers[stat] = modifier;
            }
        }

        return equipment;
    }

    bool isWorn() const
    {
        return durability < maxDurability / 5;
    }

    // Apply this equipment's modifiers to mount stats
    void applyModifiers(MountStats& stats) const
    {
        for (const auto& [stat, modifier] : statModifiers) {
            if (stat == "speed")
                stats.speed += modifier;
            else if (stat == "stamina")
                stats.maxStamina += modifier;
            else if (stat == "carryCapacity")
                stats.carryCapacity += modifier;
            else if (stat == "staminaRegen")
                stats.staminaRegen += modifier;
        }
    }

    // Damage equipment through use
    void use(int intensity = 1)
    {
        durability -= intensity;
        if (durability < 0)
            durability = 0;
    }

    // Repair equipment
    void repair(int amount)
    {
        durability += amount;
        if (durability > maxDurability)
            durability = maxDurability;
    }
};

// Structure to hold special ability information
struct SpecialAbilityInfo {
    std::string id;
    std::string name;
    std::string description;
    int staminaCost;
    int skillRequired;
    std::string trainingType;
    int unlockThreshold; // Training level needed to unlock this ability

    static SpecialAbilityInfo fromJson(const json& j)
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
};

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

    MountBreed(const std::string& breedId, const std::string& breedName)
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

    // Create from JSON
    static MountBreed* createFromJson(const json& j)
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

    // Initialize a new mount with this breed's characteristics
    void initializeMountStats(MountStats& stats) const
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
};

// Struct to hold global mount system configuration
struct MountSystemConfig {
    // Map of all special abilities by ID
    std::map<std::string, SpecialAbilityInfo> specialAbilities;

    // Training types
    std::vector<std::pair<std::string, std::string>> trainingTypes;

    // Available mount colors
    std::vector<std::string> colors;

    // Calculate if a mount can unlock a special ability based on training
    bool canUnlockAbility(const MountStats& stats, const std::string& abilityId)
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

    // Check if a mount has the requirements to use an ability
    bool canUseAbility(const MountStats& stats, const std::string& abilityId)
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

    // Get a random color from the available colors
    std::string getRandomColor()
    {
        if (colors.empty()) {
            return "Brown"; // Default fallback
        }

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(0, colors.size() - 1);
        return colors[dist(gen)];
    }

    // Get the stamina cost for using an ability
    int getAbilityStaminaCost(const std::string& abilityId)
    {
        if (specialAbilities.find(abilityId) != specialAbilities.end()) {
            return specialAbilities[abilityId].staminaCost;
        }
        return 0; // Default if ability not found
    }
};

// Complete mount class including stats, equipment, and state
class Mount {
public:
    std::string id;
    std::string name;
    MountBreed* breed;
    MountStats stats;
    int age; // In months
    std::string color;

    // Current state
    bool isOwned;
    bool isStabled;
    bool isSummoned; // Currently following player
    bool isMounted; // Player is riding

    // Equipped items
    std::map<MountEquipmentSlot, MountEquipment*> equippedItems;

    Mount(const std::string& mountId, const std::string& mountName, MountBreed* mountBreed)
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

    // Create from JSON template
    static Mount* createFromTemplate(const json& templateJson, MountBreed* breed, MountSystemConfig& config)
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

    // Apply all equipment modifiers to get effective stats
    MountStats getEffectiveStats() const
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

    // Equip an item in the appropriate slot
    bool equipItem(MountEquipment* equipment)
    {
        if (!equipment)
            return false;

        // Replace any existing equipment in this slot
        equippedItems[equipment->slot] = equipment;
        return true;
    }

    // Remove equipment from a slot
    MountEquipment* unequipItem(MountEquipmentSlot slot)
    {
        if (equippedItems.find(slot) != equippedItems.end()) {
            MountEquipment* removed = equippedItems[slot];
            equippedItems.erase(slot);
            return removed;
        }
        return nullptr;
    }

    // Update mount state over time
    void update(int minutes)
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

    // Calculate travel time modifier (how much faster/slower than on foot)
    float getTravelTimeModifier() const
    {
        if (!isMounted)
            return 1.0f; // No change if not mounted

        MountStats effectiveStats = getEffectiveStats();
        int effectiveSpeed = effectiveStats.getEffectiveSpeed();

        // 100 is standard human walking speed
        return 100.0f / static_cast<float>(effectiveSpeed);
    }

    // Try to use a special ability with config-based settings
    bool useSpecialAbility(const std::string& ability, const MountSystemConfig& config)
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

    // Get descriptive state of the mount
    std::string getStateDescription() const
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
};

// Mount stable location for managing multiple mounts
class MountStable {
public:
    std::string id;
    std::string name;
    std::string location;
    int capacity;
    int dailyFeedCost;
    int dailyCareCost;

    std::vector<Mount*> stabledMounts;
    std::vector<Mount*> availableForPurchase;

    MountStable(const std::string& stableId, const std::string& stableName, const std::string& stableLocation, int stableCapacity = 5)
        : id(stableId)
        , name(stableName)
        , location(stableLocation)
        , capacity(stableCapacity)
        , dailyFeedCost(5)
        , dailyCareCost(3)
    {
    }

    // Create from JSON
    static MountStable* createFromJson(const json& j, std::map<std::string, MountBreed*>& breedTypes, MountSystemConfig& config)
    {
        std::string id = j["id"];
        std::string name = j["name"];
        std::string location = j["location"];
        int capacity = j.value("capacity", 5);

        MountStable* stable = new MountStable(id, name, location, capacity);
        stable->dailyFeedCost = j.value("dailyFeedCost", 5);
        stable->dailyCareCost = j.value("dailyCareCost", 3);

        // Create mounts available for purchase
        if (j.contains("availableMounts") && j["availableMounts"].is_array()) {
            for (const auto& mountTemplate : j["availableMounts"]) {
                std::string breedId = mountTemplate["templateId"];

                // Find the breed
                if (breedTypes.find(breedId) != breedTypes.end()) {
                    MountBreed* breed = breedTypes[breedId];
                    Mount* mount = Mount::createFromTemplate(mountTemplate, breed, config);

                    if (mount) {
                        stable->availableForPurchase.push_back(mount);
                    }
                }
            }
        }

        return stable;
    }

    bool hasSpace() const
    {
        return stabledMounts.size() < capacity;
    }

    bool stableMount(Mount* mount)
    {
        if (!mount || !hasSpace()) {
            return false;
        }

        // Update mount state
        mount->isStabled = true;
        mount->isSummoned = false;
        mount->isMounted = false;

        // Add to stabled mounts
        stabledMounts.push_back(mount);
        return true;
    }

    Mount* retrieveMount(const std::string& mountId)
    {
        auto it = std::find_if(stabledMounts.begin(), stabledMounts.end(),
            [&mountId](Mount* m) { return m->id == mountId; });

        if (it != stabledMounts.end()) {
            Mount* mount = *it;
            stabledMounts.erase(it);
            mount->isStabled = false;
            return mount;
        }

        return nullptr;
    }

    // Calculate daily cost for all stabled mounts
    int getDailyCost() const
    {
        return stabledMounts.size() * (dailyFeedCost + dailyCareCost);
    }

    // Care for all stabled mounts (call daily)
    void provideDailyCare()
    {
        for (Mount* mount : stabledMounts) {
            if (mount) {
                // Feed and care for mount
                mount->stats.feed(50); // Good daily feeding
                mount->stats.rest(480); // 8 hours of good rest

                // Slight health recovery from stable care
                mount->stats.health += 5;
                if (mount->stats.health > mount->stats.maxHealth) {
                    mount->stats.health = mount->stats.maxHealth;
                }

                // Repair equipment slightly
                for (auto& [slot, equipment] : mount->equippedItems) {
                    if (equipment) {
                        equipment->repair(1);
                    }
                }
            }
        }
    }
};

// Mount training session for improving mount abilities
class MountTrainingSession {
public:
    Mount* mount;
    std::string trainingType;
    int duration; // in minutes
    int difficulty; // 1-100
    int successChance;
    int experienceGain;
    MountSystemConfig& config;

    MountTrainingSession(Mount* targetMount, const std::string& type, int sessionDuration, int sessionDifficulty, MountSystemConfig& cfg)
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

    bool conductTraining()
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
};

// Mount system node types for the RPG game

// Mount stable interaction node
class MountStableNode : public TANode {
public:
    MountStable* stable;

    MountStableNode(const std::string& name, MountStable* targetStable)
        : TANode(name)
        , stable(targetStable)
    {
    }

    void onEnter(GameContext* context) override
    {
        std::cout << "Welcome to " << stable->name << " Stables!" << std::endl;
        std::cout << "Currently housing " << stable->stabledMounts.size() << " of "
                  << stable->capacity << " available stalls." << std::endl;

        // List stabled mounts
        if (!stable->stabledMounts.empty()) {
            std::cout << "\nYour stabled mounts:" << std::endl;
            for (size_t i = 0; i < stable->stabledMounts.size(); i++) {
                Mount* mount = stable->stabledMounts[i];
                std::cout << i + 1 << ". " << mount->getStateDescription() << std::endl;
            }
            std::cout << "\nDaily care cost: " << stable->getDailyCost() << " gold" << std::endl;
        }

        // List mounts for sale
        if (!stable->availableForPurchase.empty()) {
            std::cout << "\nMounts available for purchase:" << std::endl;
            for (size_t i = 0; i < stable->availableForPurchase.size(); i++) {
                Mount* mount = stable->availableForPurchase[i];
                int price = calculatePrice(mount);
                std::cout << i + 1 << ". " << mount->name << " (" << mount->breed->name
                          << ") - " << price << " gold" << std::endl;
            }
        }
    }

    // Calculate price based on mount quality
    int calculatePrice(Mount* mount) const
    {
        if (!mount)
            return 0;

        int basePrice = 200; // Base price for any mount

        // Adjust for breed
        if (mount->breed) {
            basePrice += (mount->breed->baseSpeed - 100) * 2;
            basePrice += (mount->breed->baseStamina - 100) * 2;
            basePrice += (mount->breed->baseTrainability - 50) * 3;

            // Premium for special abilities
            if (mount->breed->naturalSwimmer)
                basePrice += 100;
            if (mount->breed->naturalJumper)
                basePrice += 100;
            if (mount->breed->naturalClimber)
                basePrice += 200;
        }

        // Adjust for training level
        basePrice += mount->stats.training * 5;

        // Adjust for special abilities
        if (mount->stats.canJump)
            basePrice += 50;
        if (mount->stats.canSwim)
            basePrice += 50;
        if (mount->stats.canClimb)
            basePrice += 100;

        // Adjust for age (prime age is worth more)
        int ageModifier = 0;
        if (mount->age < 24) { // Young
            ageModifier = -50;
        } else if (mount->age > 120) { // Old
            ageModifier = -100;
        } else if (mount->age >= 36 && mount->age <= 84) { // Prime age
            ageModifier = 50;
        }
        basePrice += ageModifier;

        return basePrice;
    }

    std::vector<TAAction> getAvailableActions() override
    {
        std::vector<TAAction> actions = TANode::getAvailableActions();

        // Add stable-specific actions

        // Retrieve a mount action
        if (!stable->stabledMounts.empty()) {
            actions.push_back({ "retrieve_mount", "Retrieve a mount from the stable",
                [this]() -> TAInput {
                    return { "stable_action", { { "action", std::string("retrieve") } } };
                } });
        }

        // Buy a mount action
        if (!stable->availableForPurchase.empty()) {
            actions.push_back({ "buy_mount", "Purchase a new mount",
                [this]() -> TAInput {
                    return { "stable_action", { { "action", std::string("buy") } } };
                } });
        }

        // Stable a mount action (if player has an active mount)
        actions.push_back({ "stable_mount", "Leave your mount at the stable",
            [this]() -> TAInput {
                return { "stable_action", { { "action", std::string("stable") } } };
            } });

        // Train mount action
        if (!stable->stabledMounts.empty()) {
            actions.push_back({ "train_mount", "Train one of your stabled mounts",
                [this]() -> TAInput {
                    return { "stable_action", { { "action", std::string("train") } } };
                } });
        }

        // Exit action
        actions.push_back({ "exit_stable", "Leave the stable",
            [this]() -> TAInput {
                return { "stable_action", { { "action", std::string("exit") } } };
            } });

        return actions;
    }

    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override
    {
        if (input.type == "stable_action") {
            std::string action = std::get<std::string>(input.parameters.at("action"));

            // Handle different stable actions
            if (action == "exit") {
                // Find the exit transition
                for (const auto& rule : transitionRules) {
                    if (rule.description == "Exit") {
                        outNextNode = rule.targetNode;
                        return true;
                    }
                }
            }
            // Other actions would be processed in the game logic
            // and would likely stay in the same node
            outNextNode = this;
            return true;
        }

        return TANode::evaluateTransition(input, outNextNode);
    }
};

// Mount interaction node - for when actively using a mount
class MountInteractionNode : public TANode {
public:
    Mount* activeMount;
    MountSystemConfig* config;

    MountInteractionNode(const std::string& name, Mount* mount = nullptr, MountSystemConfig* cfg = nullptr)
        : TANode(name)
        , activeMount(mount)
        , config(cfg)
    {
    }

    void setActiveMount(Mount* mount)
    {
        activeMount = mount;
    }

    void setConfig(MountSystemConfig* cfg)
    {
        config = cfg;
    }

    void onEnter(GameContext* context) override
    {
        if (!activeMount) {
            std::cout << "You don't currently have an active mount." << std::endl;
            return;
        }

        std::cout << "Interacting with " << activeMount->name << std::endl;
        std::cout << activeMount->getStateDescription() << std::endl;

        // Show mount stats
        MountStats effectiveStats = activeMount->getEffectiveStats();
        std::cout << "\nStats:" << std::endl;
        std::cout << "Health: " << effectiveStats.health << "/" << effectiveStats.maxHealth << std::endl;
        std::cout << "Stamina: " << effectiveStats.stamina << "/" << effectiveStats.maxStamina << std::endl;
        std::cout << "Speed: " << effectiveStats.speed << " (" << effectiveStats.getEffectiveSpeed() << " effective)" << std::endl;
        std::cout << "Hunger: " << effectiveStats.hunger << "/100" << std::endl;
        std::cout << "Fatigue: " << effectiveStats.fatigue << "/100" << std::endl;
        std::cout << "Carry Capacity: " << effectiveStats.carryCapacity << " additional units" << std::endl;

        // Show training levels
        std::cout << "\nTraining:" << std::endl;
        for (const auto& [skill, level] : effectiveStats.specialTraining) {
            std::cout << skill << ": " << level << "/100" << std::endl;
        }

        // Show special abilities
        std::cout << "\nSpecial Abilities:" << std::endl;
        if (effectiveStats.canJump)
            std::cout << "- Can jump over obstacles" << std::endl;
        if (effectiveStats.canSwim)
            std::cout << "- Can swim across water" << std::endl;
        if (effectiveStats.canClimb)
            std::cout << "- Can climb steep slopes" << std::endl;

        // Show equipped items
        std::cout << "\nEquipment:" << std::endl;
        if (activeMount->equippedItems.empty()) {
            std::cout << "No equipment" << std::endl;
        } else {
            for (const auto& [slot, equipment] : activeMount->equippedItems) {
                if (equipment) {
                    std::cout << slotToString(slot) << ": " << equipment->name;
                    if (equipment->isWorn())
                        std::cout << " (Worn)";
                    std::cout << " - " << equipment->durability << "/" << equipment->maxDurability << std::endl;
                }
            }
        }
    }

    std::vector<TAAction> getAvailableActions() override
    {
        std::vector<TAAction> actions = TANode::getAvailableActions();

        if (!activeMount) {
            actions.push_back({ "summon_mount", "Summon one of your mounts",
                [this]() -> TAInput {
                    return { "mount_action", { { "action", std::string("summon") } } };
                } });
            return actions;
        }

        // Mount/dismount
        if (activeMount->isMounted) {
            actions.push_back({ "dismount", "Dismount",
                [this]() -> TAInput {
                    return { "mount_action", { { "action", std::string("dismount") } } };
                } });
        } else {
            actions.push_back({ "mount", "Mount",
                [this]() -> TAInput {
                    return { "mount_action", { { "action", std::string("mount") } } };
                } });
        }

        // Feed mount
        actions.push_back({ "feed_mount", "Feed your mount",
            [this]() -> TAInput {
                return { "mount_action", { { "action", std::string("feed") } } };
            } });

        // Rest mount
        actions.push_back({ "rest_mount", "Allow your mount to rest",
            [this]() -> TAInput {
                return { "mount_action", { { "action", std::string("rest") } } };
            } });

        // Groom mount (improves relationship)
        actions.push_back({ "groom_mount", "Groom your mount",
            [this]() -> TAInput {
                return { "mount_action", { { "action", std::string("groom") } } };
            } });

        // Manage equipment
        actions.push_back({ "manage_equipment", "Manage mount equipment",
            [this]() -> TAInput {
                return { "mount_action", { { "action", std::string("equipment") } } };
            } });

        // Special abilities (if mounted)
        if (activeMount->isMounted && config) {
            MountStats effectiveStats = activeMount->getEffectiveStats();

            // Add actions for abilities the mount can use
            for (const auto& [abilityId, abilityInfo] : config->specialAbilities) {
                bool canUse = config->canUseAbility(effectiveStats, abilityId);

                if (canUse) {
                    actions.push_back({ abilityId, abilityInfo.name,
                        [this, abilityId]() -> TAInput {
                            return { "mount_action", { { "action", std::string("ability") }, { "ability", abilityId } } };
                        } });
                }
            }
        }

        // Dismiss mount (send back to stable or camp)
        if (activeMount->isSummoned) {
            actions.push_back({ "dismiss_mount", "Dismiss your mount",
                [this]() -> TAInput {
                    return { "mount_action", { { "action", std::string("dismiss") } } };
                } });
        }

        // Exit interaction
        actions.push_back({ "exit_interaction", "Stop interacting with mount",
            [this]() -> TAInput {
                return { "mount_action", { { "action", std::string("exit") } } };
            } });

        return actions;
    }

    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override
    {
        if (input.type == "mount_action") {
            std::string action = std::get<std::string>(input.parameters.at("action"));

            // Handle different mount actions
            if (action == "exit") {
                // Find the exit transition
                for (const auto& rule : transitionRules) {
                    if (rule.description == "Exit") {
                        outNextNode = rule.targetNode;
                        return true;
                    }
                }
            } else if (action == "ability" && activeMount && config) {
                // Handle special abilities
                std::string ability = std::get<std::string>(input.parameters.at("ability"));

                bool success = activeMount->useSpecialAbility(ability, *config);

                if (success) {
                    std::cout << activeMount->name << " successfully performs "
                              << config->specialAbilities[ability].name << "!" << std::endl;
                } else {
                    std::cout << activeMount->name << " is unable to perform "
                              << config->specialAbilities[ability].name << " right now." << std::endl;
                }

                outNextNode = this;
                return true;
            }

            // Most actions would be processed in the game logic
            // and would likely stay in the same node
            outNextNode = this;
            return true;
        }

        return TANode::evaluateTransition(input, outNextNode);
    }
};

// Mount training node - for dedicated training sessions
class MountTrainingNode : public TANode {
public:
    Mount* trainingMount;
    std::vector<std::string> trainingTypes;
    MountSystemConfig* config;

    MountTrainingNode(const std::string& name, Mount* mount = nullptr, MountSystemConfig* cfg = nullptr)
        : TANode(name)
        , trainingMount(mount)
        , config(cfg)
    {
    }

    void setTrainingMount(Mount* mount)
    {
        trainingMount = mount;
    }

    void setConfig(MountSystemConfig* cfg)
    {
        config = cfg;

        // Update training types from config
        trainingTypes.clear();
        if (config) {
            for (const auto& [id, _] : config->trainingTypes) {
                trainingTypes.push_back(id);
            }
        }
    }

    void onEnter(GameContext* context) override
    {
        if (!trainingMount) {
            std::cout << "No mount selected for training." << std::endl;
            return;
        }

        std::cout << "Training Session with " << trainingMount->name << std::endl;
        std::cout << trainingMount->getStateDescription() << std::endl;

        // Show current training levels
        std::cout << "\nCurrent Training Levels:" << std::endl;
        for (const auto& [skill, level] : trainingMount->stats.specialTraining) {
            std::cout << skill << ": " << level << "/100" << std::endl;
        }

        // Show mount's condition
        std::cout << "\nMount Condition:" << std::endl;
        std::cout << "Stamina: " << trainingMount->stats.stamina << "/" << trainingMount->stats.maxStamina << std::endl;
        std::cout << "Hunger: " << trainingMount->stats.hunger << "/100" << std::endl;
        std::cout << "Fatigue: " << trainingMount->stats.fatigue << "/100" << std::endl;

        if (trainingMount->stats.isExhausted()) {
            std::cout << "\nWARNING: Your mount is too exhausted for effective training!" << std::endl;
        }

        if (trainingMount->stats.isStarving()) {
            std::cout << "\nWARNING: Your mount is too hungry for effective training!" << std::endl;
        }

        std::cout << "\nAvailable Training Types:" << std::endl;
        if (config) {
            for (const auto& [typeId, typeDesc] : config->trainingTypes) {
                std::cout << "- " << typeId << ": " << typeDesc << std::endl;
            }
        } else {
            for (const auto& type : trainingTypes) {
                std::cout << "- " << type << std::endl;
            }
        }
    }

    std::vector<TAAction> getAvailableActions() override
    {
        std::vector<TAAction> actions = TANode::getAvailableActions();

        if (!trainingMount) {
            actions.push_back({ "select_mount", "Select a mount to train",
                [this]() -> TAInput {
                    return { "training_action", { { "action", std::string("select") } } };
                } });
            return actions;
        }

        // Training type actions
        for (const auto& type : trainingTypes) {
            actions.push_back({ "train_" + type, "Train " + type,
                [this, type]() -> TAInput {
                    return { "training_action", { { "action", std::string("train") }, { "type", type } } };
                } });
        }

        // Feed mount before training
        actions.push_back({ "feed_before_training", "Feed your mount",
            [this]() -> TAInput {
                return { "training_action", { { "action", std::string("feed") } } };
            } });

        // Rest mount before training
        actions.push_back({ "rest_before_training", "Let your mount rest",
            [this]() -> TAInput {
                return { "training_action", { { "action", std::string("rest") } } };
            } });

        // Exit training
        actions.push_back({ "exit_training", "End training session",
            [this]() -> TAInput {
                return { "training_action", { { "action", std::string("exit") } } };
            } });

        return actions;
    }

    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override
    {
        if (input.type == "training_action") {
            std::string action = std::get<std::string>(input.parameters.at("action"));

            if (action == "exit") {
                // Find the exit transition
                for (const auto& rule : transitionRules) {
                    if (rule.description == "Exit") {
                        outNextNode = rule.targetNode;
                        return true;
                    }
                }
            } else if (action == "train" && trainingMount && config) {
                std::string trainingType = std::get<std::string>(input.parameters.at("type"));

                // Create and run a training session
                MountTrainingSession session(trainingMount, trainingType, 60, 50, *config);
                bool success = session.conductTraining();

                if (success) {
                    std::cout << "Training successful! " << trainingMount->name << " has improved "
                              << trainingType << " skills." << std::endl;
                } else {
                    std::cout << "Training was difficult. " << trainingMount->name
                              << " struggled with the exercises." << std::endl;
                }

                // Stay in the same node
                outNextNode = this;
                return true;
            }

            // Other actions would stay in the same node
            outNextNode = this;
            return true;
        }

        return TANode::evaluateTransition(input, outNextNode);
    }
};

// Mount equipment shop node
class MountEquipmentShopNode : public TANode {
public:
    std::string shopName;
    std::vector<MountEquipment*> availableEquipment;

    MountEquipmentShopNode(const std::string& name, const std::string& shop)
        : TANode(name)
        , shopName(shop)
    {
    }

    void onEnter(GameContext* context) override
    {
        std::cout << "Welcome to " << shopName << "!" << std::endl;
        std::cout << "We offer the finest equipment for your mount." << std::endl;

        if (availableEquipment.empty()) {
            std::cout << "Unfortunately, we're out of stock at the moment." << std::endl;
            return;
        }

        std::cout << "\nAvailable Equipment:" << std::endl;
        for (size_t i = 0; i < availableEquipment.size(); i++) {
            MountEquipment* equip = availableEquipment[i];
            std::cout << i + 1 << ". " << equip->name << " - " << equip->price << " gold" << std::endl;
            std::cout << "   " << equip->description << std::endl;

            // Show stat modifiers
            if (!equip->statModifiers.empty()) {
                std::cout << "   Effects: ";
                bool first = true;
                for (const auto& [stat, mod] : equip->statModifiers) {
                    if (!first)
                        std::cout << ", ";
                    std::cout << stat << " " << (mod >= 0 ? "+" : "") << mod;
                    first = false;
                }
                std::cout << std::endl;
            }
        }
    }

    std::vector<TAAction> getAvailableActions() override
    {
        std::vector<TAAction> actions = TANode::getAvailableActions();

        // Buy equipment action
        if (!availableEquipment.empty()) {
            actions.push_back({ "buy_equipment", "Purchase equipment",
                [this]() -> TAInput {
                    return { "shop_action", { { "action", std::string("buy") } } };
                } });
        }

        // Sell equipment action
        actions.push_back({ "sell_equipment", "Sell equipment",
            [this]() -> TAInput {
                return { "shop_action", { { "action", std::string("sell") } } };
            } });

        // Repair equipment action
        actions.push_back({ "repair_equipment", "Repair equipment",
            [this]() -> TAInput {
                return { "shop_action", { { "action", std::string("repair") } } };
            } });

        // Exit shop action
        actions.push_back({ "exit_shop", "Leave shop",
            [this]() -> TAInput {
                return { "shop_action", { { "action", std::string("exit") } } };
            } });

        return actions;
    }

    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override
    {
        if (input.type == "shop_action") {
            std::string action = std::get<std::string>(input.parameters.at("action"));

            if (action == "exit") {
                // Find the exit transition
                for (const auto& rule : transitionRules) {
                    if (rule.description == "Exit") {
                        outNextNode = rule.targetNode;
                        return true;
                    }
                }
            }

            // Other actions would stay in same node
            outNextNode = this;
            return true;
        }

        return TANode::evaluateTransition(input, outNextNode);
    }
};

// Mount racing event node
class MountRacingNode : public TANode {
public:
    std::string trackName;
    float trackLength; // in arbitrary distance units
    int difficulty; // 1-100
    int entryFee;
    int firstPrize;
    int secondPrize;
    int thirdPrize;

    struct RaceCompetitor {
        std::string name;
        int speed;
        int stamina;
        int skill;
    };
    std::vector<RaceCompetitor> competitors;

    MountRacingNode(const std::string& name, const std::string& track, float length, int diff)
        : TANode(name)
        , trackName(track)
        , trackLength(length)
        , difficulty(diff)
        , entryFee(diff * 2)
        , firstPrize(diff * 10)
        , secondPrize(diff * 5)
        , thirdPrize(diff * 2)
    {
    }

    // Create from JSON
    static MountRacingNode* createFromJson(const std::string& name, const json& j)
    {
        std::string trackName = j["name"];
        float trackLength = j["length"];
        int difficulty = j["difficulty"];

        MountRacingNode* node = new MountRacingNode(name, trackName, trackLength, difficulty);

        // Set prizes
        if (j.contains("prizes") && j["prizes"].is_object()) {
            node->firstPrize = j["prizes"]["first"];
            node->secondPrize = j["prizes"]["second"];
            node->thirdPrize = j["prizes"]["third"];
        }

        // Set entry fee
        node->entryFee = j.value("entryFee", difficulty * 2);

        // Generate competitors using name lists from JSON
        if (j.contains("competitorNames") && j.contains("competitorLastNames")) {
            std::vector<std::string> names = j["competitorNames"];
            std::vector<std::string> lastNames = j["competitorLastNames"];
            node->generateCompetitors(5, names, lastNames, difficulty);
        } else {
            node->generateCompetitors(5);
        }

        return node;
    }

    void generateCompetitors(int count, const std::vector<std::string>& names = {},
        const std::vector<std::string>& lastNames = {}, int baseDifficulty = 0)
    {
        // Use provided names or defaults
        std::vector<std::string> competitorNames = names;
        std::vector<std::string> competitorLastNames = lastNames;

        // Default names if none provided
        if (competitorNames.empty()) {
            competitorNames = {
                "Thunder", "Lightning", "Shadow", "Storm", "Arrow",
                "Wind", "Blaze", "Whisper", "Flash", "Midnight"
            };
        }

        if (competitorLastNames.empty()) {
            competitorLastNames = {
                "Runner", "Galloper", "Dasher", "Swift", "Racer",
                "Hooves", "Striker", "Chaser", "Bolt", "Charge"
            };
        }

        // Random generator
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> nameDist(0, competitorNames.size() - 1);
        std::uniform_int_distribution<> lastNameDist(0, competitorLastNames.size() - 1);

        // Stats distribution based on difficulty
        int baseValue = baseDifficulty ? 80 + baseDifficulty / 2 : 80 + difficulty / 2;

        std::uniform_int_distribution<> speedVar(-20, 20);
        std::uniform_int_distribution<> staminaVar(-20, 20);
        std::uniform_int_distribution<> skillVar(-20, 20);

        // Generate competitors
        competitors.clear();
        for (int i = 0; i < count; i++) {
            RaceCompetitor comp;
            comp.name = competitorNames[nameDist(gen)] + " " + competitorLastNames[lastNameDist(gen)];
            comp.speed = baseValue + speedVar(gen);
            comp.stamina = baseValue + staminaVar(gen);
            comp.skill = baseValue + skillVar(gen);

            competitors.push_back(comp);
        }
    }

    void onEnter(GameContext* context) override
    {
        std::cout << "Welcome to the " << trackName << " Racing Track!" << std::endl;
        std::cout << "Track Length: " << trackLength << " units" << std::endl;
        std::cout << "Difficulty: " << difficulty << "/100" << std::endl;
        std::cout << "Entry Fee: " << entryFee << " gold" << std::endl;
        std::cout << "Prizes: 1st - " << firstPrize << " gold, 2nd - "
                  << secondPrize << " gold, 3rd - " << thirdPrize << " gold" << std::endl;

        std::cout << "\nToday's Competitors:" << std::endl;
        for (const auto& comp : competitors) {
            std::cout << "- " << comp.name << std::endl;
        }
    }

    struct RaceResult {
        std::string name;
        float time;
        int position;
    };

    // Simulate a race with the player's mount
    std::vector<RaceResult> simulateRace(Mount* playerMount)
    {
        if (!playerMount)
            return {};

        std::vector<RaceResult> results;
        std::random_device rd;
        std::mt19937 gen(rd());

        // Add player mount to results
        RaceResult playerResult;
        playerResult.name = playerMount->name + " (You)";

        // Calculate player's time
        MountStats stats = playerMount->getEffectiveStats();

        // Base speed affects time directly
        float baseTimePlayer = trackLength / (stats.getEffectiveSpeed() * 0.1f);

        // Stamina affects endurance - worse stamina = more slowdown
        float staminaFactorPlayer = 1.0f + (stats.maxStamina - stats.stamina) * 0.005f;

        // Training in racing skill helps optimize performance
        float skillFactorPlayer = 1.0f - (stats.specialTraining.at("racing") * 0.002f);

        // Random factor (±10%)
        std::uniform_real_distribution<> randFactorDist(0.9f, 1.1f);
        float randomFactorPlayer = randFactorDist(gen);

        // Final time calculation
        playerResult.time = baseTimePlayer * staminaFactorPlayer * skillFactorPlayer * randomFactorPlayer;

        // Calculate time for each NPC competitor
        for (const auto& comp : competitors) {
            RaceResult npcResult;
            npcResult.name = comp.name;

            float baseTimeNPC = trackLength / (comp.speed * 0.1f);
            float staminaFactorNPC = 1.0f + ((100 - comp.stamina) * 0.005f);
            float skillFactorNPC = 1.0f - (comp.skill * 0.002f);
            float randomFactorNPC = randFactorDist(gen);

            npcResult.time = baseTimeNPC * staminaFactorNPC * skillFactorNPC * randomFactorNPC;

            results.push_back(npcResult);
        }

        // Add player result
        results.push_back(playerResult);

        // Sort by time (ascending)
        std::sort(results.begin(), results.end(),
            [](const RaceResult& a, const RaceResult& b) {
                return a.time < b.time;
            });

        // Assign positions
        for (size_t i = 0; i < results.size(); i++) {
            results[i].position = i + 1;
        }

        // Fatigue the mount based on race intensity
        playerMount->stats.useStamina(trackLength * 0.5f);
        playerMount->stats.fatigue += trackLength * 0.3f;
        if (playerMount->stats.fatigue > 100)
            playerMount->stats.fatigue = 100;

        return results;
    }

    std::vector<TAAction> getAvailableActions() override
    {
        std::vector<TAAction> actions = TANode::getAvailableActions();

        // Enter race action
        actions.push_back({ "enter_race", "Enter the race",
            [this]() -> TAInput {
                return { "race_action", { { "action", std::string("enter") } } };
            } });

        // View competitors action
        actions.push_back({ "view_competitors", "Study your competition",
            [this]() -> TAInput {
                return { "race_action", { { "action", std::string("view") } } };
            } });

        // Practice on track action
        actions.push_back({ "practice", "Practice on the track",
            [this]() -> TAInput {
                return { "race_action", { { "action", std::string("practice") } } };
            } });

        // Exit racing area action
        actions.push_back({ "exit_racing", "Leave racing area",
            [this]() -> TAInput {
                return { "race_action", { { "action", std::string("exit") } } };
            } });

        return actions;
    }

    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override
    {
        if (input.type == "race_action") {
            std::string action = std::get<std::string>(input.parameters.at("action"));

            if (action == "exit") {
                // Find the exit transition
                for (const auto& rule : transitionRules) {
                    if (rule.description == "Exit") {
                        outNextNode = rule.targetNode;
                        return true;
                    }
                }
            }

            // Other actions stay in same node
            outNextNode = this;
            return true;
        }

        return TANode::evaluateTransition(input, outNextNode);
    }
};

// Mount breeding center node
class MountBreedingNode : public TANode {
public:
    std::string centerName;
    std::vector<Mount*> availableForBreeding;
    int breedingFee;
    MountSystemConfig* config;

    MountBreedingNode(const std::string& name, const std::string& center, int fee = 200, MountSystemConfig* cfg = nullptr)
        : TANode(name)
        , centerName(center)
        , breedingFee(fee)
        , config(cfg)
    {
    }

    void setConfig(MountSystemConfig* cfg)
    {
        config = cfg;
    }

    // Create from JSON
    static MountBreedingNode* createFromJson(const std::string& name, const json& j,
        std::map<std::string, MountBreed*>& breedTypes,
        MountSystemConfig& config)
    {
        std::string centerName = j["name"];
        int fee = j["fee"];

        MountBreedingNode* node = new MountBreedingNode(name, centerName, fee, &config);

        // Create breeding stock from templates
        if (j.contains("breedingStock") && j["breedingStock"].is_array()) {
            for (const auto& mountTemplate : j["breedingStock"]) {
                std::string breedId = mountTemplate["templateId"];

                // Find the breed
                if (breedTypes.find(breedId) != breedTypes.end()) {
                    MountBreed* breed = breedTypes[breedId];
                    Mount* mount = Mount::createFromTemplate(mountTemplate, breed, config);

                    if (mount) {
                        node->availableForBreeding.push_back(mount);
                    }
                }
            }
        }

        return node;
    }

    void onEnter(GameContext* context) override
    {
        std::cout << "Welcome to the " << centerName << " Breeding Center!" << std::endl;
        std::cout << "Breeding Fee: " << breedingFee << " gold" << std::endl;

        if (availableForBreeding.empty()) {
            std::cout << "We currently have no mounts available for breeding." << std::endl;
            return;
        }

        std::cout << "\nMounts Available for Breeding:" << std::endl;
        for (size_t i = 0; i < availableForBreeding.size(); i++) {
            Mount* mount = availableForBreeding[i];
            std::cout << i + 1 << ". " << mount->name << " (" << mount->breed->name << ")" << std::endl;
            std::cout << "   Age: " << (mount->age / 12) << " years, "
                      << (mount->age % 12) << " months" << std::endl;

            // Show special traits
            std::cout << "   Notable Traits: ";
            bool hasTraits = false;

            if (mount->stats.canJump) {
                std::cout << "Jumping";
                hasTraits = true;
            }

            if (mount->stats.canSwim) {
                if (hasTraits)
                    std::cout << ", ";
                std::cout << "Swimming";
                hasTraits = true;
            }

            if (mount->stats.canClimb) {
                if (hasTraits)
                    std::cout << ", ";
                std::cout << "Climbing";
                hasTraits = true;
            }

            for (const auto& [skill, level] : mount->stats.specialTraining) {
                if (level >= 70) {
                    if (hasTraits)
                        std::cout << ", ";
                    std::cout << "High " << skill;
                    hasTraits = true;
                }
            }

            if (!hasTraits) {
                std::cout << "None";
            }

            std::cout << std::endl;
        }
    }

    // Generate a new foal from two parent mounts
    Mount* breedMounts(Mount* playerMount, Mount* centerMount, const std::string& foalName)
    {
        if (!playerMount || !centerMount || !config)
            return nullptr;

        // Create new mount breed object if needed (hybrid of parents)
        std::string hybridBreedId = playerMount->breed->id + "_" + centerMount->breed->id;
        std::string hybridBreedName = playerMount->breed->name + "-" + centerMount->breed->name + " Cross";

        MountBreed* hybridBreed = new MountBreed(hybridBreedId, hybridBreedName);

        // Inherit base stats from parents (average with slight randomization)
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> variationDist(-10, 10);

        hybridBreed->baseSpeed = (playerMount->breed->baseSpeed + centerMount->breed->baseSpeed) / 2 + variationDist(gen);
        hybridBreed->baseStamina = (playerMount->breed->baseStamina + centerMount->breed->baseStamina) / 2 + variationDist(gen);
        hybridBreed->baseCarryCapacity = (playerMount->breed->baseCarryCapacity + centerMount->breed->baseCarryCapacity) / 2 + variationDist(gen);
        hybridBreed->baseTrainability = (playerMount->breed->baseTrainability + centerMount->breed->baseTrainability) / 2 + variationDist(gen);

        // Natural abilities have a chance to be inherited
        std::uniform_int_distribution<> inheritDist(1, 100);
        hybridBreed->naturalSwimmer = (playerMount->breed->naturalSwimmer || centerMount->breed->naturalSwimmer) && inheritDist(gen) <= 70;
        hybridBreed->naturalJumper = (playerMount->breed->naturalJumper || centerMount->breed->naturalJumper) && inheritDist(gen) <= 70;
        hybridBreed->naturalClimber = (playerMount->breed->naturalClimber || centerMount->breed->naturalClimber) && inheritDist(gen) <= 60;

        // Create the new foal
        std::string foalId = hybridBreedId + "_foal_" + std::to_string(gen());
        Mount* foal = new Mount(foalId, foalName, hybridBreed);

        // Young age (6 months)
        foal->age = 6;

        // Initialize foal with breed characteristics
        hybridBreed->initializeMountStats(foal->stats);

        // Set as owned by player
        foal->isOwned = true;

        // Chance to inherit special abilities directly from parents
        // even if they're not natural to the breed
        if ((playerMount->stats.canJump || centerMount->stats.canJump) && inheritDist(gen) <= 40) {
            foal->stats.canJump = true;
        }

        if ((playerMount->stats.canSwim || centerMount->stats.canSwim) && inheritDist(gen) <= 40) {
            foal->stats.canSwim = true;
        }

        if ((playerMount->stats.canClimb || centerMount->stats.canClimb) && inheritDist(gen) <= 30) {
            foal->stats.canClimb = true;
        }

        // Assign a color to the foal - use from config if available
        foal->color = config->getRandomColor();

        return foal;
    }

    std::vector<TAAction> getAvailableActions() override
    {
        std::vector<TAAction> actions = TANode::getAvailableActions();

        if (!availableForBreeding.empty()) {
            actions.push_back({ "breed_mount", "Breed your mount",
                [this]() -> TAInput {
                    return { "breeding_action", { { "action", std::string("breed") } } };
                } });
        }

        // View available mounts in detail
        actions.push_back({ "view_breeding_stock", "Examine breeding stock",
            [this]() -> TAInput {
                return { "breeding_action", { { "action", std::string("view") } } };
            } });

        // Ask about breeding process
        actions.push_back({ "breeding_info", "Ask about the breeding process",
            [this]() -> TAInput {
                return { "breeding_action", { { "action", std::string("info") } } };
            } });

        // Exit breeding center
        actions.push_back({ "exit_breeding", "Leave breeding center",
            [this]() -> TAInput {
                return { "breeding_action", { { "action", std::string("exit") } } };
            } });

        return actions;
    }

    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override
    {
        if (input.type == "breeding_action") {
            std::string action = std::get<std::string>(input.parameters.at("action"));

            if (action == "exit") {
                // Find the exit transition
                for (const auto& rule : transitionRules) {
                    if (rule.description == "Exit") {
                        outNextNode = rule.targetNode;
                        return true;
                    }
                }
            }

            // Other actions stay in same node
            outNextNode = this;
            return true;
        }

        return TANode::evaluateTransition(input, outNextNode);
    }
};

// Mount system main controller
class MountSystemController : public TANode {
public:
    // Player's owned mounts
    std::vector<Mount*> ownedMounts;

    // Currently active mount
    Mount* activeMount;

    // Registered stables
    std::vector<MountStable*> stables;

    // Registered breed types
    std::map<std::string, MountBreed*> breedTypes;

    // Available mount equipment
    std::vector<MountEquipment*> knownEquipment;

    // Global configuration
    MountSystemConfig config;

    // Path to the config file
    std::string configPath;

    MountSystemController(const std::string& name, const std::string& jsonPath = "Mount.JSON")
        : TANode(name)
        , activeMount(nullptr)
        , configPath(jsonPath)
    {
        // Load configuration from JSON
        loadConfig();
    }

    // Load all configuration from JSON file
    void loadConfig()
    {
        try {
            // Check if file exists
            if (!std::filesystem::exists(configPath)) {
                std::cerr << "Config file not found: " << configPath << std::endl;
                // Initialize with basic defaults
                initializeBasicDefaults();
                return;
            }

            // Open and parse JSON file
            std::ifstream file(configPath);
            json mountConfig;
            file >> mountConfig;

            // Load breed types
            if (mountConfig.contains("breeds") && mountConfig["breeds"].is_object()) {
                for (auto& [id, breedJson] : mountConfig["breeds"].items()) {
                    MountBreed* breed = MountBreed::createFromJson(breedJson);
                    breedTypes[id] = breed;
                }
            }

            // Load equipment
            if (mountConfig.contains("equipment") && mountConfig["equipment"].is_object()) {
                for (auto& [id, equipJson] : mountConfig["equipment"].items()) {
                    MountEquipment* equipment = MountEquipment::createFromJson(equipJson);
                    knownEquipment.push_back(equipment);
                }
            }

            // Load stables
            if (mountConfig.contains("stables") && mountConfig["stables"].is_object()) {
                for (auto& [id, stableJson] : mountConfig["stables"].items()) {
                    MountStable* stable = MountStable::createFromJson(stableJson, breedTypes, config);
                    stables.push_back(stable);
                }
            }

            // Load special abilities
            if (mountConfig.contains("specialAbilities") && mountConfig["specialAbilities"].is_object()) {
                for (auto& [id, abilityJson] : mountConfig["specialAbilities"].items()) {
                    SpecialAbilityInfo ability = SpecialAbilityInfo::fromJson(abilityJson);
                    config.specialAbilities[id] = ability;
                }
            }

            // Load training types
            if (mountConfig.contains("trainingTypes") && mountConfig["trainingTypes"].is_array()) {
                for (const auto& trainingJson : mountConfig["trainingTypes"]) {
                    std::string id = trainingJson["id"];
                    std::string description = trainingJson["description"];
                    config.trainingTypes.push_back({ id, description });
                }
            }

            // Load mount colors
            if (mountConfig.contains("colors") && mountConfig["colors"].is_array()) {
                for (const auto& color : mountConfig["colors"]) {
                    config.colors.push_back(color);
                }
            }

            std::cout << "Mount system configuration loaded successfully from " << configPath << std::endl;

        } catch (const std::exception& e) {
            std::cerr << "Error loading mount configuration: " << e.what() << std::endl;
            // Initialize with basic defaults
            initializeBasicDefaults();
        }
    }

    // Initialize with basic defaults if JSON loading fails
    void initializeBasicDefaults()
    {
        std::cout << "Initializing mount system with default values" << std::endl;

        // Create default breeds
        MountBreed* standardHorse = new MountBreed("standard_horse", "Standard Horse");
        standardHorse->baseSpeed = 100;
        standardHorse->baseStamina = 100;
        standardHorse->baseCarryCapacity = 50;
        standardHorse->baseTrainability = 50;
        breedTypes["standard_horse"] = standardHorse;

        // Set default training types
        config.trainingTypes = {
            { "combat", "Fighting from mountback and defensive maneuvers" },
            { "endurance", "Long-distance travel and stamina management" },
            { "agility", "Jumping, balance, and difficult terrain navigation" },
            { "racing", "Burst speed and racing techniques" }
        };

        // Default colors
        config.colors = { "Bay", "Chestnut", "Black", "Gray", "White" };

        // Default special abilities
        SpecialAbilityInfo jump;
        jump.id = "jump";
        jump.name = "Jump";
        jump.description = "Jump over obstacles";
        jump.staminaCost = 25;
        jump.skillRequired = 30;
        jump.trainingType = "agility";
        jump.unlockThreshold = 50;
        config.specialAbilities["jump"] = jump;

        SpecialAbilityInfo swim;
        swim.id = "swim";
        swim.name = "Swim";
        swim.description = "Swim across water";
        swim.staminaCost = 15;
        swim.skillRequired = 30;
        swim.trainingType = "endurance";
        swim.unlockThreshold = 60;
        config.specialAbilities["swim"] = swim;
    }

    void registerStable(MountStable* stable)
    {
        if (stable) {
            stables.push_back(stable);
        }
    }

    Mount* createMount(const std::string& name, const std::string& breedId)
    {
        if (breedTypes.find(breedId) == breedTypes.end()) {
            return nullptr;
        }

        std::string mountId = breedId + "_" + name + "_" + std::to_string(ownedMounts.size() + 1);
        Mount* newMount = new Mount(mountId, name, breedTypes[breedId]);

        // Set a random color from the config
        newMount->color = config.getRandomColor();

        return newMount;
    }

    bool addMount(Mount* mount)
    {
        if (!mount)
            return false;

        mount->isOwned = true;
        ownedMounts.push_back(mount);
        return true;
    }

    bool removeMount(const std::string& mountId)
    {
        auto it = std::find_if(ownedMounts.begin(), ownedMounts.end(),
            [&mountId](Mount* m) { return m->id == mountId; });

        if (it != ownedMounts.end()) {
            if (activeMount == *it) {
                activeMount = nullptr;
            }

            Mount* mount = *it;
            ownedMounts.erase(it);
            delete mount; // Free the mount memory
            return true;
        }

        return false;
    }

    Mount* findMount(const std::string& mountId)
    {
        auto it = std::find_if(ownedMounts.begin(), ownedMounts.end(),
            [&mountId](Mount* m) { return m->id == mountId; });

        if (it != ownedMounts.end()) {
            return *it;
        }

        return nullptr;
    }

    void setActiveMount(Mount* mount)
    {
        // Dismount the current mount if any
        if (activeMount && activeMount->isMounted) {
            activeMount->isMounted = false;
        }

        // Set the new active mount
        activeMount = mount;

        if (activeMount) {
            // Summon the mount if it's not already
            activeMount->isSummoned = true;
            activeMount->isStabled = false;

            std::cout << activeMount->name << " is now your active mount." << std::endl;
        }
    }

    bool mountActive()
    {
        if (!activeMount) {
            std::cout << "You don't have an active mount." << std::endl;
            return false;
        }

        activeMount->isMounted = true;
        std::cout << "You mount " << activeMount->name << "." << std::endl;
        return true;
    }

    void dismountActive()
    {
        if (activeMount && activeMount->isMounted) {
            activeMount->isMounted = false;
            std::cout << "You dismount " << activeMount->name << "." << std::endl;
        }
    }

    // Find the nearest stable to the player's location
    MountStable* findNearestStable(const std::string& playerLocation)
    {
        for (MountStable* stable : stables) {
            if (stable->location == playerLocation) {
                return stable;
            }
        }

        // If no exact match, return first stable (would use distance in real implementation)
        return stables.empty() ? nullptr : stables[0];
    }

    void onEnter(GameContext* context) override
    {
        std::cout << "=== Mount Management System ===" << std::endl;

        if (ownedMounts.empty()) {
            std::cout << "You don't own any mounts." << std::endl;
        } else {
            std::cout << "Your mounts:" << std::endl;
            for (size_t i = 0; i < ownedMounts.size(); i++) {
                Mount* mount = ownedMounts[i];
                std::cout << i + 1 << ". " << mount->getStateDescription();

                if (mount == activeMount) {
                    std::cout << " (Active)";
                }

                std::cout << std::endl;
            }
        }

        std::cout << "\nNearby stables:" << std::endl;
        if (stables.empty()) {
            std::cout << "No stables in this region." << std::endl;
        } else {
            for (MountStable* stable : stables) {
                std::cout << "- " << stable->name << " in " << stable->location << std::endl;
            }
        }
    }

    std::vector<TAAction> getAvailableActions() override
    {
        std::vector<TAAction> actions = TANode::getAvailableActions();

        // Actions for owned mounts
        if (!ownedMounts.empty()) {
            actions.push_back({ "view_mounts", "View your mounts",
                [this]() -> TAInput {
                    return { "mount_system", { { "action", std::string("view") } } };
                } });

            actions.push_back({ "select_mount", "Select active mount",
                [this]() -> TAInput {
                    return { "mount_system", { { "action", std::string("select") } } };
                } });
        }

        // Mount/dismount if there's an active mount
        if (activeMount) {
            if (activeMount->isMounted) {
                actions.push_back({ "dismount", "Dismount",
                    [this]() -> TAInput {
                        return { "mount_system", { { "action", std::string("dismount") } } };
                    } });
            } else {
                actions.push_back({ "mount", "Mount",
                    [this]() -> TAInput {
                        return { "mount_system", { { "action", std::string("mount") } } };
                    } });
            }

            // Interact with mount
            actions.push_back({ "interact_mount", "Interact with mount",
                [this]() -> TAInput {
                    return { "mount_system", { { "action", std::string("interact") } } };
                } });
        }

        // Stable actions if there are stables
        if (!stables.empty()) {
            actions.push_back({ "visit_stable", "Visit a stable",
                [this]() -> TAInput {
                    return { "mount_system", { { "action", std::string("stable") } } };
                } });
        }

        return actions;
    }

    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override
    {
        if (input.type == "mount_system") {
            std::string action = std::get<std::string>(input.parameters.at("action"));

            // Most actions would be processed by the game logic
            // and would potentially set outNextNode to a different system node

            outNextNode = this; // Stay in the same node by default
            return true;
        }

        return TANode::evaluateTransition(input, outNextNode);
    }

    // Update all owned mounts' state
    void updateMounts(int minutes)
    {
        for (Mount* mount : ownedMounts) {
            if (mount) {
                mount->update(minutes);
            }
        }
    }

    // Calculate travel effects when mounted
    void applyTravelEffects(float distance)
    {
        if (!activeMount || !activeMount->isMounted)
            return;

        // Use stamina based on distance
        activeMount->stats.useStamina(distance * 5);

        // Increase hunger and fatigue
        activeMount->stats.hunger += distance * 2;
        if (activeMount->stats.hunger > 100)
            activeMount->stats.hunger = 100;

        activeMount->stats.fatigue += distance * 3;
        if (activeMount->stats.fatigue > 100)
            activeMount->stats.fatigue = 100;

        // Wear equipment
        for (auto& [slot, equipment] : activeMount->equippedItems) {
            if (equipment) {
                equipment->use(distance * 0.5f);
            }
        }
    }

    // Get the mounted travel speed modifier
    float getMountedSpeedModifier()
    {
        if (!activeMount || !activeMount->isMounted)
            return 1.0f;

        return activeMount->getTravelTimeModifier();
    }

    // Check if the active mount can perform a special movement
    bool canPerformSpecialMovement(const std::string& movementType)
    {
        if (!activeMount || !activeMount->isMounted)
            return false;

        return config.canUseAbility(activeMount->stats, movementType);
    }
};

// Main function to set up the mount system
void setupMountSystem(TAController& controller)
{
    std::cout << "Setting up Mount System..." << std::endl;

    // Create the mount system controller with JSON config path
    MountSystemController* mountSystem = dynamic_cast<MountSystemController*>(
        controller.createNode<MountSystemController>("MountSystem", "Mount.JSON"));

    // Create mount interaction node
    MountInteractionNode* mountInteraction = dynamic_cast<MountInteractionNode*>(
        controller.createNode<MountInteractionNode>("MountInteraction", nullptr, &mountSystem->config));

    // Create mount training node
    MountTrainingNode* mountTraining = dynamic_cast<MountTrainingNode*>(
        controller.createNode<MountTrainingNode>("MountTraining", nullptr, &mountSystem->config));

    // Get stable nodes from loaded configuration
    std::vector<MountStableNode*> stableNodes;
    for (MountStable* stable : mountSystem->stables) {
        MountStableNode* stableNode = dynamic_cast<MountStableNode*>(
            controller.createNode<MountStableNode>(stable->name.replace(stable->name.find(" "), 1, ""), stable));
        stableNodes.push_back(stableNode);
    }

    // Create equipment shop
    MountEquipmentShopNode* equipmentShop = dynamic_cast<MountEquipmentShopNode*>(
        controller.createNode<MountEquipmentShopNode>("MountEquipmentShop", "Horseman's Gear"));

    // Add known equipment to shop
    for (MountEquipment* equipment : mountSystem->knownEquipment) {
        equipmentShop->availableEquipment.push_back(equipment);
    }

    // Create a mount racing node from JSON config
    json config;
    try {
        std::ifstream file("Mount.JSON");
        file >> config;

        if (config.contains("racetrack")) {
            MountRacingNode* racingNode = MountRacingNode::createFromJson(
                "MountRacing", config["racetrack"]);

            // Connect racing node
            mountSystem->addTransition(
                [](const TAInput& input) {
                    return input.type == "mount_system" && std::get<std::string>(input.parameters.at("action")) == "race";
                },
                racingNode, "Go to racetrack");

            racingNode->addTransition(
                [](const TAInput& input) {
                    return input.type == "race_action" && std::get<std::string>(input.parameters.at("action")) == "exit";
                },
                mountSystem, "Exit");
        }

        // Create a mount breeding node
        if (config.contains("breedingCenter")) {
            MountBreedingNode* breedingNode = MountBreedingNode::createFromJson(
                "MountBreeding", config["breedingCenter"], mountSystem->breedTypes, mountSystem->config);

            // Connect breeding node
            mountSystem->addTransition(
                [](const TAInput& input) {
                    return input.type == "mount_system" && std::get<std::string>(input.parameters.at("action")) == "breed";
                },
                breedingNode, "Visit breeding center");

            breedingNode->addTransition(
                [](const TAInput& input) {
                    return input.type == "breeding_action" && std::get<std::string>(input.parameters.at("action")) == "exit";
                },
                mountSystem, "Exit");
        }

    } catch (const std::exception& e) {
        std::cerr << "Error loading racing/breeding configuration: " << e.what() << std::endl;
    }

    // Set up connections between nodes
    mountSystem->addTransition(
        [](const TAInput& input) {
            return input.type == "mount_system" && std::get<std::string>(input.parameters.at("action")) == "interact";
        },
        mountInteraction, "Interact with mount");

    mountInteraction->addTransition(
        [](const TAInput& input) {
            return input.type == "mount_action" && std::get<std::string>(input.parameters.at("action")) == "exit";
        },
        mountSystem, "Return to mount system");

    // Connect mount system to stable nodes
    for (MountStableNode* stableNode : stableNodes) {
        MountStable* stable = stableNode->stable;
        mountSystem->addTransition(
            [stable](const TAInput& input) {
                return input.type == "mount_system" && std::get<std::string>(input.parameters.at("action")) == "stable" &&
                    // In a real system, would check player location to find nearest stable
                    true; // Simplified for this example
            },
            stableNode, "Visit " + stable->name);

        // Connect back to mount system
        stableNode->addTransition(
            [](const TAInput& input) {
                return input.type == "stable_action" && std::get<std::string>(input.parameters.at("action")) == "exit";
            },
            mountSystem, "Exit");
    }

    // Connect mount interaction to training
    mountInteraction->addTransition(
        [](const TAInput& input) {
            return input.type == "mount_action" && std::get<std::string>(input.parameters.at("action")) == "train";
        },
        mountTraining, "Train mount");

    mountTraining->addTransition(
        [](const TAInput& input) {
            return input.type == "training_action" && std::get<std::string>(input.parameters.at("action")) == "exit";
        },
        mountInteraction, "Exit");

    // Connect stables to equipment shop
    if (!stableNodes.empty() && equipmentShop) {
        stableNodes[0]->addTransition(
            [](const TAInput& input) {
                return input.type == "stable_action" && std::get<std::string>(input.parameters.at("action")) == "shop";
            },
            equipmentShop, "Visit equipment shop");

        equipmentShop->addTransition(
            [](const TAInput& input) {
                return input.type == "shop_action" && std::get<std::string>(input.parameters.at("action")) == "exit";
            },
            stableNodes[0], "Exit");
    }

    // Register the mount system
    controller.setSystemRoot("MountSystem", mountSystem);

    std::cout << "Mount System setup complete!" << std::endl;
}

int main()
{
    std::cout << "=== Mount System for Oath RPG ===" << std::endl;

    // Create the automaton controller
    TAController controller;

    // Set up the mount system
    setupMountSystem(controller);

    // The system would now be integrated with the main game
    std::cout << "Mount System is ready to be integrated with the main game." << std::endl;

    // In a real game, you would connect this to the world system, character system, etc.

    return 0;
}