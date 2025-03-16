// System_FactionReputation_JSON.hpp
#ifndef SYSTEM_FACTION_REPUTATION_JSON_HPP
#define SYSTEM_FACTION_REPUTATION_JSON_HPP

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
class NPC;
class Recipe;
class GameContext;
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

// Faction relationship structure
struct FactionRelationship {
    std::string factionA;
    std::string factionB;
    int relationValue; // -100 to 100
    std::string relationState; // "war", "hostile", "unfriendly", "neutral", "friendly", "allied"
    std::map<std::string, int> relationThresholds; // Thresholds for different relation states

    FactionRelationship(const std::string& a, const std::string& b, int value = 0);
    void loadThresholds(const json& thresholds);
    void updateState();
    void changeRelation(int amount);
};

// Faction class
class Faction {
public:
    std::string id;
    std::string name;
    std::string description;
    std::vector<std::string> territories; // Region IDs this faction controls
    std::vector<std::string> leaders; // NPC IDs of faction leaders
    std::set<std::string> members; // NPC IDs of faction members
    std::map<std::string, int> resourceInfluence; // Economic impact on resources
    std::map<std::string, int> skillBonuses; // Skill bonuses for faction members
    std::string primaryCulture; // Cultural background
    std::string politicalAlignment; // Political stance
    std::string dominantReligion; // Main religious alignment

    // Faction state management
    std::string currentState; // "stable", "growing", "declining", "at_war", "in_crisis"
    int economicPower; // 0-100
    int militaryPower; // 0-100
    int politicalInfluence; // 0-100

    // Player-specific data
    int playerRank; // 0-10, 0 = not a member
    int playerReputation; // -100 to 100
    std::string playerReputationState; // "exalted", "revered", "honored", "friendly", "neutral", "unfriendly", "hostile", "hated"
    bool playerKnown; // Does player know about this faction
    std::vector<std::string> completedQuests; // Quests completed for this faction

    // Special faction features
    std::set<std::string> specialPrivileges; // Special access granted at certain ranks
    std::map<int, std::string> rankTitles; // Titles for different ranks
    std::vector<std::string> specialLocations; // Faction-specific locations

    // Reputation state thresholds
    std::map<std::string, int> reputationThresholds;

    // Rank requirements
    std::map<int, std::pair<int, int>> rankRequirements; // rank -> {min_reputation, min_quests}

    // Rank privilege unlocks
    std::map<int, std::string> rankPrivileges;

    Faction();
    Faction(const std::string& factionId, const std::string& factionName);

    // Load faction from JSON
    static Faction fromJson(const json& j);

    // Convert faction to JSON for saving
    json toJson() const;

    void updatePlayerReputationState();
    void changeReputation(int amount);
    bool canAdvanceRank() const;
    bool tryAdvanceRank();
    float getTradeModifier() const;
    std::string getCurrentRankTitle() const;
    std::string getNextRankTitle() const;
    std::string getReputationDescription() const;
};

// Main faction system node
class FactionSystemNode : public TANode {
public:
    std::map<std::string, Faction> factions;
    std::map<std::string, std::map<std::string, FactionRelationship>> factionRelations;
    std::string jsonFilePath;

    FactionSystemNode(const std::string& name, const std::string& configFile = "FactionReputation.json");

    // Load faction system data from JSON file
    void loadFromJson();

    // Save faction system data to JSON file
    bool saveToJson(const std::string& outputPath = "");

    void addFaction(const Faction& faction);
    bool adjustFactionRelation(const std::string& factionA, const std::string& factionB, int amount);
    int getFactionRelation(const std::string& factionA, const std::string& factionB) const;
    std::string getFactionRelationState(const std::string& factionA, const std::string& factionB) const;

    // Change player reputation with a faction
    bool changePlayerReputation(const std::string& factionId, int amount, GameContext* context = nullptr);

    // Apply reputation changes to other factions based on their relationships
    void applyReputationRippleEffects(const std::string& primaryFactionId, int primaryAmount);

    void onEnter(GameContext* context) override;
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;

    void displayFactionDetails(const std::string& factionId);
    void displayFactionRelations();
    void advanceFactionRank(const std::string& factionId);
};

// Faction-specific quest node
class FactionQuestNode : public QuestNode {
public:
    std::string factionId;
    int reputationReward;
    bool rankAdvancement;
    std::map<std::string, int> reputationEffectsOnOtherFactions;

    FactionQuestNode(const std::string& name, const std::string& faction);
    void onExit(GameContext* context) override;

private:
    // Helper function to find the faction system
    FactionSystemNode* findFactionSystem(GameContext* context);
};

// Faction-related dialogue node
class FactionDialogueNode : public DialogueNode {
public:
    std::string factionId;
    int reputationEffect;

    FactionDialogueNode(const std::string& name, const std::string& speaker,
        const std::string& text, const std::string& faction = "",
        int repEffect = 0);
    void onEnter(GameContext* context) override;

private:
    // Helper function to find the faction system
    FactionSystemNode* findFactionSystem(GameContext* context);
};

// Faction-specific location with access control
class FactionLocationNode : public LocationNode {
public:
    std::string controllingFactionId;
    int minReputationRequired;
    int minRankRequired;
    bool restrictAccess;
    std::map<std::string, float> servicePriceModifiers; // Affects prices for different services

    FactionLocationNode(const std::string& name, const std::string& location,
        const std::string& faction, int minReputation = 0, int minRank = 0);

    bool canAccess(const GameContext& context) override;
    void onEnter(GameContext* context) override;
    float getPriceModifier(const std::string& serviceType, GameContext* context) const;

private:
    // Helper function to find the faction system
    FactionSystemNode* findFactionSystem(const GameContext* context) const;
};

// Political shift event that changes faction relations
class FactionPoliticalShiftEvent {
public:
    std::string name;
    std::string description;
    std::map<std::string, int> factionPowerShifts; // Changes to economic/military/political power
    std::vector<std::tuple<std::string, std::string, int>> relationShifts; // Changes to relations (factionA, factionB, amount)
    std::function<bool(const GameContext&)> condition;
    bool hasOccurred;
    int daysTillNextCheck;

    FactionPoliticalShiftEvent(const std::string& eventName, const std::string& eventDesc);

    // Create from JSON
    static FactionPoliticalShiftEvent fromJson(const json& j);

    bool checkAndExecute(GameContext* context, FactionSystemNode* factionSystem);

private:
    void updateFactionState(Faction& faction);
};

#endif // SYSTEM_FACTION_REPUTATION_JSON_HPP