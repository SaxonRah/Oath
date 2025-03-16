#include "core/TAAction.hpp"
#include "core/TAController.hpp"
#include "core/TAInput.hpp"
#include "data/GameContext.hpp"
#include "data/Item.hpp"
#include "systems/crafting/CraftingNode.hpp"
#include "systems/dialogue/NPC.hpp"
#include "systems/progression/SkillNode.hpp"
#include "systems/quest/QuestNode.hpp"
#include "systems/world/TimeNode.hpp"
#include "utils/JSONLoader.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

int main()
{
    std::cout << "___ Starting Oath RPG Engine ___" << std::endl;

    // Create the automaton controller
    TAController controller;

    // Load all game data from JSON files
    std::cout << "___ LOADING GAME DATA FROM JSON FILES ___" << std::endl;
    if (!loadGameData(controller)) {
        std::cerr << "Failed to load game data. Exiting." << std::endl;
        return 1;
    }

    std::cout << "\n___ GAME DATA LOADED SUCCESSFULLY ___\n"
              << std::endl;

    // Initialize player inventory with some items
    controller.gameContext.playerInventory.addItem(
        Item("iron_ingot", "Iron Ingot", "material", 10, 5));
    controller.gameContext.playerInventory.addItem(
        Item("leather_strips", "Leather Strips", "material", 5, 10));
    controller.gameContext.playerInventory.addItem(
        Item("torch", "Torch", "tool", 2, 3));
    controller.gameContext.playerInventory.addItem(
        Item("red_herb", "Red Herb", "herb", 3, 5));
    controller.gameContext.playerInventory.addItem(
        Item("water_flask", "Water Flask", "container", 1, 2));

    // Set initial skills
    controller.gameContext.playerStats.improveSkill("combat", 2);
    controller.gameContext.playerStats.improveSkill("survival", 1);
    controller.gameContext.playerStats.improveSkill("crafting", 1);

    // Example: Start at village and talk to elder
    std::cout << "\n=== WORLD AND DIALOGUE EXAMPLE ===\n"
              << std::endl;

    // Start in village region
    controller.processInput("WorldSystem", {});

    // Travel to village center
    TAInput travelInput = {
        "region_action",
        { { "action", std::string("travel_location") }, { "location_index", 0 } }
    };
    controller.processInput("WorldSystem", travelInput);

    // Get the Elder Marius NPC from the game data
    NPC* elderMarius = controller.gameData["npcs"]["elder_marius"];

    // Talk to the village elder
    if (elderMarius) {
        std::cout << "\nTalking to Elder Marius...\n"
                  << std::endl;
        elderMarius->startDialogue(&controller.gameContext);

        // Example: Choose dialogue option 0 (Ask about threat)
        elderMarius->processResponse(0, &controller.gameContext);

        // Example: Choose dialogue option 0 (Ask how to help)
        elderMarius->processResponse(0, &controller.gameContext);

        // Example: Choose dialogue option 0 (Accept quest)
        elderMarius->processResponse(0, &controller.gameContext);

        // Example: Choose dialogue option 0 (I'll get started)
        elderMarius->processResponse(0, &controller.gameContext);
    }

    // Example: Try crafting
    std::cout << "\n=== CRAFTING EXAMPLE ===\n"
              << std::endl;

    // Access the crafting system
    controller.processInput("CraftingSystem", {});

    // Navigate to blacksmith station (first child)
    TAInput craftingInput = {
        "crafting_action",
        { { "action", std::string("select_station") }, { "index", 0 } }
    };
    controller.processInput("CraftingSystem", craftingInput);

    // Craft a sword (recipe index 0)
    TAInput craftSwordInput = {
        "crafting_action",
        { { "action", std::string("craft") }, { "recipe_index", 0 } }
    };
    controller.processInput("CraftingSystem", craftSwordInput);

    // Example: Skill progression
    std::cout << "\n=== SKILL PROGRESSION EXAMPLE ===\n"
              << std::endl;

    // Initialize progression system
    controller.processInput("ProgressionSystem", {});

    // Learn combat basics skill
    SkillNode* combatNode = dynamic_cast<SkillNode*>(
        controller.currentNodes["ProgressionSystem"]->childNodes[0]);
    if (combatNode && combatNode->canLearn(controller.gameContext)) {
        combatNode->learnSkill(&controller.gameContext);
    }

    // Find swordsmanship skill
    SkillNode* swordsmanship = nullptr;
    if (combatNode) {
        for (TANode* child : combatNode->childNodes) {
            if (child->nodeName == "Swordsmanship") {
                swordsmanship = dynamic_cast<SkillNode*>(child);
                break;
            }
        }
    }

    // Learn swordsmanship if available
    if (swordsmanship && swordsmanship->canLearn(controller.gameContext)) {
        swordsmanship->learnSkill(&controller.gameContext);
        std::cout << "Unlocked ability: power_attack" << std::endl;
    } else {
        std::cout << "Cannot learn swordsmanship yet - requirements not met" << std::endl;
    }

    // Example: Complete part of the main quest
    std::cout << "\n=== QUEST PROGRESSION EXAMPLE ===\n"
              << std::endl;

    // Initialize quest system with main quest
    controller.processInput("QuestSystem", {});

    // Find the repair walls subquest
    QuestNode* mainQuest = dynamic_cast<QuestNode*>(controller.systemRoots["QuestSystem"]);
    QuestNode* repairWalls = nullptr;

    for (TANode* child : mainQuest->childNodes) {
        if (child->nodeName == "RepairWalls") {
            repairWalls = dynamic_cast<QuestNode*>(child);
            break;
        }
    }

    // Access and complete the repair walls subquest
    if (repairWalls) {
        // Make this the current quest node
        controller.currentNodes["QuestSystem"] = repairWalls;
        repairWalls->onEnter(&controller.gameContext);

        // Complete the repair walls quest
        TAInput completeQuestInput = { "action",
            { { "name", std::string("repair_complete") } } };
        controller.processInput("QuestSystem", completeQuestInput);
    }

    // Track quest progress
    std::cout << "\nQuest journal:" << std::endl;
    for (const auto& [quest, status] : controller.gameContext.questJournal) {
        std::cout << "- " << quest << ": " << status << std::endl;
    }

    // Example: Time passage
    std::cout << "\n=== TIME SYSTEM EXAMPLE ===\n"
              << std::endl;

    // Initialize time system
    controller.processInput("TimeSystem", {});

    // Wait 5 hours
    TimeNode* timeSystem = dynamic_cast<TimeNode*>(controller.currentNodes["TimeSystem"]);
    if (timeSystem) {
        for (int i = 0; i < 5; i++) {
            timeSystem->advanceHour(&controller.gameContext);
        }
    }

    // Save/Load demonstration
    std::cout << "\n=== SAVE/LOAD SYSTEM EXAMPLE ===\n"
              << std::endl;

    // Initialize persistent IDs before saving
    controller.initializePersistentIDs();

    // Ensure save directory exists
    std::filesystem::path savePath = "resources/saves";
    if (!std::filesystem::exists(savePath)) {
        std::filesystem::create_directories(savePath);
    }

    // Save the game state
    std::string saveFilePath = savePath.string() + "/game_save.json";
    controller.saveState(saveFilePath);
    std::cout << "Game state saved to " << saveFilePath << std::endl;

    // Load the state from the saved file
    std::cout << "\nLoading saved game...\n"
              << std::endl;

    // Check if file exists first
    std::ifstream checkFile(saveFilePath);
    if (!checkFile.good()) {
        std::cout << "Save file doesn't exist or can't be opened." << std::endl;
    } else {
        checkFile.close();

        try {
            bool load_result = controller.loadState(saveFilePath);
            if (load_result) {
                std::cout << "Game state loaded successfully!" << std::endl;

                // Display the loaded state information
                std::cout << "\nLoaded game information:" << std::endl;

                // Display current time
                TimeNode* loadedTime = dynamic_cast<TimeNode*>(controller.currentNodes["TimeSystem"]);
                if (loadedTime) {
                    std::cout << "Time: Day " << loadedTime->day << ", " << loadedTime->hour
                              << ":00, " << loadedTime->timeOfDay << " ("
                              << loadedTime->season << ")" << std::endl;
                }

                // Display world state
                std::cout << "\nWorld state:" << std::endl;
                std::cout << "Days passed: " << controller.gameContext.worldState.daysPassed
                          << std::endl;
                std::cout << "Current season: "
                          << controller.gameContext.worldState.currentSeason << std::endl;

                // Display quest journal
                std::cout << "\nQuest journal:" << std::endl;
                for (const auto& [quest, status] : controller.gameContext.questJournal) {
                    std::cout << "- " << quest << ": " << status << std::endl;
                }

                // Display player stats
                std::cout << "\nPlayer stats:" << std::endl;
                std::cout << "Strength: " << controller.gameContext.playerStats.strength
                          << std::endl;
                std::cout << "Dexterity: " << controller.gameContext.playerStats.dexterity
                          << std::endl;
                std::cout << "Constitution: "
                          << controller.gameContext.playerStats.constitution << std::endl;
                std::cout << "Intelligence: "
                          << controller.gameContext.playerStats.intelligence << std::endl;
                std::cout << "Wisdom: " << controller.gameContext.playerStats.wisdom
                          << std::endl;
                std::cout << "Charisma: " << controller.gameContext.playerStats.charisma
                          << std::endl;

                // Display skills
                std::cout << "\nSkills:" << std::endl;
                for (const auto& [skill, level] :
                    controller.gameContext.playerStats.skills) {
                    std::cout << "- " << skill << ": " << level << std::endl;
                }

                // Display inventory
                std::cout << "\nInventory:" << std::endl;
                for (const auto& item : controller.gameContext.playerInventory.items) {
                    std::cout << "- " << item.name << " (" << item.quantity << ")"
                              << std::endl;
                }
            } else {
                std::cout << "Failed to load game state." << std::endl;
            }
        } catch (const std::exception& e) {
            std::cout << "Exception during load: " << e.what() << std::endl;
        } catch (...) {
            std::cout << "Unknown error during loading process." << std::endl;
        }
    }

    // Interactive mode option
    std::cout << "\n=== INTERACTIVE MODE ===\n"
              << std::endl;
    std::cout << "Enter 'i' to start interactive mode or any other key to exit: ";
    char choice;
    std::cin >> choice;

    if (choice == 'i' || choice == 'I') {
        // if (choice == "i" || choice == "I") {
        bool running = true;
        std::string command;

        while (running) {
            std::cout << "\nEnter command (h for help, q to quit): ";
            std::cin >> command;

            if (command == "q" || command == "quit") {
                running = false;
            } else if (command == "h" || command == "help") {
                std::cout << "Available commands:" << std::endl;
                std::cout << "  q, quit - Exit interactive mode" << std::endl;
                std::cout << "  h, help - Show this help menu" << std::endl;
                std::cout << "  s, status - Show character status" << std::endl;
                std::cout << "  i, inventory - Show inventory" << std::endl;
                std::cout << "  j, journal - Show quest journal" << std::endl;
                std::cout << "  t, time - Advance time by 1 hour" << std::endl;
                std::cout << "  save - Save game" << std::endl;
                std::cout << "  load - Load game" << std::endl;
            } else if (command == "s" || command == "status") {
                // Display character status
                std::cout << "\nCharacter Status:" << std::endl;
                std::cout << "Strength: " << controller.gameContext.playerStats.strength << std::endl;
                std::cout << "Dexterity: " << controller.gameContext.playerStats.dexterity << std::endl;
                std::cout << "Constitution: " << controller.gameContext.playerStats.constitution << std::endl;
                std::cout << "Intelligence: " << controller.gameContext.playerStats.intelligence << std::endl;
                std::cout << "Wisdom: " << controller.gameContext.playerStats.wisdom << std::endl;
                std::cout << "Charisma: " << controller.gameContext.playerStats.charisma << std::endl;

                std::cout << "\nSkills:" << std::endl;
                for (const auto& [skill, level] : controller.gameContext.playerStats.skills) {
                    std::cout << "- " << skill << ": " << level << std::endl;
                }
            } else if (command == "i" || command == "inventory") {
                // Display inventory
                std::cout << "\nInventory:" << std::endl;
                for (const auto& item : controller.gameContext.playerInventory.items) {
                    std::cout << "- " << item.name << " (" << item.quantity << ")" << std::endl;
                }
            } else if (command == "j" || command == "journal") {
                // Display quest journal
                std::cout << "\nQuest Journal:" << std::endl;
                for (const auto& [quest, status] : controller.gameContext.questJournal) {
                    std::cout << "- " << quest << ": " << status << std::endl;
                }
            } else if (command == "t" || command == "time") {
                // Advance time
                TimeNode* timeNode = dynamic_cast<TimeNode*>(controller.currentNodes["TimeSystem"]);
                if (timeNode) {
                    timeNode->advanceHour(&controller.gameContext);
                } else {
                    std::cout << "Time system not active. Initializing..." << std::endl;
                    controller.processInput("TimeSystem", {});
                    timeNode = dynamic_cast<TimeNode*>(controller.currentNodes["TimeSystem"]);
                    if (timeNode) {
                        timeNode->advanceHour(&controller.gameContext);
                    }
                }
            } else if (command == "save") {
                // Save game
                controller.saveState(saveFilePath);
                std::cout << "Game saved to " << saveFilePath << std::endl;
            } else if (command == "load") {
                // Load game
                if (controller.loadState(saveFilePath)) {
                    std::cout << "Game loaded successfully!" << std::endl;
                } else {
                    std::cout << "Failed to load game." << std::endl;
                }
            } else {
                std::cout << "Unknown command. Type 'h' for help." << std::endl;
            }
        }
    }

    std::cout << "\nExiting Oath RPG Engine. Thanks for playing!" << std::endl;
    return 0;
}