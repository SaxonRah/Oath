// EmergentNPC.h
#pragma once

#include "EmergentAI.h"

class EmergentNPC : public NPC {
public:
    EmergentNPC(const std::string& id, const std::string& name);

    // Character stats
    int getStrength() const { return m_strength; }
    int getIntelligence() const { return m_intelligence; }
    int getAgility() const { return m_agility; }
    int getEndurance() const { return m_endurance; }
    int getCharisma() const { return m_charisma; }

    void setStrength(int value) { m_strength = value; }
    void setIntelligence(int value) { m_intelligence = value; }
    void setAgility(int value) { m_agility = value; }
    void setEndurance(int value) { m_endurance = value; }
    void setCharisma(int value) { m_charisma = value; }

    // Inventory management
    bool hasItem(const std::string& itemId, int quantity = 1) const;
    void addItem(const std::string& itemId, int quantity = 1);
    bool removeItem(const std::string& itemId, int quantity = 1);
    const std::map<std::string, int>& getInventory() const { return m_inventory; }

    // Skills
    int getSkillLevel(const std::string& skillId) const;
    void setSkillLevel(const std::string& skillId, int level);
    void improveSkill(const std::string& skillId, int amount = 1);

    // Faction standing
    float getFactionStanding(const std::string& factionId) const;
    void setFactionStanding(const std::string& factionId, float value);
    void changeFactionStanding(const std::string& factionId, float delta);

    // Relationships with other NPCs
    float getRelationship(const std::string& targetNpcId) const;
    void setRelationship(const std::string& targetNpcId, float value);
    void changeRelationship(const std::string& targetNpcId, float delta);

    // Location tracking
    const std::string& getCurrentLocation() const { return m_currentLocation; }
    void setCurrentLocation(const std::string& locationId) { m_currentLocation = locationId; }

    // Daily schedule
    struct ScheduleEntry {
        std::string actionId;
        std::string locationId;
        int startHour;
        int endHour;
    };

    void addScheduleEntry(int priority, const ScheduleEntry& entry);
    ScheduleEntry* getCurrentScheduleEntry(int currentHour);
    void updateBasedOnSchedule(GameContext* context);

    // Quest tracking
    void assignQuest(const std::string& questId);
    void completeQuest(const std::string& questId);
    bool hasActiveQuest(const std::string& questId) const;
    const std::vector<std::string>& getActiveQuests() const { return m_activeQuests; }
    const std::vector<std::string>& getCompletedQuests() const { return m_completedQuests; }

    // Override base methods
    void update(GameContext* context, float deltaTime) override;
    json toJson() const override;
    void fromJson(const json& data) override;

private:
    // Character stats
    int m_strength;
    int m_intelligence;
    int m_agility;
    int m_endurance;
    int m_charisma;

    // Inventory (moved from base NPC to make implementation visible)
    std::map<std::string, int> m_inventory;

    // Skills (moved from base NPC)
    std::map<std::string, int> m_skills;

    // Faction relations (moved from base NPC)
    std::map<std::string, float> m_factionStanding;

    // NPC relationships (moved from base NPC)
    std::map<std::string, float> m_relationships;

    // Current location (moved from base NPC)
    std::string m_currentLocation;

    // Daily schedule
    std::map<int, ScheduleEntry> m_schedule;

    // Quest tracking
    std::vector<std::string> m_activeQuests;
    std::vector<std::string> m_completedQuests;
};