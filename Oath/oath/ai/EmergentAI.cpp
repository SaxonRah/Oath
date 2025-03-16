// EmergentAI.cpp
#include "EmergentAI.h"
#include <fstream>
#include <iostream>

// Need implementation
Need::Need(const std::string& id, float initialValue, float decayRate)
    : m_id(id)
    , m_value(initialValue)
    , m_decayRate(decayRate)
{
}

void Need::update(float deltaTime)
{
    m_value = std::max(0.0f, m_value - (m_decayRate * deltaTime));
}

void Need::satisfy(float amount)
{
    m_value = std::min(1.0f, m_value + amount);
}

Need::Priority Need::getPriority() const
{
    if (m_value < 0.25f)
        return Priority::CRITICAL;
    if (m_value < 0.5f)
        return Priority::HIGH;
    if (m_value < 0.75f)
        return Priority::MEDIUM;
    return Priority::LOW;
}

json Need::toJson() const
{
    json j;
    j["id"] = m_id;
    j["value"] = m_value;
    j["decayRate"] = m_decayRate;
    return j;
}

void Need::fromJson(const json& data)
{
    m_id = data["id"];
    m_value = data["value"];
    m_decayRate = data["decayRate"];
}

// Action implementation
Action::Action(const std::string& id, const std::map<std::string, float>& needEffects)
    : m_id(id)
    , m_needEffects(needEffects)
    , m_duration(1.0f)
{
}

float Action::getEffectOnNeed(const std::string& needId) const
{
    auto it = m_needEffects.find(needId);
    if (it != m_needEffects.end()) {
        return it->second;
    }
    return 0.0f;
}

bool Action::canPerform(GameContext* context, const std::string& npcId) const
{
    return checkRequirements(context, npcId);
}

void Action::execute(GameContext* context, const std::string& npcId)
{
    // Default implementation does nothing
}

float Action::getUtility(GameContext* context, const std::string& npcId) const
{
    if (!canPerform(context, npcId)) {
        return 0.0f;
    }

    auto npc = context->getNPC(npcId);
    if (!npc) {
        return 0.0f;
    }

    float utility = 0.0f;
    for (const auto& need : npc->getNeeds()) {
        float effect = getEffectOnNeed(need->getId());
        // Weight effect by need priority
        float priorityMult = 1.0f;
        switch (need->getPriority()) {
        case Need::Priority::LOW:
            priorityMult = 1.0f;
            break;
        case Need::Priority::MEDIUM:
            priorityMult = 2.0f;
            break;
        case Need::Priority::HIGH:
            priorityMult = 4.0f;
            break;
        case Need::Priority::CRITICAL:
            priorityMult = 8.0f;
            break;
        }
        utility += effect * priorityMult * (1.0f - need->getValue());
    }

    return utility;
}

bool Action::checkRequirements(GameContext* context, const std::string& npcId) const
{
    // Base implementation assumes no requirements
    return true;
}

json Action::toJson() const
{
    json j;
    j["id"] = m_id;
    j["needEffects"] = m_needEffects;
    j["duration"] = m_duration;
    return j;
}

void Action::fromJson(const json& data)
{
    m_id = data["id"];
    m_needEffects = data["needEffects"].get<std::map<std::string, float>>();
    m_duration = data["duration"];
}

// NPC implementation
NPC::NPC(const std::string& id, const std::string& name)
    : m_id(id)
    , m_name(name)
    , m_currentAction(nullptr)
    , m_actionProgress(0.0f)
{
}

std::shared_ptr<Need> NPC::getNeed(const std::string& needId) const
{
    for (const auto& need : m_needs) {
        if (need->getId() == needId) {
            return need;
        }
    }
    return nullptr;
}

void NPC::addNeed(std::shared_ptr<Need> need)
{
    m_needs.push_back(need);
}

void NPC::update(GameContext* context, float deltaTime)
{
    // Update needs
    for (auto& need : m_needs) {
        need->update(deltaTime);
    }

    // Update current action
    if (m_currentAction) {
        m_actionProgress += deltaTime / m_currentAction->getDuration();
        if (m_actionProgress >= 1.0f) {
            completeCurrentAction(context);
        }
    } else {
        // Select a new action if we don't have one
        auto actions = context->getAllActions();
        auto bestAction = selectBestAction(context, actions);
        if (bestAction) {
            performAction(context, bestAction);
        }
    }
}

void NPC::performAction(GameContext* context, std::shared_ptr<Action> action)
{
    if (!action->canPerform(context, m_id)) {
        return;
    }

    m_currentAction = action;
    m_actionProgress = 0.0f;
    action->execute(context, m_id);
}

std::shared_ptr<Action> NPC::selectBestAction(GameContext* context, const std::vector<std::shared_ptr<Action>>& availableActions)
{
    std::shared_ptr<Action> bestAction = nullptr;
    float bestUtility = 0.0f;

    for (const auto& action : availableActions) {
        float utility = action->getUtility(context, m_id);
        if (utility > bestUtility) {
            bestUtility = utility;
            bestAction = action;
        }
    }

    return bestAction;
}

void NPC::completeCurrentAction(GameContext* context)
{
    if (!m_currentAction) {
        return;
    }

    // Apply need effects
    for (auto& need : m_needs) {
        float effect = m_currentAction->getEffectOnNeed(need->getId());
        if (effect > 0.0f) {
            need->satisfy(effect);
        }
    }

    m_currentAction = nullptr;
    m_actionProgress = 0.0f;
}

json NPC::toJson() const
{
    json j;
    j["id"] = m_id;
    j["name"] = m_name;
    j["currentLocation"] = m_currentLocation;

    json needsJson = json::array();
    for (const auto& need : m_needs) {
        needsJson.push_back(need->toJson());
    }
    j["needs"] = needsJson;

    j["inventory"] = m_inventory;
    j["skills"] = m_skills;
    j["factionStanding"] = m_factionStanding;
    j["relationships"] = m_relationships;

    if (m_currentAction) {
        j["currentAction"] = m_currentAction->getId();
        j["actionProgress"] = m_actionProgress;
    } else {
        j["currentAction"] = nullptr;
        j["actionProgress"] = 0.0f;
    }

    return j;
}

void NPC::fromJson(const json& data)
{
    m_id = data["id"];
    m_name = data["name"];
    m_currentLocation = data["currentLocation"];

    m_needs.clear();
    for (const auto& needJson : data["needs"]) {
        auto need = std::make_shared<Need>("", 0.0f, 0.0f);
        need->fromJson(needJson);
        m_needs.push_back(need);
    }

    m_inventory = data["inventory"].get<std::map<std::string, int>>();
    m_skills = data["skills"].get<std::map<std::string, int>>();
    m_factionStanding = data["factionStanding"].get<std::map<std::string, float>>();
    m_relationships = data["relationships"].get<std::map<std::string, float>>();

    // Current action will be resolved after all actions are loaded
    m_actionProgress = data["actionProgress"];
}

// GameContext implementation
GameContext::GameContext()
{
    initializeSystems();
}

void GameContext::initializeSystems()
{
    m_questSystem = std::make_unique<QuestSystem>(this);
    m_dialogueSystem = std::make_unique<DialogueSystem>(this);
    m_progressionSystem = std::make_unique<CharacterProgressionSystem>(this);
    m_craftingSystem = std::make_unique<CraftingSystem>(this);
    m_timeSystem = std::make_unique<TimeSystem>(this);
    m_weatherSystem = std::make_unique<WeatherSystem>(this);
    m_crimeSystem = std::make_unique<CrimeSystem>(this);
    m_healthSystem = std::make_unique<HealthSystem>(this);
    m_economySystem = std::make_unique<EconomySystem>(this);
    m_factionSystem = std::make_unique<FactionSystem>(this);
    m_relationshipSystem = std::make_unique<RelationshipSystem>(this);
    m_religionSystem = std::make_unique<ReligionSystem>(this);
    m_spellCraftingSystem = std::make_unique<SpellCraftingSystem>(this);
}

void GameContext::addNPC(std::shared_ptr<NPC> npc)
{
    m_npcs[npc->getId()] = npc;
}

std::shared_ptr<NPC> GameContext::getNPC(const std::string& npcId) const
{
    auto it = m_npcs.find(npcId);
    if (it != m_npcs.end()) {
        return it->second;
    }
    return nullptr;
}

std::vector<std::shared_ptr<NPC>> GameContext::getAllNPCs() const
{
    std::vector<std::shared_ptr<NPC>> result;
    for (const auto& pair : m_npcs) {
        result.push_back(pair.second);
    }
    return result;
}

void GameContext::registerAction(std::shared_ptr<Action> action)
{
    m_actions[action->getId()] = action;
}

std::shared_ptr<Action> GameContext::getAction(const std::string& actionId) const
{
    auto it = m_actions.find(actionId);
    if (it != m_actions.end()) {
        return it->second;
    }
    return nullptr;
}

std::vector<std::shared_ptr<Action>> GameContext::getAllActions() const
{
    std::vector<std::shared_ptr<Action>> result;
    for (const auto& pair : m_actions) {
        result.push_back(pair.second);
    }
    return result;
}

void GameContext::update(float deltaTime)
{
    // Update systems
    m_timeSystem->update(deltaTime);
    m_weatherSystem->update(deltaTime);
    m_crimeSystem->update(deltaTime);
    m_healthSystem->update(deltaTime);
    m_economySystem->update(deltaTime);
    m_factionSystem->update(deltaTime);
    m_relationshipSystem->update(deltaTime);
    m_religionSystem->update(deltaTime);
    m_spellCraftingSystem->update(deltaTime);
    m_questSystem->update(deltaTime);
    m_dialogueSystem->update(deltaTime);
    m_progressionSystem->update(deltaTime);
    m_craftingSystem->update(deltaTime);

    // Update NPCs
    for (auto& npcPair : m_npcs) {
        npcPair.second->update(this, deltaTime);
    }
}

json GameContext::toJson() const
{
    json j;

    // Save systems
    j["questSystem"] = m_questSystem->toJson();
    j["dialogueSystem"] = m_dialogueSystem->toJson();
    j["progressionSystem"] = m_progressionSystem->toJson();
    j["craftingSystem"] = m_craftingSystem->toJson();
    j["timeSystem"] = m_timeSystem->toJson();
    j["weatherSystem"] = m_weatherSystem->toJson();
    j["crimeSystem"] = m_crimeSystem->toJson();
    j["healthSystem"] = m_healthSystem->toJson();
    j["economySystem"] = m_economySystem->toJson();
    j["factionSystem"] = m_factionSystem->toJson();
    j["relationshipSystem"] = m_relationshipSystem->toJson();
    j["religionSystem"] = m_religionSystem->toJson();
    j["spellCraftingSystem"] = m_spellCraftingSystem->toJson();

    // Save NPCs
    json npcsJson = json::object();
    for (const auto& npcPair : m_npcs) {
        npcsJson[npcPair.first] = npcPair.second->toJson();
    }
    j["npcs"] = npcsJson;

    // Save actions
    json actionsJson = json::object();
    for (const auto& actionPair : m_actions) {
        actionsJson[actionPair.first] = actionPair.second->toJson();
    }
    j["actions"] = actionsJson;

    return j;
}

void GameContext::fromJson(const json& data)
{
    // Load systems
    m_questSystem->fromJson(data["questSystem"]);
    m_dialogueSystem->fromJson(data["dialogueSystem"]);
    m_progressionSystem->fromJson(data["progressionSystem"]);
    m_craftingSystem->fromJson(data["craftingSystem"]);
    m_timeSystem->fromJson(data["timeSystem"]);
    m_weatherSystem->fromJson(data["weatherSystem"]);
    m_crimeSystem->fromJson(data["crimeSystem"]);
    m_healthSystem->fromJson(data["healthSystem"]);
    m_economySystem->fromJson(data["economySystem"]);
    m_factionSystem->fromJson(data["factionSystem"]);
    m_relationshipSystem->fromJson(data["relationshipSystem"]);
    m_religionSystem->fromJson(data["religionSystem"]);
    m_spellCraftingSystem->fromJson(data["spellCraftingSystem"]);

    // Load NPCs
    m_npcs.clear();
    for (auto it = data["npcs"].begin(); it != data["npcs"].end(); ++it) {
        auto npc = std::make_shared<NPC>("", "");
        npc->fromJson(it.value());
        m_npcs[npc->getId()] = npc;
    }

    // Load actions
    m_actions.clear();
    for (auto it = data["actions"].begin(); it != data["actions"].end(); ++it) {
        auto action = std::make_shared<Action>("", std::map<std::string, float>());
        action->fromJson(it.value());
        m_actions[action->getId()] = action;
    }

    // Resolve action references in NPCs
    for (auto& npcPair : m_npcs) {
        auto npc = npcPair.second;
        const auto& npcJson = data["npcs"][npc->getId()];
        if (!npcJson["currentAction"].is_null()) {
            std::string actionId = npcJson["currentAction"];
            npc->performAction(this, getAction(actionId));
        }
    }
}

bool GameContext::loadFromFile(const std::string& filename)
{
    try {
        std::ifstream file(filename);
        if (!file.is_open()) {
            return false;
        }

        json data = json::parse(file);
        fromJson(data);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading game state: " << e.what() << std::endl;
        return false;
    }
}

bool GameContext::saveToFile(const std::string& filename) const
{
    try {
        std::ofstream file(filename);
        if (!file.is_open()) {
            return false;
        }

        json data = toJson();
        file << data.dump(4); // Pretty print with 4-space indent
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error saving game state: " << e.what() << std::endl;
        return false;
    }
}

// main.cpp (demonstration of loading and using the system)
#include "RadiantAI.h"
#include "SpecializedActions.h"
#include <fstream>
#include <iostream>


// Example JSON configuration for needs
const char* NEEDS_CONFIG = R"(
{
    "needs": [
        {
            "id": "hunger",
            "decayRate": 0.1,
            "initialValue": 0.8,
            "description": "Need for food"
        },
        {
            "id": "thirst",
            "decayRate": 0.15,
            "initialValue": 0.9,
            "description": "Need for water"
        },
        {
            "id": "sleep",
            "decayRate": 0.05,
            "initialValue": 0.7,
            "description": "Need for rest"
        },
        {
            "id": "social",
            "decayRate": 0.03,
            "initialValue": 0.6,
            "description": "Need for social interaction"
        },
        {
            "id": "wealth",
            "decayRate": 0.01,
            "initialValue": 0.5,
            "description": "Need for money and possessions"
        },
        {
            "id": "entertainment",
            "decayRate": 0.02,
            "initialValue": 0.5,
            "description": "Need for entertainment and fun"
        },
        {
            "id": "spirituality",
            "decayRate": 0.01,
            "initialValue": 0.4,
            "description": "Need for spiritual fulfillment"
        }
    ]
}
)";

// Example JSON configuration for actions
const char* ACTIONS_CONFIG = R"(
{
    "actions": [
        {
            "id": "eat_meal",
            "type": "location",
            "locationId": "tavern",
            "duration": 1.0,
            "needEffects": {
                "hunger": 0.8,
                "thirst": 0.4,
                "social": 0.2
            },
            "description": "Eat a meal at the tavern"
        },
        {
            "id": "drink_water",
            "type": "location",
            "locationId": "well",
            "duration": 0.5,
            "needEffects": {
                "thirst": 0.7
            },
            "description": "Drink water from the well"
        },
        {
            "id": "sleep",
            "type": "location",
            "locationId": "home",
            "duration": 8.0,
            "needEffects": {
                "sleep": 0.9
            },
            "description": "Sleep at home"
        },
        {
            "id": "chat_with_friend",
            "type": "social",
            "targetNpcId": "npc_2",
            "interactionType": "chat",
            "duration": 1.0,
            "needEffects": {
                "social": 0.6,
                "entertainment": 0.3
            },
            "description": "Chat with a friend"
        },
        {
            "id": "pray_at_temple",
            "type": "religious",
            "deityId": "sun_god",
            "ritualType": "prayer",
            "duration": 1.0,
            "needEffects": {
                "spirituality": 0.7
            },
            "description": "Pray at the temple"
        },
        {
            "id": "blacksmith_work",
            "type": "crafting",
            "recipeId": "iron_sword",
            "craftingStationId": "forge",
            "duration": 3.0,
            "needEffects": {
                "wealth": 0.5,
                "hunger": -0.2,
                "thirst": -0.2
            },
            "description": "Work as a blacksmith"
        },
        {
            "id": "sell_goods",
            "type": "economic",
            "activityType": "sell",
            "resources": {
                "iron_sword": 1
            },
            "duration": 1.0,
            "needEffects": {
                "wealth": 0.6
            },
            "description": "Sell goods at the market"
        }
    ]
}
)";

// Example JSON configuration for NPCs
const char* NPCS_CONFIG = R"(
{
    "npcs": [
        {
            "id": "npc_1",
            "name": "John Smith",
            "occupation": "blacksmith",
            "home": "blacksmith_house",
            "work": "forge",
            "schedule": {
                "0": { "activity": "sleep", "location": "home", "startHour": 22, "endHour": 6 },
                "1": { "activity": "eat_meal", "location": "home", "startHour": 7, "endHour": 8 },
                "2": { "activity": "blacksmith_work", "location": "forge", "startHour": 9, "endHour": 18 },
                "3": { "activity": "eat_meal", "location": "tavern", "startHour": 19, "endHour": 20 }
            },
            "needs": ["hunger", "thirst", "sleep", "social", "wealth"]
        },
        {
            "id": "npc_2",
            "name": "Mary Baker",
            "occupation": "baker",
            "home": "bakery_house",
            "work": "bakery",
            "schedule": {
                "0": { "activity": "sleep", "location": "home", "startHour": 21, "endHour": 4 },
                "1": { "activity": "work", "location": "bakery", "startHour": 5, "endHour": 13 },
                "2": { "activity": "social", "location": "market", "startHour": 14, "endHour": 17 },
                "3": { "activity": "eat_meal", "location": "tavern", "startHour": 18, "endHour": 20 }
            },
            "needs": ["hunger", "thirst", "sleep", "social", "wealth", "spirituality"]
        }
    ]
}
)";

// Create NPC from JSON configuration
std::shared_ptr<NPC> createNpcFromJson(const json& npcJson, const json& needsJson)
{
    std::string id = npcJson["id"];
    std::string name = npcJson["name"];

    auto npc = std::make_shared<NPC>(id, name);

    // Add needs to NPC
    for (const auto& needId : npcJson["needs"]) {
        // Find need config
        for (const auto& needJson : needsJson["needs"]) {
            if (needJson["id"] == needId) {
                float initialValue = needJson["initialValue"];
                float decayRate = needJson["decayRate"];
                auto need = std::make_shared<Need>(needId, initialValue, decayRate);
                npc->addNeed(need);
                break;
            }
        }
    }

    return npc;
}

// Create action from JSON configuration
std::shared_ptr<Action> createActionFromJson(const json& actionJson)
{
    std::string id = actionJson["id"];
    std::string type = actionJson["type"];

    // Parse need effects
    std::map<std::string, float> needEffects;
    for (auto it = actionJson["needEffects"].begin(); it != actionJson["needEffects"].end(); ++it) {
        needEffects[it.key()] = it.value();
    }

    std::shared_ptr<Action> action;

    if (type == "location") {
        std::string locationId = actionJson["locationId"];
        float duration = actionJson["duration"];
        action = std::make_shared<LocationAction>(id, needEffects, locationId, duration);
    } else if (type == "social") {
        std::string targetNpcId = actionJson["targetNpcId"];
        std::string interactionType = actionJson["interactionType"];
        action = std::make_shared<SocialAction>(id, needEffects, targetNpcId, interactionType);
    } else if (type == "religious") {
        std::string deityId = actionJson["deityId"];
        std::string ritualType = actionJson["ritualType"];
        action = std::make_shared<ReligiousAction>(id, needEffects, deityId, ritualType);
    } else if (type == "crafting") {
        std::string recipeId = actionJson["recipeId"];
        std::string craftingStationId = actionJson["craftingStationId"];
        action = std::make_shared<CraftingAction>(id, needEffects, recipeId, craftingStationId);
    } else if (type == "economic") {
        std::string activityType = actionJson["activityType"];
        std::map<std::string, int> resources;
        for (auto it = actionJson["resources"].begin(); it != actionJson["resources"].end(); ++it) {
            resources[it.key()] = it.value();
        }
        action = std::make_shared<EconomicAction>(id, needEffects, activityType, resources);
    } else {
        // Default to basic action
        action = std::make_shared<Action>(id, needEffects);
    }

    return action;
}

int main()
{
    // Create game context
    GameContext context;

    // Parse JSON configurations
    json needsConfig = json::parse(NEEDS_CONFIG);
    json actionsConfig = json::parse(ACTIONS_CONFIG);
    json npcsConfig = json::parse(NPCS_CONFIG);

    // Register actions
    for (const auto& actionJson : actionsConfig["actions"]) {
        auto action = createActionFromJson(actionJson);
        context.registerAction(action);
    }

    // Create NPCs
    for (const auto& npcJson : npcsConfig["npcs"]) {
        auto npc = createNpcFromJson(npcJson, needsConfig);
        context.addNPC(npc);
    }

    // Simulation loop
    const float timeStep = 0.1f; // Simulate 0.1 game hour per iteration
    const int numSteps = 100; // Simulate 10 game hours

    for (int step = 0; step < numSteps; ++step) {
        // Update game context (including NPCs and all systems)
        context.update(timeStep);

        // For demonstration: print the current time and what NPCs are doing
        if (step % 10 == 0) { // Print every game hour
            auto timeSystem = context.getTimeSystem();
            std::cout << "=== Game time: Day " << timeSystem->getDay()
                      << ", Hour " << timeSystem->getHour() << " ===" << std::endl;

            auto weatherSystem = context.getWeatherSystem();
            std::cout << "Weather: ";
            switch (weatherSystem->getCurrentWeather()) {
            case WeatherSystem::WeatherType::CLEAR:
                std::cout << "Clear";
                break;
            case WeatherSystem::WeatherType::CLOUDY:
                std::cout << "Cloudy";
                break;
            case WeatherSystem::WeatherType::RAINY:
                std::cout << "Rainy";
                break;
            case WeatherSystem::WeatherType::STORMY:
                std::cout << "Stormy";
                break;
            case WeatherSystem::WeatherType::SNOWY:
                std::cout << "Snowy";
                break;
            }
            std::cout << std::endl;

            for (const auto& npc : context.getAllNPCs()) {
                std::cout << npc->getName() << " is ";
                auto currentAction = npc->getCurrentAction();
                if (currentAction) {
                    std::cout << "performing action: " << currentAction->getId();
                } else {
                    std::cout << "idle";
                }
                std::cout << std::endl;

                // Display needs
                std::cout << "  Needs:";
                for (const auto& need : npc->getNeeds()) {
                    std::cout << " " << need->getId() << "(" << need->getValue() << ")";
                }
                std::cout << std::endl;
            }
            std::cout << std::endl;
        }
    }

    // Save game state to file
    context.saveToFile("game_state.json");
    std::cout << "Simulation completed and saved to game_state.json" << std::endl;

    return 0;
}