### Update main.cpp:

```cpp
// /oath/main.cpp
#include "core/TAController.hpp"
#include "systems/health/HealthSetup.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    std::cout << "Starting Oath RPG...\n";
    
    // Initialize controller
    TAController controller;
    
    // Initialize existing systems
    // setupDialogueSystem(controller);
    // setupCraftingSystem(controller);
    // setupWorldSystem(controller);
    // etc...
    
    // Initialize the disease and health system
    setupDiseaseHealthSystem(controller);
    
    // Register commands to access health system
    controller.registerCommand("health", [&controller]() {
        controller.navigateToSystem("HealthSystem");
    });
    
    // For demo/testing
    // Uncomment to run a demo of the health system
    // exampleDiseaseSystemUsage(controller);
    
    // Main game loop
    controller.run();
    
    return 0;
}
```

### Integration with Player/NPC:

```cpp
// In Player/NPC.hpp
#include "systems/health/HealthState.hpp"

class Player/NPC {
public:
    // Existing methods and properties
    
    // Health-related methods
    void updateHealth(GameContext* context);
    bool isIll() const;
    void checkExposure(const std::string& regionName, const std::string& vector);
    
    // Delegated to health system
    void takeDamage(float amount);
    void heal(float amount);
    void useStamina(float amount);
    void restoreStamina(float amount);
};

// In Player/NPC.cpp
void Player/NPC::updateHealth(GameContext* context) {
    if (context) {
        // Update diseases based on days passed
        context->diseaseManager.updateDiseases(context, context->worldState.daysPassed);
    }
}

bool Player/NPC::isIll() const {
    // Check game context for active diseases
    GameContext* context = getCurrentContext(); // You'd need your own method to get context
    return !context->healthContext.playerHealth.activeDiseaseDays.empty();
}

void Player/NPC::checkExposure(const std::string& regionName, const std::string& vector) {
    GameContext* context = getCurrentContext();
    if (context) {
        context->diseaseManager.checkExposure(context, regionName, vector);
    }
}

void Player/NPC::takeDamage(float amount) {
    GameContext* context = getCurrentContext();
    if (context) {
        context->healthContext.playerHealth.takeDamage(amount);
    }
}

// Implement other health methods similarly
```

### Integration with LocationNode:

```cpp
// In LocationNode.cpp
void LocationNode::onEnter(GameContext* context) {
    TANode::onEnter(context);
    
    // Existing location logic
    
    // Check for disease exposure based on location
    std::string regionName = getRegion();
    
    // Exposure through air (default for all locations)
    context->diseaseManager.checkExposure(context, regionName, "air");
    
    // Special conditions - water sources
    if (hasWaterSource) {
        context->diseaseManager.checkExposure(context, regionName, "water");
    }
    
    // Crowded places increase contact exposure
    if (isCrowded) {
        context->diseaseManager.checkExposure(context, regionName, "contact");
    }
}
```

### Integration with Time System:

```cpp
// In TimeNode.cpp or wherever you handle time progression
void advanceTime(int hours, GameContext* context) {
    // Existing time logic
    
    // Update diseases when days change
    int days = hours / 24;
    if (days > 0) {
        for (int i = 0; i < days; i++) {
            context->worldState.advanceDay();
            context->diseaseManager.updateDiseases(context, context->worldState.daysPassed);
        }
    }
}
```

### UI Integration:

```cpp
// In UI class or function
void displayStatusBar(GameContext* context) {
    const HealthState& health = context->healthContext.playerHealth;
    
    std::cout << "HP: " << health.currentHealth << "/" << health.maxHealth;
    std::cout << " | Stamina: " << health.stamina << "/" << health.maxStamina;
    
    // Show active diseases
    if (!health.activeDiseaseDays.empty()) {
        std::cout << " | Diseased: ";
        for (const auto& [diseaseId, days] : health.activeDiseaseDays) {
            const Disease* disease = context->diseaseManager.getDiseaseById(diseaseId);
            std::string name = disease ? disease->name : diseaseId;
            std::cout << name << " ";
        }
    }
    
    std::cout << std::endl;
}
```

### Epidemic Events Integration:

```cpp
// In your event system or quest system
void checkWorldEvents(GameContext* context) {
    // Random chance for epidemic in a region
    float roll = static_cast<float>(rand()) / RAND_MAX;
    if (roll < 0.05f) { // 5% chance per check
        // Pick a random region
        std::vector<std::string> regions = {"Village", "City", "Mountain", "Forest"};
        std::string region = regions[rand() % regions.size()];
        
        // Pick a disease common in that region
        std::vector<const Disease*> diseases = context->diseaseManager.getDiseasesInRegion(region);
        if (!diseases.empty()) {
            const Disease* disease = diseases[rand() % diseases.size()];
            
            // Create epidemic node and navigate to it
            EpidemicNode* epidemicNode = controller.createNode<EpidemicNode>(
                "Epidemic_" + disease->id, 
                disease->id, 
                region, 
                1.5f + (static_cast<float>(rand()) / RAND_MAX) // Random severity 1.5-2.5
            );
            
            // Set up transitions and navigate
            // This would depend on your specific navigation system
            controller.navigateToNode(epidemicNode);
        }
    }
}
```

### Inventory Integration:

```cpp
// In Item.hpp, add a healing method field
class HealingItem : public Item {
public:
    float healAmount;
    std::map<std::string, float> diseaseEffectiveness;
    
    void use(GameContext* context) override {
        // Heal player
        context->healthContext.playerHealth.heal(healAmount);
        
        // Check for disease healing
        for (const auto& [diseaseId, effectiveness] : diseaseEffectiveness) {
            if (context->healthContext.playerHealth.hasDisease(diseaseId)) {
                float roll = static_cast<float>(rand()) / RAND_MAX;
                if (roll < effectiveness) {
                    context->healthContext.playerHealth.recoverFromDisease(
                        diseaseId, 
                        context->worldState.daysPassed, 
                        true
                    );
                    std::cout << "The " << name << " has cured your " << diseaseId << "!" << std::endl;
                } else {
                    std::cout << "The " << name << " provided some relief, but didn't cure your " 
                              << diseaseId << "." << std::endl;
                }
            }
        }
        
        // Remove from inventory
        context->playerInventory.removeItem(id, 1);
    }
};
```

### Quest Integration:

```cpp
// Create quests to help with epidemics or find cures
void createEpidemicQuest(const std::string& diseaseId, const std::string& regionName, GameContext* context) {
    const Disease* disease = context->diseaseManager.getDiseaseById(diseaseId);
    if (!disease) return;
    
    Quest quest;
    quest.id = "epidemic_" + diseaseId;
    quest.name = "Contain the " + disease->name + " Outbreak";
    quest.description = "The " + regionName + " region is suffering from a " + disease->name + 
                        " outbreak. Help contain the spread and treat the afflicted.";
    
    // Quest objectives
    quest.objectives.push_back({
        "gather_herbs", 
        "Gather medicinal herbs", 
        10
    });
    
    quest.objectives.push_back({
        "treat_villagers", 
        "Treat infected villagers", 
        5
    });
    
    // Quest rewards
    quest.rewards.push_back({
        "gold", 
        "Gold", 
        200
    });
    
    quest.rewards.push_back({
        "immunity_potion", 
        "Immunity Potion", 
        1
    });
    
    // Add to quest system
    context->questSystem.addQuest(quest);
}
```

### Custom Commands:

```cpp
// In your command handler setup
void setupHealthCommands(TAController& controller) {
    controller.registerCommand("health", [&controller]() {
        controller.navigateToSystem("HealthSystem");
    });
    
    controller.registerCommand("rest", [&controller](const std::vector<std::string>& args) {
        int hours = 8; // Default rest time
        if (!args.empty()) {
            try {
                hours = std::stoi(args[0]);
            } catch (...) {
                std::cout << "Invalid rest time. Using default 8 hours." << std::endl;
            }
        }
        
        // Find and use rest node
        RestNode* restNode = dynamic_cast<RestNode*>(controller.getNode("RestNode"));
        if (restNode) {
            restNode->applyRest(hours, &controller.gameContext);
        }
    });
    
    controller.registerCommand("diseases", [&controller]() {
        // List all known diseases
        std::cout << "Known Diseases:" << std::endl;
        for (const auto& [id, disease] : controller.gameContext.diseaseManager.diseases) {
            std::cout << "- " << disease.name << ": " << disease.description << std::endl;
        }
    });
}
```