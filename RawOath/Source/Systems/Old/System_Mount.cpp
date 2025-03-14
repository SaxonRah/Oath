// System_Mount.cpp
#include <algorithm>
#include <ctime>
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

// Forward declarations for main systems
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
        // Initialize specialties
        specialTraining["combat"] = 0; // Combat training
        specialTraining["endurance"] = 0; // Long distance travel
        specialTraining["agility"] = 0; // Jumps and difficult terrain
        specialTraining["racing"] = 0; // Speed and bursts
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

        // Unlock special abilities based on training
        if (specialTraining["agility"] >= 50)
            canJump = true;
        if (specialTraining["endurance"] >= 60)
            canSwim = true;
        if (specialTraining["agility"] >= 80)
            canClimb = true;
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

// Mount equipment item
struct MountEquipment {
    std::string id;
    std::string name;
    std::string description;
    MountEquipmentSlot slot;
    int quality; // 1-100
    int durability;
    int maxDurability;
    std::map<std::string, int> statModifiers; // Speed, stamina, etc.

    MountEquipment(const std::string& itemId, const std::string& itemName, MountEquipmentSlot itemSlot)
        : id(itemId)
        , name(itemName)
        , slot(itemSlot)
        , quality(50)
        , durability(100)
        , maxDurability(100)
    {
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

    // Try to use a special ability
    bool useSpecialAbility(const std::string& ability)
    {
        // Check if mount can perform this ability
        bool canPerform = false;
        int staminaCost = 0;
        int skillRequired = 0;

        if (ability == "jump") {
            canPerform = stats.canJump;
            staminaCost = 25;
            skillRequired = stats.specialTraining["agility"];
        } else if (ability == "swim") {
            canPerform = stats.canSwim;
            staminaCost = 15;
            skillRequired = stats.specialTraining["endurance"];
        } else if (ability == "climb") {
            canPerform = stats.canClimb;
            staminaCost = 30;
            skillRequired = stats.specialTraining["agility"];
        } else if (ability == "sprint") {
            canPerform = true; // All mounts can sprint
            staminaCost = 20;
            skillRequired = stats.specialTraining["racing"];
        } else if (ability == "kick") {
            canPerform = true; // All mounts can attempt to kick
            staminaCost = 15;
            skillRequired = stats.specialTraining["combat"];
        }

        // Check requirements
        if (!canPerform || skillRequired < 30) {
            return false;
        }

        // Use stamina
        return stats.useStamina(staminaCost);
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

    MountStable(const std::string& stableId, const std::string& stableName, const std::string& stableLocation, int stableCapacity = 5)
        : id(stableId)
        , name(stableName)
        , location(stableLocation)
        , capacity(stableCapacity)
        , dailyFeedCost(5)
        , dailyCareCost(3)
    {
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

    MountTrainingSession(Mount* targetMount, const std::string& type, int sessionDuration, int sessionDifficulty)
        : mount(targetMount)
        , trainingType(type)
        , duration(sessionDuration)
        , difficulty(sessionDifficulty)
        , successChance(70)
        , experienceGain(5)
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

            // Chance to unlock special abilities based on training type
            if (trainingType == "agility" && !mount->stats.canJump && mount->stats.specialTraining["agility"] >= 40) {
                // 10% chance per session to unlock jumping once skill is high enough
                if (dis(gen) <= 10) {
                    mount->stats.canJump = true;
                    std::cout << mount->name << " has learned to jump obstacles!" << std::endl;
                }
            } else if (trainingType == "endurance" && !mount->stats.canSwim && mount->stats.specialTraining["endurance"] >= 50) {
                // 10% chance per session to unlock swimming once skill is high enough
                if (dis(gen) <= 10) {
                    mount->stats.canSwim = true;
                    std::cout << mount->name << " has learned to swim across water!" << std::endl;
                }
            } else if (trainingType == "agility" && !mount->stats.canClimb && mount->stats.specialTraining["agility"] >= 70) {
                // 5% chance per session to unlock climbing once skill is high enough
                if (dis(gen) <= 5) {
                    mount->stats.canClimb = true;
                    std::cout << mount->name << " has learned to climb steep slopes!" << std::endl;
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
    std::vector<Mount*> availableForPurchase;

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
        if (!availableForPurchase.empty()) {
            std::cout << "\nMounts available for purchase:" << std::endl;
            for (size_t i = 0; i < availableForPurchase.size(); i++) {
                Mount* mount = availableForPurchase[i];
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
        if (!availableForPurchase.empty()) {
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

    MountInteractionNode(const std::string& name, Mount* mount = nullptr)
        : TANode(name)
        , activeMount(mount)
    {
    }

    void setActiveMount(Mount* mount)
    {
        activeMount = mount;
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
                    std::string slotName;
                    switch (slot) {
                    case MountEquipmentSlot::Saddle:
                        slotName = "Saddle";
                        break;
                    case MountEquipmentSlot::Armor:
                        slotName = "Armor";
                        break;
                    case MountEquipmentSlot::Bags:
                        slotName = "Bags";
                        break;
                    case MountEquipmentSlot::Shoes:
                        slotName = "Shoes";
                        break;
                    case MountEquipmentSlot::Accessory:
                        slotName = "Accessory";
                        break;
                    }

                    std::cout << slotName << ": " << equipment->name;
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
        if (activeMount->isMounted) {
            MountStats effectiveStats = activeMount->getEffectiveStats();

            if (effectiveStats.canJump) {
                actions.push_back({ "jump", "Jump over an obstacle",
                    [this]() -> TAInput {
                        return { "mount_action", { { "action", std::string("ability") }, { "ability", std::string("jump") } } };
                    } });
            }

            if (effectiveStats.canSwim) {
                actions.push_back({ "swim", "Swim across water",
                    [this]() -> TAInput {
                        return { "mount_action", { { "action", std::string("ability") }, { "ability", std::string("swim") } } };
                    } });
            }

            if (effectiveStats.canClimb) {
                actions.push_back({ "climb", "Climb a steep slope",
                    [this]() -> TAInput {
                        return { "mount_action", { { "action", std::string("ability") }, { "ability", std::string("climb") } } };
                    } });
            }

            // All mounts can sprint
            actions.push_back({ "sprint", "Gallop at full speed",
                [this]() -> TAInput {
                    return { "mount_action", { { "action", std::string("ability") }, { "ability", std::string("sprint") } } };
                } });
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

    MountTrainingNode(const std::string& name, Mount* mount = nullptr)
        : TANode(name)
        , trainingMount(mount)
    {
        // Initialize available training types
        trainingTypes = { "combat", "endurance", "agility", "racing" };
    }

    void setTrainingMount(Mount* mount)
    {
        trainingMount = mount;
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
        for (const auto& type : trainingTypes) {
            std::cout << "- " << type << ": ";
            switch (trainingTypes.size()) {
            case 0:
                std::cout << "Fighting from mountback and defensive maneuvers";
                break;
            case 1:
                std::cout << "Long-distance travel and stamina management";
                break;
            case 2:
                std::cout << "Jumping, balance, and difficult terrain navigation";
                break;
            case 3:
                std::cout << "Burst speed and racing techniques";
                break;
            default:
                std::cout << "General training";
                break;
            }
            std::cout << std::endl;
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
            } else if (action == "train" && trainingMount) {
                std::string trainingType = std::get<std::string>(input.parameters.at("type"));

                // Create and run a training session
                MountTrainingSession session(trainingMount, trainingType, 60, 50);
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
            std::cout << i + 1 << ". " << equip->name << " - " << calculatePrice(equip) << " gold" << std::endl;
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

    int calculatePrice(MountEquipment* equipment) const
    {
        if (!equipment)
            return 0;

        int basePrice = 50; // Base price for any equipment

        // Quality affects price
        basePrice += equipment->quality;

        // Each stat modifier adds value
        for (const auto& [_, mod] : equipment->statModifiers) {
            basePrice += std::abs(mod) * 5;
        }

        // Discount for worn equipment
        if (equipment->isWorn()) {
            basePrice = static_cast<int>(basePrice * 0.5f);
        }

        return basePrice;
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
        // Generate some competitors
        generateCompetitors(5);
    }

    void generateCompetitors(int count)
    {
        // Names for competitors
        std::vector<std::string> names = {
            "Thunder", "Lightning", "Shadow", "Storm", "Arrow",
            "Wind", "Blaze", "Whisper", "Flash", "Midnight"
        };

        std::vector<std::string> lastNames = {
            "Runner", "Galloper", "Dasher", "Swift", "Racer",
            "Hooves", "Striker", "Chaser", "Bolt", "Charge"
        };

        // Random generator
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> nameDist(0, names.size() - 1);
        std::uniform_int_distribution<> lastNameDist(0, lastNames.size() - 1);

        // Stats distribution based on difficulty
        int baseSpeed = 80 + difficulty / 2;
        int baseStamina = 80 + difficulty / 2;
        int baseSkill = 80 + difficulty / 2;

        std::uniform_int_distribution<> speedVar(-20, 20);
        std::uniform_int_distribution<> staminaVar(-20, 20);
        std::uniform_int_distribution<> skillVar(-20, 20);

        // Generate competitors
        competitors.clear();
        for (int i = 0; i < count; i++) {
            RaceCompetitor comp;
            comp.name = names[nameDist(gen)] + " " + lastNames[lastNameDist(gen)];
            comp.speed = baseSpeed + speedVar(gen);
            comp.stamina = baseStamina + staminaVar(gen);
            comp.skill = baseSkill + skillVar(gen);

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

    MountBreedingNode(const std::string& name, const std::string& center, int fee = 200)
        : TANode(name)
        , centerName(center)
        , breedingFee(fee)
    {
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
        if (!playerMount || !centerMount)
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

        // Assign a color to the foal
        std::vector<std::string> colors = {
            "Bay", "Chestnut", "Black", "Gray", "White",
            "Palomino", "Buckskin", "Dun", "Roan", "Pinto"
        };
        std::uniform_int_distribution<> colorDist(0, colors.size() - 1);
        foal->color = colors[colorDist(gen)];

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

    MountSystemController(const std::string& name)
        : TANode(name)
        , activeMount(nullptr)
    {
        // Initialize system with basic breed types
        initializeBreedTypes();
    }

    void initializeBreedTypes()
    {
        // Create standard horse breeds
        MountBreed* standardHorse = new MountBreed("standard_horse", "Standard Horse");
        standardHorse->baseSpeed = 100;
        standardHorse->baseStamina = 100;
        standardHorse->baseCarryCapacity = 50;
        standardHorse->baseTrainability = 50;
        breedTypes["standard_horse"] = standardHorse;

        // War horse breed - stronger, more stamina, better for combat
        MountBreed* warHorse = new MountBreed("war_horse", "War Horse");
        warHorse->baseSpeed = 90;
        warHorse->baseStamina = 120;
        warHorse->baseCarryCapacity = 70;
        warHorse->baseTrainability = 60;
        warHorse->naturalJumper = true;
        breedTypes["war_horse"] = warHorse;

        // Racing horse breed - faster but less carrying capacity
        MountBreed* racingHorse = new MountBreed("racing_horse", "Racing Horse");
        racingHorse->baseSpeed = 140;
        racingHorse->baseStamina = 110;
        racingHorse->baseCarryCapacity = 30;
        racingHorse->baseTrainability = 70;
        breedTypes["racing_horse"] = racingHorse;

        // Pack horse breed - slower but high carrying capacity
        MountBreed* packHorse = new MountBreed("pack_horse", "Pack Horse");
        packHorse->baseSpeed = 80;
        packHorse->baseStamina = 130;
        packHorse->baseCarryCapacity = 100;
        packHorse->baseTrainability = 40;
        breedTypes["pack_horse"] = packHorse;

        // Mountain pony breed - good on rough terrain
        MountBreed* mountainPony = new MountBreed("mountain_pony", "Mountain Pony");
        mountainPony->baseSpeed = 90;
        mountainPony->baseStamina = 120;
        mountainPony->baseCarryCapacity = 40;
        mountainPony->baseTrainability = 60;
        mountainPony->naturalClimber = true;
        breedTypes["mountain_pony"] = mountainPony;

        // River horse breed - can swim well
        MountBreed* riverHorse = new MountBreed("river_horse", "River Horse");
        riverHorse->baseSpeed = 100;
        riverHorse->baseStamina = 110;
        riverHorse->baseCarryCapacity = 50;
        riverHorse->baseTrainability = 55;
        riverHorse->naturalSwimmer = true;
        breedTypes["river_horse"] = riverHorse;
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

        MountStats stats = activeMount->getEffectiveStats();

        if (movementType == "jump")
            return stats.canJump && !stats.isExhausted();
        if (movementType == "swim")
            return stats.canSwim && !stats.isExhausted();
        if (movementType == "climb")
            return stats.canClimb && !stats.isExhausted();

        return false;
    }
};

// Main function to set up the mount system
void setupMountSystem(TAController& controller)
{
    std::cout << "Setting up Mount System..." << std::endl;

    // Create the mount system controller
    MountSystemController* mountSystem = dynamic_cast<MountSystemController*>(
        controller.createNode<MountSystemController>("MountSystem"));

    // Create mount interaction node
    MountInteractionNode* mountInteraction = dynamic_cast<MountInteractionNode*>(
        controller.createNode<MountInteractionNode>("MountInteraction"));

    // Create mount training node
    MountTrainingNode* mountTraining = dynamic_cast<MountTrainingNode*>(
        controller.createNode<MountTrainingNode>("MountTraining"));

    // Create stables
    MountStable* villageStable = new MountStable("village_stable", "Village Stables", "Oakvale Village", 6);
    MountStable* cityStable = new MountStable("city_stable", "Royal Stables", "Capital City", 10);
    MountStable* outpostStable = new MountStable("outpost_stable", "Frontier Stables", "Northern Outpost", 4);

    // Register stables with the system
    mountSystem->registerStable(villageStable);
    mountSystem->registerStable(cityStable);
    mountSystem->registerStable(outpostStable);

    // Create stable nodes
    MountStableNode* villageStableNode = dynamic_cast<MountStableNode*>(
        controller.createNode<MountStableNode>("VillageStables", villageStable));

    MountStableNode* cityStableNode = dynamic_cast<MountStableNode*>(
        controller.createNode<MountStableNode>("CityStables", cityStable));

    MountStableNode* outpostStableNode = dynamic_cast<MountStableNode*>(
        controller.createNode<MountStableNode>("OutpostStables", outpostStable));

    // Create equipment shop
    MountEquipmentShopNode* equipmentShop = dynamic_cast<MountEquipmentShopNode*>(
        controller.createNode<MountEquipmentShopNode>("MountEquipmentShop", "Horseman's Gear"));

    // Create sample equipment
    MountEquipment* basicSaddle = new MountEquipment("basic_saddle", "Basic Saddle", MountEquipmentSlot::Saddle);
    basicSaddle->description = "A simple leather saddle.";
    basicSaddle->statModifiers["speed"] = 5;

    MountEquipment* racingSaddle = new MountEquipment("racing_saddle", "Racing Saddle", MountEquipmentSlot::Saddle);
    racingSaddle->description = "A lightweight saddle designed for racing.";
    racingSaddle->statModifiers["speed"] = 15;
    racingSaddle->statModifiers["stamina"] = -5;

    MountEquipment* heavyArmor = new MountEquipment("heavy_armor", "Heavy Barding", MountEquipmentSlot::Armor);
    heavyArmor->description = "Heavy armor plating to protect your mount in combat.";
    heavyArmor->statModifiers["speed"] = -15;
    heavyArmor->statModifiers["stamina"] = -10;

    MountEquipment* saddlebags = new MountEquipment("saddlebags", "Leather Saddlebags", MountEquipmentSlot::Bags);
    saddlebags->description = "Bags that attach to your saddle for additional storage.";
    saddlebags->statModifiers["carryCapacity"] = 20;
    saddlebags->statModifiers["speed"] = -5;

    // Add equipment to shop
    equipmentShop->availableEquipment.push_back(basicSaddle);
    equipmentShop->availableEquipment.push_back(racingSaddle);
    equipmentShop->availableEquipment.push_back(heavyArmor);
    equipmentShop->availableEquipment.push_back(saddlebags);

    // Create a mount racing node
    MountRacingNode* racingNode = dynamic_cast<MountRacingNode*>(
        controller.createNode<MountRacingNode>("MountRacing", "Country Fair Races", 500.0f, 50));

    // Create a mount breeding node
    MountBreedingNode* breedingNode = dynamic_cast<MountBreedingNode*>(
        controller.createNode<MountBreedingNode>("MountBreeding", "Royal Breeding Grounds"));

    // Create some available mounts for purchase at stables
    // Village stable mounts
    Mount* villageMount1 = mountSystem->createMount("Thunder", "standard_horse");
    villageMount1->color = "Bay";
    villageStableNode->availableForPurchase.push_back(villageMount1);

    Mount* villageMount2 = mountSystem->createMount("Swift", "mountain_pony");
    villageMount2->color = "Dappled Gray";
    villageStableNode->availableForPurchase.push_back(villageMount2);

    // City stable mounts (higher quality)
    Mount* cityMount1 = mountSystem->createMount("Valor", "war_horse");
    cityMount1->color = "Black";
    cityMount1->stats.training = 40;
    cityMount1->stats.specialTraining["combat"] = 50;
    cityStableNode->availableForPurchase.push_back(cityMount1);

    Mount* cityMount2 = mountSystem->createMount("Arrow", "racing_horse");
    cityMount2->color = "Chestnut";
    cityMount2->stats.training = 45;
    cityMount2->stats.specialTraining["racing"] = 60;
    cityStableNode->availableForPurchase.push_back(cityMount2);

    // Outpost stable mounts (specialized)
    Mount* outpostMount1 = mountSystem->createMount("River", "river_horse");
    outpostMount1->color = "Palomino";
    outpostMount1->stats.canSwim = true;
    outpostStableNode->availableForPurchase.push_back(outpostMount1);

    // Breeding stock
    Mount* breedingMount1 = mountSystem->createMount("Champion", "racing_horse");
    breedingMount1->color = "Bay";
    breedingMount1->stats.training = 70;
    breedingMount1->stats.specialTraining["racing"] = 80;
    breedingNode->availableForBreeding.push_back(breedingMount1);

    Mount* breedingMount2 = mountSystem->createMount("Warrior", "war_horse");
    breedingMount2->color = "Black";
    breedingMount2->stats.training = 65;
    breedingMount2->stats.specialTraining["combat"] = 75;
    breedingMount2->stats.canJump = true;
    breedingNode->availableForBreeding.push_back(breedingMount2);

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
    mountSystem->addTransition(
        [villageStable](const TAInput& input) {
            return input.type == "mount_system" && std::get<std::string>(input.parameters.at("action")) == "stable" &&
                // In a real game, would check player location
                true; // Simplified: always go to village stable
        },
        villageStableNode, "Visit village stables");

    // Connect stable nodes back to mount system
    villageStableNode->addTransition(
        [](const TAInput& input) {
            return input.type == "stable_action" && std::get<std::string>(input.parameters.at("action")) == "exit";
        },
        mountSystem, "Exit");

    cityStableNode->addTransition(
        [](const TAInput& input) {
            return input.type == "stable_action" && std::get<std::string>(input.parameters.at("action")) == "exit";
        },
        mountSystem, "Exit");

    outpostStableNode->addTransition(
        [](const TAInput& input) {
            return input.type == "stable_action" && std::get<std::string>(input.parameters.at("action")) == "exit";
        },
        mountSystem, "Exit");

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

    // Connect stable nodes to equipment shop
    villageStableNode->addTransition(
        [](const TAInput& input) {
            return input.type == "stable_action" && std::get<std::string>(input.parameters.at("action")) == "shop";
        },
        equipmentShop, "Visit equipment shop");

    equipmentShop->addTransition(
        [](const TAInput& input) {
            return input.type == "shop_action" && std::get<std::string>(input.parameters.at("action")) == "exit";
        },
        villageStableNode, "Exit");

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