// System_ReligionDeity_JSON.hpp
#ifndef SYSTEM_RELIGION_DEITY_JSON_HPP
#define SYSTEM_RELIGION_DEITY_JSON_HPP

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

#include <nlohmann/json.hpp>
using json = nlohmann::json;

// Forward declarations
class TANode;
class TAController;
class Inventory;
class NPC;
class Recipe;
class QuestNode;
class DialogueNode;
class SkillNode;
class ClassNode;
class CraftingNode;
class LocationNode;
class TimeNode;
struct NodeID;
struct TAInput;
struct TAAction;
struct TATransitionRule;
struct CharacterStats;
struct WorldState;
struct GameContext;
struct Item;

// New forward declarations for Religion System
class DeityNode;
class TempleNode;
class RitualNode;
class PrayerNode;
class BlessingNode;
class ReligiousQuestNode;

//----------------------------------------
// RELIGION SYSTEM EXTENSIONS
//----------------------------------------

// Add religious fields to CharacterStats
struct ReligiousStats {
    std::map<std::string, int> deityFavor; // Favor level with each deity
    std::map<std::string, int> deityDevotion; // Devotion points spent
    std::map<std::string, bool> deityAlignment; // If true, player is aligned with deity
    std::string primaryDeity; // Current primary deity
    std::set<std::string> completedRituals; // Completed ritual IDs
    std::set<std::string> activeBlessing; // Active blessing effects
    std::map<std::string, int> blessingDuration; // Remaining time on blessings

    ReligiousStats();
    void changeFavor(const std::string& deity, int amount);
    void addDevotion(const std::string& deity, int points);
    bool hasMinimumFavor(const std::string& deity, int minimumFavor) const;
    bool hasBlessingActive(const std::string& blessing) const;
    void addBlessing(const std::string& blessing, int duration);
    void removeBlessing(const std::string& blessing);
    void updateBlessings();
    void setPrimaryDeity(const std::string& deity);
    bool hasCompletedRitual(const std::string& ritualId) const;
    void markRitualCompleted(const std::string& ritualId);
    void initializeDeities(const std::vector<std::string>& deityIds);
};

// Extension to GameContext to include religious stats
struct ReligiousGameContext : public GameContext {
    ReligiousStats religiousStats;
    std::map<std::string, std::string> templeJournal; // Records temple visits and rituals
    std::map<int, std::string> holyDayCalendar; // Days of the year for holy celebrations
    std::map<int, std::string> holyDayDeities; // Map days to deity IDs

    ReligiousGameContext();
    void loadHolyDays(const json& holyDayData);
    int getCurrentDayOfYear() const;
    bool isHolyDay() const;
    std::string getCurrentHolyDay() const;
    std::string getDeityOfCurrentHolyDay() const;
};

// Base class for deity-specific nodes
class DeityNode : public TANode {
public:
    std::string deityId;
    std::string deityName;
    std::string deityTitle;
    std::string deityDescription;
    std::string deityDomain;
    std::string alignmentRequirement; // "good", "neutral", "evil"

    // Opposing deities
    std::vector<std::string> opposingDeities;

    // Favored/disfavored actions
    struct DeityAlignment {
        std::string action;
        int favorChange;
        std::string description;
    };
    std::vector<DeityAlignment> favoredActions;
    std::vector<DeityAlignment> disfavoredActions;

    // Blessings this deity can grant
    std::vector<BlessingNode*> availableBlessings;

    // Temples dedicated to this deity
    std::vector<TempleNode*> temples;

    DeityNode(const std::string& name, const std::string& id, const std::string& title);
    void loadFromJson(const json& deityData);
    void onEnter(ReligiousGameContext* context);
    bool isHolyDay(ReligiousGameContext* context) const;
    std::string getFavorLevel(int favor) const;
    bool canGrantBlessing(ReligiousGameContext* context, const std::string& blessingId) const;
    virtual void processDevotionAction(ReligiousGameContext* context, const std::string& actionType);
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;
};

// Represents a temple location dedicated to a specific deity
class TempleNode : public TANode {
public:
    std::string templeId;
    std::string templeName;
    std::string templeLocation;
    std::string templeDescription;
    std::string deityId;

    // Temple personnel
    struct TemplePriest {
        std::string name;
        std::string title;
        std::string description;
        int rank; // 1-5, with 5 being highest
    };
    std::vector<TemplePriest> priests;

    // Available rituals at this temple
    std::vector<RitualNode*> availableRituals;

    // Offerings that can be made
    struct TempleOffering {
        std::string name;
        std::string itemId;
        int quantity;
        int favorReward;
        std::string description;
    };
    std::vector<TempleOffering> possibleOfferings;

    // Temple services
    bool providesBlessings;
    bool providesHealing;
    bool providesCurseRemoval;
    bool providesDivination;
    int serviceQuality; // 1-5, affects cost and effectiveness

    TempleNode(const std::string& id, const std::string& name, const std::string& location, const std::string& deity);
    void loadFromJson(const json& templeData);
    void onEnter(ReligiousGameContext* context);
    DeityNode* findDeityNode(ReligiousGameContext* context) const;
    bool makeOffering(ReligiousGameContext* context, const std::string& itemId, int quantity);
    bool provideHealing(ReligiousGameContext* context, int gold);
    bool removeCurse(ReligiousGameContext* context, int gold);
    std::string performDivination(ReligiousGameContext* context, int gold);
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;
};

// Represents a religious ritual that can be performed
class RitualNode : public TANode {
public:
    std::string ritualId;
    std::string ritualName;
    std::string ritualDescription;
    std::string deityId;

    // Requirements to perform the ritual
    int favorRequirement;
    bool requiresHolyDay;
    bool requiresPrimaryDeity;
    std::map<std::string, int> itemRequirements;
    int goldCost;

    // Effects of the ritual
    int favorReward;
    int skillBoost; // Optional skill increase
    std::string skillAffected;
    std::string blessingGranted;
    int blessingDuration;

    // Ritual complexity
    enum RitualComplexity {
        SIMPLE = 1,
        MODERATE = 2,
        COMPLEX = 3,
        ELABORATE = 4,
        GRAND = 5
    };
    RitualComplexity complexity;

    RitualNode(const std::string& id, const std::string& name, const std::string& deity);
    void loadFromJson(const json& ritualData);
    void onEnter(ReligiousGameContext* context);
    DeityNode* findDeityNode(ReligiousGameContext* context) const;
    bool canPerformRitual(ReligiousGameContext* context) const;
    bool performRitual(ReligiousGameContext* context);
    std::vector<TAAction> getAvailableActions() override;
};

// Prayer system for making requests to deities
class PrayerNode : public TANode {
public:
    std::string deityId;
    std::string prayerDescription;

    // Different prayer types
    enum PrayerType {
        GUIDANCE,
        BLESSING,
        PROTECTION,
        HEALING,
        STRENGTH,
        FORTUNE,
        FORGIVENESS
    };

    // Prayer results based on favor levels
    struct PrayerResult {
        std::string description;
        std::map<std::string, int> statEffects;
        std::string blessingGranted;
        int blessingDuration;
        bool curseRemoved;
    };

    std::map<PrayerType, std::map<int, PrayerResult>> prayerResults;

    PrayerNode(const std::string& name, const std::string& deity);
    void initializePrayerResults();
    void onEnter(ReligiousGameContext* context);
    DeityNode* findDeityNode(ReligiousGameContext* context) const;
    PrayerResult getPrayerOutcome(ReligiousGameContext* context, PrayerType type);
    void performPrayer(ReligiousGameContext* context, PrayerType type);
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;
};

//----------------------------------------
// BLESSING SYSTEM
//----------------------------------------

// Blessing effects that can be granted by deities
class BlessingNode : public TANode {
public:
    std::string blessingId;
    std::string blessingName;
    std::string blessingDescription;
    std::string deityId;

    // Blessing tier
    enum BlessingTier {
        MINOR = 1,
        MODERATE = 2,
        MAJOR = 3,
        GREATER = 4,
        DIVINE = 5
    };
    BlessingTier tier;

    // Favor requirement to receive blessing
    int favorRequirement;

    // Duration in game days
    int duration;

    // Effects of the blessing
    struct BlessingEffect {
        std::string type; // "stat", "skill", "protection", "ability", etc.
        std::string target; // Specific stat, skill, damage type, etc.
        int magnitude;
        std::string description;
    };
    std::vector<BlessingEffect> effects;

    BlessingNode(const std::string& id, const std::string& name, const std::string& deity, BlessingTier t);
    void loadFromJson(const json& blessingData);
    void onEnter(ReligiousGameContext* context);
    std::string getTierName() const;
    bool canReceiveBlessing(ReligiousGameContext* context) const;
    bool grantBlessing(ReligiousGameContext* context);
    std::vector<TAAction> getAvailableActions() override;
};

// Religious quests related to deities
class ReligiousQuestNode : public QuestNode {
public:
    std::string deityId;
    int favorReward;
    int devotionReward;

    // Effects on the world when completed
    std::map<std::string, std::string> worldStateChanges;

    ReligiousQuestNode(const std::string& name, const std::string& deity);
    void loadFromJson(const json& questData);
    void onEnter(GameContext* baseContext) override;
    void onExit(GameContext* baseContext) override;
    DeityNode* findDeityNode(ReligiousGameContext* context) const;
};

//----------------------------------------
// RELIGION SYSTEM CONTROLLER
//----------------------------------------

// Extension to TAController for Religion System
class ReligionTAController : public TAController {
public:
    // Root node for religion system
    TANode* religionRoot;

    // All deities in the pantheon
    std::vector<DeityNode*> pantheon;

    // All temples in the world
    std::vector<TempleNode*> temples;

    // All known rituals
    std::vector<RitualNode*> rituals;

    // All available blessings
    std::vector<BlessingNode*> blessings;

    // All religious quests
    std::vector<ReligiousQuestNode*> religiousQuests;

    // Convert base GameContext to ReligiousGameContext
    ReligiousGameContext* getReligiousContext();

    // Initialize the religion system from JSON
    void initializeReligionSystem(const std::string& jsonFilePath);

private:
    // Create all deities in the pantheon from JSON
    void createDeitiesFromJson(const json& deitiesData);

    // Create temples from JSON
    void createTemplesFromJson(const json& templesData);

    // Create rituals from JSON
    void createRitualsFromJson(const json& ritualsData);

    // Create blessings from JSON
    void createBlessingsFromJson(const json& blessingsData);

    // Create religious quests from JSON
    void createReligiousQuestsFromJson(const json& questsData);

    // Set up hierarchy for religion system
    void setupReligionHierarchy();
};

#endif // SYSTEM_RELIGION_DEITY_JSON_HPP