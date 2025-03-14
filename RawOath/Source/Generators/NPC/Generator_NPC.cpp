#include "Generator_NPC.hpp"

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>

// Initialize static members
std::random_device Generator_NPC::rd;
std::mt19937 Generator_NPC::gen(rd());

// Static configuration storage
json Generator_NPC::configData;

// Load configuration from JSON
void loadConfiguration(const std::string& configPath = "Generator_NPC.json")
{
    std::ifstream configFile(configPath);
    if (!configFile.is_open()) {
        throw std::runtime_error("Could not open configuration file: " + configPath);
    }
    Generator_NPC::configData = json::parse(configFile);
}

// Existing methods like selectFromProbabilityMap, generateAttributeValue, etc. remain similar

std::string Generator_NPC::selectFromProbabilityMap(const json& probMap)
{
    std::uniform_real_distribution<> dist(0.0, 1.0);
    double randomValue = dist(gen);
    double cumulativeProbability = 0.0;

    for (const auto& [key, probability] : probMap.items()) {
        cumulativeProbability += probability.get<double>();
        if (randomValue <= cumulativeProbability) {
            return key;
        }
    }

    // Fallback
    return probMap.begin().key();
}

int Generator_NPC::generateAttributeValue(const json& range)
{
    std::uniform_int_distribution<> dist(
        range["min"].get<int>(),
        range["max"].get<int>());
    return dist(gen);
}

std::vector<std::string> Generator_NPC::selectMultipleTraits(const json& traitPool, int count)
{
    std::vector<std::string> traits(traitPool.begin(), traitPool.end());
    std::shuffle(traits.begin(), traits.end(), gen);
    traits.resize(std::min(count, static_cast<int>(traits.size())));
    return traits;
}

bool Generator_NPC::validateTemplate(const json& templateJson)
{
    // Basic validation checks
    const std::vector<std::string> requiredKeys = {
        "base_attributes",
        "physical_traits",
        "personality"
    };

    for (const auto& key : requiredKeys) {
        if (!templateJson.contains(key)) {
            std::cerr << "Missing key: " << key << std::endl;
            return false;
        }
    }

    return true;
}

std::string NPCGenerator::generateName(Gender gender, const std::string& race)
{
    // Ensure configuration is loaded
    if (configData.empty()) {
        loadConfiguration();
    }

    std::string genderPrefix = (gender == Gender::Male) ? "male" : "female";
    std::string key = race + "_" + genderPrefix;

    // Get first names
    auto& firstNames = configData["names"]["first_names"];

    // Fallback to human names if specific race names not found
    if (firstNames.find(key) == firstNames.end()) {
        key = "human_" + genderPrefix;
    }

    std::string firstName = selectFromVector(firstNames[key].get<std::vector<std::string>>());

    // Get last names
    auto& lastNames = configData["names"]["last_names"];
    std::string lastName = selectFromVector(lastNames[race].get<std::vector<std::string>>());

    return firstName + " " + lastName;
}

std::string NPCGenerator::generateBackground(const NPC& npc)
{
    // Ensure configuration is loaded
    if (configData.empty()) {
        loadConfiguration();
    }

    auto& backgroundElements = configData["background_elements"];

    std::vector<std::string> backgroundParts = {
        "Born in a " + selectFromVector(backgroundElements["locations"].get<std::vector<std::string>>()),
        "Raised by " + selectFromVector(backgroundElements["upbringing"].get<std::vector<std::string>>()),
        "Experienced " + selectFromVector(backgroundElements["experiences"].get<std::vector<std::string>>()) + " during youth"
    };

    return backgroundParts[0] + ". " + backgroundParts[1] + ". " + backgroundParts[2] + ".";
}

std::vector<std::string> NPCGenerator::generatePersonalityTraits()
{
    // Ensure configuration is loaded
    if (configData.empty()) {
        loadConfiguration();
    }

    auto& allTraits = configData["personality_traits"];

    std::vector<std::string> traits(allTraits.begin(), allTraits.end());
    std::shuffle(traits.begin(), traits.end(), gen);
    return std::vector<std::string>(traits.begin(), traits.begin() + 2);
}

std::map<std::string, int> NPCGenerator::generateSkills(const NPC& npc)
{
    // Ensure configuration is loaded
    if (configData.empty()) {
        loadConfiguration();
    }

    auto& skillRanges = configData["skill_ranges"];
    std::map<std::string, int> skills;

    // Base skills based on occupation and race
    std::vector<std::string> primarySkills = { "combat", "persuasion", "survival" };

    for (const auto& skillName : primarySkills) {
        auto range = skillRanges[skillName].get<std::vector<int>>();
        skills[skillName] = std::uniform_int_distribution<>(range[0], range[1])(gen);
    }

    // Add some randomness and variety
    if (npc.age > 40) {
        auto loreRange = skillRanges["lore"].get<std::vector<int>>();
        skills["lore"] = std::uniform_int_distribution<>(loreRange[0], loreRange[1])(gen);
    }

    return skills;
}

std::vector<std::string> NPCGenerator::generateLifeEvents(int age)
{
    // Ensure configuration is loaded
    if (configData.empty()) {
        loadConfiguration();
    }

    auto& possibleEvents = configData["life_events"];
    std::vector<std::string> lifeEvents;

    // More life events for older characters
    int eventCount = age / 10;
    std::vector<std::string> events(possibleEvents.begin(), possibleEvents.end());

    std::shuffle(events.begin(), events.end(), gen);

    for (int i = 0; i < std::min(eventCount, static_cast<int>(events.size())); ++i) {
        lifeEvents.push_back(events[i]);
    }

    return lifeEvents;
}

NPC NPCGenerator::GenerateNPC()
{
    // Ensure configuration is loaded
    if (configData.empty()) {
        loadConfiguration();
    }

    NPC npc;

    // Randomize core attributes
    npc.gender = (std::uniform_int_distribution<>(0, 1)(gen) == 0) ? Gender::Male : Gender::Female;
    npc.race = selectFromVector({ "human", "elf", "dwarf" });
    npc.age = std::uniform_int_distribution<>(18, 65)(gen);

    // Procedural Generation
    npc.name = generateName(npc.gender, npc.race);
    npc.familyBackground = generateBackground(npc);
    npc.personalityTraits = generatePersonalityTraits();
    npc.significantLifeEvents = generateLifeEvents(npc.age);
    npc.skills = generateSkills(npc);

    // Physical Characteristics
    npc.height = std::uniform_real_distribution<>(150.0, 200.0)(gen);
    npc.build = selectFromVector(configData["build_types"].get<std::vector<std::string>>());
    npc.hairColor = selectFromVector(configData["hair_colors"].get<std::vector<std::string>>());
    npc.eyeColor = selectFromVector(configData["eye_colors"].get<std::vector<std::string>>());

    // Moral Alignment
    npc.moralAlignment = static_cast<Alignment>(
        std::uniform_int_distribution<>(0, 8)(gen));

    // Occupation
    npc.occupation = selectFromVector(configData["occupations"].get<std::vector<std::string>>());

    // Social Class
    npc.socialClass = selectFromVector(configData["social_classes"].get<std::vector<std::string>>());

    return npc;
}

// Existing save and load methods remain the same as in previous implementation

// Utility method to select from a vector
template <typename T>
T NPCGenerator::selectFromVector(const std::vector<T>& vec)
{
    std::uniform_int_distribution<size_t> dist(0, vec.size() - 1);
    return vec[dist(gen)];
}

// Static configuration initialization
void NPCGenerator::initializeConfiguration(const std::string& configPath)
{
    loadConfiguration(configPath);
}

// Save singular NPC to directory
void NPCGenerator::SaveNPC(const NPC& npc, const std::string& filename)
{
    json npcJson = {
        { "name", npc.name },
        { "age", npc.age },
        { "gender", npc.gender == Gender::Male ? "Male" : "Female" },
        { "race", npc.race },
        { "occupation", npc.occupation },
        { "social_class", npc.socialClass },
        { "height", npc.height },
        { "build", npc.build },
        { "hair_color", npc.hairColor },
        { "eye_color", npc.eyeColor },
        { "personality_traits", npc.personalityTraits },
        { "moral_alignment", static_cast<int>(npc.moralAlignment) },
        { "family_background", npc.familyBackground },
        { "significant_life_events", npc.significantLifeEvents },
        { "skills", npc.skills }
    };

    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file for saving NPC: " + filename);
    }

    file << std::setw(4) << npcJson << std::endl;
    file.close();
}

// Load singular NPC from directory
NPC NPCGenerator::LoadNPC(const std::string& filename)
{
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open NPC file: " + filename);
    }

    json npcJson;
    file >> npcJson;

    NPC npc;
    npc.name = npcJson["name"];
    npc.age = npcJson["age"];
    npc.gender = (npcJson["gender"] == "Male") ? Gender::Male : Gender::Female;
    npc.race = npcJson["race"];
    npc.occupation = npcJson["occupation"];
    npc.socialClass = npcJson["social_class"];
    npc.height = npcJson["height"];
    npc.build = npcJson["build"];
    npc.hairColor = npcJson["hair_color"];
    npc.eyeColor = npcJson["eye_color"];
    npc.personalityTraits = npcJson["personality_traits"].get<std::vector<std::string>>();
    npc.moralAlignment = static_cast<Alignment>(npcJson["moral_alignment"].get<int>());
    npc.familyBackground = npcJson["family_background"];
    npc.significantLifeEvents = npcJson["significant_life_events"].get<std::vector<std::string>>();
    npc.skills = npcJson["skills"].get<std::map<std::string, int>>();

    return npc;
}

// Load all NPCs from directory
std::vector<NPC> NPCGenerator::LoadAllNPCs(const std::string& directory)
{
    std::vector<NPC> npcs;
    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
        if (entry.is_regular_file() && entry.path().extension() == ".json") {
            try {
                npcs.push_back(LoadNPC(entry.path().string()));
            } catch (const std::exception& e) {
                std::cerr << "Error loading NPC from " << entry.path() << ": " << e.what() << std::endl;
            }
        }
    }
    return npcs;
}