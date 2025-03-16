```cpp
// main.cpp
#include "core/TAController.hpp"
#include "systems/faction/FactionSystemNode.hpp"
#include "systems/faction/FactionQuestNode.hpp"
#include "systems/faction/FactionLocationNode.hpp"
#include "systems/faction/PoliticalEvent.hpp"
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace oath;

int main() {
    // Create TAController
    TAController controller;

    // Create Faction System Node with path to config file
    FactionSystemNode* factionSystem = 
        static_cast<FactionSystemNode*>(controller.createNode<FactionSystemNode>("FactionSystem", "resources/config/FactionReputation.json"));

    // Load political events from JSON
    std::vector<PoliticalEvent> politicalEvents;
    try {
        std::ifstream file("resources/config/FactionReputation.json");
        if (file.is_open()) {
            json j;
            file >> j;
            file.close();

            if (j.contains("politicalEvents") && j["politicalEvents"].is_array()) {
                for (const auto& eventJson : j["politicalEvents"]) {
                    politicalEvents.push_back(PoliticalEvent::fromJson(eventJson));
                }
                std::cout << "Loaded " << politicalEvents.size() << " political events from JSON" << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error loading political events: " << e.what() << std::endl;
    }

    // Create some faction quests
    FactionQuestNode* merchantQuest = 
        static_cast<FactionQuestNode*>(controller.createNode<FactionQuestNode>("MerchantTradingCompetition", "merchants_guild"));
    merchantQuest->questTitle = "The Grand Trading Competition";
    merchantQuest->questDescription = "The Merchants Guild is holding its annual trading competition.";
    merchantQuest->reputationReward = 15;

    // Create faction locations
    FactionLocationNode* merchantsGuildHall = 
        static_cast<FactionLocationNode*>(controller.createNode<FactionLocationNode>(
            "MerchantsGuildHall", "Merchants Guild Hall", "merchants_guild"));
    merchantsGuildHall->description = "A grand building with ornate columns and gold-plated doors.";

    // Register the faction system
    controller.setSystemNode("FactionSystem", factionSystem);

    // Initialize the player with some faction reputation
    controller.gameContext.playerStats.factionReputation["merchants_guild"] = 15;
    controller.gameContext.playerStats.factionReputation["noble_houses"] = 5;
    controller.gameContext.playerStats.factionReputation["city_guard"] = 10;

    // Update faction system with these initial values
    auto it_merchants = factionSystem->factions.find("merchants_guild");
    if (it_merchants != factionSystem->factions.end()) {
        it_merchants->second.playerReputation = 15;
        it_merchants->second.updatePlayerReputationState();
        it_merchants->second.playerRank = 1; // Join as initiate
    }

    // Display faction system
    controller.processInput("FactionSystem", {});

    // View Merchants Guild details
    TAInput viewMerchantsInput = {
        "faction_action",
        { { "action", std::string("view") }, { "faction_id", std::string("merchants_guild") } }
    };
    controller.processInput("FactionSystem", viewMerchantsInput);

    // Complete a quest for the Merchants Guild and gain reputation
    std::cout << "\nCompleting a trading mission for the Merchants Guild..." << std::endl;
    factionSystem->changePlayerReputation("merchants_guild", 10, &controller.gameContext);
    
    // View faction relations
    std::cout << "\nExamining the political landscape of the kingdom..." << std::endl;
    TAInput viewRelationsInput = {
        "faction_action",
        { { "action", std::string("view_relations") } }
    };
    controller.processInput("FactionSystem", viewRelationsInput);

    // Simulate a political event
    if (!politicalEvents.empty()) {
        std::cout << "\nSimulating political event: " << politicalEvents[0].name << std::endl;
        politicalEvents[0].checkAndExecute(&controller.gameContext, factionSystem);
    }

    // Check for rank advancement
    if (it_merchants != factionSystem->factions.end() && it_merchants->second.canAdvanceRank()) {
        TAInput advanceRankInput = {
            "faction_action",
            { { "action", std::string("advance_rank") }, { "faction_id", std::string("merchants_guild") } }
        };
        controller.processInput("FactionSystem", advanceRankInput);
    }

    // Final faction status check
    std::cout << "\nCurrent faction standings after all events:" << std::endl;
    controller.processInput("FactionSystem", {});

    // Save state demonstration
    std::cout << "\nSaving faction system state..." << std::endl;
    factionSystem->saveToJson("resources/saves/faction_save.json");

    return 0;
}
```