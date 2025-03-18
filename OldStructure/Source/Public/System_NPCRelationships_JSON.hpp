// System_NPCRelationships_JSON.hpp
#ifndef SYSTEM_NPC_RELATIONSHIPS_JSON_HPP
#define SYSTEM_NPC_RELATIONSHIPS_JSON_HPP

#include <algorithm>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <random>
#include <set>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

// Forward declarations for Raw Oath types
class TANode;
class TAController;
class GameContext;
struct TAInput;
struct NodeID;
struct CharacterStats;

// Personality trait enumeration
enum class PersonalityTrait {
    Agreeable,
    Ambitious,
    Brave,
    Cautious,
    Cheerful,
    Curious,
    Disciplined,
    Generous,
    Honest,
    Introverted,
    Extroverted,
    Loyal,
    Materialistic,
    Merciful,
    Modest,
    Practical,
    Rational,
    Religious,
    Romantic,
    Scholarly,
    Spiritual,
    Stubborn,
    Traditional,
    Trustworthy,
    Vengeful,
    Wise,
    Independent,
    Intelligent,
    Diplomatic,
    Suspicious
};

// Relationship types
enum class RelationshipType {
    None,
    Acquaintance,
    Friend,
    CloseFriend,
    BestFriend,
    Rival,
    Enemy,
    Family,
    Mentor,
    Student,
    RomanticInterest,
    Partner,
    Spouse
};

// Relationship states (current mood/disposition)
enum class RelationshipState {
    Neutral,
    Happy,
    Sad,
    Angry,
    Fearful,
    Trusting,
    Suspicious,
    Grateful,
    Jealous,
    Impressed,
    Disappointed,
    Forgiving,
    Resentful
};

// Gift preference categories
enum class GiftCategory {
    Weapon,
    Armor,
    Jewelry,
    Book,
    Food,
    Drink,
    Potion,
    Clothing,
    Crafting,
    Decoration,
    Tool,
    Magic,
    ReligiousItem,
    Trophy,
    Luxury
};

// Global config class for relationship system
class RelationshipConfig {
private:
    json configData;
    static RelationshipConfig* instance;

    // Private constructor for singleton
    RelationshipConfig();

public:
    // Singleton access
    static RelationshipConfig& getInstance();
    bool loadConfig(const std::string& filename);

    // Getter methods for constants
    int getMinRelationship() const;
    int getMaxRelationship() const;
    int getHatredThreshold() const;
    int getDislikeThreshold() const;
    int getNeutralThreshold() const;
    int getFriendlyThreshold() const;
    int getCloseThreshold() const;
    int getIntimateThreshold() const;
    float getSharedTraitBonus() const;
    float getOpposingTraitPenalty() const;
    float getFavoriteGiftMultiplier() const;
    float getLikedGiftMultiplier() const;
    float getDislikedGiftMultiplier() const;
    float getHatedGiftMultiplier() const;
    int getDailyDecayAmount() const;
    int getMinDaysBetweenGifts() const;

    // Get all opposing traits pairs
    std::map<PersonalityTrait, PersonalityTrait> getOpposingTraits() const;

    // Get all NPCs from config
    json getNPCs() const;

    // Get default relationships
    json getDefaultRelationships() const;

    // Helper methods for enum conversions
    PersonalityTrait getPersonalityTraitFromString(const std::string& traitName) const;
    std::string getPersonalityTraitString(PersonalityTrait trait) const;
    RelationshipType getRelationshipTypeFromString(const std::string& typeName) const;
    std::string getRelationshipTypeString(RelationshipType type) const;
    RelationshipState getRelationshipStateFromString(const std::string& stateName) const;
    std::string getRelationshipStateString(RelationshipState state) const;
    GiftCategory getGiftCategoryFromString(const std::string& categoryName) const;
    std::string getGiftCategoryString(GiftCategory category) const;
};

// Extended NPC class with relationship features
class RelationshipNPC {
public:
    std::string id;
    std::string name;
    std::string occupation;
    int age;
    std::string gender;
    std::string race;
    std::string faction;
    std::string homeLocation;

    // Personality traits influence relationship dynamics
    std::set<PersonalityTrait> personalityTraits;

    // Daily schedule tracking
    struct ScheduleEntry {
        int startHour;
        int endHour;
        std::string location;
        std::string activity;
    };
    std::vector<ScheduleEntry> weekdaySchedule;
    std::vector<ScheduleEntry> weekendSchedule;

    // Gift preferences
    std::map<GiftCategory, float> giftPreferences; // -1.0 to 1.0 preference scale
    std::set<std::string> favoriteItems;
    std::set<std::string> dislikedItems;

    // Conversation preferences
    std::set<std::string> conversationTopics;
    std::set<std::string> tabooTopics;

    // Relationship network with other NPCs
    struct NPCRelationship {
        std::string npcId;
        RelationshipType type;
        int value; // -100 to 100
        RelationshipState currentState;
        std::string historyNotes;
    };
    std::vector<NPCRelationship> relationships;

    RelationshipNPC(const std::string& npcId, const std::string& npcName);
    RelationshipNPC(const json& npcData);

    void addTrait(PersonalityTrait trait);
    float calculateTraitCompatibility(const RelationshipNPC& other) const;
    NPCRelationship* findRelationship(const std::string& npcId);
    ScheduleEntry getCurrentSchedule(int day, int hour);
    float getGiftReaction(const std::string& itemId, GiftCategory category);
    void addScheduleEntry(bool weekend, int start, int end, const std::string& location, const std::string& activity);
    void setGiftPreference(GiftCategory category, float preference);
    json toJson() const;
};

// Player-NPC relationship tracker
class NPCRelationshipManager {
private:
    std::map<std::string, RelationshipNPC> npcs;
    std::map<std::string, int> playerRelationships; // NPC ID -> relationship value
    std::map<std::string, RelationshipType> playerRelationshipTypes;
    std::map<std::string, RelationshipState> playerRelationshipStates;
    std::map<std::string, int> lastGiftDay; // NPC ID -> game day when last gift was given
    int currentGameDay;

public:
    NPCRelationshipManager();

    void loadNPCsFromConfig();
    void registerNPC(const RelationshipNPC& npc);
    RelationshipNPC* getNPC(const std::string& npcId);
    void changeRelationship(const std::string& npcId, int amount);
    void updateRelationshipType(const std::string& npcId);
    bool giveGift(const std::string& npcId, const std::string& itemId, GiftCategory category, int itemValue);
    void handleConversation(const std::string& npcId, const std::string& topic, bool isPositive);
    void advanceDay();
    void handleTaskCompletion(const std::string& npcId, int importance);
    void handleBetrayal(const std::string& npcId, int severity);
    std::string getRelationshipDescription(const std::string& npcId);
    std::vector<std::string> getNPCsAtLocation(const std::string& location, int day, int hour);
    bool changeRelationshipType(const std::string& npcId, RelationshipType newType, bool force = false);
    int getRelationshipValue(const std::string& npcId);
    RelationshipType getRelationshipType(const std::string& npcId);
    RelationshipState getRelationshipState(const std::string& npcId);
    bool saveRelationships(const std::string& filename);
    bool loadRelationships(const std::string& filename);

    // Getter for player relationships map
    const std::map<std::string, int>& getPlayerRelationships() const
    {
        return playerRelationships;
    }
};

// Node for NPC interaction
class NPCInteractionNode : public TANode {
private:
    NPCRelationshipManager* relationshipManager;
    std::string currentNPCId;

public:
    NPCInteractionNode(const std::string& name, NPCRelationshipManager* manager);
    void setCurrentNPC(const std::string& npcId);
    void onEnter(GameContext* context) override;
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;
};

// Node for viewing NPC information
class NPCInfoNode : public TANode {
private:
    NPCRelationshipManager* relationshipManager;
    std::string currentNPCId;

public:
    NPCInfoNode(const std::string& name, NPCRelationshipManager* manager);
    void setCurrentNPC(const std::string& npcId);
    void onEnter(GameContext* context) override;
    std::vector<TAAction> getAvailableActions() override;
};

// Node for browsing relationships
class RelationshipBrowserNode : public TANode {
private:
    NPCRelationshipManager* relationshipManager;
    std::vector<std::string> currentNPCList;
    NPCInteractionNode* interactionNode;
    NPCInfoNode* infoNode;

public:
    // Filter type for browsing
    enum class BrowseFilter {
        All,
        Friends,
        RomanticRelations,
        FamilyMembers,
        Acquaintances,
        Rivals
    };

    BrowseFilter currentFilter;

    RelationshipBrowserNode(const std::string& name, NPCRelationshipManager* manager,
        NPCInteractionNode* interaction, NPCInfoNode* info);

    void onEnter(GameContext* context) override;
    void updateNPCList();
    void displayNPCList();
    std::string getFilterName(BrowseFilter filter);
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;
};

// Main relationship system controller
class RelationshipSystemController {
private:
    NPCRelationshipManager relationshipManager;
    TAController* controller;

    // Nodes for the relationship system
    RelationshipBrowserNode* browserNode;
    NPCInteractionNode* interactionNode;
    NPCInfoNode* infoNode;

public:
    RelationshipSystemController(TAController* gameController);

    // Helper method to access relationship manager
    NPCRelationshipManager* getRelationshipManager();

    // Update time of day - call this when game time advances
    void updateTimeOfDay(int day, int hour);

    // Process events like NPC birthdays, special occasions
    void processSpecialEvents(int day);

    // Save/load relationship system
    bool saveRelationshipSystem(const std::string& filename);
    bool loadRelationshipSystem(const std::string& filename);
};

#endif // SYSTEM_NPC_RELATIONSHIPS_JSON_HPP