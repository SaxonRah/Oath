#pragma once

#include "core/TAController.hpp"
#include "systems/religion/ReligiousGameContext.hpp"
#include <string>
#include <vector>

// Forward declarations
class DeityNode;
class TempleNode;
class RitualNode;
class BlessingNode;
class ReligiousQuestNode;
class TANode;

class ReligionController : public TAController {
public:
    // Root node for religion system
    TANode* religionRoot;

    // All deities in the pantheon
    std::vector<DeityNode*> pantheon;

    // All temples in the world
    std::vector<TempleNode*> temples;

    // All known rituals
    std::vector<RitualNode*> rituals;

    // All available blessings
    std::vector<BlessingNode*> blessings;

    // All religious quests
    std::vector<ReligiousQuestNode*> religiousQuests;

    // Convert base GameContext to ReligiousGameContext
    ReligiousGameContext* getReligiousContext();

    // Initialize the religion system from JSON
    void initializeReligionSystem(const std::string& jsonFilePath);

private:
    // Create all deities in the pantheon from JSON
    void createDeitiesFromJson(const nlohmann::json& deitiesData);

    // Create temples from JSON
    void createTemplesFromJson(const nlohmann::json& templesData);

    // Create rituals from JSON
    void createRitualsFromJson(const nlohmann::json& ritualsData);

    // Create blessings from JSON
    void createBlessingsFromJson(const nlohmann::json& blessingsData);

    // Create religious quests from JSON
    void createReligiousQuestsFromJson(const nlohmann::json& questsData);

    // Set up hierarchy for religion system
    void setupReligionHierarchy();
};