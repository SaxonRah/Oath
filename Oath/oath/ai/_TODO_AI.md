
```cpp
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
    FullGameContext context;

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
```