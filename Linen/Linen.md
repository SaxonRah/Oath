## Core Architecture Redesign

### 1. Unified Plugin Interface

We need a common plugin interface that all systems can implement.

```cpp
// GameSystemPlugin.hpp
#pragma once

#include <string>
#include <memory>
#include "TACore.hpp"  // Include core TA system

class GameContext;  // Forward declaration

class GameSystemPlugin {
public:
    virtual ~GameSystemPlugin() = default;
    
    // Core functionality
    virtual std::string getSystemName() const = 0;
    virtual std::string getDescription() const = 0;
    virtual TANode* getRootNode() const = 0;
    
    // Lifecycle hooks
    virtual void initialize(TAController* controller, GameContext* context) = 0;
    virtual void shutdown() = 0;
    
    // System update (called every game frame/tick)
    virtual void update(float deltaTime) = 0;
    
    // Save/load system data
    virtual json saveState() const = 0;
    virtual bool loadState(const json& data) = 0;
    
    // Event handling (allows cross-system communication)
    virtual bool handleEvent(const std::string& eventType, const json& eventData) = 0;
};

// Helper for creating plugin instances
template<typename T>
std::unique_ptr<GameSystemPlugin> createPlugin() {
    return std::make_unique<T>();
}
```

### 2. Plugin Registry and Manager

We need to create a central manager to handle all plugins.

```cpp
// GameSystemManager.hpp
#pragma once

#include <map>
#include <memory>
#include <vector>
#include "GameSystemPlugin.hpp"

class TAController;
class GameContext;

class GameSystemManager {
private:
    TAController* controller;
    GameContext* gameContext;
    std::map<std::string, std::unique_ptr<GameSystemPlugin>> plugins;
    
public:
    GameSystemManager(TAController* ctrl, GameContext* ctx);
    
    // Plugin registration
    void registerPlugin(std::unique_ptr<GameSystemPlugin> plugin);
    
    // Plugin access
    GameSystemPlugin* getPlugin(const std::string& name);
    
    // System-wide operations
    void initializeAll();
    void updateAll(float deltaTime);
    void shutdownAll();
    
    // Save/load all systems
    json saveAllSystems() const;
    bool loadAllSystems(const json& data);
    
    // Global event dispatcher
    bool dispatchEvent(const std::string& eventType, const json& eventData);
};
```

### 3. Enhanced GameContext

Expand GameContext to provide access to the system manager and shared resources:

```cpp
// GameContext.hpp
#pragma once

#include <memory>
#include "CharacterStats.hpp"
#include "WorldState.hpp"
#include "Inventory.hpp"

class GameSystemManager;

class GameContext {
public:
    // Core game state
    CharacterStats playerStats;
    WorldState worldState;
    Inventory playerInventory;
    std::map<std::string, std::string> questJournal;
    std::map<std::string, std::string> dialogueHistory;
    
    // Additional contexts for various systems
    // These would be initialized by respective plugins
    struct HealthContext {
        // ...health system state...
        HealthState playerHealth;
    } healthContext;
    
    struct EconomyContext {
        // ...economy system state...
        int playerGold = 1000;
    } economyContext;
    
    struct CrimeLawContext {
        // ...crime system state...
        CriminalRecord criminalRecord;
    } crimeLawContext;
    
    struct FactionContext {
        // ...faction system state...
        std::map<std::string, int> factionStanding;
    } factionContext;
    
    struct RelationshipContext {
        // ...NPC relationship state...
        std::map<std::string, int> npcRelationships;
    } relationshipContext;
    
    // Access to the system manager (set externally)
    GameSystemManager* systemManager = nullptr;
    
    // Helper methods for system interaction
    template<typename T>
    T* getSystem() {
        if (!systemManager) return nullptr;
        return dynamic_cast<T*>(systemManager->getPlugin(T::SystemName));
    }
};
```

## System Implementations

Redefine each system as a plugin implementing the common interface.

### 1. Crime and Law System

```cpp
// CrimeLawSystem.hpp
#pragma once

#include "GameSystemPlugin.hpp"
#include "CrimeLaw_Core.hpp"  // Core crime system functionality

class CrimeLawSystem : public GameSystemPlugin {
private:
    TAController* controller = nullptr;
    GameContext* context = nullptr;
    
    // System-specific nodes
    CrimeSystemNode* criminalStatusNode = nullptr;
    TheftNode* theftNode = nullptr;
    GuardEncounterNode* guardNode = nullptr;
    JailNode* jailNode = nullptr;
    BountyPaymentNode* bountyNode = nullptr;
    
    // Configuration
    json crimeLawConfig;
    
public:
    static constexpr const char* SystemName = "CrimeLawSystem";
    
    // GameSystemPlugin interface
    std::string getSystemName() const override { return SystemName; }
    std::string getDescription() const override { return "System for crime, bounties, and law enforcement."; }
    TANode* getRootNode() const override { return criminalStatusNode; }
    
    void initialize(TAController* ctrl, GameContext* ctx) override;
    void shutdown() override;
    void update(float deltaTime) override;
    json saveState() const override;
    bool loadState(const json& data) override;
    bool handleEvent(const std::string& eventType, const json& eventData) override;
    
    // Crime-specific methods
    void commitCrime(const std::string& crimeType, int severity, const std::string& region);
    bool payBounty(const std::string& region, int amount);
    CriminalRecord* getPlayerCriminalRecord();
    // ... more crime-specific methods
};
```

### 2. Disease and Health System

```cpp
// DiseaseHealthSystem.hpp
#pragma once

#include "GameSystemPlugin.hpp"
#include "DiseaseHealth_Core.hpp"  // Core health system functionality

class DiseaseHealthSystem : public GameSystemPlugin {
private:
    TAController* controller = nullptr;
    GameContext* context = nullptr;
    
    // System-specific nodes
    HealthStateNode* healthRoot = nullptr;
    DiseaseManager diseaseManager;
    
    // Configuration
    json healthConfig;
    
public:
    static constexpr const char* SystemName = "DiseaseHealthSystem";
    
    // GameSystemPlugin interface
    std::string getSystemName() const override { return SystemName; }
    std::string getDescription() const override { return "System for health, disease, and healing."; }
    TANode* getRootNode() const override { return healthRoot; }
    
    void initialize(TAController* ctrl, GameContext* ctx) override;
    void shutdown() override;
    void update(float deltaTime) override;
    json saveState() const override;
    bool loadState(const json& data) override;
    bool handleEvent(const std::string& eventType, const json& eventData) override;
    
    // Health-specific methods
    void contractDisease(const std::string& diseaseId);
    void heal(float amount);
    bool checkExposure(const std::string& vector);
    // ... more health-specific methods
};
```

### 3. Economy and Market System

```cpp
// EconomyMarketSystem.hpp
#pragma once

#include "GameSystemPlugin.hpp"
#include "EconomyMarket_Core.hpp"  // Core economy functionality

class EconomyMarketSystem : public GameSystemPlugin {
private:
    TAController* controller = nullptr;
    GameContext* context = nullptr;
    
    // System-specific nodes
    EconomicSystemNode* economicSystem = nullptr;
    std::vector<Market*> markets;
    std::vector<TradeRoute> tradeRoutes;
    
    // Configuration
    json economyConfig;
    
public:
    static constexpr const char* SystemName = "EconomyMarketSystem";
    
    // GameSystemPlugin interface
    std::string getSystemName() const override { return SystemName; }
    std::string getDescription() const override { return "System for economy, markets, and trade."; }
    TANode* getRootNode() const override { return economicSystem; }
    
    void initialize(TAController* ctrl, GameContext* ctx) override;
    void shutdown() override;
    void update(float deltaTime) override;
    json saveState() const override;
    bool loadState(const json& data) override;
    bool handleEvent(const std::string& eventType, const json& eventData) override;
    
    // Economy-specific methods
    Market* getMarketById(const std::string& id);
    void adjustPrice(const std::string& commodityId, float multiplier);
    int getPlayerGold() const;
    void changePlayerGold(int amount);
    // ... more economy-specific methods
};
```

### 4. Faction and Reputation System

```cpp
// FactionReputationSystem.hpp
#pragma once

#include "GameSystemPlugin.hpp"
#include "FactionReputation_Core.hpp"  // Core faction functionality

class FactionReputationSystem : public GameSystemPlugin {
private:
    TAController* controller = nullptr;
    GameContext* context = nullptr;
    
    // System-specific nodes
    FactionSystemNode* factionSystem = nullptr;
    std::map<std::string, Faction> factions;
    
    // Configuration
    json factionConfig;
    
public:
    static constexpr const char* SystemName = "FactionReputationSystem";
    
    // GameSystemPlugin interface
    std::string getSystemName() const override { return SystemName; }
    std::string getDescription() const override { return "System for factions, reputation, and politics."; }
    TANode* getRootNode() const override { return factionSystem; }
    
    void initialize(TAController* ctrl, GameContext* ctx) override;
    void shutdown() override;
    void update(float deltaTime) override;
    json saveState() const override;
    bool loadState(const json& data) override;
    bool handleEvent(const std::string& eventType, const json& eventData) override;
    
    // Faction-specific methods
    Faction* getFaction(const std::string& id);
    void changeReputation(const std::string& factionId, int amount);
    int getReputation(const std::string& factionId) const;
    // ... more faction-specific methods
};
```

### 5. NPC Relationship System

```cpp
// NPCRelationshipSystem.hpp
#pragma once

#include "GameSystemPlugin.hpp"
#include "NPCRelationship_Core.hpp"  // Core NPC relationship functionality

class NPCRelationshipSystem : public GameSystemPlugin {
private:
    TAController* controller = nullptr;
    GameContext* context = nullptr;
    
    // System-specific nodes
    RelationshipBrowserNode* browserNode = nullptr;
    NPCRelationshipManager relationshipManager;
    
    // Configuration
    json relationshipConfig;
    
public:
    static constexpr const char* SystemName = "NPCRelationshipSystem";
    
    // GameSystemPlugin interface
    std::string getSystemName() const override { return SystemName; }
    std::string getDescription() const override { return "System for NPC relationships and interactions."; }
    TANode* getRootNode() const override { return browserNode; }
    
    void initialize(TAController* ctrl, GameContext* ctx) override;
    void shutdown() override;
    void update(float deltaTime) override;
    json saveState() const override;
    bool loadState(const json& data) override;
    bool handleEvent(const std::string& eventType, const json& eventData) override;
    
    // Relationship-specific methods
    RelationshipNPC* getNPC(const std::string& id);
    void changeRelationship(const std::string& npcId, int amount);
    bool giveGift(const std::string& npcId, const std::string& itemId);
    // ... more relationship-specific methods
};
```

## Cross-System Integration

### 1. Event-Based Communication

Define a set of game events that systems can listen for.

```cpp
// GameEvents.hpp
#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace GameEvents {
    // Crime events
    const std::string CrimeCommitted = "event.crime.committed";
    const std::string BountyPaid = "event.crime.bounty_paid";
    const std::string ArrestOccurred = "event.crime.arrested";
    
    // Health events
    const std::string HealthChanged = "event.health.changed";
    const std::string DiseaseContracted = "event.health.disease_contracted";
    const std::string DiseaseRecovered = "event.health.disease_recovered";
    
    // Economy events
    const std::string TransactionCompleted = "event.economy.transaction";
    const std::string MarketPriceChanged = "event.economy.price_changed";
    const std::string PropertyPurchased = "event.economy.property_purchased";
    
    // Faction events
    const std::string ReputationChanged = "event.faction.reputation_changed";
    const std::string FactionRelationChanged = "event.faction.relation_changed";
    const std::string RankAdvanced = "event.faction.rank_advanced";
    
    // NPC relationship events
    const std::string RelationshipChanged = "event.npc.relationship_changed";
    const std::string GiftGiven = "event.npc.gift_given";
    const std::string DialogueCompleted = "event.npc.dialogue_completed";
    
    // Create event data for a specific event
    inline nlohmann::json createEventData(const std::string& eventType, const nlohmann::json& data) {
        nlohmann::json eventData;
        eventData["type"] = eventType;
        eventData["data"] = data;
        eventData["timestamp"] = std::time(nullptr);
        return eventData;
    }
}
```

### 2. System Interdependencies

Implement event handlers in each system to react to events from other systems.

```cpp
// Example event handler in FactionReputationSystem
bool FactionReputationSystem::handleEvent(const std::string& eventType, const json& eventData) {
    if (eventType == GameEvents::CrimeCommitted) {
        // Crime affects faction reputation
        std::string factionId = eventData["data"]["targetFaction"];
        int severity = eventData["data"]["severity"];
        changeReputation(factionId, -severity * 2);
        return true;
    }
    else if (eventType == GameEvents::TransactionCompleted) {
        // Economy transactions can affect faction reputation
        if (eventData["data"].contains("factionId")) {
            std::string factionId = eventData["data"]["factionId"];
            int value = eventData["data"]["value"];
            changeReputation(factionId, value / 100); // Small boost for commerce
            return true;
        }
    }
    // Handle other events...
    return false;
}
```

### 3. Integration Example

Here's how the systems would interact when a crime is committed:

```cpp
// When player steals from a faction shop
void handleTheft(GameContext* context, const std::string& factionId, const std::string& itemId, int value) {
    // 1. Get relevant systems
    auto crimeSystem = context->getSystem<CrimeLawSystem>();
    auto economySystem = context->getSystem<EconomyMarketSystem>();
    auto factionSystem = context->getSystem<FactionReputationSystem>();
    
    if (!crimeSystem || !economySystem || !factionSystem) {
        std::cerr << "Required systems not found!" << std::endl;
        return;
    }
    
    // 2. Determine region from current location
    std::string region = context->worldState.getCurrentRegion();
    
    // 3. Commit crime in the crime system
    crimeSystem->commitCrime("theft", calculateTheftSeverity(value), region);
    
    // 4. Create event data
    json eventData = GameEvents::createEventData(
        GameEvents::CrimeCommitted,
        {
            {"type", "theft"},
            {"targetFaction", factionId},
            {"itemId", itemId},
            {"value", value},
            {"severity", calculateTheftSeverity(value)},
            {"region", region}
        }
    );
    
    // 5. Dispatch event to all systems
    context->systemManager->dispatchEvent(GameEvents::CrimeCommitted, eventData);
    
    // 6. Add item to inventory
    Item stolenItem(itemId, "Stolen " + itemId, "stolen", value, 1);
    context->playerInventory.addItem(stolenItem);
}
```

## Main Implementation

### 1. Plugin Registration

```cpp
// main.cpp
#include "GameSystemManager.hpp"
#include "CrimeLawSystem.hpp"
#include "DiseaseHealthSystem.hpp"
#include "EconomyMarketSystem.hpp"
#include "FactionReputationSystem.hpp"
#include "NPCRelationshipSystem.hpp"

int main() {
    // Create the TA controller
    TAController controller;
    
    // Create game context
    GameContext gameContext;
    
    // Create system manager
    GameSystemManager systemManager(&controller, &gameContext);
    gameContext.systemManager = &systemManager;
    
    // Register all plugins
    systemManager.registerPlugin(createPlugin<CrimeLawSystem>());
    systemManager.registerPlugin(createPlugin<DiseaseHealthSystem>());
    systemManager.registerPlugin(createPlugin<EconomyMarketSystem>());
    systemManager.registerPlugin(createPlugin<FactionReputationSystem>());
    systemManager.registerPlugin(createPlugin<NPCRelationshipSystem>());
    
    // Initialize all systems
    systemManager.initializeAll();
    
    // Load saved game if exists
    if (std::filesystem::exists("save_game.json")) {
        std::ifstream file("save_game.json");
        json saveData;
        file >> saveData;
        systemManager.loadAllSystems(saveData);
    }
    
    // Game loop
    bool running = true;
    while (running) {
        float deltaTime = 0.016f; // 60 FPS
        
        // Update all systems
        systemManager.updateAll(deltaTime);
        
        // Process player input and transitions
        // ...
        
        // Check game exit condition
        // ...
    }
    
    // Save game before exit
    json saveData = systemManager.saveAllSystems();
    std::ofstream file("save_game.json");
    file << saveData.dump(4);
    
    // Shutdown all systems
    systemManager.shutdownAll();
    
    return 0;
}
```

### 2. System Implementation Example

Implement part of the Economy system as an example.

```cpp
// EconomyMarketSystem.cpp
#include "EconomyMarketSystem.hpp"
#include "GameEvents.hpp"

void EconomyMarketSystem::initialize(TAController* ctrl, GameContext* ctx) {
    controller = ctrl;
    context = ctx;
    
    // Load configuration
    std::ifstream file("EconomyMarket.json");
    if (file.is_open()) {
        file >> economyConfig;
        file.close();
    }
    
    // Create root economic system node
    economicSystem = dynamic_cast<EconomicSystemNode*>(
        controller->createNode<EconomicSystemNode>("EconomicSystem"));
    
    // Initialize markets from config
    if (economyConfig.contains("markets") && economyConfig["markets"].is_array()) {
        for (const auto& marketData : economyConfig["markets"]) {
            Market* market = Market::fromJson(marketData, economyConfig["commodities"]);
            markets.push_back(market);
            
            // Create market node
            MarketNode* marketNode = dynamic_cast<MarketNode*>(
                controller->createNode<MarketNode>("Market_" + market->id, market));
            
            // Add exit transition back to economic system
            marketNode->addTransition(
                [](const TAInput& input) {
                    return input.type == "market_action" && 
                           std::get<std::string>(input.parameters.at("action")) == "exit";
                },
                economicSystem, "Exit");
            
            // Add as child node
            economicSystem->addChild(marketNode);
        }
    }
    
    // Initialize trade routes
    if (economyConfig.contains("tradeRoutes") && economyConfig["tradeRoutes"].is_array()) {
        for (const auto& routeData : economyConfig["tradeRoutes"]) {
            tradeRoutes.push_back(TradeRoute::fromJson(routeData));
        }
    }
    
    // Register with TAController
    controller->setSystemRoot("EconomySystem", economicSystem);
    
    // Initialize context
    context->economyContext.playerGold = 1000; // Starting gold
}

void EconomyMarketSystem::update(float deltaTime) {
    // Process natural economy updates
    static float accumulatedTime = 0.0f;
    accumulatedTime += deltaTime;
    
    // Daily updates (assuming 1 game day = 24 real seconds)
    if (accumulatedTime >= 24.0f) {
        accumulatedTime = 0.0f;
        
        // Process trade routes
        economicSystem->processTradeRoutes();
        
        // Process economic events
        economicSystem->processEconomicEvents();
        
        // Update markets
        for (auto* market : markets) {
            market->advanceDay();
        }
    }
}

bool EconomyMarketSystem::handleEvent(const std::string& eventType, const json& eventData) {
    if (eventType == GameEvents::CrimeCommitted) {
        // Theft affects market inventory
        if (eventData["data"]["type"] == "theft") {
            std::string marketId = eventData["data"]["targetMarket"];
            std::string itemId = eventData["data"]["itemId"];
            
            Market* market = getMarketById(marketId);
            if (market) {
                // Remove stolen item from market
                for (auto it = market->inventory.items.begin(); it != market->inventory.items.end(); ++it) {
                    if (it->id == itemId) {
                        it->quantity -= 1;
                        if (it->quantity <= 0) {
                            market->inventory.items.erase(it);
                        }
                        break;
                    }
                }
                return true;
            }
        }
    }
    else if (eventType == GameEvents::FactionRelationChanged) {
        // Faction relations affect trade prices
        std::string faction1 = eventData["data"]["faction1"];
        std::string faction2 = eventData["data"]["faction2"];
        int newRelation = eventData["data"]["newValue"];
        
        // Adjust prices for markets owned by these factions
        for (auto* market : markets) {
            if (market->faction == faction1 || market->faction == faction2) {
                // War or hostility increases prices
                if (newRelation < -50) {
                    adjustMarketPrices(market, 1.25f); // 25% markup
                }
                // Alliance or friendship decreases prices
                else if (newRelation > 50) {
                    adjustMarketPrices(market, 0.9f); // 10% discount
                }
            }
        }
        return true;
    }
    
    return false;
}

Market* EconomyMarketSystem::getMarketById(const std::string& id) {
    for (auto* market : markets) {
        if (market->id == id) {
            return market;
        }
    }
    return nullptr;
}

void EconomyMarketSystem::adjustMarketPrices(Market* market, float multiplier) {
    if (!market) return;
    
    // Apply multiplier to all commodities
    for (auto& commodity : market->commodities) {
        commodity.currentPrice *= multiplier;
    }
    
    // Dispatch event for price changes
    json eventData = GameEvents::createEventData(
        GameEvents::MarketPriceChanged,
        {
            {"marketId", market->id},
            {"multiplier", multiplier}
        }
    );
    
    context->systemManager->dispatchEvent(GameEvents::MarketPriceChanged, eventData);
}

int EconomyMarketSystem::getPlayerGold() const {
    return context->economyContext.playerGold;
}

void EconomyMarketSystem::changePlayerGold(int amount) {
    context->economyContext.playerGold += amount;
    
    // Dispatch event for gold change
    json eventData = GameEvents::createEventData(
        GameEvents::TransactionCompleted,
        {
            {"amount", amount},
            {"newBalance", context->economyContext.playerGold}
        }
    );
    
    context->systemManager->dispatchEvent(GameEvents::TransactionCompleted, eventData);
}

// Implement other methods...
```

## Shared Configuration Format

To standardize configuration, define a common format for all system configs.

```json
{
    "systemName": "EconomyMarketSystem",
    "version": "1.0",
    "description": "Economy and market system configuration",
    "settings": {
        "dailyMarketUpdate": true,
        "priceFluctuation": 0.1,
        "taxRate": 0.05,
        "tradeRouteDecay": 0.01
    },
    "markets": [
        // Market definitions...
    ],
    "commodities": [
        // Commodity definitions...
    ],
    "tradeRoutes": [
        // Trade route definitions...
    ],
    "economicEvents": [
        // Event definitions...
    ]
}
```

## Conclusion

This redesign creates a modular, pluggable architecture where:

1. Each system implements a common interface (GameSystemPlugin)
2. Systems communicate through standardized events
3. The central GameSystemManager coordinates all systems
4. GameContext provides shared access to game state
5. Each system maintains its own configuration and internal state
6. Systems can be added, removed, or replaced independently

---

# Main File Using the Plugin System

Example of how the main file would use the plugin system, including initialization, game loop, save/load functionality, and proper shutdown.

```cpp
// main.cpp
#include <chrono>
#include <iostream>
#include <thread>
#include <filesystem>
#include <fstream>

#include "TAController.hpp"
#include "GameContext.hpp"
#include "GameSystemManager.hpp"

// Include all system plugins
#include "CrimeLawSystem.hpp"
#include "DiseaseHealthSystem.hpp"
#include "EconomyMarketSystem.hpp"
#include "FactionReputationSystem.hpp"
#include "NPCRelationshipSystem.hpp"

// Helper function for parsing player input
TAInput parsePlayerInput(const std::string& input, const std::vector<TAAction>& availableActions);

int main() {
    std::cout << "=== Starting Oath RPG Engine ===\n" << std::endl;
    
    //----------------------------------------
    // ENGINE INITIALIZATION
    //----------------------------------------
    
    // Create the TA controller (core engine)
    TAController controller;
    
    // Create game context (shared game state)
    GameContext gameContext;
    
    // Create system manager
    GameSystemManager systemManager(&controller, &gameContext);
    gameContext.systemManager = &systemManager;
    
    //----------------------------------------
    // PLUGIN REGISTRATION
    //----------------------------------------
    std::cout << "Registering game systems..." << std::endl;
    
    // Register all plugin systems
    systemManager.registerPlugin(createPlugin<CrimeLawSystem>());
    systemManager.registerPlugin(createPlugin<DiseaseHealthSystem>());
    systemManager.registerPlugin(createPlugin<EconomyMarketSystem>());
    systemManager.registerPlugin(createPlugin<FactionReputationSystem>());
    systemManager.registerPlugin(createPlugin<NPCRelationshipSystem>());
    
    // Initialize all registered systems
    systemManager.initializeAll();
    std::cout << "All systems initialized successfully.\n" << std::endl;
    
    //----------------------------------------
    // LOAD SAVED GAME (if available)
    //----------------------------------------
    const std::string SAVE_FILE = "save_game.json";
    bool newGame = true;
    
    if (std::filesystem::exists(SAVE_FILE)) {
        std::cout << "Save file found. Do you want to load the saved game? (y/n): ";
        std::string response;
        std::getline(std::cin, response);
        
        if (response == "y" || response == "Y") {
            std::cout << "Loading saved game..." << std::endl;
            try {
                std::ifstream file(SAVE_FILE);
                if (file.is_open()) {
                    json saveData;
                    file >> saveData;
                    file.close();
                    
                    if (systemManager.loadAllSystems(saveData)) {
                        std::cout << "Game loaded successfully!" << std::endl;
                        newGame = false;
                    } else {
                        std::cout << "Failed to load saved game. Starting new game." << std::endl;
                    }
                }
            } catch (const std::exception& e) {
                std::cerr << "Error loading saved game: " << e.what() << std::endl;
                std::cout << "Starting new game instead." << std::endl;
            }
        }
    }
    
    //----------------------------------------
    // NEW GAME INITIALIZATION (if needed)
    //----------------------------------------
    if (newGame) {
        std::cout << "Starting new game..." << std::endl;
        
        // Initialize player starting inventory
        gameContext.playerInventory.addItem(Item("gold_coin", "Gold", "currency", 1, 1000));
        gameContext.playerInventory.addItem(Item("iron_sword", "Iron Sword", "weapon", 100, 1));
        gameContext.playerInventory.addItem(Item("leather_armor", "Leather Armor", "armor", 80, 1));
        gameContext.playerInventory.addItem(Item("health_potion", "Health Potion", "potion", 25, 3));
        
        // Set starting player stats
        gameContext.playerStats.strength = 10;
        gameContext.playerStats.dexterity = 10;
        gameContext.playerStats.constitution = 10;
        gameContext.playerStats.intelligence = 10;
        gameContext.playerStats.wisdom = 10;
        gameContext.playerStats.charisma = 10;
        
        // Initialize basic skills
        gameContext.playerStats.improveSkill("combat", 2);
        gameContext.playerStats.improveSkill("survival", 1);
        gameContext.playerStats.improveSkill("persuasion", 1);
        
        // Set initial world state
        gameContext.worldState.setLocationState("village", "current");
        gameContext.worldState.currentSeason = "spring";
        
        // Start with some basic faction reputation
        gameContext.playerStats.changeFactionRep("villagers", 10);
        gameContext.playerStats.changeFactionRep("merchants", 5);
        
        // Initialize health
        auto* healthSystem = gameContext.getSystem<DiseaseHealthSystem>();
        if (healthSystem) {
            healthSystem->setMaxHealth(100);
            healthSystem->heal(100); // Full health
        }
        
        std::cout << "New game initialized successfully!" << std::endl;
    }
    
    //----------------------------------------
    // MAIN GAME LOOP
    //----------------------------------------
    std::cout << "\n=== Game Starting ===\n" << std::endl;
    
    // Set current active system to start with
    std::string currentSystemName = "WorldSystem"; // Default starting system
    bool gameRunning = true;
    
    auto lastUpdateTime = std::chrono::high_resolution_clock::now();
    
    while (gameRunning) {
        // Calculate delta time
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastUpdateTime).count();
        lastUpdateTime = currentTime;
        
        // Update all systems
        systemManager.updateAll(deltaTime);
        
        // Get current system
        TANode* currentNode = controller.getCurrentNode(currentSystemName);
        if (!currentNode) {
            std::cerr << "Error: No active node in system " << currentSystemName << std::endl;
            break;
        }
        
        // Display current node description
        if (controller.hasNodeChanged(currentSystemName)) {
            controller.resetNodeChangedFlag(currentSystemName);
            currentNode->onEnter(&gameContext);
        }
        
        // Get available actions from current node
        std::vector<TAAction> availableActions = controller.getAvailableActions(currentSystemName);
        
        // Display available actions
        std::cout << "\nAvailable actions:" << std::endl;
        for (size_t i = 0; i < availableActions.size(); i++) {
            std::cout << i + 1 << ". " << availableActions[i].description << std::endl;
        }
        
        // Special system commands
        std::cout << "\nSystem commands:" << std::endl;
        std::cout << "s - Save game" << std::endl;
        std::cout << "q - Quit game" << std::endl;
        std::cout << "h - Help" << std::endl;
        
        // Get player input
        std::cout << "\nEnter your choice: ";
        std::string input;
        std::getline(std::cin, input);
        
        // Process special commands first
        if (input == "q" || input == "Q") {
            std::cout << "Are you sure you want to quit? (y/n): ";
            std::getline(std::cin, input);
            if (input == "y" || input == "Y") {
                gameRunning = false;
                continue;
            }
        }
        else if (input == "s" || input == "S") {
            std::cout << "Saving game..." << std::endl;
            try {
                json saveData = systemManager.saveAllSystems();
                std::ofstream file(SAVE_FILE);
                file << saveData.dump(4);
                file.close();
                std::cout << "Game saved successfully!" << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "Error saving game: " << e.what() << std::endl;
            }
            continue;
        }
        else if (input == "h" || input == "H") {
            std::cout << "\n=== HELP ===\n" << std::endl;
            std::cout << "Enter the number of an action to perform it." << std::endl;
            std::cout << "Type 's' to save the game." << std::endl;
            std::cout << "Type 'q' to quit the game." << std::endl;
            std::cout << "Type 'h' to show this help message." << std::endl;
            std::cout << "Available systems:" << std::endl;
            
            const auto& plugins = systemManager.getPlugins();
            for (const auto& [name, _] : plugins) {
                std::cout << "- " << name << std::endl;
            }
            
            std::cout << "\nPress Enter to continue..." << std::endl;
            std::cin.get();
            continue;
        }
        
        // Parse action from input
        TAInput actionInput = parsePlayerInput(input, availableActions);
        
        // If valid action, process it
        if (!actionInput.type.empty()) {
            // Process action and get next node
            TANode* nextNode = nullptr;
            bool processed = controller.processInput(currentSystemName, actionInput, nextNode);
            
            if (processed && nextNode) {
                // Check if we need to switch systems
                for (const auto& [name, plugin] : systemManager.getPlugins()) {
                    if (plugin->getRootNode() == nextNode) {
                        currentSystemName = name;
                        break;
                    }
                }
            }
        }
        else {
            std::cout << "Invalid input. Try again." << std::endl;
        }
        
        // Short delay to prevent CPU hogging
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    //----------------------------------------
    // GAME SHUTDOWN
    //----------------------------------------
    std::cout << "\n=== Shutting down game ===\n" << std::endl;
    
    // Final save
    std::cout << "Saving game before exit..." << std::endl;
    try {
        json saveData = systemManager.saveAllSystems();
        std::ofstream file(SAVE_FILE);
        file << saveData.dump(4);
        file.close();
        std::cout << "Game saved successfully!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error saving game: " << e.what() << std::endl;
    }
    
    // Shutdown all systems
    systemManager.shutdownAll();
    std::cout << "All systems shut down successfully." << std::endl;
    
    std::cout << "\nThank you for playing Oath RPG!" << std::endl;
    return 0;
}

// Helper function to parse player input
TAInput parsePlayerInput(const std::string& input, const std::vector<TAAction>& availableActions) {
    // Try to parse as action number
    try {
        int actionIndex = std::stoi(input) - 1;
        if (actionIndex >= 0 && actionIndex < static_cast<int>(availableActions.size())) {
            // Call action generator to create input
            return availableActions[actionIndex].inputGenerator();
        }
    } catch (...) {
        // Not a number, continue to other parsing methods
    }
    
    // Try to match action description (partial match)
    for (const auto& action : availableActions) {
        if (action.description.find(input) != std::string::npos) {
            return action.inputGenerator();
        }
    }
    
    // No match found
    return TAInput{};
}
```

## Additional Implementation: GameSystemManager

Implementation of the GameSystemManager class that's central to the plugin system.

```cpp
// GameSystemManager.cpp
#include "GameSystemManager.hpp"
#include <iostream>

GameSystemManager::GameSystemManager(TAController* ctrl, GameContext* ctx)
    : controller(ctrl), gameContext(ctx) {
}

void GameSystemManager::registerPlugin(std::unique_ptr<GameSystemPlugin> plugin) {
    std::string name = plugin->getSystemName();
    std::cout << "Registering plugin: " << name << " - " << plugin->getDescription() << std::endl;
    plugins[name] = std::move(plugin);
}

GameSystemPlugin* GameSystemManager::getPlugin(const std::string& name) {
    auto it = plugins.find(name);
    if (it != plugins.end()) {
        return it->second.get();
    }
    return nullptr;
}

const std::map<std::string, std::unique_ptr<GameSystemPlugin>>& GameSystemManager::getPlugins() const {
    return plugins;
}

void GameSystemManager::initializeAll() {
    for (auto& [name, plugin] : plugins) {
        std::cout << "Initializing system: " << name << "..." << std::endl;
        plugin->initialize(controller, gameContext);
    }
}

void GameSystemManager::updateAll(float deltaTime) {
    for (auto& [name, plugin] : plugins) {
        plugin->update(deltaTime);
    }
}

void GameSystemManager::shutdownAll() {
    for (auto& [name, plugin] : plugins) {
        std::cout << "Shutting down system: " << name << "..." << std::endl;
        plugin->shutdown();
    }
}

json GameSystemManager::saveAllSystems() const {
    json saveData;
    
    // Save global game context
    saveData["worldState"] = {
        {"daysPassed", gameContext->worldState.daysPassed},
        {"currentSeason", gameContext->worldState.currentSeason},
        {"locationStates", gameContext->worldState.locationStates},
        {"factionStates", gameContext->worldState.factionStates},
        {"worldFlags", gameContext->worldState.worldFlags}
    };
    
    // Save player stats
    saveData["playerStats"] = {
        {"strength", gameContext->playerStats.strength},
        {"dexterity", gameContext->playerStats.dexterity},
        {"constitution", gameContext->playerStats.constitution},
        {"intelligence", gameContext->playerStats.intelligence},
        {"wisdom", gameContext->playerStats.wisdom},
        {"charisma", gameContext->playerStats.charisma},
        {"factionReputation", gameContext->playerStats.factionReputation},
        {"skills", gameContext->playerStats.skills}
    };
    
    // Save individual systems
    json systemsData;
    for (const auto& [name, plugin] : plugins) {
        systemsData[name] = plugin->saveState();
    }
    saveData["systems"] = systemsData;
    
    return saveData;
}

bool GameSystemManager::loadAllSystems(const json& data) {
    try {
        // Load global game context
        if (data.contains("worldState")) {
            gameContext->worldState.daysPassed = data["worldState"]["daysPassed"];
            gameContext->worldState.currentSeason = data["worldState"]["currentSeason"];
            gameContext->worldState.locationStates = data["worldState"]["locationStates"].get<std::map<std::string, std::string>>();
            gameContext->worldState.factionStates = data["worldState"]["factionStates"].get<std::map<std::string, std::string>>();
            gameContext->worldState.worldFlags = data["worldState"]["worldFlags"].get<std::map<std::string, bool>>();
        }
        
        // Load player stats
        if (data.contains("playerStats")) {
            gameContext->playerStats.strength = data["playerStats"]["strength"];
            gameContext->playerStats.dexterity = data["playerStats"]["dexterity"];
            gameContext->playerStats.constitution = data["playerStats"]["constitution"];
            gameContext->playerStats.intelligence = data["playerStats"]["intelligence"];
            gameContext->playerStats.wisdom = data["playerStats"]["wisdom"];
            gameContext->playerStats.charisma = data["playerStats"]["charisma"];
            gameContext->playerStats.factionReputation = data["playerStats"]["factionReputation"].get<std::map<std::string, int>>();
            gameContext->playerStats.skills = data["playerStats"]["skills"].get<std::map<std::string, int>>();
        }
        
        // Load individual systems
        if (data.contains("systems")) {
            for (auto& [name, plugin] : plugins) {
                if (data["systems"].contains(name)) {
                    if (!plugin->loadState(data["systems"][name])) {
                        std::cerr << "Error loading state for system: " << name << std::endl;
                    }
                }
            }
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading systems: " << e.what() << std::endl;
        return false;
    }
}

bool GameSystemManager::dispatchEvent(const std::string& eventType, const json& eventData) {
    bool handled = false;
    
    // Send event to all plugins
    for (auto& [name, plugin] : plugins) {
        if (plugin->handleEvent(eventType, eventData)) {
            handled = true;
        }
    }
    
    return handled;
}
```

This implementation:

1. **Manages plugin lifecycle**: Handles registration, initialization, and shutdown of all plugins
2. **Updates all systems**: Calls each plugin's update method during the game loop
3. **Handles saving/loading**: Coordinates saving and loading of all plugin state plus global state
4. **Provides event system**: Allows plugins to communicate using the event system
5. **Provides access to plugins**: Lets the game access specific systems when needed

With this architecture, the main game can focus on high-level game flow while delegating specific game mechanics to the appropriate plugin systems. Each plugin handles its own internal state and logic but can also interact with other systems when needed through the event system or direct access.