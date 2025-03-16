// EmergentNPC.cpp
#include "EmergentNPC.h"

EmergentNPC::EmergentNPC(const std::string& id, const std::string& name)
    : NPC(id, name)
    , m_strength(5)
    , m_intelligence(5)
    , m_agility(5)
    , m_endurance(5)
    , m_charisma(5)
{
}

bool EmergentNPC::hasItem(const std::string& itemId, int quantity) const
{
    auto it = m_inventory.find(itemId);
    if (it != m_inventory.end()) {
        return it->second >= quantity;
    }
    return false;
}

void EmergentNPC::addItem(const std::string& itemId, int quantity)
{
    m_inventory[itemId] += quantity;
}

bool EmergentNPC::removeItem(const std::string& itemId, int quantity)
{
    auto it = m_inventory.find(itemId);
    if (it != m_inventory.end() && it->second >= quantity) {
        it->second -= quantity;
        if (it->second <= 0) {
            m_inventory.erase(it);
        }
        return true;
    }
    return false;
}

int EmergentNPC::getSkillLevel(const std::string& skillId) const
{
    auto it = m_skills.find(skillId);
    if (it != m_skills.end()) {
        return it->second;
    }
    return 0;
}

void EmergentNPC::setSkillLevel(const std::string& skillId, int level)
{
    m_skills[skillId] = level;
}

void EmergentNPC::improveSkill(const std::string& skillId, int amount)
{
    m_skills[skillId] += amount;
}

float EmergentNPC::getFactionStanding(const std::string& factionId) const
{
    auto it = m_factionStanding.find(factionId);
    if (it != m_factionStanding.end()) {
        return it->second;
    }
    return 0.0f;
}

void EmergentNPC::setFactionStanding(const std::string& factionId, float value)
{
    m_factionStanding[factionId] = std::max(-1.0f, std::min(1.0f, value));
}

void EmergentNPC::changeFactionStanding(const std::string& factionId, float delta)
{
    float current = getFactionStanding(factionId);
    setFactionStanding(factionId, current + delta);
}

float EmergentNPC::getRelationship(const std::string& targetNpcId) const
{
    auto it = m_relationships.find(targetNpcId);
    if (it != m_relationships.end()) {
        return it->second;
    }
    return 0.0f;
}

void EmergentNPC::setRelationship(const std::string& targetNpcId, float value)
{
    m_relationships[targetNpcId] = std::max(-1.0f, std::min(1.0f, value));
}

void EmergentNPC::changeRelationship(const std::string& targetNpcId, float delta)
{
    float current = getRelationship(targetNpcId);
    setRelationship(targetNpcId, current + delta);
}

void EmergentNPC::addScheduleEntry(int priority, const ScheduleEntry& entry)
{
    m_schedule[priority] = entry;
}

EmergentNPC::ScheduleEntry* EmergentNPC::getCurrentScheduleEntry(int currentHour)
{
    for (auto& pair : m_schedule) {
        auto& entry = pair.second;
        if (entry.startHour <= currentHour && currentHour < entry.endHour) {
            return &entry;
        }
    }
    return nullptr;
}

void EmergentNPC::updateBasedOnSchedule(GameContext* context)
{
    TimeSystem* timeSystem = context->getTimeSystem();
    if (!timeSystem) {
        return;
    }

    int currentHour = timeSystem->getHour();
    auto entry = getCurrentScheduleEntry(currentHour);

    if (entry) {
        // If we're not in the right location, move there
        if (m_currentLocation != entry->locationId) {
            setCurrentLocation(entry->locationId);
        }

        // If we're not doing the scheduled action, start it
        auto currentAction = getCurrentAction();
        if (!currentAction || currentAction->getId() != entry->actionId) {
            auto action = context->getAction(entry->actionId);
            if (action && action->canPerform(context, getId())) {
                performAction(context, action);
            }
        }
    }
}

void EmergentNPC::assignQuest(const std::string& questId)
{
    if (!hasActiveQuest(questId)) {
        m_activeQuests.push_back(questId);
    }
}

void EmergentNPC::completeQuest(const std::string& questId)
{
    auto it = std::find(m_activeQuests.begin(), m_activeQuests.end(), questId);
    if (it != m_activeQuests.end()) {
        m_activeQuests.erase(it);
        m_completedQuests.push_back(questId);
    }
}

bool EmergentNPC::hasActiveQuest(const std::string& questId) const
{
    return std::find(m_activeQuests.begin(), m_activeQuests.end(), questId) != m_activeQuests.end();
}

void EmergentNPC::update(GameContext* context, float deltaTime)
{
    // Update needs
    for (auto& need : getNeeds()) {
        need->update(deltaTime);
    }

    // Update based on schedule
    updateBasedOnSchedule(context);

    // Call base update if not following schedule
    if (!getCurrentAction()) {
        NPC::update(context, deltaTime);
    }
}

json EmergentNPC::toJson() const
{
    json j = NPC::toJson();

    // Add Emergent properties
    j["strength"] = m_strength;
    j["intelligence"] = m_intelligence;
    j["agility"] = m_agility;
    j["endurance"] = m_endurance;
    j["charisma"] = m_charisma;

    // Schedule
    json scheduleJson = json::object();
    for (const auto& pair : m_schedule) {
        json entryJson;
        entryJson["actionId"] = pair.second.actionId;
        entryJson["locationId"] = pair.second.locationId;
        entryJson["startHour"] = pair.second.startHour;
        entryJson["endHour"] = pair.second.endHour;
        scheduleJson[std::to_string(pair.first)] = entryJson;
    }
    j["schedule"] = scheduleJson;

    // Quests
    j["activeQuests"] = m_activeQuests;
    j["completedQuests"] = m_completedQuests;

    return j;
}

void EmergentNPC::fromJson(const json& data)
{
    NPC::fromJson(data);

    // Load Emergent properties
    m_strength = data["strength"];
    m_intelligence = data["intelligence"];
    m_agility = data["agility"];
    m_endurance = data["endurance"];
    m_charisma = data["charisma"];

    // Schedule
    m_schedule.clear();
    for (auto it = data["schedule"].begin(); it != data["schedule"].end(); ++it) {
        int priority = std::stoi(it.key());
        ScheduleEntry entry;
        entry.actionId = it.value()["actionId"];
        entry.locationId = it.value()["locationId"];
        entry.startHour = it.value()["startHour"];
        entry.endHour = it.value()["endHour"];
        m_schedule[priority] = entry;
    }

    // Quests
    m_activeQuests = data["activeQuests"].get<std::vector<std::string>>();
    m_completedQuests = data["completedQuests"].get<std::vector<std::string>>();
}