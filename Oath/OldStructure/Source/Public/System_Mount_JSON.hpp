// System_Mount_JSON.hpp
#ifndef SYSTEM_MOUNT_JSON_HPP
#define SYSTEM_MOUNT_JSON_HPP

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
    int stamina;
    int maxStamina;
    int staminaRegen;
    int speed;
    int carryCapacity;
    int loyalty;
    int training;

    // Track the mount's condition
    int hunger;
    int fatigue;
    int health;
    int maxHealth;

    // Special ability flags
    bool canJump;
    bool canSwim;
    bool canClimb;

    // Training specialties
    std::map<std::string, int> specialTraining;

    MountStats();
    void loadFromJson(const json& j);
    bool isExhausted() const;
    bool isStarving() const;
    bool isInjured() const;
    int getEffectiveSpeed() const;
    bool useStamina(int amount);
    void rest(int minutes);
    void feed(int nutritionValue);
    void train(const std::string& area, int amount);
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
MountEquipmentSlot stringToSlot(const std::string& slotStr);

// Helper function to convert MountEquipmentSlot to string
std::string slotToString(MountEquipmentSlot slot);

// Mount equipment item
struct MountEquipment {
    std::string id;
    std::string name;
    std::string description;
    MountEquipmentSlot slot;
    int quality;
    int durability;
    int maxDurability;
    int price;
    std::map<std::string, int> statModifiers;

    MountEquipment(const std::string& itemId, const std::string& itemName, MountEquipmentSlot itemSlot);
    static MountEquipment* createFromJson(const json& j);
    bool isWorn() const;
    void applyModifiers(MountStats& stats) const;
    void use(int intensity = 1);
    void repair(int amount);
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

    static SpecialAbilityInfo fromJson(const json& j);
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

    MountBreed(const std::string& breedId, const std::string& breedName);
    static MountBreed* createFromJson(const json& j);
    void initializeMountStats(MountStats& stats) const;
};

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

// Complete mount class including stats, equipment, and state
class Mount {
public:
    std::string id;
    std::string name;
    MountBreed* breed;
    MountStats stats;
    int age;
    std::string color;

    // Current state
    bool isOwned;
    bool isStabled;
    bool isSummoned;
    bool isMounted;

    // Equipped items
    std::map<MountEquipmentSlot, MountEquipment*> equippedItems;

    Mount(const std::string& mountId, const std::string& mountName, MountBreed* mountBreed);
    static Mount* createFromTemplate(const json& templateJson, MountBreed* breed, MountSystemConfig& config);
    MountStats getEffectiveStats() const;
    bool equipItem(MountEquipment* equipment);
    MountEquipment* unequipItem(MountEquipmentSlot slot);
    void update(int minutes);
    float getTravelTimeModifier() const;
    bool useSpecialAbility(const std::string& ability, const MountSystemConfig& config);
    std::string getStateDescription() const;
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

    MountStable(const std::string& stableId, const std::string& stableName, const std::string& stableLocation, int stableCapacity = 5);
    static MountStable* createFromJson(const json& j, std::map<std::string, MountBreed*>& breedTypes, MountSystemConfig& config);
    bool hasSpace() const;
    bool stableMount(Mount* mount);
    Mount* retrieveMount(const std::string& mountId);
    int getDailyCost() const;
    void provideDailyCare();
};

// Mount training session for improving mount abilities
class MountTrainingSession {
public:
    Mount* mount;
    std::string trainingType;
    int duration;
    int difficulty;
    int successChance;
    int experienceGain;
    MountSystemConfig& config;

    MountTrainingSession(Mount* targetMount, const std::string& type, int sessionDuration, int sessionDifficulty, MountSystemConfig& cfg);
    bool conductTraining();
};

// Mount system node types for the RPG game

// Mount stable interaction node
class MountStableNode : public TANode {
public:
    MountStable* stable;

    MountStableNode(const std::string& name, MountStable* targetStable);
    void onEnter(GameContext* context) override;
    int calculatePrice(Mount* mount) const;
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;
};

// Mount interaction node - for when actively using a mount
class MountInteractionNode : public TANode {
public:
    Mount* activeMount;
    MountSystemConfig* config;

    MountInteractionNode(const std::string& name, Mount* mount = nullptr, MountSystemConfig* cfg = nullptr);
    void setActiveMount(Mount* mount);
    void setConfig(MountSystemConfig* cfg);
    void onEnter(GameContext* context) override;
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;
};

// Mount training node - for dedicated training sessions
class MountTrainingNode : public TANode {
public:
    Mount* trainingMount;
    std::vector<std::string> trainingTypes;
    MountSystemConfig* config;

    MountTrainingNode(const std::string& name, Mount* mount = nullptr, MountSystemConfig* cfg = nullptr);
    void setTrainingMount(Mount* mount);
    void setConfig(MountSystemConfig* cfg);
    void onEnter(GameContext* context) override;
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;
};

// Mount equipment shop node
class MountEquipmentShopNode : public TANode {
public:
    std::string shopName;
    std::vector<MountEquipment*> availableEquipment;

    MountEquipmentShopNode(const std::string& name, const std::string& shop);
    void onEnter(GameContext* context) override;
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;
};

// Mount racing event node
class MountRacingNode : public TANode {
public:
    std::string trackName;
    float trackLength;
    int difficulty;
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

    MountRacingNode(const std::string& name, const std::string& track, float length, int diff);
    static MountRacingNode* createFromJson(const std::string& name, const json& j);
    void generateCompetitors(int count, const std::vector<std::string>& names = {},
        const std::vector<std::string>& lastNames = {}, int baseDifficulty = 0);
    void onEnter(GameContext* context) override;

    struct RaceResult {
        std::string name;
        float time;
        int position;
    };

    std::vector<RaceResult> simulateRace(Mount* playerMount);
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;
};

// Mount breeding center node
class MountBreedingNode : public TANode {
public:
    std::string centerName;
    std::vector<Mount*> availableForBreeding;
    int breedingFee;
    MountSystemConfig* config;

    MountBreedingNode(const std::string& name, const std::string& center, int fee = 200, MountSystemConfig* cfg = nullptr);
    void setConfig(MountSystemConfig* cfg);
    static MountBreedingNode* createFromJson(const std::string& name, const json& j,
        std::map<std::string, MountBreed*>& breedTypes, MountSystemConfig& config);
    void onEnter(GameContext* context) override;
    Mount* breedMounts(Mount* playerMount, Mount* centerMount, const std::string& foalName);
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;
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

    MountSystemController(const std::string& name, const std::string& jsonPath = "Mount.JSON");
    void loadConfig();
    void initializeBasicDefaults();
    void registerStable(MountStable* stable);
    Mount* createMount(const std::string& name, const std::string& breedId);
    bool addMount(Mount* mount);
    bool removeMount(const std::string& mountId);
    Mount* findMount(const std::string& mountId);
    void setActiveMount(Mount* mount);
    bool mountActive();
    void dismountActive();
    MountStable* findNearestStable(const std::string& playerLocation);
    void onEnter(GameContext* context) override;
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;
    void updateMounts(int minutes);
    void applyTravelEffects(float distance);
    float getMountedSpeedModifier();
    bool canPerformSpecialMovement(const std::string& movementType);
};

// Function to set up the mount system
void setupMountSystem(TAController& controller);

#endif // SYSTEM_MOUNT_JSON_HPP