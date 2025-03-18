// System_NPCRelationships_JSON.cpp

#include "System_NPCRelationships_JSON.hpp"

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
    RelationshipConfig()
    {
        loadConfig("NPCRelationships.json");
    }

public:
    // Singleton access
    static RelationshipConfig& getInstance()
    {
        if (instance == nullptr) {
            instance = new RelationshipConfig();
        }
        return *instance;
    }

    bool loadConfig(const std::string& filename)
    {
        try {
            std::ifstream file(filename);
            if (!file.is_open()) {
                std::cerr << "Error: Could not open config file: " << filename << std::endl;
                return false;
            }
            file >> configData;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error loading config: " << e.what() << std::endl;
            return false;
        }
    }

    // Getter methods for constants
    int getMinRelationship() const
    {
        return configData["relationshipConstants"]["minRelationship"];
    }

    int getMaxRelationship() const
    {
        return configData["relationshipConstants"]["maxRelationship"];
    }

    int getHatredThreshold() const
    {
        return configData["relationshipConstants"]["thresholds"]["hatred"];
    }

    int getDislikeThreshold() const
    {
        return configData["relationshipConstants"]["thresholds"]["dislike"];
    }

    int getNeutralThreshold() const
    {
        return configData["relationshipConstants"]["thresholds"]["neutral"];
    }

    int getFriendlyThreshold() const
    {
        return configData["relationshipConstants"]["thresholds"]["friendly"];
    }

    int getCloseThreshold() const
    {
        return configData["relationshipConstants"]["thresholds"]["close"];
    }

    int getIntimateThreshold() const
    {
        return configData["relationshipConstants"]["thresholds"]["intimate"];
    }

    float getSharedTraitBonus() const
    {
        return configData["relationshipConstants"]["traitInfluence"]["sharedTraitBonus"];
    }

    float getOpposingTraitPenalty() const
    {
        return configData["relationshipConstants"]["traitInfluence"]["opposingTraitPenalty"];
    }

    float getFavoriteGiftMultiplier() const
    {
        return configData["relationshipConstants"]["giftMultipliers"]["favorite"];
    }

    float getLikedGiftMultiplier() const
    {
        return configData["relationshipConstants"]["giftMultipliers"]["liked"];
    }

    float getDislikedGiftMultiplier() const
    {
        return configData["relationshipConstants"]["giftMultipliers"]["disliked"];
    }

    float getHatedGiftMultiplier() const
    {
        return configData["relationshipConstants"]["giftMultipliers"]["hated"];
    }

    int getDailyDecayAmount() const
    {
        return configData["relationshipConstants"]["timeConstants"]["dailyDecayAmount"];
    }

    int getMinDaysBetweenGifts() const
    {
        return configData["relationshipConstants"]["timeConstants"]["minDaysBetweenGifts"];
    }

    // Get all opposing traits pairs
    std::map<PersonalityTrait, PersonalityTrait> getOpposingTraits() const
    {
        std::map<PersonalityTrait, PersonalityTrait> result;

        for (const auto& pair : configData["opposingTraits"]) {
            std::string trait1 = pair["trait1"];
            std::string trait2 = pair["trait2"];

            result[getPersonalityTraitFromString(trait1)] = getPersonalityTraitFromString(trait2);
        }

        return result;
    }

    // Get all NPCs from config
    json getNPCs() const
    {
        return configData["npcs"];
    }

    // Get default relationships
    json getDefaultRelationships() const
    {
        return configData["defaultRelationships"];
    }

    // Helper method to convert string to PersonalityTrait enum
    PersonalityTrait getPersonalityTraitFromString(const std::string& traitName) const
    {
        for (size_t i = 0; i < configData["personalityTraits"].size(); i++) {
            if (configData["personalityTraits"][i] == traitName) {
                return static_cast<PersonalityTrait>(i);
            }
        }
        // Default if not found
        return PersonalityTrait::Agreeable;
    }

    // Helper method to convert PersonalityTrait enum to string
    std::string getPersonalityTraitString(PersonalityTrait trait) const
    {
        int index = static_cast<int>(trait);
        if (index >= 0 && index < static_cast<int>(configData["personalityTraits"].size())) {
            return configData["personalityTraits"][index];
        }
        return "Unknown";
    }

    // Helper method to convert string to RelationshipType enum
    RelationshipType getRelationshipTypeFromString(const std::string& typeName) const
    {
        for (size_t i = 0; i < configData["relationshipTypes"].size(); i++) {
            if (configData["relationshipTypes"][i] == typeName) {
                return static_cast<RelationshipType>(i);
            }
        }
        return RelationshipType::None;
    }

    // Helper method to convert RelationshipType enum to string
    std::string getRelationshipTypeString(RelationshipType type) const
    {
        int index = static_cast<int>(type);
        if (index >= 0 && index < static_cast<int>(configData["relationshipTypes"].size())) {
            return configData["relationshipTypes"][index];
        }
        return "Unknown";
    }

    // Helper method to convert string to RelationshipState enum
    RelationshipState getRelationshipStateFromString(const std::string& stateName) const
    {
        for (size_t i = 0; i < configData["relationshipStates"].size(); i++) {
            if (configData["relationshipStates"][i] == stateName) {
                return static_cast<RelationshipState>(i);
            }
        }
        return RelationshipState::Neutral;
    }

    // Helper method to convert RelationshipState enum to string
    std::string getRelationshipStateString(RelationshipState state) const
    {
        int index = static_cast<int>(state);
        if (index >= 0 && index < static_cast<int>(configData["relationshipStates"].size())) {
            return configData["relationshipStates"][index];
        }
        return "Unknown";
    }

    // Helper method to convert string to GiftCategory enum
    GiftCategory getGiftCategoryFromString(const std::string& categoryName) const
    {
        for (size_t i = 0; i < configData["giftCategories"].size(); i++) {
            if (configData["giftCategories"][i] == categoryName) {
                return static_cast<GiftCategory>(i);
            }
        }
        return GiftCategory::Weapon; // Default
    }

    // Helper method to convert GiftCategory enum to string
    std::string getGiftCategoryString(GiftCategory category) const
    {
        int index = static_cast<int>(category);
        if (index >= 0 && index < static_cast<int>(configData["giftCategories"].size())) {
            return configData["giftCategories"][index];
        }
        return "Unknown";
    }
};

// Initialize the static instance pointer
RelationshipConfig* RelationshipConfig::instance = nullptr;

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
    // Relationship network with other NPCs
    struct NPCRelationship {
        std::string npcId;
        RelationshipType type;
        int value; // -100 to 100
        RelationshipState currentState;
        std::string historyNotes;
    };
    std::vector<NPCRelationship> relationships;

    RelationshipNPC(const std::string& npcId, const std::string& npcName)
        : id(npcId)
        , name(npcName)
        , age(30)
        , gender("Unknown")
        , race("Human")
        , faction("Neutral")
        , homeLocation("Nowhere")
    {
    }

    // Constructor from JSON data
    RelationshipNPC(const json& npcData)
    {
        id = npcData["id"];
        name = npcData["name"];
        occupation = npcData["occupation"];
        age = npcData["age"];
        gender = npcData["gender"];
        race = npcData["race"];
        faction = npcData["faction"];
        homeLocation = npcData["homeLocation"];

        // Load personality traits
        RelationshipConfig& config = RelationshipConfig::getInstance();
        for (const auto& traitStr : npcData["personalityTraits"]) {
            personalityTraits.insert(config.getPersonalityTraitFromString(traitStr));
        }

        // Load gift preferences
        if (npcData.contains("giftPreferences")) {
            for (const auto& [categoryStr, value] : npcData["giftPreferences"].items()) {
                giftPreferences[config.getGiftCategoryFromString(categoryStr)] = value;
            }
        }

        // Load favorite and disliked items
        if (npcData.contains("favoriteItems")) {
            for (const auto& item : npcData["favoriteItems"]) {
                favoriteItems.insert(item);
            }
        }

        if (npcData.contains("dislikedItems")) {
            for (const auto& item : npcData["dislikedItems"]) {
                dislikedItems.insert(item);
            }
        }

        // Load conversation topics
        if (npcData.contains("conversationTopics")) {
            for (const auto& topic : npcData["conversationTopics"]) {
                conversationTopics.insert(topic);
            }
        }

        if (npcData.contains("tabooTopics")) {
            for (const auto& topic : npcData["tabooTopics"]) {
                tabooTopics.insert(topic);
            }
        }

        // Load schedule
        if (npcData.contains("schedule")) {
            if (npcData["schedule"].contains("weekday")) {
                for (const auto& entry : npcData["schedule"]["weekday"]) {
                    addScheduleEntry(false, entry["startHour"], entry["endHour"],
                        entry["location"], entry["activity"]);
                }
            }

            if (npcData["schedule"].contains("weekend")) {
                for (const auto& entry : npcData["schedule"]["weekend"]) {
                    addScheduleEntry(true, entry["startHour"], entry["endHour"],
                        entry["location"], entry["activity"]);
                }
            }
        }
    }

    // Add a personality trait
    void addTrait(PersonalityTrait trait)
    {
        personalityTraits.insert(trait);
    }

    // Calculate trait compatibility with another NPC
    float calculateTraitCompatibility(const RelationshipNPC& other) const
    {
        float compatibilityScore = 0.0f;
        RelationshipConfig& config = RelationshipConfig::getInstance();

        // Bonus for shared traits
        for (const auto& trait : personalityTraits) {
            if (other.personalityTraits.find(trait) != other.personalityTraits.end()) {
                compatibilityScore += config.getSharedTraitBonus();
            }
        }

        // Penalty for opposing traits
        auto opposingTraits = config.getOpposingTraits();
        for (const auto& [trait1, trait2] : opposingTraits) {
            bool hasTrait1 = personalityTraits.find(trait1) != personalityTraits.end();
            bool hasTrait2 = personalityTraits.find(trait2) != personalityTraits.end();
            bool otherHasTrait1 = other.personalityTraits.find(trait1) != other.personalityTraits.end();
            bool otherHasTrait2 = other.personalityTraits.find(trait2) != other.personalityTraits.end();

            if ((hasTrait1 && otherHasTrait2) || (hasTrait2 && otherHasTrait1)) {
                compatibilityScore -= config.getOpposingTraitPenalty();
            }
        }

        return compatibilityScore;
    }

    // Find relationship with a specific NPC
    NPCRelationship* findRelationship(const std::string& npcId)
    {
        for (auto& rel : relationships) {
            if (rel.npcId == npcId) {
                return &rel;
            }
        }
        return nullptr;
    }

    // Get current schedule based on time
    ScheduleEntry getCurrentSchedule(int day, int hour)
    {
        bool isWeekend = (day % 7 == 5 || day % 7 == 6); // Days 5 and 6 are weekend
        const auto& schedule = isWeekend ? weekendSchedule : weekdaySchedule;

        for (const auto& entry : schedule) {
            if (hour >= entry.startHour && hour < entry.endHour) {
                return entry;
            }
        }

        // Default schedule if nothing matches
        return { 0, 24, homeLocation, "resting" };
    }

    // Get gift reaction based on preference
    float getGiftReaction(const std::string& itemId, GiftCategory category)
    {
        RelationshipConfig& config = RelationshipConfig::getInstance();

        // Check specific item preferences first
        if (favoriteItems.find(itemId) != favoriteItems.end()) {
            return config.getFavoriteGiftMultiplier();
        }

        if (dislikedItems.find(itemId) != dislikedItems.end()) {
            return config.getHatedGiftMultiplier();
        }

        // Otherwise check category preferences
        if (giftPreferences.find(category) != giftPreferences.end()) {
            float preference = giftPreferences[category];
            if (preference > 0.5f) {
                return config.getLikedGiftMultiplier();
            } else if (preference < -0.5f) {
                return config.getDislikedGiftMultiplier();
            }
        }

        // Default neutral reaction
        return 1.0f;
    }

    // Add schedule entry
    void addScheduleEntry(bool weekend, int start, int end, const std::string& location, const std::string& activity)
    {
        ScheduleEntry entry { start, end, location, activity };
        if (weekend) {
            weekendSchedule.push_back(entry);
        } else {
            weekdaySchedule.push_back(entry);
        }
    }

    // Set gift category preference
    void setGiftPreference(GiftCategory category, float preference)
    {
        // Clamp preference between -1.0 and 1.0
        preference = std::max(-1.0f, std::min(1.0f, preference));
        giftPreferences[category] = preference;
    }

    // Convert to JSON for serialization
    json toJson() const
    {
        RelationshipConfig& config = RelationshipConfig::getInstance();
        json j;

        j["id"] = id;
        j["name"] = name;
        j["occupation"] = occupation;
        j["age"] = age;
        j["gender"] = gender;
        j["race"] = race;
        j["faction"] = faction;
        j["homeLocation"] = homeLocation;

        // Personality traits
        json traits = json::array();
        for (const auto& trait : personalityTraits) {
            traits.push_back(config.getPersonalityTraitString(trait));
        }
        j["personalityTraits"] = traits;

        // Gift preferences
        json giftPrefs;
        for (const auto& [category, value] : giftPreferences) {
            giftPrefs[config.getGiftCategoryString(category)] = value;
        }
        j["giftPreferences"] = giftPrefs;

        // Favorite and disliked items
        j["favoriteItems"] = json(favoriteItems);
        j["dislikedItems"] = json(dislikedItems);

        // Conversation topics
        j["conversationTopics"] = json(conversationTopics);
        j["tabooTopics"] = json(tabooTopics);

        // Schedule
        json schedule;

        json weekdayEntries = json::array();
        for (const auto& entry : weekdaySchedule) {
            json e;
            e["startHour"] = entry.startHour;
            e["endHour"] = entry.endHour;
            e["location"] = entry.location;
            e["activity"] = entry.activity;
            weekdayEntries.push_back(e);
        }
        schedule["weekday"] = weekdayEntries;

        json weekendEntries = json::array();
        for (const auto& entry : weekendSchedule) {
            json e;
            e["startHour"] = entry.startHour;
            e["endHour"] = entry.endHour;
            e["location"] = entry.location;
            e["activity"] = entry.activity;
            weekendEntries.push_back(e);
        }
        schedule["weekend"] = weekendEntries;

        j["schedule"] = schedule;

        // Relationships with other NPCs
        json npcRelationships = json::array();
        for (const auto& rel : relationships) {
            json r;
            r["npcId"] = rel.npcId;
            r["type"] = config.getRelationshipTypeString(rel.type);
            r["value"] = rel.value;
            r["state"] = config.getRelationshipStateString(rel.currentState);
            r["historyNotes"] = rel.historyNotes;
            npcRelationships.push_back(r);
        }
        j["relationships"] = npcRelationships;

        return j;
    }
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
    NPCRelationshipManager()
        : currentGameDay(0)
    {
        loadNPCsFromConfig();
    }

    // Load NPCs from configuration file
    void loadNPCsFromConfig()
    {
        RelationshipConfig& config = RelationshipConfig::getInstance();

        // Load NPC definitions
        const json& npcsData = config.getNPCs();
        for (const auto& npcData : npcsData) {
            RelationshipNPC npc(npcData);
            registerNPC(npc);
        }

        // Set up default relationships
        const json& defaultRelationships = config.getDefaultRelationships();
        for (const auto& rel : defaultRelationships) {
            std::string npcId = rel["npcId"];
            int value = rel["value"];
            RelationshipType type = config.getRelationshipTypeFromString(rel["type"]);

            playerRelationships[npcId] = value;
            playerRelationshipTypes[npcId] = type;
        }
    }

    // Register a new NPC
    void registerNPC(const RelationshipNPC& npc)
    {
        npcs[npc.id] = npc;
        if (playerRelationships.find(npc.id) == playerRelationships.end()) {
            playerRelationships[npc.id] = 0; // Start neutral
            playerRelationshipTypes[npc.id] = RelationshipType::None;
            playerRelationshipStates[npc.id] = RelationshipState::Neutral;
            lastGiftDay[npc.id] = -RelationshipConfig::getInstance().getMinDaysBetweenGifts(); // Allow immediate gift
        }
    }

    // Get a registered NPC
    RelationshipNPC* getNPC(const std::string& npcId)
    {
        if (npcs.find(npcId) != npcs.end()) {
            return &npcs[npcId];
        }
        return nullptr;
    }

    // Change relationship with NPC
    void changeRelationship(const std::string& npcId, int amount)
    {
        if (npcs.find(npcId) == npcs.end())
            return;

        RelationshipConfig& config = RelationshipConfig::getInstance();

        // Apply personality trait modifiers
        const RelationshipNPC& npc = npcs[npcId];

        // Vengeful NPCs remember negative actions more
        if (amount < 0 && npc.personalityTraits.find(PersonalityTrait::Vengeful) != npc.personalityTraits.end()) {
            amount = static_cast<int>(amount * 1.5f);
        }

        // Forgiving NPCs are less affected by negative actions
        if (amount < 0 && npc.personalityTraits.find(PersonalityTrait::Merciful) != npc.personalityTraits.end()) {
            amount = static_cast<int>(amount * 0.5f);
        }

        // Update relationship value with limits
        playerRelationships[npcId] = std::max(
            config.getMinRelationship(),
            std::min(config.getMaxRelationship(), playerRelationships[npcId] + amount));

        // Update relationship type based on new value
        updateRelationshipType(npcId);
    }

    // Update the relationship type based on the current value
    void updateRelationshipType(const std::string& npcId)
    {
        RelationshipConfig& config = RelationshipConfig::getInstance();
        int value = playerRelationships[npcId];

        // Update type based on value thresholds
        if (value <= config.getHatredThreshold()) {
            playerRelationshipTypes[npcId] = RelationshipType::Enemy;
        } else if (value <= config.getDislikeThreshold()) {
            playerRelationshipTypes[npcId] = RelationshipType::Rival;
        } else if (value <= config.getNeutralThreshold()) {
            playerRelationshipTypes[npcId] = RelationshipType::None;
        } else if (value <= config.getFriendlyThreshold()) {
            playerRelationshipTypes[npcId] = RelationshipType::Acquaintance;
        } else if (value <= config.getCloseThreshold()) {
            playerRelationshipTypes[npcId] = RelationshipType::Friend;
        } else if (value <= config.getIntimateThreshold()) {
            playerRelationshipTypes[npcId] = RelationshipType::CloseFriend;
        } else {
            playerRelationshipTypes[npcId] = RelationshipType::BestFriend;
        }
    }

    // Process giving a gift to an NPC
    bool giveGift(const std::string& npcId, const std::string& itemId, GiftCategory category, int itemValue)
    {
        if (npcs.find(npcId) == npcs.end())
            return false;

        RelationshipConfig& config = RelationshipConfig::getInstance();

        // Check if enough time has passed since last gift
        if (currentGameDay - lastGiftDay[npcId] < config.getMinDaysBetweenGifts()) {
            return false; // Too soon for another gift
        }

        RelationshipNPC& npc = npcs[npcId];

        // Calculate gift impact based on NPC preferences and item value
        float reactionMultiplier = npc.getGiftReaction(itemId, category);
        int relationshipChange = static_cast<int>(itemValue * reactionMultiplier * 0.1f);

        // Update relationship
        changeRelationship(npcId, relationshipChange);

        // Update gift timestamp
        lastGiftDay[npcId] = currentGameDay;

        // Update state based on gift reaction
        if (reactionMultiplier >= config.getFavoriteGiftMultiplier()) {
            playerRelationshipStates[npcId] = RelationshipState::Happy;
        } else if (reactionMultiplier <= config.getHatedGiftMultiplier()) {
            playerRelationshipStates[npcId] = RelationshipState::Disappointed;
        } else if (reactionMultiplier >= config.getLikedGiftMultiplier()) {
            playerRelationshipStates[npcId] = RelationshipState::Grateful;
        } else if (reactionMultiplier <= config.getDislikedGiftMultiplier()) {
            playerRelationshipStates[npcId] = RelationshipState::Disappointed;
        }

        return true;
    }

    // Process a conversation with an NPC about a topic
    void handleConversation(const std::string& npcId, const std::string& topic, bool isPositive)
    {
        if (npcs.find(npcId) == npcs.end())
            return;

        RelationshipNPC& npc = npcs[npcId];
        int relationshipChange = 0;

        // Check if topic is liked or disliked
        if (npc.conversationTopics.find(topic) != npc.conversationTopics.end()) {
            relationshipChange = isPositive ? 3 : 1;
        } else if (npc.tabooTopics.find(topic) != npc.tabooTopics.end()) {
            relationshipChange = isPositive ? -1 : -5;
        } else {
            relationshipChange = isPositive ? 1 : -1;
        }

        // Apply change
        changeRelationship(npcId, relationshipChange);

        // Update state based on conversation
        if (isPositive) {
            if (npc.conversationTopics.find(topic) != npc.conversationTopics.end()) {
                playerRelationshipStates[npcId] = RelationshipState::Happy;
            } else {
                playerRelationshipStates[npcId] = RelationshipState::Neutral;
            }
        } else {
            if (npc.tabooTopics.find(topic) != npc.tabooTopics.end()) {
                playerRelationshipStates[npcId] = RelationshipState::Angry;
            } else {
                playerRelationshipStates[npcId] = RelationshipState::Disappointed;
            }
        }
    }

    // Process a day passing for all relationships
    void advanceDay()
    {
        RelationshipConfig& config = RelationshipConfig::getInstance();
        currentGameDay++;

        // Natural decay in relationships over time if not maintained
        for (auto& [npcId, relationshipValue] : playerRelationships) {
            // Skip decay for close relationships
            if (relationshipValue > config.getCloseThreshold())
                continue;

            // Apply small decay
            relationshipValue += config.getDailyDecayAmount();

            // Ensure it doesn't fall below a minimum
            if (relationshipValue < config.getMinRelationship()) {
                relationshipValue = config.getMinRelationship();
            }

            // Update type if needed
            updateRelationshipType(npcId);
        }

        // Reset temporary states back to neutral after a few days
        for (auto& [npcId, state] : playerRelationshipStates) {
            if (state != RelationshipState::Neutral && rand() % 3 == 0) {
                state = RelationshipState::Neutral;
            }
        }
    }

    // Handle helping an NPC with a task/quest
    void handleTaskCompletion(const std::string& npcId, int importance)
    {
        if (npcs.find(npcId) == npcs.end())
            return;

        // Importance ranges from 1 (minor) to 10 (life-changing)
        int relationshipChange = importance * 2;
        changeRelationship(npcId, relationshipChange);

        // Update state
        if (importance >= 8) {
            playerRelationshipStates[npcId] = RelationshipState::Grateful;
        } else if (importance >= 4) {
            playerRelationshipStates[npcId] = RelationshipState::Happy;
        } else {
            playerRelationshipStates[npcId] = RelationshipState::Impressed;
        }
    }

    // Handle betraying or failing an NPC
    void handleBetrayal(const std::string& npcId, int severity)
    {
        if (npcs.find(npcId) == npcs.end())
            return;

        // Severity ranges from 1 (minor) to 10 (unforgivable)
        int relationshipChange = -severity * 3;
        changeRelationship(npcId, relationshipChange);

        // Update state
        if (severity >= 8) {
            playerRelationshipStates[npcId] = RelationshipState::Angry;
        } else if (severity >= 4) {
            playerRelationshipStates[npcId] = RelationshipState::Disappointed;
        } else {
            playerRelationshipStates[npcId] = RelationshipState::Sad;
        }
    }

    // Get relationship status for UI/dialogue
    std::string getRelationshipDescription(const std::string& npcId)
    {
        if (npcs.find(npcId) == npcs.end())
            return "Unknown";

        RelationshipConfig& config = RelationshipConfig::getInstance();
        int value = playerRelationships[npcId];
        RelationshipType type = playerRelationshipTypes[npcId];
        RelationshipState state = playerRelationshipStates[npcId];

        std::string description;

        // Base description on relationship type
        description = config.getRelationshipTypeString(type);

        // Add current state as modifier
        if (state != RelationshipState::Neutral) {
            description += " (" + config.getRelationshipStateString(state) + ")";
        }

        return description;
    }

    // Get all NPCs at a specific location at the current time
    std::vector<std::string> getNPCsAtLocation(const std::string& location, int day, int hour)
    {
        std::vector<std::string> presentNPCs;

        for (auto& [npcId, npc] : npcs) {
            auto schedule = npc.getCurrentSchedule(day, hour);
            if (schedule.location == location) {
                presentNPCs.push_back(npcId);
            }
        }

        return presentNPCs;
    }

    // Attempt to change relationship type (for special events like marriage)
    bool changeRelationshipType(const std::string& npcId, RelationshipType newType, bool force = false)
    {
        if (npcs.find(npcId) == npcs.end())
            return false;

        RelationshipConfig& config = RelationshipConfig::getInstance();
        int value = playerRelationships[npcId];

        // Check requirements for each type, unless forcing
        if (!force) {
            switch (newType) {
            case RelationshipType::Friend:
                if (value < config.getFriendlyThreshold())
                    return false;
                break;
            case RelationshipType::CloseFriend:
                if (value < config.getCloseThreshold())
                    return false;
                break;
            case RelationshipType::BestFriend:
                if (value < config.getIntimateThreshold())
                    return false;
                break;
            case RelationshipType::Partner:
                if (value < config.getIntimateThreshold())
                    return false;
                break;
            case RelationshipType::Spouse:
                if (value < config.getIntimateThreshold())
                    return false;
                // Check if already married to someone else
                for (auto& [id, type] : playerRelationshipTypes) {
                    if (type == RelationshipType::Spouse && id != npcId)
                        return false;
                }
                break;
            default:
                break;
            }
        }

        // Apply the change
        playerRelationshipTypes[npcId] = newType;

        // Boost relationship value for positive type changes
        if (newType == RelationshipType::Partner || newType == RelationshipType::Spouse || newType == RelationshipType::BestFriend) {
            playerRelationships[npcId] = config.getMaxRelationship();
        }

        return true;
    }

    // Get current relationship value
    int getRelationshipValue(const std::string& npcId)
    {
        if (playerRelationships.find(npcId) != playerRelationships.end()) {
            return playerRelationships[npcId];
        }
        return 0;
    }

    // Get current relationship type
    RelationshipType getRelationshipType(const std::string& npcId)
    {
        if (playerRelationshipTypes.find(npcId) != playerRelationshipTypes.end()) {
            return playerRelationshipTypes[npcId];
        }
        return RelationshipType::None;
    }

    // Get current relationship state
    RelationshipState getRelationshipState(const std::string& npcId)
    {
        if (playerRelationshipStates.find(npcId) != playerRelationshipStates.end()) {
            return playerRelationshipStates[npcId];
        }
        return RelationshipState::Neutral;
    }

    // Save relationship data
    bool saveRelationships(const std::string& filename)
    {
        try {
            // Create JSON structure for saving
            json saveData;

            // Save current day
            saveData["currentGameDay"] = currentGameDay;

            // Save NPCs
            json npcsData = json::array();
            for (const auto& [npcId, npc] : npcs) {
                npcsData.push_back(npc.toJson());
            }
            saveData["npcs"] = npcsData;

            // Save player relationships
            json relationships = json::array();
            for (const auto& [npcId, value] : playerRelationships) {
                RelationshipConfig& config = RelationshipConfig::getInstance();

                json rel;
                rel["npcId"] = npcId;
                rel["value"] = value;
                rel["type"] = config.getRelationshipTypeString(playerRelationshipTypes[npcId]);
                rel["state"] = config.getRelationshipStateString(playerRelationshipStates[npcId]);
                rel["lastGiftDay"] = lastGiftDay[npcId];
                relationships.push_back(rel);
            }
            saveData["playerRelationships"] = relationships;

            // Write to file
            std::ofstream file(filename);
            if (!file.is_open()) {
                return false;
            }

            file << saveData.dump(4); // Pretty print with 4-space indentation
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error saving relationships: " << e.what() << std::endl;
            return false;
        }
    }

    // Load relationship data
    bool loadRelationships(const std::string& filename)
    {
        try {
            // Open and parse the save file
            std::ifstream file(filename);
            if (!file.is_open()) {
                return false;
            }

            json saveData;
            file >> saveData;

            // Load current day
            currentGameDay = saveData["currentGameDay"];

            // Clear current data
            npcs.clear();
            playerRelationships.clear();
            playerRelationshipTypes.clear();
            playerRelationshipStates.clear();
            lastGiftDay.clear();

            // Load NPCs
            for (const auto& npcData : saveData["npcs"]) {
                RelationshipNPC npc(npcData);
                npcs[npc.id] = npc;
            }

            // Load player relationships
            RelationshipConfig& config = RelationshipConfig::getInstance();
            for (const auto& rel : saveData["playerRelationships"]) {
                std::string npcId = rel["npcId"];
                int value = rel["value"];
                RelationshipType type = config.getRelationshipTypeFromString(rel["type"]);
                RelationshipState state = config.getRelationshipStateFromString(rel["state"]);
                int lastGift = rel["lastGiftDay"];

                playerRelationships[npcId] = value;
                playerRelationshipTypes[npcId] = type;
                playerRelationshipStates[npcId] = state;
                lastGiftDay[npcId] = lastGift;
            }

            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error loading relationships: " << e.what() << std::endl;
            return false;
        }
    }
};

//----------------------------------------
// RELATIONSHIP INTERACTION NODES
//----------------------------------------

// Node for NPC interaction
class NPCInteractionNode : public TANode {
private:
    NPCRelationshipManager* relationshipManager;
    std::string currentNPCId;

public:
    NPCInteractionNode(const std::string& name, NPCRelationshipManager* manager)
        : TANode(name)
        , relationshipManager(manager)
        , currentNPCId("")
    {
    }

    void setCurrentNPC(const std::string& npcId)
    {
        currentNPCId = npcId;
    }

    void onEnter(GameContext* context) override
    {
        TANode::onEnter(context);

        if (currentNPCId.empty()) {
            std::cout << "No NPC selected for interaction." << std::endl;
            return;
        }

        RelationshipNPC* npc = relationshipManager->getNPC(currentNPCId);
        if (!npc) {
            std::cout << "Selected NPC not found in database." << std::endl;
            return;
        }

        std::string relationshipDesc = relationshipManager->getRelationshipDescription(currentNPCId);
        int relationshipValue = relationshipManager->getRelationshipValue(currentNPCId);

        std::cout << "Interacting with " << npc->name << " (" << npc->occupation << ")" << std::endl;
        std::cout << "Current relationship: " << relationshipDesc << " (" << relationshipValue << ")" << std::endl;
    }

    std::vector<TAAction> getAvailableActions() override
    {
        std::vector<TAAction> actions = TANode::getAvailableActions();

        if (currentNPCId.empty()) {
            return actions;
        }

        RelationshipNPC* npc = relationshipManager->getNPC(currentNPCId);
        if (!npc) {
            return actions;
        }

        RelationshipConfig& config = RelationshipConfig::getInstance();

        // Get relationship value to determine available actions
        int value = relationshipManager->getRelationshipValue(currentNPCId);
        RelationshipType type = relationshipManager->getRelationshipType(currentNPCId);
        // Basic actions available to everyone
        actions.push_back({ "talk_to_npc", "Talk with " + npc->name,
            [this]() -> TAInput {
                return { "npc_action", { { "action", std::string("talk") } } };
            } });

        actions.push_back({ "give_gift", "Give a gift to " + npc->name,
            [this]() -> TAInput {
                return { "npc_action", { { "action", std::string("give_gift") } } };
            } });

        // Only available for neutral+ relationships
        if (value >= config.getNeutralThreshold()) {
            actions.push_back({ "ask_about_self", "Ask about " + npc->name,
                [this]() -> TAInput {
                    return { "npc_action", { { "action", std::string("ask_about") } } };
                } });
        }

        // Only available for friendly+ relationships
        if (value >= config.getFriendlyThreshold()) {
            actions.push_back({ "request_help", "Ask for help",
                [this]() -> TAInput {
                    return { "npc_action", { { "action", std::string("request_help") } } };
                } });
        }

        // Only available for close friends or better
        if (value >= config.getCloseThreshold()) {
            actions.push_back({ "personal_request", "Make personal request",
                [this]() -> TAInput {
                    return { "npc_action", { { "action", std::string("personal_request") } } };
                } });
        }

        // Romantic options only available if not enemies/rivals and not already in a relationship
        if (value >= config.getFriendlyThreshold() && type != RelationshipType::Enemy && type != RelationshipType::Rival && type != RelationshipType::Spouse && type != RelationshipType::Partner) {

            actions.push_back({ "flirt", "Flirt with " + npc->name,
                [this]() -> TAInput {
                    return { "npc_action", { { "action", std::string("flirt") } } };
                } });
        }

        // Marriage proposal - only if already partners
        if (type == RelationshipType::Partner) {
            actions.push_back({ "propose", "Propose marriage to " + npc->name,
                [this]() -> TAInput {
                    return { "npc_action", { { "action", std::string("propose") } } };
                } });
        }

        return actions;
    }

    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override
    {
        if (input.type != "npc_action") {
            return TANode::evaluateTransition(input, outNextNode);
        }

        std::string action = std::get<std::string>(input.parameters.at("action"));
        RelationshipNPC* npc = relationshipManager->getNPC(currentNPCId);

        if (!npc) {
            return false;
        }

        RelationshipConfig& config = RelationshipConfig::getInstance();

        // Process different interaction types
        if (action == "talk") {
            // Get available topics based on relationship
            std::vector<std::string> topics;
            for (const auto& topic : npc->conversationTopics) {
                topics.push_back(topic);
            }

            // Add generic topics if we don't have enough
            if (topics.empty()) {
                topics = { "weather", "town_news", "personal", "work" };
            }

            std::cout << "What would you like to talk about with " << npc->name << "?" << std::endl;
            for (size_t i = 0; i < topics.size(); i++) {
                std::cout << i + 1 << ". " << topics[i] << std::endl;
            }

            // In a real implementation, we'd get user input here
            int topicIndex = 0; // For demonstration

            if (topicIndex >= 0 && topicIndex < static_cast<int>(topics.size())) {
                std::string topic = topics[topicIndex];
                bool isPositive = true; // Would be based on player's dialogue choices

                relationshipManager->handleConversation(currentNPCId, topic, isPositive);

                if (isPositive) {
                    std::cout << npc->name << " seems to enjoy talking about this topic." << std::endl;
                } else {
                    std::cout << npc->name << " doesn't seem interested in this topic." << std::endl;
                }
            }
        } else if (action == "give_gift") {
            // Simulate gift selection (would be UI-based in real game)
            std::string itemId = "silver_necklace";
            GiftCategory category = GiftCategory::Jewelry;
            int itemValue = 50;

            bool giftAccepted = relationshipManager->giveGift(currentNPCId, itemId, category, itemValue);

            if (giftAccepted) {
                float reaction = npc->getGiftReaction(itemId, category);

                if (reaction >= config.getFavoriteGiftMultiplier()) {
                    std::cout << npc->name << " loves this gift!" << std::endl;
                } else if (reaction >= config.getLikedGiftMultiplier()) {
                    std::cout << npc->name << " likes this gift." << std::endl;
                } else if (reaction <= config.getHatedGiftMultiplier()) {
                    std::cout << npc->name << " hates this gift!" << std::endl;
                } else if (reaction <= config.getDislikedGiftMultiplier()) {
                    std::cout << npc->name << " doesn't like this gift much." << std::endl;
                } else {
                    std::cout << npc->name << " accepts your gift politely." << std::endl;
                }
            } else {
                std::cout << "You've already given " << npc->name << " a gift recently." << std::endl;
            }
        } else if (action == "ask_about") {
            std::cout << npc->name << " tells you a bit about their life:" << std::endl;
            std::cout << "They work as a " << npc->occupation << " and live in " << npc->homeLocation << "." << std::endl;

            // Share some personal details based on relationship level
            int value = relationshipManager->getRelationshipValue(currentNPCId);
            if (value >= config.getCloseThreshold()) {
                std::cout << "They share some personal details about their past and aspirations." << std::endl;
                relationshipManager->changeRelationship(currentNPCId, 2);
            } else if (value >= config.getFriendlyThreshold()) {
                std::cout << "They tell you about their current projects and interests." << std::endl;
                relationshipManager->changeRelationship(currentNPCId, 1);
            } else {
                std::cout << "They share basic information, but remain somewhat guarded." << std::endl;
            }
        } else if (action == "flirt") {
            int value = relationshipManager->getRelationshipValue(currentNPCId);
            bool receptive = false;

            // Check if NPC is receptive to romance (based on relationship and personality)
            if (value >= config.getCloseThreshold()) {
                receptive = true;
            } else if (value >= config.getFriendlyThreshold()) {
                // More likely if the NPC has romantic personality trait
                receptive = npc->personalityTraits.find(PersonalityTrait::Romantic) != npc->personalityTraits.end();
            }

            if (receptive) {
                std::cout << npc->name << " responds positively to your flirtation." << std::endl;
                relationshipManager->changeRelationship(currentNPCId, 3);

                // If relationship is already strong, potentially advance to romantic interest
                if (value >= config.getIntimateThreshold()) {
                    relationshipManager->changeRelationshipType(currentNPCId, RelationshipType::RomanticInterest);
                    std::cout << npc->name << " seems to be developing romantic feelings for you." << std::endl;
                }
            } else {
                std::cout << npc->name << " politely deflects your advances." << std::endl;
                if (value < config.getFriendlyThreshold()) {
                    relationshipManager->changeRelationship(currentNPCId, -1);
                }
            }
        } else if (action == "propose") {
            int value = relationshipManager->getRelationshipValue(currentNPCId);

            // NPC will accept if relationship is very high
            if (value >= config.getIntimateThreshold()) {
                bool accepted = relationshipManager->changeRelationshipType(currentNPCId, RelationshipType::Spouse);

                if (accepted) {
                    std::cout << npc->name << " joyfully accepts your proposal!" << std::endl;
                    relationshipManager->changeRelationship(currentNPCId, 20);
                } else {
                    std::cout << "Something prevents " << npc->name << " from accepting your proposal." << std::endl;
                }
            } else {
                std::cout << npc->name << " isn't ready for that level of commitment yet." << std::endl;
                relationshipManager->changeRelationship(currentNPCId, -5);
            }
        } else if (action == "request_help") {
            int value = relationshipManager->getRelationshipValue(currentNPCId);

            if (value >= config.getFriendlyThreshold()) {
                std::cout << npc->name << " agrees to help you." << std::endl;

                // The level of help would depend on relationship strength
                if (value >= config.getIntimateThreshold()) {
                    std::cout << "They are willing to go to great lengths to assist you." << std::endl;
                } else if (value >= config.getCloseThreshold()) {
                    std::cout << "They offer significant assistance." << std::endl;
                } else {
                    std::cout << "They provide basic help." << std::endl;
                }
            } else {
                std::cout << npc->name << " declines to help at this time." << std::endl;
            }
        }

        // Stay in the same node after interaction
        outNextNode = this;
        return true;
    }
};

// Node for viewing NPC information
class NPCInfoNode : public TANode {
private:
    NPCRelationshipManager* relationshipManager;
    std::string currentNPCId;

public:
    NPCInfoNode(const std::string& name, NPCRelationshipManager* manager)
        : TANode(name)
        , relationshipManager(manager)
        , currentNPCId("")
    {
    }

    void setCurrentNPC(const std::string& npcId)
    {
        currentNPCId = npcId;
    }

    void onEnter(GameContext* context) override
    {
        TANode::onEnter(context);

        if (currentNPCId.empty()) {
            std::cout << "No NPC selected for viewing." << std::endl;
            return;
        }

        RelationshipNPC* npc = relationshipManager->getNPC(currentNPCId);
        if (!npc) {
            std::cout << "Selected NPC not found in database." << std::endl;
            return;
        }

        RelationshipConfig& config = RelationshipConfig::getInstance();

        // Display basic NPC info
        std::cout << "===== NPC Information =====" << std::endl;
        std::cout << "Name: " << npc->name << std::endl;
        std::cout << "Occupation: " << npc->occupation << std::endl;
        std::cout << "Age: " << npc->age << std::endl;
        std::cout << "Gender: " << npc->gender << std::endl;
        std::cout << "Race: " << npc->race << std::endl;
        std::cout << "Faction: " << npc->faction << std::endl;
        std::cout << "Home: " << npc->homeLocation << std::endl;

        // Relationship information
        std::string relationshipDesc = relationshipManager->getRelationshipDescription(currentNPCId);
        int relationshipValue = relationshipManager->getRelationshipValue(currentNPCId);
        std::cout << "\nRelationship: " << relationshipDesc << " (" << relationshipValue << ")" << std::endl;

        // Only show personality traits if relationship is good enough
        if (relationshipValue >= config.getFriendlyThreshold()) {
            std::cout << "\nPersonality traits:" << std::endl;
            for (const auto& trait : npc->personalityTraits) {
                std::cout << " - " << config.getPersonalityTraitString(trait) << std::endl;
            }
        }

        // Show gift preferences if relationship is good
        if (relationshipValue >= config.getCloseThreshold()) {
            std::cout << "\nGift preferences:" << std::endl;
            for (const auto& [category, preference] : npc->giftPreferences) {
                if (preference > 0.5f) {
                    std::cout << " - Likes " << config.getGiftCategoryString(category) << std::endl;
                } else if (preference < -0.5f) {
                    std::cout << " - Dislikes " << config.getGiftCategoryString(category) << std::endl;
                }
            }
        }

        // Show favorite items only to very close friends/partners
        if (relationshipValue >= config.getIntimateThreshold()) {
            if (!npc->favoriteItems.empty()) {
                std::cout << "\nFavorite items:" << std::endl;
                for (const auto& item : npc->favoriteItems) {
                    std::cout << " - " << item << std::endl;
                }
            }
        }

        // Show daily schedule if friends or better
        if (relationshipValue >= config.getFriendlyThreshold()) {
            std::cout << "\nTypical daily schedule:" << std::endl;
            for (const auto& entry : npc->weekdaySchedule) {
                std::cout << " - " << entry.startHour << ":00 to " << entry.endHour
                          << ":00: " << entry.activity << " at " << entry.location << std::endl;
            }
        }
    }

    std::vector<TAAction> getAvailableActions() override
    {
        std::vector<TAAction> actions = TANode::getAvailableActions();

        actions.push_back({ "return_to_interaction", "Return to interaction",
            [this]() -> TAInput {
                return { "info_action", { { "action", std::string("return") } } };
            } });

        return actions;
    }
};

// Node for browsing relationships
class RelationshipBrowserNode : public TANode {
private:
    NPCRelationshipManager* relationshipManager;
    std::vector<std::string> currentNPCList;
    NPCInteractionNode* interactionNode;
    NPCInfoNode* infoNode;

public:
    RelationshipBrowserNode(const std::string& name, NPCRelationshipManager* manager,
        NPCInteractionNode* interaction, NPCInfoNode* info)
        : TANode(name)
        , relationshipManager(manager)
        , interactionNode(interaction)
        , infoNode(info)
    {
    }

    // Filter type for browsing
    enum class BrowseFilter {
        All,
        Friends,
        RomanticRelations,
        FamilyMembers,
        Acquaintances,
        Rivals
    };

    BrowseFilter currentFilter = BrowseFilter::All;

    void onEnter(GameContext* context) override
    {
        TANode::onEnter(context);

        std::cout << "==== Relationship Browser ====" << std::endl;

        // Apply current filter
        updateNPCList();

        // Display NPCs
        displayNPCList();
    }

    void updateNPCList()
    {
        currentNPCList.clear();
        RelationshipConfig& config = RelationshipConfig::getInstance();

        // Get all NPCs with relationships
        for (const auto& [npcId, _] : relationshipManager->getPlayerRelationships()) {
            RelationshipType type = relationshipManager->getRelationshipType(npcId);
            bool includeNPC = false;

            switch (currentFilter) {
            case BrowseFilter::All:
                includeNPC = true;
                break;
            case BrowseFilter::Friends:
                includeNPC = (type == RelationshipType::Friend || type == RelationshipType::CloseFriend || type == RelationshipType::BestFriend);
                break;
            case BrowseFilter::RomanticRelations:
                includeNPC = (type == RelationshipType::RomanticInterest || type == RelationshipType::Partner || type == RelationshipType::Spouse);
                break;
            case BrowseFilter::FamilyMembers:
                includeNPC = (type == RelationshipType::Family);
                break;
            case BrowseFilter::Acquaintances:
                includeNPC = (type == RelationshipType::Acquaintance);
                break;
            case BrowseFilter::Rivals:
                includeNPC = (type == RelationshipType::Rival || type == RelationshipType::Enemy);
                break;
            }

            if (includeNPC) {
                currentNPCList.push_back(npcId);
            }
        }
    }

    void displayNPCList()
    {
        std::cout << "Current filter: " << getFilterName(currentFilter) << std::endl;

        if (currentNPCList.empty()) {
            std::cout << "No relationships found with this filter." << std::endl;
            return;
        }

        std::cout << "Select an NPC to interact with:" << std::endl;
        for (size_t i = 0; i < currentNPCList.size(); i++) {
            RelationshipNPC* npc = relationshipManager->getNPC(currentNPCList[i]);
            std::string name = npc ? npc->name : currentNPCList[i];
            std::string relationshipDesc = relationshipManager->getRelationshipDescription(currentNPCList[i]);

            std::cout << i + 1 << ". " << name << " - " << relationshipDesc << std::endl;
        }
    }

    std::string getFilterName(BrowseFilter filter)
    {
        switch (filter) {
        case BrowseFilter::All:
            return "All Relationships";
        case BrowseFilter::Friends:
            return "Friends";
        case BrowseFilter::RomanticRelations:
            return "Romantic Relations";
        case BrowseFilter::FamilyMembers:
            return "Family Members";
        case BrowseFilter::Acquaintances:
            return "Acquaintances";
        case BrowseFilter::Rivals:
            return "Rivals & Enemies";
        default:
            return "Unknown";
        }
    }

    std::vector<TAAction> getAvailableActions() override
    {
        std::vector<TAAction> actions = TANode::getAvailableActions();

        // Add filter options
        actions.push_back({ "filter_all", "Show All Relationships",
            [this]() -> TAInput {
                return { "browser_action", { { "action", std::string("filter") }, { "filter", std::string("all") } } };
            } });

        actions.push_back({ "filter_friends", "Show Friends Only",
            [this]() -> TAInput {
                return { "browser_action", { { "action", std::string("filter") }, { "filter", std::string("friends") } } };
            } });

        actions.push_back({ "filter_romantic", "Show Romantic Relations",
            [this]() -> TAInput {
                return { "browser_action", { { "action", std::string("filter") }, { "filter", std::string("romantic") } } };
            } });

        actions.push_back({ "filter_family", "Show Family Members",
            [this]() -> TAInput {
                return { "browser_action", { { "action", std::string("filter") }, { "filter", std::string("family") } } };
            } });

        // Add selection options for each NPC in the current list
        for (size_t i = 0; i < currentNPCList.size(); i++) {
            RelationshipNPC* npc = relationshipManager->getNPC(currentNPCList[i]);
            std::string name = npc ? npc->name : currentNPCList[i];

            actions.push_back({ "select_npc_" + std::to_string(i), "Select " + name,
                [this, i]() -> TAInput {
                    return { "browser_action", { { "action", std::string("select") }, { "index", static_cast<int>(i) } } };
                } });
        }

        return actions;
    }

    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override
    {
        if (input.type != "browser_action") {
            return TANode::evaluateTransition(input, outNextNode);
        }

        std::string action = std::get<std::string>(input.parameters.at("action"));

        if (action == "filter") {
            std::string filterStr = std::get<std::string>(input.parameters.at("filter"));

            if (filterStr == "all") {
                currentFilter = BrowseFilter::All;
            } else if (filterStr == "friends") {
                currentFilter = BrowseFilter::Friends;
            } else if (filterStr == "romantic") {
                currentFilter = BrowseFilter::RomanticRelations;
            } else if (filterStr == "family") {
                currentFilter = BrowseFilter::FamilyMembers;
            } else if (filterStr == "acquaintances") {
                currentFilter = BrowseFilter::Acquaintances;
            } else if (filterStr == "rivals") {
                currentFilter = BrowseFilter::Rivals;
            }

            updateNPCList();
            displayNPCList();

            // Stay in browser node after filtering
            outNextNode = this;
            return true;
        } else if (action == "select") {
            int index = std::get<int>(input.parameters.at("index"));

            if (index >= 0 && index < static_cast<int>(currentNPCList.size())) {
                std::string selectedNpcId = currentNPCList[index];

                // Set the selected NPC for the interaction node
                interactionNode->setCurrentNPC(selectedNpcId);
                infoNode->setCurrentNPC(selectedNpcId);

                // Transition to interaction node
                outNextNode = interactionNode;
                return true;
            }
        }

        return false;
    }
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
    RelationshipSystemController(TAController* gameController)
        : controller(gameController)
    {
        // Create the necessary nodes
        interactionNode = dynamic_cast<NPCInteractionNode*>(
            controller->createNode<NPCInteractionNode>("NPCInteraction", &relationshipManager));

        infoNode = dynamic_cast<NPCInfoNode*>(
            controller->createNode<NPCInfoNode>("NPCInfo", &relationshipManager));

        browserNode = dynamic_cast<RelationshipBrowserNode*>(
            controller->createNode<RelationshipBrowserNode>(
                "RelationshipBrowser", &relationshipManager, interactionNode, infoNode));

        // Set up transitions
        interactionNode->addTransition(
            [](const TAInput& input) {
                return input.type == "npc_action" && std::get<std::string>(input.parameters.at("action")) == "view_info";
            },
            infoNode, "View NPC information");

        interactionNode->addTransition(
            [](const TAInput& input) {
                return input.type == "npc_action" && std::get<std::string>(input.parameters.at("action")) == "return_to_browser";
            },
            browserNode, "Return to relationship browser");

        infoNode->addTransition(
            [](const TAInput& input) {
                return input.type == "info_action" && std::get<std::string>(input.parameters.at("action")) == "return";
            },
            interactionNode, "Return to interaction");

        // Register the relationship system root node
        controller->setSystemRoot("RelationshipSystem", browserNode);
    }

    // Helper method to access relationship manager
    NPCRelationshipManager* getRelationshipManager()
    {
        return &relationshipManager;
    }

    // Update time of day - call this when game time advances
    void updateTimeOfDay(int day, int hour)
    {
        // Move NPCs according to their schedules

        // Check for daily relationship decay
        static int lastProcessedDay = -1;
        if (day > lastProcessedDay) {
            relationshipManager.advanceDay();
            lastProcessedDay = day;
        }
    }

    // Process events like NPC birthdays, special occasions
    void processSpecialEvents(int day)
    {
        // This would check for birthdays, anniversaries, etc.
        // and potentially update relationships or trigger special interactions
    }

    // Save/load relationship system
    bool saveRelationshipSystem(const std::string& filename)
    {
        return relationshipManager.saveRelationships(filename);
    }

    bool loadRelationshipSystem(const std::string& filename)
    {
        return relationshipManager.loadRelationships(filename);
    }
};

//----------------------------------------
// INTEGRATION WITH MAIN GAME
//----------------------------------------

int main()
{
    // Create the main game controller
    TAController gameController;

    // Initialize the configuration (loads NPCRelationships.json)
    RelationshipConfig::getInstance();

    // Create and initialize the relationship system
    RelationshipSystemController relationshipSystem(&gameController);

    // Example of player entering village and meeting NPCs
    std::cout << "You enter the village and see several people going about their business." << std::endl;

    // Get NPCs at current location and time
    std::vector<std::string> npcsPresent = relationshipSystem.getRelationshipManager()->getNPCsAtLocation("Village Center", 1, 14);

    // Display available NPCs to interact with
    std::cout << "NPCs present:" << std::endl;
    for (const auto& npcId : npcsPresent) {
        RelationshipNPC* npc = relationshipSystem.getRelationshipManager()->getNPC(npcId);
        if (npc) {
            std::string relationshipDesc = relationshipSystem.getRelationshipManager()->getRelationshipDescription(npcId);
            std::cout << "- " << npc->name << " (" << npc->occupation << ") - " << relationshipDesc << std::endl;
        }
    }

    // Player chooses to interact with an NPC
    // Then the game would transition to the RelationshipSystem
    gameController.processInput("RelationshipSystem", {});

    // Save relationships before exiting
    relationshipSystem.saveRelationshipSystem("saved_relationships.json");

    // Game loop would continue...
    return 0;
}