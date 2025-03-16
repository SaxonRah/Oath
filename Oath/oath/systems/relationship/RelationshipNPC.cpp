#include "RelationshipNPC.hpp"
#include "RelationshipConfig.hpp"
#include <algorithm>


RelationshipNPC::RelationshipNPC(const std::string& npcId, const std::string& npcName)
    : id(npcId)
    , name(npcName)
    , age(30)
    , gender("Unknown")
    , race("Human")
    , faction("Neutral")
    , homeLocation("Nowhere")
{
}

RelationshipNPC::RelationshipNPC(const nlohmann::json& npcData)
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

void RelationshipNPC::addTrait(PersonalityTrait trait)
{
    personalityTraits.insert(trait);
}

float RelationshipNPC::calculateTraitCompatibility(const RelationshipNPC& other) const
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

RelationshipNPC::NPCRelationship* RelationshipNPC::findRelationship(const std::string& npcId)
{
    for (auto& rel : relationships) {
        if (rel.npcId == npcId) {
            return &rel;
        }
    }
    return nullptr;
}

RelationshipNPC::ScheduleEntry RelationshipNPC::getCurrentSchedule(int day, int hour)
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

float RelationshipNPC::getGiftReaction(const std::string& itemId, GiftCategory category)
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

void RelationshipNPC::addScheduleEntry(bool weekend, int start, int end, const std::string& location, const std::string& activity)
{
    ScheduleEntry entry { start, end, location, activity };
    if (weekend) {
        weekendSchedule.push_back(entry);
    } else {
        weekdaySchedule.push_back(entry);
    }
}

void RelationshipNPC::setGiftPreference(GiftCategory category, float preference)
{
    // Clamp preference between -1.0 and 1.0
    preference = std::max(-1.0f, std::min(1.0f, preference));
    giftPreferences[category] = preference;
}

nlohmann::json RelationshipNPC::toJson() const
{
    RelationshipConfig& config = RelationshipConfig::getInstance();
    nlohmann::json j;

    j["id"] = id;
    j["name"] = name;
    j["occupation"] = occupation;
    j["age"] = age;
    j["gender"] = gender;
    j["race"] = race;
    j["faction"] = faction;
    j["homeLocation"] = homeLocation;

    // Personality traits
    nlohmann::json traits = nlohmann::json::array();
    for (const auto& trait : personalityTraits) {
        traits.push_back(config.getPersonalityTraitString(trait));
    }
    j["personalityTraits"] = traits;

    // Gift preferences
    nlohmann::json giftPrefs;
    for (const auto& [category, value] : giftPreferences) {
        giftPrefs[config.getGiftCategoryString(category)] = value;
    }
    j["giftPreferences"] = giftPrefs;

    // Favorite and disliked items
    j["favoriteItems"] = nlohmann::json(favoriteItems);
    j["dislikedItems"] = nlohmann::json(dislikedItems);

    // Conversation topics
    j["conversationTopics"] = nlohmann::json(conversationTopics);
    j["tabooTopics"] = nlohmann::json(tabooTopics);

    // Schedule
    nlohmann::json schedule;

    nlohmann::json weekdayEntries = nlohmann::json::array();
    for (const auto& entry : weekdaySchedule) {
        nlohmann::json e;
        e["startHour"] = entry.startHour;
        e["endHour"] = entry.endHour;
        e["location"] = entry.location;
        e["activity"] = entry.activity;
        weekdayEntries.push_back(e);
    }
    schedule["weekday"] = weekdayEntries;

    nlohmann::json weekendEntries = nlohmann::json::array();
    for (const auto& entry : weekendSchedule) {
        nlohmann::json e;
        e["startHour"] = entry.startHour;
        e["endHour"] = entry.endHour;
        e["location"] = entry.location;
        e["activity"] = entry.activity;
        weekendEntries.push_back(e);
    }
    schedule["weekend"] = weekendEntries;

    j["schedule"] = schedule;

    // Relationships with other NPCs
    nlohmann::json npcRelationships = nlohmann::json::array();
    for (const auto& rel : relationships) {
        nlohmann::json r;
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