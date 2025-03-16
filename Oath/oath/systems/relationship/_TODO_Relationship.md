## Update main.cpp

```cpp
#include "core/TAController.hpp"
#include "data/GameContext.hpp"
#include "systems/relationship/RelationshipSystemController.hpp"
#include <iostream>
#include <memory>

int main(int argc, char* argv[]) {
    // Create the main game controller
    TAController gameController;
    
    // Create game context
    GameContext gameContext;
    
    // Initialize the relationship system
    std::unique_ptr<RelationshipSystemController> relationshipSystem = 
        std::make_unique<RelationshipSystemController>(&gameController);
    
    // Initial time setup (day 1, 8:00)
    int currentDay = 1;
    int currentHour = 8;
    
    std::cout << "Welcome to Oath Game!\n";
    
    // Get NPCs at current location and time
    std::vector<std::string> npcsPresent = relationshipSystem->getRelationshipManager()->
        getNPCsAtLocation("Village Center", currentDay, currentHour);
    
    // Display available NPCs to interact with
    std::cout << "NPCs present at Village Center:\n";
    for (const auto& npcId : npcsPresent) {
        RelationshipNPC* npc = relationshipSystem->getRelationshipManager()->getNPC(npcId);
        if (npc) {
            std::string relationshipDesc = relationshipSystem->getRelationshipManager()->
                getRelationshipDescription(npcId);
            std::cout << "- " << npc->name << " (" << npc->occupation << ") - " 
                      << relationshipDesc << std::endl;
        }
    }
    
    // Main game loop would go here
    bool running = true;
    while (running) {
        // Process player input
        std::string command;
        std::cout << "\nWhat would you like to do? ";
        std::getline(std::cin, command);
        
        if (command == "quit" || command == "exit") {
            running = false;
        } 
        else if (command == "relationships") {
            // Enter the relationship system
            gameController.processInput("RelationshipSystem", {});
        }
        else if (command == "advance_time") {
            // Advance game time by an hour
            currentHour++;
            if (currentHour >= 24) {
                currentHour = 0;
                currentDay++;
            }
            
            // Update the relationship system with new time
            relationshipSystem->updateTimeOfDay(currentDay, currentHour);
            
            std::cout << "Time advanced to Day " << currentDay << ", " << currentHour << ":00\n";
            
            // Update NPCs at current location
            npcsPresent = relationshipSystem->getRelationshipManager()->
                getNPCsAtLocation("Village Center", currentDay, currentHour);
            
            std::cout << "NPCs now present at Village Center:\n";
            for (const auto& npcId : npcsPresent) {
                RelationshipNPC* npc = relationshipSystem->getRelationshipManager()->getNPC(npcId);
                if (npc) {
                    std::cout << "- " << npc->name << " (" << npc->occupation << ")\n";
                }
            }
        }
        else if (command == "save") {
            // Save relationship data
            if (relationshipSystem->saveRelationshipSystem("resources/saves/relationships_save.json")) {
                std::cout << "Relationships saved successfully.\n";
            } else {
                std::cout << "Failed to save relationships.\n";
            }
        }
        else if (command == "load") {
            // Load relationship data
            if (relationshipSystem->loadRelationshipSystem("resources/saves/relationships_save.json")) {
                std::cout << "Relationships loaded successfully.\n";
            } else {
                std::cout << "Failed to load relationships.\n";
            }
        }
        else {
            std::cout << "Unknown command. Try 'relationships', 'advance_time', 'save', 'load', or 'quit'.\n";
        }
    }
    
    std::cout << "Thanks for playing!\n";
    return 0;
}
```

## Additional Integration Points

### In LocationNode.cpp:
```cpp
#include "systems/relationship/RelationshipSystemController.hpp"

// When a player enters a location, update available NPCs
void LocationNode::onEnter(GameContext* context) {
    TANode::onEnter(context);
    
    // Get the relationship system from context
    auto* relationshipSystem = context->getSystem<RelationshipSystemController>("relationship");
    if (relationshipSystem) {
        // Get NPCs at this location at the current time
        int currentDay = context->getTimeSystem()->getCurrentDay();
        int currentHour = context->getTimeSystem()->getCurrentHour();
        
        std::vector<std::string> npcsPresent = relationshipSystem->getRelationshipManager()->
            getNPCsAtLocation(locationName, currentDay, currentHour);
            
        // Update available NPCs in this location
        availableNPCs = npcsPresent;
    }
}
```

### In QuestNode.cpp:
```cpp
#include "systems/relationship/RelationshipSystemController.hpp"

// When a quest is completed for an NPC
void QuestNode::completeQuest(GameContext* context, const std::string& npcId) {
    // Reward the player
    // ...

    // Improve relationship with the quest giver
    auto* relationshipSystem = context->getSystem<RelationshipSystemController>("relationship");
    if (relationshipSystem && !npcId.empty()) {
        // Importance level based on quest difficulty (1-10 scale)
        int questImportance = getQuestDifficulty();
        relationshipSystem->getRelationshipManager()->handleTaskCompletion(npcId, questImportance);
        
        // Inform the player
        RelationshipNPC* npc = relationshipSystem->getRelationshipManager()->getNPC(npcId);
        if (npc) {
            std::cout << npc->name << " is grateful for your help!" << std::endl;
        }
    }
}
```

### In DialogueNode.cpp:
```cpp
#include "systems/relationship/RelationshipSystemController.hpp"

// When dialogue with an NPC affects relationship
void DialogueNode::selectOption(GameContext* context, int optionIndex) {
    DialogueOption& option = options[optionIndex];
    
    // Process dialogue option
    // ...
    
    // If this option affects relationship
    if (option.affectsRelationship && !currentNpcId.empty()) {
        auto* relationshipSystem = context->getSystem<RelationshipSystemController>("relationship");
        if (relationshipSystem) {
            // Handle conversation with positive/negative outcome
            relationshipSystem->getRelationshipManager()->handleConversation(
                currentNpcId, option.topic, option.isPositive);
        }
    }
}
```

### In TimeNode.cpp:
```cpp
#include "systems/relationship/RelationshipSystemController.hpp"

// When game time advances
void TimeNode::advanceTime(GameContext* context, int hours) {
    // Update internal time
    currentHour += hours;
    while (currentHour >= 24) {
        currentHour -= 24;
        currentDay++;
    }
    
    // Update relationship system
    auto* relationshipSystem = context->getSystem<RelationshipSystemController>("relationship");
    if (relationshipSystem) {
        relationshipSystem->updateTimeOfDay(currentDay, currentHour);
    }
    
    // Check for special events
    if (newDay) {
        if (relationshipSystem) {
            relationshipSystem->processSpecialEvents(currentDay);
        }
    }
}
```