#include "Generator_NPC.h"
#include <filesystem>
#include <iomanip>
#include <iostream>

void printNPC(const NPCGenerator::NPC& npc)
{
    std::cout << "=== NPC Details ===" << std::endl;
    std::cout << "Name: " << npc.name << std::endl;
    std::cout << "Age: " << npc.age << std::endl;
    std::cout << "Gender: " << (npc.gender == NPCGenerator::Gender::Male ? "Male" : "Female") << std::endl;
    std::cout << "Race: " << npc.race << std::endl;
    std::cout << "Height: " << npc.height << " cm" << std::endl;
    std::cout << "Build: " << npc.build << std::endl;
    std::cout << "Hair Color: " << npc.hairColor << std::endl;

    std::cout << "\nPersonality Traits:" << std::endl;
    for (const auto& trait : npc.personalityTraits) {
        std::cout << "- " << trait << std::endl;
    }

    std::cout << "\nMoral Alignment: " << static_cast<int>(npc.moralAlignment) << std::endl;
}

int main()
{

    // Optional: Specify a custom config path
    NPCGenerator::initializeConfiguration("Generator_NPC.json");

    try {
        // Use a tavern_keeper.json template
        NPCGenerator::NPC tavernKeeper = NPCGenerator::GenerateNPC("crafted_npcs/tavern_keeper.json");
        printNPC(tavernKeeper);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    // Generate multiple procedural NPCs
    for (int i = 0; i < 5; ++i) {
        try {
            NPCGenerator::NPC proceduralNPC = NPCGenerator::GenerateNPC();
            printNPC(proceduralNPC);
            std::cout << "\nBackground: " << proceduralNPC.familyBackground << std::endl;

            std::cout << "\nLife Events:" << std::endl;
            for (const auto& event : proceduralNPC.significantLifeEvents) {
                std::cout << "- " << event << std::endl;
            }

            std::cout << "\nSkills:" << std::endl;
            for (const auto& [skill, level] : proceduralNPC.skills) {
                std::cout << skill << ": " << level << std::endl;
            }

            std::cout << "\n-------------------\n"
                      << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error generating NPC: " << e.what() << std::endl;
        }
    }

    // Save generated NPCs
    std::filesystem::create_directory("saved_npcs");
    for (int i = 0; i < 5; ++i) {
        NPCGenerator::NPC proceduralNPC = NPCGenerator::GenerateNPC();
        std::string filename = "saved_npcs/npc_" + std::to_string(i) + ".json";
        NPCGenerator::SaveNPC(proceduralNPC, filename);
    }

    // Load and print saved NPCs
    std::vector<NPCGenerator::NPC> loadedNPCs = NPCGenerator::LoadAllNPCs("saved_npcs");
    std::cout << "\n=== Loaded NPCs ===" << std::endl;
    for (const auto& loadedNPC : loadedNPCs) {
        printNPC(loadedNPC);
    }

    return 0;
}