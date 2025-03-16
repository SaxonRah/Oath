### Update main.cpp

```cpp
#include "core/TAController.hpp"
#include "systems/world/LocationNode.hpp"
#include "systems/dialogue/DialogueNode.hpp"
#include "systems/quest/QuestNode.hpp"
#include "systems/mount/MountSystem.hpp"  // Include the mount system
#include "utils/JSONLoader.hpp"
#include <iostream>

int main() {
    std::cout << "Starting OATH RPG Engine..." << std::endl;

    // Initialize main controller
    TAController gameController;
    
    // Set up game systems
    JSONLoader loader("resources/json/world.json");
    setupWorldSystem(gameController, loader);
    setupQuestSystem(gameController, "resources/json/quests.json");
    setupDialogueSystem(gameController, "resources/json/npcs.json");
    
    // Set up mount system
    setupMountSystem(gameController, "resources/json/mount.json");
    
    // Create a sample json configuration file for the mount system
    createSampleMountConfig();
    
    // Set starting node
    gameController.setCurrentNode("StartLocation");
    
    // Start the game loop
    gameController.run();
    
    return 0;
}

// Helper function to create a sample mount configuration file
void createSampleMountConfig() {
    // Check if file already exists
    if (std::filesystem::exists("resources/json/mount.json")) {
        return;
    }
    
    std::cout << "Creating sample mount system configuration..." << std::endl;
    
    nlohmann::json config;
    
    // Add basic breeds
    config["breeds"]["standard_horse"] = {
        {"id", "standard_horse"},
        {"name", "Standard Horse"},
        {"description", "A reliable all-around mount suitable for beginners."},
        {"baseSpeed", 100},
        {"baseStamina", 100},
        {"baseCarryCapacity", 50},
        {"baseTrainability", 50}
    };
    
    config["breeds"]["war_horse"] = {
        {"id", "war_horse"},
        {"name", "War Horse"},
        {"description", "A strong, courageous mount bred for battle."},
        {"baseSpeed", 90},
        {"baseStamina", 120},
        {"baseCarryCapacity", 70},
        {"baseTrainability", 60},
        {"naturalJumper", true}
    };
    
    config["breeds"]["swift_steed"] = {
        {"id", "swift_steed"},
        {"name", "Swift Steed"},
        {"description", "A fast racing horse with excellent endurance."},
        {"baseSpeed", 130},
        {"baseStamina", 110},
        {"baseCarryCapacity", 40},
        {"baseTrainability", 70}
    };
    
    // Add special abilities
    config["specialAbilities"]["jump"] = {
        {"id", "jump"},
        {"name", "Jump"},
        {"description", "Jump over obstacles"},
        {"staminaCost", 25},
        {"skillRequired", 30},
        {"trainingType", "agility"},
        {"unlockThreshold", 50}
    };
    
    config["specialAbilities"]["swim"] = {
        {"id", "swim"},
        {"name", "Swim"},
        {"description", "Swim across water"},
        {"staminaCost", 15},
        {"skillRequired", 30},
        {"trainingType", "endurance"},
        {"unlockThreshold", 60}
    };
    
    config["specialAbilities"]["climb"] = {
        {"id", "climb"},
        {"name", "Climb"},
        {"description", "Climb steep slopes"},
        {"staminaCost", 30},
        {"skillRequired", 40},
        {"trainingType", "agility"},
        {"unlockThreshold", 70}
    };
    
    // Add training types
    config["trainingTypes"] = {
        {{"id", "combat"}, {"description", "Fighting from mountback and defensive maneuvers"}},
        {{"id", "endurance"}, {"description", "Long-distance travel and stamina management"}},
        {{"id", "agility"}, {"description", "Jumping, balance, and difficult terrain navigation"}},
        {{"id", "racing"}, {"description", "Burst speed and racing techniques"}}
    };
    
    // Add mount colors
    config["colors"] = {"Bay", "Chestnut", "Black", "Gray", "White", "Dapple", "Palomino", "Roan"};
    
    // Add a stable
    config["stables"]["riverdale_stable"] = {
        {"id", "riverdale_stable"},
        {"name", "Riverdale Stables"},
        {"location", "Riverdale"},
        {"capacity", 8},
        {"dailyFeedCost", 5},
        {"dailyCareCost", 3},
        {"availableMounts", {
            {
                {"templateId", "standard_horse"},
                {"name", "Shadow"},
                {"age", 48}
            },
            {
                {"templateId", "war_horse"},
                {"name", "Thunder"},
                {"age", 60}
            }
        }}
    };
    
    // Create the json file
    std::ofstream file("resources/json/mount.json");
    file << std::setw(4) << config << std::endl;
    std::cout << "Sample mount configuration created." << std::endl;
}
```

### Connect Mount System to World System
```cpp
// In your LocationNode.cpp or wherever travel is handled
bool LocationNode::travel(const std::string& destination, GameContext* context) {
    // Find destination in connected locations
    auto destIt = std::find_if(connections.begin(), connections.end(),
        [&destination](const LocationConnection& conn) {
            return conn.targetLocation == destination;
        });
    
    if (destIt == connections.end())
        return false;
    
    float distance = destIt->distance;
    float travelTime = distance;
    
    // Check if player is mounted and adjust travel time
    MountSystemController* mountSystem = 
        dynamic_cast<MountSystemController*>(context->controller->getSystemRoot("MountSystem"));
    
    if (mountSystem && mountSystem->activeMount && mountSystem->activeMount->isMounted) {
        // Apply mount speed modifier
        float speedModifier = mountSystem->getMountedSpeedModifier();
        travelTime *= speedModifier;
        
        // Apply travel effects to the mount
        mountSystem->applyTravelEffects(distance);
        
        // Check for special terrain handling
        if (destIt->terrainType == "river" && mountSystem->canPerformSpecialMovement("swim")) {
            std::cout << "Your mount swims across the river!" << std::endl;
        }
        else if (destIt->terrainType == "cliff" && mountSystem->canPerformSpecialMovement("climb")) {
            std::cout << "Your mount carefully climbs the steep path!" << std::endl;
        }
        else if (destIt->terrainType == "ravine" && mountSystem->canPerformSpecialMovement("jump")) {
            std::cout << "Your mount leaps across the ravine!" << std::endl;
        }
    }
    
    // Apply travel time to world clock
    if (context->worldState) {
        context->worldState->advanceTime(travelTime);
    }
    
    // Update mount states based on travel time (in minutes)
    if (mountSystem) {
        mountSystem->updateMounts(travelTime * 60);
    }
    
    return true;
}
```

### UI Integration

```cpp
// In your UI manager class
void UIManager::updateMountStatusDisplay() {
    MountSystemController* mountSystem = 
        dynamic_cast<MountSystemController*>(gameController.getSystemRoot("MountSystem"));
    
    if (mountSystem && mountSystem->activeMount) {
        // Update mount status UI elements
        mountStatusPanel.setVisible(true);
        mountNameLabel.setText(mountSystem->activeMount->name);
        mountHealthBar.setValue(mountSystem->activeMount->stats.health);
        mountStaminaBar.setValue(mountSystem->activeMount->stats.stamina);
        mountHungerBar.setValue(mountSystem->activeMount->stats.hunger);
        mountStateLabel.setText(mountSystem->activeMount->getStateDescription());
    } else {
        mountStatusPanel.setVisible(false);
    }
}
```