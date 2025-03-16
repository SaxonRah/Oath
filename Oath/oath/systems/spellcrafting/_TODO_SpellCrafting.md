## main

```cpp
#include <iostream>
#include <memory>
#include <string>

#include "core/TAController.hpp"
#include "core/TANode.hpp"
#include "core/TAInput.hpp"
#include "data/GameContext.hpp"
#include "systems/world/LocationNode.hpp"
#include "systems/world/RegionNode.hpp"
#include "systems/dialogue/DialogueNode.hpp"
#include "systems/quest/QuestNode.hpp"
#include "systems/spellcrafting/SpellCraftingSystem.hpp"
#include "systems/spellcrafting/SpellCraftingNode.hpp"
#include "utils/JSONLoader.hpp"

int main(int argc, char* argv[]) {
    std::cout << "Initializing Oath RPG Engine..." << std::endl;
    
    // Create the main game controller
    TAController gameController;
    
    // Load configuration settings
    JSONLoader configLoader;
    if (!configLoader.loadFromFile("resources/config/settings.json")) {
        std::cerr << "Failed to load configuration! Using defaults." << std::endl;
    }
    
    // Create the world structure
    TANode* worldRoot = gameController.createNode("WorldRoot");
    RegionNode* startingRegion = dynamic_cast<RegionNode*>(
        gameController.createNode<RegionNode>("Silvervale"));
    
    // Add locations to the starting region
    LocationNode* townSquare = dynamic_cast<LocationNode*>(
        gameController.createNode<LocationNode>("TownSquare", "The central plaza of Silvervale"));
    LocationNode* magesGuild = dynamic_cast<LocationNode*>(
        gameController.createNode<LocationNode>("MagesGuild", "The headquarters of the Mages Guild"));
    LocationNode* blacksmith = dynamic_cast<LocationNode*>(
        gameController.createNode<LocationNode>("Blacksmith", "A busy forge with the sound of hammers"));
    
    // Set up the world hierarchy
    worldRoot->addChild(startingRegion);
    startingRegion->addChild(townSquare);
    startingRegion->addChild(magesGuild);
    startingRegion->addChild(blacksmith);
    
    // Set up transitions between locations
    townSquare->addTransition(
        [](const TAInput& input) {
            return input.type == "location_action" && 
                   std::get<std::string>(input.parameters.at("action")) == "go_to_mages_guild";
        },
        magesGuild, "Go to the Mages Guild");
        
    townSquare->addTransition(
        [](const TAInput& input) {
            return input.type == "location_action" && 
                   std::get<std::string>(input.parameters.at("action")) == "go_to_blacksmith";
        },
        blacksmith, "Go to the Blacksmith");
        
    magesGuild->addTransition(
        [](const TAInput& input) {
            return input.type == "location_action" && 
                   std::get<std::string>(input.parameters.at("action")) == "return_to_town";
        },
        townSquare, "Return to Town Square");
        
    blacksmith->addTransition(
        [](const TAInput& input) {
            return input.type == "location_action" && 
                   std::get<std::string>(input.parameters.at("action")) == "return_to_town";
        },
        townSquare, "Return to Town Square");
    
    // Initialize player stats
    gameController.gameContext.playerStats.name = "Apprentice Wizard";
    gameController.gameContext.playerStats.intelligence = 15;
    gameController.gameContext.playerStats.mana = 100;
    gameController.gameContext.playerStats.health = 50;
    gameController.gameContext.playerStats.improveSkill("destruction", 3);
    gameController.gameContext.playerStats.improveSkill("restoration", 2);
    gameController.gameContext.playerStats.improveSkill("alteration", 1);
    
    // Initialize inventory with basic items
    gameController.gameContext.inventory.addItem(
        Item("magic_dust", "Magic Dust", "A pinch of shimmering arcane powder", 5));
    gameController.gameContext.inventory.addItem(
        Item("quill", "Enchanted Quill", "A quill that never runs out of ink", 1));
        
    // Set up quest system
    QuestNode* introQuest = dynamic_cast<QuestNode*>(
        gameController.createNode<QuestNode>("IntroQuest", "The Apprentice's Trial"));
    worldRoot->addChild(introQuest);
    
    // Initialize dialogue system for an NPC
    DialogueNode* archmageDialogue = dynamic_cast<DialogueNode*>(
        gameController.createNode<DialogueNode>("ArchmageDialogue", "Archmage Elowen"));
    magesGuild->addChild(archmageDialogue);
    
    // Add transitions to dialogue
    magesGuild->addTransition(
        [](const TAInput& input) {
            return input.type == "location_action" && 
                   std::get<std::string>(input.parameters.at("action")) == "talk_to_archmage";
        },
        archmageDialogue, "Speak with Archmage Elowen");
    
    // Set up the spell crafting system
    setupSpellCraftingSystem(gameController, worldRoot);
    
    // Connect the archmage dialogue to the spell crafting system
    archmageDialogue->addTransition(
        [](const TAInput& input) {
            return input.type == "dialogue_action" && 
                   std::get<std::string>(input.parameters.at("action")) == "learn_spellcrafting";
        },
        findNodeByName(worldRoot, "SpellCrafting"), "Learn about spell crafting");
    
    // Add the integration of the spellcrafting knowledge with quests
    introQuest->addObjective("Learn a Fire Spell", [&gameController]() -> bool {
        // Check if player has learned any fire spells
        SpellCraftingNode* spellNode = dynamic_cast<SpellCraftingNode*>(
            findNodeByName(gameController.getSystemRoot("SpellSystem"), "SpellCrafting"));
        
        if (!spellNode) return false;
        
        for (const auto& spell : spellNode->knownSpells) {
            for (const auto& component : spell->components) {
                if (component->effectType == SpellEffectType::Damage) {
                    return true;
                }
            }
        }
        return false;
    });
    
    // Set the starting location
    gameController.setCurrentNode(townSquare);
    
    // Game loop
    bool running = true;
    while (running) {
        // Display current location
        std::cout << "\n===== " << gameController.currentNode->nodeName << " =====" << std::endl;
        
        // Enter the node (trigger any node-specific logic)
        gameController.currentNode->onEnter(&gameController.gameContext);
        
        // Display available actions
        std::vector<TAAction> actions = gameController.currentNode->getAvailableActions();
        std::cout << "\nAvailable Actions:" << std::endl;
        for (size_t i = 0; i < actions.size(); i++) {
            std::cout << (i + 1) << ". " << actions[i].description << std::endl;
        }
        
        // Add quit action
        std::cout << (actions.size() + 1) << ". Quit Game" << std::endl;
        
        // Get player input
        int choice;
        std::cout << "\nEnter your choice: ";
        std::cin >> choice;
        
        if (choice <= 0 || choice > static_cast<int>(actions.size() + 1)) {
            std::cout << "Invalid choice, please try again." << std::endl;
            continue;
        }
        
        if (choice == static_cast<int>(actions.size() + 1)) {
            running = false;
            continue;
        }
        
        // Process the chosen action
        TAInput input = actions[choice - 1].getInput();
        
        // Handle special cases for spell crafting
        if (input.type == "spellcraft_action") {
            std::string action = std::get<std::string>(input.parameters.at("action"));
            
            if (action == "start_new") {
                std::string spellName;
                std::cout << "Enter a name for your new spell: ";
                std::cin.ignore();
                std::getline(std::cin, spellName);
                
                SpellCraftingNode* spellNode = dynamic_cast<SpellCraftingNode*>(gameController.currentNode);
                spellNode->startNewSpell(spellName, &gameController.gameContext);
                continue;
            }
            else if (action == "add_component") {
                SpellCraftingNode* spellNode = dynamic_cast<SpellCraftingNode*>(gameController.currentNode);
                spellNode->listAvailableComponents();
                
                int componentChoice;
                std::cout << "Select a component number: ";
                std::cin >> componentChoice;
                
                spellNode->addComponent(componentChoice - 1, &gameController.gameContext);
                continue;
            }
            else if (action == "add_modifier") {
                SpellCraftingNode* spellNode = dynamic_cast<SpellCraftingNode*>(gameController.currentNode);
                spellNode->listAvailableModifiers();
                
                int modifierChoice;
                std::cout << "Select a modifier number: ";
                std::cin >> modifierChoice;
                
                spellNode->addModifier(modifierChoice - 1, &gameController.gameContext);
                continue;
            }
            else if (action == "set_delivery") {
                SpellCraftingNode* spellNode = dynamic_cast<SpellCraftingNode*>(gameController.currentNode);
                spellNode->listAvailableDeliveryMethods();
                
                int deliveryChoice;
                std::cout << "Select a delivery method number: ";
                std::cin >> deliveryChoice;
                
                spellNode->setDeliveryMethod(deliveryChoice - 1, &gameController.gameContext);
                continue;
            }
            else if (action == "set_target") {
                std::cout << "Select a target type:\n"
                          << "1. Single Target\n"
                          << "2. Multi Target\n"
                          << "3. Self\n"
                          << "4. Allies Only\n"
                          << "5. Enemies Only\n"
                          << "6. Area Effect\n";
                
                int targetChoice;
                std::cout << "Enter your choice: ";
                std::cin >> targetChoice;
                
                SpellTargetType targetType;
                switch (targetChoice) {
                    case 1: targetType = SpellTargetType::SingleTarget; break;
                    case 2: targetType = SpellTargetType::MultiTarget; break;
                    case 3: targetType = SpellTargetType::Self; break;
                    case 4: targetType = SpellTargetType::AlliesOnly; break;
                    case 5: targetType = SpellTargetType::EnemiesOnly; break;
                    case 6: targetType = SpellTargetType::AreaEffect; break;
                    default: targetType = SpellTargetType::SingleTarget; break;
                }
                
                SpellCraftingNode* spellNode = dynamic_cast<SpellCraftingNode*>(gameController.currentNode);
                spellNode->setTargetType(targetType, &gameController.gameContext);
                continue;
            }
            else if (action == "finalize") {
                SpellCraftingNode* spellNode = dynamic_cast<SpellCraftingNode*>(gameController.currentNode);
                spellNode->finalizeSpell(&gameController.gameContext);
                continue;
            }
            else if (action == "abandon") {
                SpellCraftingNode* spellNode = dynamic_cast<SpellCraftingNode*>(gameController.currentNode);
                spellNode->abandonSpell();
                continue;
            }
            else if (action == "cast_spell") {
                SpellCraftingNode* spellNode = dynamic_cast<SpellCraftingNode*>(gameController.currentNode);
                spellNode->listKnownSpells();
                
                int spellChoice;
                std::cout << "Select a spell to cast: ";
                std::cin >> spellChoice;
                
                spellNode->castSpell(spellChoice - 1, &gameController.gameContext);
                continue;
            }
            else if (action == "research") {
                std::cout << "Available schools for research:\n"
                          << "1. destruction\n"
                          << "2. restoration\n"
                          << "3. alteration\n"
                          << "4. conjuration\n"
                          << "5. illusion\n"
                          << "6. mysticism\n";
                
                int schoolChoice;
                std::cout << "Select a school to research: ";
                std::cin >> schoolChoice;
                
                std::string researchArea;
                switch (schoolChoice) {
                    case 1: researchArea = "destruction"; break;
                    case 2: researchArea = "restoration"; break;
                    case 3: researchArea = "alteration"; break;
                    case 4: researchArea = "conjuration"; break;
                    case 5: researchArea = "illusion"; break;
                    case 6: researchArea = "mysticism"; break;
                    default: researchArea = "destruction"; break;
                }
                
                int hours;
                std::cout << "How many hours to spend (1-8): ";
                std::cin >> hours;
                
                SpellCraftingNode* spellNode = dynamic_cast<SpellCraftingNode*>(gameController.currentNode);
                SpellResearchResult result = spellNode->conductResearch(researchArea, hours, &gameController.gameContext);
                
                // Update quest progress after research
                introQuest->checkObjectives();
                continue;
            }
        }
        
        // Process the transition
        gameController.processInput(input);
        
        // Check quest progress after movement
        introQuest->checkObjectives();
    }
    
    std::cout << "Thank you for playing Oath RPG!" << std::endl;
    return 0;
}
```

### Integration with Character Stats (CharacterStats.hpp)

```cpp
#pragma once

#include <map>
#include <string>
#include <vector>

class CharacterStats {
public:
    // Basic attributes
    std::string name;
    int level;
    int experience;
    int health;
    int maxHealth;
    int mana;           // Added for spellcasting
    int maxMana;        // Added for spellcasting
    int stamina;
    int maxStamina;
    
    // Primary attributes
    int strength;
    int dexterity;
    int constitution;
    int intelligence;   // Crucial for spellcasting
    int wisdom;
    int charisma;
    
    // Skills
    std::map<std::string, int> skills;
    
    // Special abilities
    std::vector<std::string> abilities;
    
    CharacterStats();
    
    // Improve a skill by a certain amount
    void improveSkill(const std::string& skillName, int amount);
    
    // Check if character has a skill at or above a certain level
    bool hasSkill(const std::string& skillName, int requiredLevel) const;
    
    // Check if character has a specific ability
    bool hasAbility(const std::string& abilityName) const;
    
    // Calculate spell success chance based on relevant skills and intelligence
    int calculateSpellSuccessChance(const std::string& school, int complexity) const;
    
    // Recover mana based on intelligence and wisdom
    void recoverMana(float restTime);
};
```

### Integration with Quest System

```cpp
#pragma once

#include "../core/TANode.hpp"
#include <functional>
#include <string>
#include <vector>

struct QuestObjective {
    std::string description;
    std::function<bool()> checkCompletion;
    bool completed;
    
    QuestObjective(const std::string& desc, std::function<bool()> checker)
        : description(desc), checkCompletion(checker), completed(false) {}
};

class QuestNode : public TANode {
private:
    std::string questDescription;
    std::vector<QuestObjective> objectives;
    bool completed;
    bool failed;
    std::string reward;
    
public:
    QuestNode(const std::string& name, const std::string& description);
    
    void onEnter(GameContext* context) override;
    
    // Add an objective with a completion checker
    void addObjective(const std::string& description, std::function<bool()> completionChecker);
    
    // Add a spellcasting-specific objective
    void addSpellCastingObjective(const std::string& description, 
                                  const std::string& spellEffectType,
                                  int minPower);
    
    // Check if all objectives are complete
    bool checkObjectives();
    
    // Complete the quest and give rewards
    void completeQuest(GameContext* context);
    
    // Fail the quest
    void failQuest();
    
    // Set the reward
    void setReward(const std::string& rewardDescription);
    
    // Get available actions
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;
};
```

### Inventory Integration

```cpp
#pragma once

#include "Item.hpp"
#include <map>
#include <string>
#include <vector>

// Enum for item types, including magical components
enum class ItemType {
    Weapon,
    Armor,
    Consumable,
    Quest,
    Material,
    MagicalIngredient,  // Added for spellcrafting
    SpellScroll         // Added for spellcrafting
};

class Inventory {
private:
    std::map<std::string, Item> items;
    int gold;
    int maxWeight;
    int currentWeight;
    
public:
    Inventory();
    
    // Basic inventory operations
    bool addItem(const Item& item);
    bool removeItem(const std::string& itemId, int count = 1);
    Item* getItem(const std::string& itemId);
    
    // Gold operations
    int getGold() const;
    void addGold(int amount);
    bool removeGold(int amount);
    
    // Weight operations
    int getCurrentWeight() const;
    int getMaxWeight() const;
    void setMaxWeight(int weight);
    
    // Get all items
    std::vector<Item> getAllItems() const;
    
    // Get items by type
    std::vector<Item> getItemsByType(ItemType type) const;
    
    // Get magical ingredients (for spellcrafting)
    std::vector<Item> getMagicalIngredients() const;
    
    // Get spell scrolls
    std::vector<Item> getSpellScrolls() const;
    
    // Check if has required ingredients for a spell
    bool hasIngredientsForSpell(const std::map<std::string, int>& requiredIngredients) const;
    
    // Consume ingredients used in spellcrafting
    void consumeSpellIngredients(const std::map<std::string, int>& ingredients);
};
```

### Item.hpp Extension for Magical Items

```cpp
#pragma once

#include <map>
#include <string>
#include <vector>

enum class ItemType;  // Forward declaration, defined in Inventory.hpp

class Item {
private:
    std::string id;
    std::string name;
    std::string description;
    ItemType type;
    int value;
    int weight;
    int count;
    std::map<std::string, int> stats;
    std::vector<std::string> effects;
    
    // Magical properties for spell components
    std::string magicalElement;
    int magicalPotency;
    
public:
    Item(const std::string& itemId, const std::string& itemName, 
         const std::string& itemDesc, int itemCount = 1);
    
    // Getters
    std::string getId() const;
    std::string getName() const;
    std::string getDescription() const;
    ItemType getType() const;
    int getValue() const;
    int getWeight() const;
    int getCount() const;
    
    // Setters
    void setType(ItemType newType);
    void setValue(int newValue);
    void setWeight(int newWeight);
    void setCount(int newCount);
    
    // Stats and effects
    void addStat(const std::string& statName, int value);
    int getStat(const std::string& statName) const;
    void addEffect(const std::string& effect);
    std::vector<std::string> getEffects() const;
    
    // Magical properties
    void setMagicalElement(const std::string& element);
    std::string getMagicalElement() const;
    void setMagicalPotency(int potency);
    int getMagicalPotency() const;
    
    // Create a magical ingredient specific for spellcrafting
    static Item createMagicalIngredient(const std::string& id, const std::string& name,
                                       const std::string& description, const std::string& element,
                                       int potency);
    
    // Create a spell scroll
    static Item createSpellScroll(const std::string& id, const std::string& spellName,
                                 const std::string& description, int power);
};
```
