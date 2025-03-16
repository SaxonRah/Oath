#pragma once

#include "RelationshipTypes.hpp"
#include <map>
#include <nlohmann/json.hpp>
#include <set>
#include <string>
#include <vector>


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
    RelationshipNPC(const nlohmann::json& npcData);

    void addTrait(PersonalityTrait trait);
    float calculateTraitCompatibility(const RelationshipNPC& other) const;
    NPCRelationship* findRelationship(const std::string& npcId);
    ScheduleEntry getCurrentSchedule(int day, int hour);
    float getGiftReaction(const std::string& itemId, GiftCategory category);
    void addScheduleEntry(bool weekend, int start, int end, const std::string& location, const std::string& activity);
    void setGiftPreference(GiftCategory category, float preference);
    nlohmann::json toJson() const;
};