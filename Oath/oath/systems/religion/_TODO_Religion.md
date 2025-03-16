
### Integrating with Character Stats and Progression

```cpp
// In CharacterStats.hpp, add:
#include "systems/religion/ReligiousStats.hpp"

class CharacterStats {
public:
    // Base stats
    int strength;
    int dexterity;
    int constitution;
    int intelligence;
    int wisdom;
    int charisma;
    
    // Derived stats
    int getEffectiveStrength(const ReligiousStats& religiousStats) const;
    int getEffectiveDexterity(const ReligiousStats& religiousStats) const;
    // etc.
    
    void improveSkill(const std::string& skillName, int amount);
    
    // Skills
    std::map<std::string, int> skills;
};
```

```cpp
// In CharacterStats.cpp:
int CharacterStats::getEffectiveStrength(const ReligiousStats& religiousStats) const 
{
    int effective = strength;
    
    // Apply religious blessing effects
    for (const auto& blessing : religiousStats.activeBlessing) {
        if (blessing == "blessing_of_might") effective += 2;
        else if (blessing == "divine_strength") effective += 5;
        // etc.
    }
    
    return effective;
}
```

### Integrating with Combat System

```cpp
// In CombatSystem.cpp:
void CombatSystem::calculateDamage(Entity* attacker, Entity* defender, Attack& attack) 
{
    // Base damage calculation
    int damage = calculateBaseDamage(attacker, attack);
    
    // Apply religious modifiers
    ReligiousStats* religiousStats = attacker->getComponent<ReligiousStats>();
    if (religiousStats) {
        // Add deity-specific damage bonuses
        if (religiousStats->primaryDeity == "war_god" && attack.type == "melee") {
            damage += 2;
        }
        
        // Apply blessing effects
        if (religiousStats->hasBlessingActive("divine_weapon")) {
            damage += 5;
            // Maybe add holy damage type
            attack.damageType = "holy";
        }
    }
    
    // Check for defender's religious protections
    religiousStats = defender->getComponent<ReligiousStats>();
    if (religiousStats) {
        if (religiousStats->hasBlessingActive("divine_protection")) {
            damage = static_cast<int>(damage * 0.8); // 20% damage reduction
        }
    }
    
    // Apply final damage
    defender->takeDamage(damage, attack.damageType);
}
```

### Integrating with NPC System

```cpp
// In DialogueNode.cpp:
void DialogueNode::onEnter(GameContext* baseContext) 
{
    // Get base dialogue
    std::string dialogueText = currentDialogue;
    
    // Modify based on religious factors if applicable
    ReligiousGameContext* religiousContext = dynamic_cast<ReligiousGameContext*>(baseContext);
    if (religiousContext && !npcDeityAffiliation.empty()) {
        int favor = religiousContext->religiousStats.deityFavor[npcDeityAffiliation];
        
        // Modify dialogue based on favor
        if (favor >= 50) {
            dialogueText += "\n\nThe priest recognizes your devotion to " + npcDeityAffiliation + 
                            " and speaks more openly.";
            // Could unlock special dialogue options
        } else if (favor <= -25) {
            dialogueText += "\n\nThe priest senses your disdain for " + npcDeityAffiliation + 
                            " and seems guarded.";
        }
    }
    
    std::cout << dialogueText << std::endl;
}
```

## Main.cpp Integration

```cpp
// File: main.cpp
#include "GameManager.hpp"
#include <iostream>
#include <string>
#include <filesystem>

void displayIntro() 
{
    std::cout << "            OATH OF THE GODS            " << std::endl;
}

void displayHelp() 
{
    std::cout << "\nAvailable commands:" << std::endl;
    std::cout << "  help        - Display this help text" << std::endl;
    std::cout << "  look        - Examine your surroundings" << std::endl;
    std::cout << "  go [dir]    - Move in a direction" << std::endl;
    std::cout << "  talk [npc]  - Speak with someone" << std::endl;
    std::cout << "  pray [deity]- Pray to a deity" << std::endl;
    std::cout << "  temple      - Visit a temple if one is nearby" << std::endl;
    std::cout << "  quit        - Exit the game" << std::endl;
    std::cout << "  save        - Save your progress" << std::endl;
    std::cout << "  load        - Load a saved game" << std::endl;
}

int main(int argc, char* argv[]) 
{
    // Check if resources exist
    if (!std::filesystem::exists("resources/json/religion.json")) {
        std::cerr << "Error: Required resource files not found!" << std::endl;
        std::cerr << "Make sure the game is being run from the correct directory." << std::endl;
        return 1;
    }
    
    // Display intro
    displayIntro();
    
    // Initialize game
    GameManager gameManager;
    try {
        gameManager.initialize();
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize game: " << e.what() << std::endl;
        return 1;
    }
    
    // Main game loop
    bool running = true;
    std::string command;
    
    displayHelp();
    std::cout << "\nYour journey begins..." << std::endl;
    
    while (running) {
        std::cout << "\n> ";
        std::getline(std::cin, command);
        
        if (command == "quit") {
            std::cout << "Are you sure you want to quit? (y/n): ";
            std::string confirm;
            std::getline(std::cin, confirm);
            if (confirm == "y" || confirm == "Y") {
                running = false;
            }
        } else if (command == "help") {
            displayHelp();
        } else if (command == "save") {
            std::cout << "Enter save name: ";
            std::string saveName;
            std::getline(std::cin, saveName);
            gameManager.saveGame("resources/saves/" + saveName + ".json");
            std::cout << "Game saved." << std::endl;
        } else if (command == "load") {
            std::cout << "Enter save name: ";
            std::string saveName;
            std::getline(std::cin, saveName);
            try {
                gameManager.loadGame("resources/saves/" + saveName + ".json");
                std::cout << "Game loaded." << std::endl;
            } catch (const std::exception& e) {
                std::cout << "Failed to load save: " << e.what() << std::endl;
            }
        } else {
            // Process game command
            gameManager.processCommand(command);
        }
        
        // Update game state
        gameManager.update();
    }
    
    std::cout << "Thank you for playing Oath of the Gods!" << std::endl;
    return 0;
}
```
