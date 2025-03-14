#pragma once

#include <../../include/nlohmann/json.hpp>
#include <map>
#include <optional>
#include <random>
#include <string>
#include <vector>

using json = nlohmann::json;

class Generator_NPC {
public:
    // Enums for typed attributes
    enum class Gender { Male,
        Female,
        Other };
    enum class Alignment {
        LawfulGood,
        NeutralGood,
        ChaoticGood,
        LawfulNeutral,
        TrueNeutral,
        ChaoticNeutral,
        LawfulEvil,
        NeutralEvil,
        ChaoticEvil
    };

    // Core NPC Structure
    struct NPC {
        // Basic Attributes
        std::string name;
        int age;
        Gender gender;
        std::string race;
        std::string occupation;
        std::string socialClass;

        // Physical Characteristics
        float height;
        std::string build;
        std::string hairColor;
        std::string eyeColor;
        std::string distinguishingFeature;

        // Personality
        std::vector<std::string> personalityTraits;
        Alignment moralAlignment;
        std::string motivation;
        std::string fear;

        // Background
        std::string originLocation;
        std::string familyBackground;
        std::vector<std::string> significantLifeEvents;

        // Skills
        std::map<std::string, int> skills;
        std::vector<std::string> uniqueTalents;
    };

    // Static Methods
    static NPC GenerateNPC(const std::string& templatePath);
    static NPC GenerateNPC(const json& templateJson);

    // Random
    static std::random_device rd;
    static std::mt19937 gen;

    // Helper Generation Methods
    static std::string selectFromProbabilityMap(const json& probMap);
    static int generateAttributeValue(const json& range);
    static std::vector<std::string> selectMultipleTraits(const json& traitPool, int count);

    // Validation and Sanity Checks
    static bool validateTemplate(const json& templateJson);

    // Saving and Loading NPCs
    static void SaveNPC(const NPC& npc, const std::string& filename);
    static NPC LoadNPC(const std::string& filename);
    static std::vector<NPC> LoadAllNPCs(const std::string& directory);

private:
    // Procedural Name Generation
    static std::string generateName(Gender gender, const std::string& race);

    // Procedural Background Generation
    static std::string generateBackground(const NPC& npc);

    // Procedural Skill Generation
    static std::map<std::string, int> generateSkills(const NPC& npc);

    // Procedural Personality Generation
    static std::vector<std::string> generatePersonalityTraits();

    // Procedural Life Events Generation
    static std::vector<std::string> generateLifeEvents(int age);

    // Name Generation Resources
    static const std::map<std::string, std::vector<std::string>> FirstNamesByRaceAndGender;
    static const std::map<std::string, std::vector<std::string>> LastNames;

    // Skill Definitions
    static const std::map<std::string, std::pair<int, int>> SkillRanges;
};