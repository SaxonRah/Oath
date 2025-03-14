// System_NPCRelationships.cpp
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

// Forward declarations for Raw Oath types
class TANode;
class TAController;
class GameContext;
struct TAInput;
struct NodeID;
struct CharacterStats;

// NPC Relationship System Constants
namespace NPCRelationshipConstants {
// Relationship levels range from -100 to 100
const int MIN_RELATIONSHIP = -100;
const int MAX_RELATIONSHIP = 100;

// Threshold values for different relationship states
const int HATRED_THRESHOLD = -75;
const int DISLIKE_THRESHOLD = -30;
const int NEUTRAL_THRESHOLD = -10;
const int FRIENDLY_THRESHOLD = 30;
const int CLOSE_THRESHOLD = 60;
const int INTIMATE_THRESHOLD = 80;

// Personality trait influence weights
const float SHARED_TRAIT_BONUS = 0.2f;
const float OPPOSING_TRAIT_PENALTY = 0.1f;

// Gift value multipliers
const float FAVORITE_GIFT_MULTIPLIER = 2.0f;
const float LIKED_GIFT_MULTIPLIER = 1.5f;
const float DISLIKED_GIFT_MULTIPLIER = 0.5f;
const float HATED_GIFT_MULTIPLIER = -1.0f;

// Time constants
const int DAILY_DECAY_AMOUNT = -1; // Relationship slowly decays if not maintained
const int MIN_DAYS_BETWEEN_GIFTS = 3; // To prevent gift spamming
}

// NPC Personality Traits
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
    Vengeful
};

// Opposing trait pairs
const std::map<PersonalityTrait, PersonalityTrait> OPPOSING_TRAITS = {
    { PersonalityTrait::Introverted, PersonalityTrait::Extroverted },
    { PersonalityTrait::Cautious, PersonalityTrait::Brave },
    { PersonalityTrait::Practical, PersonalityTrait::Romantic },
    { PersonalityTrait::Rational, PersonalityTrait::Spiritual },
    { PersonalityTrait::Merciful, PersonalityTrait::Vengeful },
    { PersonalityTrait::Modest, PersonalityTrait::Materialistic },
    { PersonalityTrait::Traditional, PersonalityTrait::Curious }
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

    // Add a personality trait
    void addTrait(PersonalityTrait trait)
    {
        personalityTraits.insert(trait);
    }

    // Calculate trait compatibility with another NPC
    float calculateTraitCompatibility(const RelationshipNPC& other) const
    {
        float compatibilityScore = 0.0f;

        // Bonus for shared traits
        for (const auto& trait : personalityTraits) {
            if (other.personalityTraits.find(trait) != other.personalityTraits.end()) {
                compatibilityScore += NPCRelationshipConstants::SHARED_TRAIT_BONUS;
            }
        }

        // Penalty for opposing traits
        for (const auto& [trait1, trait2] : OPPOSING_TRAITS) {
            bool hasTrait1 = personalityTraits.find(trait1) != personalityTraits.end();
            bool hasTrait2 = personalityTraits.find(trait2) != personalityTraits.end();
            bool otherHasTrait1 = other.personalityTraits.find(trait1) != other.personalityTraits.end();
            bool otherHasTrait2 = other.personalityTraits.find(trait2) != other.personalityTraits.end();

            if ((hasTrait1 && otherHasTrait2) || (hasTrait2 && otherHasTrait1)) {
                compatibilityScore -= NPCRelationshipConstants::OPPOSING_TRAIT_PENALTY;
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
        // Check specific item preferences first
        if (favoriteItems.find(itemId) != favoriteItems.end()) {
            return NPCRelationshipConstants::FAVORITE_GIFT_MULTIPLIER;
        }

        if (dislikedItems.find(itemId) != dislikedItems.end()) {
            return NPCRelationshipConstants::HATED_GIFT_MULTIPLIER;
        }

        // Otherwise check category preferences
        if (giftPreferences.find(category) != giftPreferences.end()) {
            float preference = giftPreferences[category];
            if (preference > 0.5f) {
                return NPCRelationshipConstants::LIKED_GIFT_MULTIPLIER;
            } else if (preference < -0.5f) {
                return NPCRelationshipConstants::DISLIKED_GIFT_MULTIPLIER;
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
    }

    // Register a new NPC
    void registerNPC(const RelationshipNPC& npc)
    {
        npcs[npc.id] = npc;
        if (playerRelationships.find(npc.id) == playerRelationships.end()) {
            playerRelationships[npc.id] = 0; // Start neutral
            playerRelationshipTypes[npc.id] = RelationshipType::None;
            playerRelationshipStates[npc.id] = RelationshipState::Neutral;
            lastGiftDay[npc.id] = -NPCRelationshipConstants::MIN_DAYS_BETWEEN_GIFTS; // Allow immediate gift
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
            NPCRelationshipConstants::MIN_RELATIONSHIP,
            std::min(NPCRelationshipConstants::MAX_RELATIONSHIP, playerRelationships[npcId] + amount));

        // Update relationship type based on new value
        updateRelationshipType(npcId);
    }

    // Update the relationship type based on the current value
    void updateRelationshipType(const std::string& npcId)
    {
        int value = playerRelationships[npcId];

        // Update type based on value thresholds
        if (value <= NPCRelationshipConstants::HATRED_THRESHOLD) {
            playerRelationshipTypes[npcId] = RelationshipType::Enemy;
        } else if (value <= NPCRelationshipConstants::DISLIKE_THRESHOLD) {
            playerRelationshipTypes[npcId] = RelationshipType::Rival;
        } else if (value <= NPCRelationshipConstants::NEUTRAL_THRESHOLD) {
            playerRelationshipTypes[npcId] = RelationshipType::None;
        } else if (value <= NPCRelationshipConstants::FRIENDLY_THRESHOLD) {
            playerRelationshipTypes[npcId] = RelationshipType::Acquaintance;
        } else if (value <= NPCRelationshipConstants::CLOSE_THRESHOLD) {
            playerRelationshipTypes[npcId] = RelationshipType::Friend;
        } else if (value <= NPCRelationshipConstants::INTIMATE_THRESHOLD) {
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

        // Check if enough time has passed since last gift
        if (currentGameDay - lastGiftDay[npcId] < NPCRelationshipConstants::MIN_DAYS_BETWEEN_GIFTS) {
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
        if (reactionMultiplier >= NPCRelationshipConstants::FAVORITE_GIFT_MULTIPLIER) {
            playerRelationshipStates[npcId] = RelationshipState::Happy;
        } else if (reactionMultiplier <= NPCRelationshipConstants::HATED_GIFT_MULTIPLIER) {
            playerRelationshipStates[npcId] = RelationshipState::Disappointed;
        } else if (reactionMultiplier >= NPCRelationshipConstants::LIKED_GIFT_MULTIPLIER) {
            playerRelationshipStates[npcId] = RelationshipState::Grateful;
        } else if (reactionMultiplier <= NPCRelationshipConstants::DISLIKED_GIFT_MULTIPLIER) {
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
        currentGameDay++;

        // Natural decay in relationships over time if not maintained
        for (auto& [npcId, relationshipValue] : playerRelationships) {
            // Skip decay for close relationships
            if (relationshipValue > NPCRelationshipConstants::CLOSE_THRESHOLD)
                continue;

            // Apply small decay
            relationshipValue += NPCRelationshipConstants::DAILY_DECAY_AMOUNT;

            // Ensure it doesn't fall below a minimum
            if (relationshipValue < NPCRelationshipConstants::MIN_RELATIONSHIP) {
                relationshipValue = NPCRelationshipConstants::MIN_RELATIONSHIP;
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

        int value = playerRelationships[npcId];
        RelationshipType type = playerRelationshipTypes[npcId];
        RelationshipState state = playerRelationshipStates[npcId];

        std::string description;

        // Base description on relationship type
        switch (type) {
        case RelationshipType::None:
            description = "Stranger";
            break;
        case RelationshipType::Acquaintance:
            description = "Acquaintance";
            break;
        case RelationshipType::Friend:
            description = "Friend";
            break;
        case RelationshipType::CloseFriend:
            description = "Close Friend";
            break;
        case RelationshipType::BestFriend:
            description = "Best Friend";
            break;
        case RelationshipType::Rival:
            description = "Rival";
            break;
        case RelationshipType::Enemy:
            description = "Enemy";
            break;
        case RelationshipType::Family:
            description = "Family";
            break;
        case RelationshipType::Mentor:
            description = "Mentor";
            break;
        case RelationshipType::Student:
            description = "Student";
            break;
        case RelationshipType::RomanticInterest:
            description = "Romantic Interest";
            break;
        case RelationshipType::Partner:
            description = "Partner";
            break;
        case RelationshipType::Spouse:
            description = "Spouse";
            break;
        default:
            description = "Unknown";
        }

        // Add current state as modifier
        switch (state) {
        case RelationshipState::Happy:
            description += " (Happy)";
            break;
        case RelationshipState::Sad:
            description += " (Sad)";
            break;
        case RelationshipState::Angry:
            description += " (Angry)";
            break;
        case RelationshipState::Fearful:
            description += " (Fearful)";
            break;
        case RelationshipState::Trusting:
            description += " (Trusting)";
            break;
        case RelationshipState::Suspicious:
            description += " (Suspicious)";
            break;
        case RelationshipState::Grateful:
            description += " (Grateful)";
            break;
        case RelationshipState::Jealous:
            description += " (Jealous)";
            break;
        case RelationshipState::Impressed:
            description += " (Impressed)";
            break;
        case RelationshipState::Disappointed:
            description += " (Disappointed)";
            break;
        case RelationshipState::Forgiving:
            description += " (Forgiving)";
            break;
        case RelationshipState::Resentful:
            description += " (Resentful)";
            break;
        default:
            break; // No modifier for neutral
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

        int value = playerRelationships[npcId];

        // Check requirements for each type, unless forcing
        if (!force) {
            switch (newType) {
            case RelationshipType::Friend:
                if (value < NPCRelationshipConstants::FRIENDLY_THRESHOLD)
                    return false;
                break;
            case RelationshipType::CloseFriend:
                if (value < NPCRelationshipConstants::CLOSE_THRESHOLD)
                    return false;
                break;
            case RelationshipType::BestFriend:
                if (value < NPCRelationshipConstants::INTIMATE_THRESHOLD)
                    return false;
                break;
            case RelationshipType::Partner:
                if (value < NPCRelationshipConstants::INTIMATE_THRESHOLD)
                    return false;
                break;
            case RelationshipType::Spouse:
                if (value < NPCRelationshipConstants::INTIMATE_THRESHOLD)
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
            playerRelationships[npcId] = NPCRelationshipConstants::MAX_RELATIONSHIP;
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
        std::ofstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }

        // Save current day
        file.write(reinterpret_cast<const char*>(&currentGameDay), sizeof(currentGameDay));

        // Save player relationships
        size_t relationshipCount = playerRelationships.size();
        file.write(reinterpret_cast<const char*>(&relationshipCount), sizeof(relationshipCount));

        for (const auto& [npcId, value] : playerRelationships) {
            // Write NPC ID
            size_t idLength = npcId.length();
            file.write(reinterpret_cast<const char*>(&idLength), sizeof(idLength));
            file.write(npcId.c_str(), idLength);

            // Write relationship value
            file.write(reinterpret_cast<const char*>(&value), sizeof(value));

            // Write relationship type
            RelationshipType type = playerRelationshipTypes[npcId];
            file.write(reinterpret_cast<const char*>(&type), sizeof(type));

            // Write relationship state
            RelationshipState state = playerRelationshipStates[npcId];
            file.write(reinterpret_cast<const char*>(&state), sizeof(state));

            // Write last gift day
            int lastGift = lastGiftDay[npcId];
            file.write(reinterpret_cast<const char*>(&lastGift), sizeof(lastGift));
        }

        return true;
    }

    // Load relationship data
    bool loadRelationships(const std::string& filename)
    {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }

        // Load current day
        file.read(reinterpret_cast<char*>(&currentGameDay), sizeof(currentGameDay));

        // Load player relationships
        size_t relationshipCount;
        file.read(reinterpret_cast<char*>(&relationshipCount), sizeof(relationshipCount));

        playerRelationships.clear();
        playerRelationshipTypes.clear();
        playerRelationshipStates.clear();
        lastGiftDay.clear();

        for (size_t i = 0; i < relationshipCount; i++) {
            // Read NPC ID
            size_t idLength;
            file.read(reinterpret_cast<char*>(&idLength), sizeof(idLength));
            std::string npcId(idLength, ' ');
            file.read(&npcId[0], idLength);

            // Read relationship value
            int value;
            file.read(reinterpret_cast<char*>(&value), sizeof(value));

            // Read relationship type
            RelationshipType type;
            file.read(reinterpret_cast<char*>(&type), sizeof(type));

            // Read relationship state
            RelationshipState state;
            file.read(reinterpret_cast<char*>(&state), sizeof(state));

            // Read last gift day
            int lastGift;
            file.read(reinterpret_cast<char*>(&lastGift), sizeof(lastGift));

            // Store the data
            playerRelationships[npcId] = value;
            playerRelationshipTypes[npcId] = type;
            playerRelationshipStates[npcId] = state;
            lastGiftDay[npcId] = lastGift;
        }

        return true;
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
        if (value >= NPCRelationshipConstants::NEUTRAL_THRESHOLD) {
            actions.push_back({ "ask_about_self", "Ask about " + npc->name,
                [this]() -> TAInput {
                    return { "npc_action", { { "action", std::string("ask_about") } } };
                } });
        }

        // Only available for friendly+ relationships
        if (value >= NPCRelationshipConstants::FRIENDLY_THRESHOLD) {
            actions.push_back({ "request_help", "Ask for help",
                [this]() -> TAInput {
                    return { "npc_action", { { "action", std::string("request_help") } } };
                } });
        }

        // Only available for close friends or better
        if (value >= NPCRelationshipConstants::CLOSE_THRESHOLD) {
            actions.push_back({ "personal_request", "Make personal request",
                [this]() -> TAInput {
                    return { "npc_action", { { "action", std::string("personal_request") } } };
                } });
        }

        // Romantic options only available if not enemies/rivals and not already in a relationship
        if (value >= NPCRelationshipConstants::FRIENDLY_THRESHOLD && type != RelationshipType::Enemy && type != RelationshipType::Rival && type != RelationshipType::Spouse && type != RelationshipType::Partner) {

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

        // Process different interaction types
        if (action == "talk") {
            // Simulate conversation options
            std::vector<std::string> topics = { "weather", "town_news", "personal", "work" };
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

                if (reaction >= NPCRelationshipConstants::FAVORITE_GIFT_MULTIPLIER) {
                    std::cout << npc->name << " loves this gift!" << std::endl;
                } else if (reaction >= NPCRelationshipConstants::LIKED_GIFT_MULTIPLIER) {
                    std::cout << npc->name << " likes this gift." << std::endl;
                } else if (reaction <= NPCRelationshipConstants::HATED_GIFT_MULTIPLIER) {
                    std::cout << npc->name << " hates this gift!" << std::endl;
                } else if (reaction <= NPCRelationshipConstants::DISLIKED_GIFT_MULTIPLIER) {
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
            if (value >= NPCRelationshipConstants::CLOSE_THRESHOLD) {
                std::cout << "They share some personal details about their past and aspirations." << std::endl;
                relationshipManager->changeRelationship(currentNPCId, 2);
            } else if (value >= NPCRelationshipConstants::FRIENDLY_THRESHOLD) {
                std::cout << "They tell you about their current projects and interests." << std::endl;
                relationshipManager->changeRelationship(currentNPCId, 1);
            } else {
                std::cout << "They share basic information, but remain somewhat guarded." << std::endl;
            }
        } else if (action == "flirt") {
            int value = relationshipManager->getRelationshipValue(currentNPCId);
            bool receptive = false;

            // Check if NPC is receptive to romance (based on relationship and personality)
            if (value >= NPCRelationshipConstants::CLOSE_THRESHOLD) {
                receptive = true;
            } else if (value >= NPCRelationshipConstants::FRIENDLY_THRESHOLD) {
                // More likely if the NPC has romantic personality trait
                receptive = npc->personalityTraits.find(PersonalityTrait::Romantic) != npc->personalityTraits.end();
            }

            if (receptive) {
                std::cout << npc->name << " responds positively to your flirtation." << std::endl;
                relationshipManager->changeRelationship(currentNPCId, 3);

                // If relationship is already strong, potentially advance to romantic interest
                if (value >= NPCRelationshipConstants::INTIMATE_THRESHOLD) {
                    relationshipManager->changeRelationshipType(currentNPCId, RelationshipType::RomanticInterest);
                    std::cout << npc->name << " seems to be developing romantic feelings for you." << std::endl;
                }
            } else {
                std::cout << npc->name << " politely deflects your advances." << std::endl;
                if (value < NPCRelationshipConstants::FRIENDLY_THRESHOLD) {
                    relationshipManager->changeRelationship(currentNPCId, -1);
                }
            }
        } else if (action == "propose") {
            int value = relationshipManager->getRelationshipValue(currentNPCId);

            // NPC will accept if relationship is very high
            if (value >= NPCRelationshipConstants::INTIMATE_THRESHOLD) {
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

            if (value >= NPCRelationshipConstants::FRIENDLY_THRESHOLD) {
                std::cout << npc->name << " agrees to help you." << std::endl;

                // The level of help would depend on relationship strength
                if (value >= NPCRelationshipConstants::INTIMATE_THRESHOLD) {
                    std::cout << "They are willing to go to great lengths to assist you." << std::endl;
                } else if (value >= NPCRelationshipConstants::CLOSE_THRESHOLD) {
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
        if (relationshipValue >= NPCRelationshipConstants::FRIENDLY_THRESHOLD) {
            std::cout << "\nPersonality traits:" << std::endl;
            for (const auto& trait : npc->personalityTraits) {
                std::cout << " - " << static_cast<int>(trait) << std::endl; // Would convert to string in full implementation
            }
        }

        // Show gift preferences if relationship is good
        if (relationshipValue >= NPCRelationshipConstants::CLOSE_THRESHOLD) {
            std::cout << "\nGift preferences:" << std::endl;
            for (const auto& [category, preference] : npc->giftPreferences) {
                if (preference > 0.5f) {
                    std::cout << " - Likes " << static_cast<int>(category) << std::endl; // Would convert to string
                } else if (preference < -0.5f) {
                    std::cout << " - Dislikes " << static_cast<int>(category) << std::endl;
                }
            }
        }

        // Show favorite items only to very close friends/partners
        if (relationshipValue >= NPCRelationshipConstants::INTIMATE_THRESHOLD) {
            if (!npc->favoriteItems.empty()) {
                std::cout << "\nFavorite items:" << std::endl;
                for (const auto& item : npc->favoriteItems) {
                    std::cout << " - " << item << std::endl;
                }
            }
        }

        // Show daily schedule if friends or better
        if (relationshipValue >= NPCRelationshipConstants::FRIENDLY_THRESHOLD) {
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

        // This would pull from the relationship manager
        // For now, we'll populate with samples
        std::map<std::string, RelationshipType> allRelationships;

        // In a real implementation, this would query all NPCs with relationships
        // For demonstration, we'll add some sample NPCs
        allRelationships["elderMarius"] = RelationshipType::Acquaintance;
        allRelationships["tavernKeeper"] = RelationshipType::Friend;
        allRelationships["blacksmith"] = RelationshipType::Friend;
        allRelationships["villageGuard"] = RelationshipType::Rival;
        allRelationships["companion1"] = RelationshipType::CloseFriend;
        allRelationships["noblewoman"] = RelationshipType::RomanticInterest;
        allRelationships["uncle"] = RelationshipType::Family;

        // Apply filter
        for (const auto& [npcId, type] : allRelationships) {
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

        // Initialize with some example NPCs
        initializeExampleNPCs();
    }

    void initializeExampleNPCs()
    {
        // Create example NPCs with detailed personality profiles

        // Elder Marius - Village elder from the main example
        RelationshipNPC elderMarius("elderMarius", "Elder Marius");
        elderMarius.occupation = "Village Elder";
        elderMarius.age = 68;
        elderMarius.gender = "Male";
        elderMarius.race = "Human";
        elderMarius.faction = "Village Council";
        elderMarius.homeLocation = "Elder's Cottage";

        // Personality traits
        elderMarius.addTrait(PersonalityTrait::Wise);
        elderMarius.addTrait(PersonalityTrait::Traditional);
        elderMarius.addTrait(PersonalityTrait::Cautious);

        // Gift preferences
        elderMarius.setGiftPreference(GiftCategory::Book, 0.8f);
        elderMarius.setGiftPreference(GiftCategory::ReligiousItem, 0.6f);
        elderMarius.setGiftPreference(GiftCategory::Weapon, -0.7f);
        elderMarius.favoriteItems.insert("ancient_tome");
        elderMarius.dislikedItems.insert("cheap_ale");

        // Schedule
        elderMarius.addScheduleEntry(false, 6, 9, "Elder's Cottage", "morning prayer");
        elderMarius.addScheduleEntry(false, 9, 12, "Village Center", "council duties");
        elderMarius.addScheduleEntry(false, 12, 13, "Village Inn", "lunch");
        elderMarius.addScheduleEntry(false, 13, 18, "Village Center", "meeting villagers");
        elderMarius.addScheduleEntry(false, 18, 22, "Elder's Cottage", "reading");
        elderMarius.addScheduleEntry(false, 22, 6, "Elder's Cottage", "sleeping");

        // Weekend is slightly different
        elderMarius.addScheduleEntry(true, 7, 10, "Elder's Cottage", "morning prayer");
        elderMarius.addScheduleEntry(true, 10, 14, "Temple", "religious service");
        elderMarius.addScheduleEntry(true, 14, 18, "Elder's Cottage", "meeting visitors");
        elderMarius.addScheduleEntry(true, 18, 22, "Village Inn", "community dinner");
        elderMarius.addScheduleEntry(true, 22, 7, "Elder's Cottage", "sleeping");

        // Register NPC
        relationshipManager.registerNPC(elderMarius);

        // Tavern Keeper - Jolly and sociable
        RelationshipNPC tavernKeeper("tavernKeeper", "Hilda the Tavern Keeper");
        tavernKeeper.occupation = "Innkeeper";
        tavernKeeper.age = 42;
        tavernKeeper.gender = "Female";
        tavernKeeper.race = "Human";
        tavernKeeper.faction = "Merchants Guild";
        tavernKeeper.homeLocation = "Village Inn";

        // Personality traits
        tavernKeeper.addTrait(PersonalityTrait::Cheerful);
        tavernKeeper.addTrait(PersonalityTrait::Extroverted);
        tavernKeeper.addTrait(PersonalityTrait::Generous);

        // Gift preferences
        tavernKeeper.setGiftPreference(GiftCategory::Food, 0.9f);
        tavernKeeper.setGiftPreference(GiftCategory::Drink, 0.7f);
        tavernKeeper.setGiftPreference(GiftCategory::Book, -0.5f);
        tavernKeeper.favoriteItems.insert("exotic_spices");
        tavernKeeper.dislikedItems.insert("raw_meat");

        // Schedule (mostly at the inn)
        tavernKeeper.addScheduleEntry(false, 5, 8, "Village Inn", "preparing breakfast");
        tavernKeeper.addScheduleEntry(false, 8, 14, "Village Inn", "serving customers");
        tavernKeeper.addScheduleEntry(false, 14, 16, "Village Inn", "cleaning and inventory");
        tavernKeeper.addScheduleEntry(false, 16, 23, "Village Inn", "serving dinner and drinks");
        tavernKeeper.addScheduleEntry(false, 23, 5, "Village Inn", "sleeping");

        // Weekend schedule
        tavernKeeper.addScheduleEntry(true, 6, 9, "Village Inn", "preparing breakfast");
        tavernKeeper.addScheduleEntry(true, 9, 12, "Village Market", "buying supplies");
        tavernKeeper.addScheduleEntry(true, 12, 24, "Village Inn", "hosting weekend festivities");
        tavernKeeper.addScheduleEntry(true, 0, 6, "Village Inn", "sleeping");

        // Register NPC
        relationshipManager.registerNPC(tavernKeeper);

        // Blacksmith - Skilled craftsman
        RelationshipNPC blacksmith("blacksmith", "Gareth the Blacksmith");
        blacksmith.occupation = "Blacksmith";
        blacksmith.age = 38;
        blacksmith.gender = "Male";
        blacksmith.race = "Human";
        blacksmith.faction = "Craftsmen Guild";
        blacksmith.homeLocation = "Blacksmith's House";

        // Personality traits
        blacksmith.addTrait(PersonalityTrait::Disciplined);
        blacksmith.addTrait(PersonalityTrait::Honest);
        blacksmith.addTrait(PersonalityTrait::Practical);

        // Gift preferences
        blacksmith.setGiftPreference(GiftCategory::Tool, 0.9f);
        blacksmith.setGiftPreference(GiftCategory::Crafting, 0.8f);
        blacksmith.setGiftPreference(GiftCategory::Luxury, -0.6f);
        blacksmith.favoriteItems.insert("rare_metal");
        blacksmith.dislikedItems.insert("fragile_ornament");

        // Schedule
        blacksmith.addScheduleEntry(false, 5, 7, "Blacksmith's House", "early breakfast");
        blacksmith.addScheduleEntry(false, 7, 12, "Village Forge", "smithing work");
        blacksmith.addScheduleEntry(false, 12, 13, "Blacksmith's House", "lunch");
        blacksmith.addScheduleEntry(false, 13, 19, "Village Forge", "smithing work");
        blacksmith.addScheduleEntry(false, 19, 21, "Village Inn", "evening meal");
        blacksmith.addScheduleEntry(false, 21, 5, "Blacksmith's House", "sleeping");

        // Weekend - still works but shorter hours
        blacksmith.addScheduleEntry(true, 6, 8, "Blacksmith's House", "breakfast");
        blacksmith.addScheduleEntry(true, 8, 14, "Village Forge", "smithing work");
        blacksmith.addScheduleEntry(true, 14, 18, "Blacksmith's House", "personal projects");
        blacksmith.addScheduleEntry(true, 18, 22, "Village Inn", "relaxing");
        blacksmith.addScheduleEntry(true, 22, 6, "Blacksmith's House", "sleeping");

        // Register NPC
        relationshipManager.registerNPC(blacksmith);

        // Village Guard - Somewhat antagonistic
        RelationshipNPC villageGuard("villageGuard", "Sergeant Bram");
        villageGuard.occupation = "Village Guard Captain";
        villageGuard.age = 35;
        villageGuard.gender = "Male";
        villageGuard.race = "Human";
        villageGuard.faction = "Village Guard";
        villageGuard.homeLocation = "Guard Barracks";

        // Personality traits
        villageGuard.addTrait(PersonalityTrait::Disciplined);
        villageGuard.addTrait(PersonalityTrait::Stubborn);
        villageGuard.addTrait(PersonalityTrait::Suspicious);

        // Gift preferences
        villageGuard.setGiftPreference(GiftCategory::Weapon, 0.7f);
        villageGuard.setGiftPreference(GiftCategory::Armor, 0.6f);
        villageGuard.setGiftPreference(GiftCategory::Book, -0.4f);
        villageGuard.favoriteItems.insert("fine_sword");
        villageGuard.dislikedItems.insert("contraband");

        // Schedule
        villageGuard.addScheduleEntry(false, 5, 6, "Guard Barracks", "waking up");
        villageGuard.addScheduleEntry(false, 6, 8, "Training Grounds", "morning drills");
        villageGuard.addScheduleEntry(false, 8, 12, "Village Gates", "guard duty");
        villageGuard.addScheduleEntry(false, 12, 13, "Guard Barracks", "lunch");
        villageGuard.addScheduleEntry(false, 13, 18, "Village Center", "patrol");
        villageGuard.addScheduleEntry(false, 18, 19, "Guard Barracks", "dinner");
        villageGuard.addScheduleEntry(false, 19, 22, "Village Inn", "off-duty");
        villageGuard.addScheduleEntry(false, 22, 5, "Guard Barracks", "sleeping");

        // Weekend - similar schedule as guard duty continues
        villageGuard.addScheduleEntry(true, 5, 6, "Guard Barracks", "waking up");
        villageGuard.addScheduleEntry(true, 6, 8, "Training Grounds", "training recruits");
        villageGuard.addScheduleEntry(true, 8, 12, "Village Center", "patrol");
        villageGuard.addScheduleEntry(true, 12, 13, "Guard Barracks", "lunch");
        villageGuard.addScheduleEntry(true, 13, 19, "Village Gates", "guard duty");
        villageGuard.addScheduleEntry(true, 19, 23, "Village Inn", "off-duty");
        villageGuard.addScheduleEntry(true, 23, 5, "Guard Barracks", "sleeping");

        // Register NPC
        relationshipManager.registerNPC(villageGuard);

        // Companion - Close friend
        RelationshipNPC companion("companion1", "Lyra the Huntress");
        companion.occupation = "Hunter";
        companion.age = 27;
        companion.gender = "Female";
        companion.race = "Elf";
        companion.faction = "Forest Guardians";
        companion.homeLocation = "Forest Cabin";

        // Personality traits
        companion.addTrait(PersonalityTrait::Brave);
        companion.addTrait(PersonalityTrait::Loyal);
        companion.addTrait(PersonalityTrait::Independent);

        // Gift preferences
        companion.setGiftPreference(GiftCategory::Weapon, 0.8f);
        companion.setGiftPreference(GiftCategory::Food, 0.6f);
        companion.setGiftPreference(GiftCategory::Luxury, -0.5f);
        companion.favoriteItems.insert("fine_bow");
        companion.dislikedItems.insert("fancy_clothes");

        // Schedule
        companion.addScheduleEntry(false, 4, 8, "Forest", "early hunting");
        companion.addScheduleEntry(false, 8, 10, "Forest Cabin", "preparing game");
        companion.addScheduleEntry(false, 10, 14, "Village Market", "selling game");
        companion.addScheduleEntry(false, 14, 18, "Forest", "afternoon hunting");
        companion.addScheduleEntry(false, 18, 21, "Village Inn", "relaxing");
        companion.addScheduleEntry(false, 21, 4, "Forest Cabin", "sleeping");

        // Weekend
        companion.addScheduleEntry(true, 5, 12, "Deep Forest", "extended hunting trip");
        companion.addScheduleEntry(true, 12, 16, "Forest Cabin", "crafting arrows and traps");
        companion.addScheduleEntry(true, 16, 22, "Village Inn", "socializing");
        companion.addScheduleEntry(true, 22, 5, "Forest Cabin", "sleeping");

        // Register NPC with initial close friendship
        relationshipManager.registerNPC(companion);
        relationshipManager.changeRelationship("companion1", 65); // Close friend
        relationshipManager.updateRelationshipType("companion1");

        // Noble - Potential romantic interest
        RelationshipNPC noblewoman("noblewoman", "Lady Eleanor");
       noblewoman.occupation = "Noble");
       noblewoman.age = 26;
       noblewoman.gender = "Female";
       noblewoman.race = "Human";
       noblewoman.faction = "Nobility";
       noblewoman.homeLocation = "Manor House";

       // Personality traits
       noblewoman.addTrait(PersonalityTrait::Ambitious);
       noblewoman.addTrait(PersonalityTrait::Intelligent);
       noblewoman.addTrait(PersonalityTrait::Diplomatic);

       // Gift preferences
       noblewoman.setGiftPreference(GiftCategory::Jewelry, 0.9f);
       noblewoman.setGiftPreference(GiftCategory::Book, 0.7f);
       noblewoman.setGiftPreference(GiftCategory::Tool, -0.6f);
       noblewoman.favoriteItems.insert("gold_necklace");
       noblewoman.dislikedItems.insert("common_tools");

       // Schedule
       noblewoman.addScheduleEntry(false, 7, 9, "Manor House", "breakfast");
       noblewoman.addScheduleEntry(false, 9, 12, "Manor Library", "reading");
       noblewoman.addScheduleEntry(false, 12, 14, "Manor Garden", "lunch and walk");
       noblewoman.addScheduleEntry(false, 14, 17, "Village Center", "social calls");
       noblewoman.addScheduleEntry(false, 17, 19, "Manor House", "preparing for dinner");
       noblewoman.addScheduleEntry(false, 19, 22, "Manor House", "hosting dinner");
       noblewoman.addScheduleEntry(false, 22, 7, "Manor House", "sleeping");

       // Weekend
       noblewoman.addScheduleEntry(true, 8, 10, "Manor House", "breakfast");
       noblewoman.addScheduleEntry(true, 10, 13, "Temple", "religious service");
       noblewoman.addScheduleEntry(true, 13, 16, "Manor Garden", "tea with guests");
       noblewoman.addScheduleEntry(true, 16, 19, "Village Center", "charity work");
       noblewoman.addScheduleEntry(true, 19, 23, "Manor House", "hosting evening entertainment");
       noblewoman.addScheduleEntry(true, 23, 8, "Manor House", "sleeping");

       // Register NPC with initial romantic interest
       relationshipManager.registerNPC(noblewoman);
       relationshipManager.changeRelationship("noblewoman", 40); // Friendly enough for romance
       relationshipManager.changeRelationshipType("noblewoman", RelationshipType::RomanticInterest);

       // Family Member - Uncle
       RelationshipNPC uncle("uncle", "Uncle Tomas");
       uncle.occupation = "Retired Soldier");
       uncle.age = 58;
       uncle.gender = "Male";
       uncle.race = "Human";
       uncle.faction = "Village Council";
       uncle.homeLocation = "Farm Outside Village";

       // Personality traits
       uncle.addTrait(PersonalityTrait::Brave);
       uncle.addTrait(PersonalityTrait::Loyal);
       uncle.addTrait(PersonalityTrait::Traditional);

       // Gift preferences
       uncle.setGiftPreference(GiftCategory::Weapon, 0.7f);
       uncle.setGiftPreference(GiftCategory::Drink, 0.8f);
       uncle.setGiftPreference(GiftCategory::Luxury, -0.4f);
       uncle.favoriteItems.insert("aged_whiskey");
       uncle.dislikedItems.insert("fancy_perfume");

       // Schedule
       uncle.addScheduleEntry(false, 5, 8, "Farm", "morning chores");
       uncle.addScheduleEntry(false, 8, 12, "Farm", "tending fields");
       uncle.addScheduleEntry(false, 12, 13, "Farm", "lunch");
       uncle.addScheduleEntry(false, 13, 16, "Farm", "afternoon work");
       uncle.addScheduleEntry(false, 16, 18, "Village Center", "errands");
       uncle.addScheduleEntry(false, 18, 22, "Village Inn", "socializing");
       uncle.addScheduleEntry(false, 22, 5, "Farm", "sleeping");

       // Weekend
       uncle.addScheduleEntry(true, 6, 10, "Farm", "light chores");
       uncle.addScheduleEntry(true, 10, 14, "Temple", "religious service");
       uncle.addScheduleEntry(true, 14, 18, "Village Center", "visiting family");
       uncle.addScheduleEntry(true, 18, 23, "Village Inn", "drinking and telling war stories");
       uncle.addScheduleEntry(true, 23, 6, "Farm", "sleeping");

       // Register NPC with family relationship
       relationshipManager.registerNPC(uncle);
       relationshipManager.changeRelationship("uncle", 70); // Strong family bond
       relationshipManager.changeRelationshipType("uncle", RelationshipType::Family, true);
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

    // Create and initialize all systems
    // ...

    // Initialize relationship system
    RelationshipSystemController relationshipSystem(&gameController);

    // Example of player entering village and meeting NPCs
    std::cout << "You enter the village and see several people going about their business." << std::endl;

    // Get NPCs at current location and time
    std::vector<std::string> npcsPresent = relationshipSystem.getNPCsAtLocation("Village Center", 1, 14);

    // Display available NPCs to interact with
    std::cout << "NPCs present:" << std::endl;
    for (const auto& npcId : npcsPresent) {
        RelationshipNPC* npc = relationshipSystem.getNPC(npcId);
        if (npc) {
            std::string relationshipDesc = relationshipSystem.getRelationshipDescription(npcId);
            std::cout << "- " << npc->name << " (" << npc->occupation << ") - " << relationshipDesc << std::endl;
        }
    }

    // Player chooses to interact with an NPC
    // Then the game would transition to the RelationshipSystem
    gameController.processInput("RelationshipSystem", {});

    // Game loop would continue...
    return 0;
}
