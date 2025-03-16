### Main Integration

**Updated main.cpp:**

```cpp
#include "systems/crime/CrimeLawSystem.hpp"
#include "core/TAController.hpp"
#include <iostream>

// Other includes for your game systems...

int main() {
    try {
        // Initialize main controller
        TAController controller;
        
        // Initialize existing systems
        // ... other system initializations ...
        
        // Initialize the Crime & Law System
        CrimeLawSystem crimeSystem(&controller);
        
        // Register all systems with the controller (if not done in constructors)
        // ...
        
        std::cout << "All systems initialized successfully." << std::endl;
        
        // Start the game loop
        while (true) {
            // Process player input
            std::string input;
            std::cout << "> ";
            std::getline(std::cin, input);
            
            if (input == "quit" || input == "exit") {
                break;
            }
            
            // Parse input into TAInput structure
            TAInput gameInput = parseInput(input);  // You'd have your own input parser
            
            // Process the input through the controller
            controller.processInput(gameInput.type, gameInput);
            
            // Update game state
            // ...
        }
        
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
```

### Integrating with other Game Systems

**1. Integration with the World System**

In the `LocationNode.cpp` or similar file, add code to check for wanted status when the player enters a new location:

```cpp
// In LocationNode.cpp or similar

#include "systems/crime/CrimeLawContext.hpp"

// Inside the onEnter method or location transition logic
void LocationNode::onEnter(GameContext* context) {
    // Existing location entry code...
    
    // Get current region and check wanted status
    std::string region = // get region from the location
    
    // Get the crime law context (you might need to design a way to access this)
    CrimeLawContext* lawContext = getLawContext(); // You'd need a function to access this
    
    // Check if player is wanted
    if (lawContext && lawContext->criminalRecord.isWanted(region)) {
        // Determine if guards are present in this location
        bool guardsPresent = (location.type == "town" || location.type == "city");
        
        if (guardsPresent) {
            // Random chance to encounter guards based on bounty amount
            int bounty = lawContext->criminalRecord.getBounty(region);
            int encounterChance = std::min(80, 20 + (bounty / 100));
            
            if (rand() % 100 < encounterChance) {
                // Transition to guard encounter
                controller->transitionToNode("GuardEncounter");
                return;
            }
        }
    }
    
    // Continue with normal location entry
    // ...
}
```

**2. Integration with NPC Dialogue System**

Modify NPCs to react to the player's criminal status:

```cpp
// In DialogueNode.cpp or similar

#include "systems/crime/CrimeLawContext.hpp"

// When selecting dialogue options
void NPCDialogue::getDialogueOptions(GameContext* context) {
    std::vector<DialogueOption> options;
    
    // Get crime context
    CrimeLawContext* lawContext = getLawContext(); // You'd need a function to access this
    std::string region = context->worldState.getFactionState("current_region");
    
    // Check criminal reputation and adjust dialogue
    if (lawContext) {
        int reputation = lawContext->criminalRecord.getReputation(region);
        
        if (reputation < -50) {
            // Criminal dialogue options
            options.push_back({"I don't speak with criminals like you!", "feared_response"});
        }
        else if (reputation < -20) {
            // Suspicious dialogue
            options.push_back({"I've heard things about you...", "suspicious_response"});
        }
        else {
            // Normal dialogue options
            options.push_back({"Hello there, traveler!", "normal_greeting"});
        }
        
        // If NPC is a guard and player is wanted
        if (npcType == "guard" && lawContext->criminalRecord.isWanted(region)) {
            options.push_back({"Wait, aren't you a wanted criminal?", "guard_recognition"});
            
            // Add option to transition to guard encounter
            // This could trigger the guard encounter node
        }
    }
    
    return options;
}
```

**3. Integration with the Player Progress System**

Update player skills based on criminal actions:

```cpp
// Add to CrimeSystemNode.cpp or wherever you handle crime execution

void updatePlayerSkills(GameContext* context, const std::string& crimeType, bool success) {
    if (!context) return;
    
    // Different crimes improve different skills
    if (success) {
        if (crimeType == CrimeType::THEFT() || crimeType == CrimeType::PICKPOCKETING()) {
            // Chance to improve theft-related skills
            if (rand() % 100 < 30) {
                context->playerStats.improveSkill("pickpocket", 1);
                std::cout << "Your pickpocketing skill has improved!" << std::endl;
            }
        }
        else if (crimeType == CrimeType::TRESPASSING()) {
            // Chance to improve stealth
            if (rand() % 100 < 25) {
                context->playerStats.improveSkill("stealth", 1);
                std::cout << "Your stealth skill has improved!" << std::endl;
            }
        }
    }
    else {
        // Failed crimes give smaller chance for improvement
        if (rand() % 100 < 10) {
            if (crimeType == CrimeType::THEFT() || crimeType == CrimeType::PICKPOCKETING()) {
                context->playerStats.improveSkill("pickpocket", 1);
                std::cout << "Even failure teaches lessons. Your pickpocketing skill has improved slightly!" << std::endl;
            }
        }
    }
}
```

**4. Global Crime System Access**

Create a singleton or other global access pattern to get the crime law context from anywhere.

```cpp
// Create a global accessor in CrimeLawSystem.hpp

class CrimeLawSystem {
public:
    // ... existing code ...
    
    // Static accessor
    static CrimeLawSystem* getInstance() {
        return instance;
    }
    
    CrimeLawContext* getLawContext() {
        return getLawContextFromController();
    }

private:
    // ... existing code ...
    
    // Static instance
    static CrimeLawSystem* instance;
};

// Then in CrimeLawSystem.cpp
CrimeLawSystem* CrimeLawSystem::instance = nullptr;

CrimeLawSystem::CrimeLawSystem(TAController* controller)
    : controller(controller)
{
    instance = this;
    // ... rest of constructor ...
}

// Now you can access from anywhere with:
// CrimeLawContext* lawContext = CrimeLawSystem::getInstance()->getLawContext();
```