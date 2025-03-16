#include "systems/religion/ReligionController.hpp"
#include "systems/religion/BlessingNode.hpp"
#include "systems/religion/DeityNode.hpp"
#include "systems/religion/PrayerNode.hpp"
#include "systems/religion/ReligiousQuestNode.hpp"
#include "systems/religion/RitualNode.hpp"
#include "systems/religion/TempleNode.hpp"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

ReligiousGameContext* ReligionController::getReligiousContext()
{
    return dynamic_cast<ReligiousGameContext*>(&gameContext);
}

void ReligionController::initializeReligionSystem(const std::string& jsonFilePath)
{
    std::cout << "Initializing Religion System from JSON..." << std::endl;

    // Load JSON data
    nlohmann::json religionData;
    try {
        std::ifstream file(jsonFilePath);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << jsonFilePath << std::endl;
            return;
        }
        file >> religionData;
    } catch (const std::exception& e) {
        std::cerr << "Error parsing JSON: " << e.what() << std::endl;
        return;
    }

    // Create pantheon root node
    religionRoot = createNode("ReligionSystem");

    // Create deities
    std::cout << "Creating pantheon of deities..." << std::endl;
    createDeitiesFromJson(religionData["deities"]);

    // Create temples
    std::cout << "Creating temples..." << std::endl;
    createTemplesFromJson(religionData["temples"]);

    // Create rituals
    std::cout << "Creating rituals..." << std::endl;
    createRitualsFromJson(religionData["rituals"]);

    // Create blessings
    std::cout << "Creating blessings..." << std::endl;
    createBlessingsFromJson(religionData["blessings"]);

    // Create religious quests
    std::cout << "Creating religious quests..." << std::endl;
    createReligiousQuestsFromJson(religionData["religiousQuests"]);

    // Set up holy day calendar
    std::cout << "Setting up holy day calendar..." << std::endl;
    auto* context = getReligiousContext();
    if (context) {
        context->loadHolyDays(religionData["holyDayCalendar"]);

        // Initialize religious stats with all deity IDs
        std::vector<std::string> deityIds;
        for (auto* deity : pantheon) {
            deityIds.push_back(deity->deityId);
        }
        context->religiousStats.initializeDeities(deityIds);
    }

    // Set up hierarchy
    std::cout << "Setting up Religion System hierarchy..." << std::endl;
    setupReligionHierarchy();

    // Register the religion system root
    setSystemRoot("ReligionSystem", religionRoot);

    std::cout << "Religion System initialization complete." << std::endl;
}

void ReligionController::createDeitiesFromJson(const nlohmann::json& deitiesData)
{
    for (const auto& deityData : deitiesData) {
        std::string id = deityData["id"];
        std::string name = deityData["name"];
        std::string title = deityData["title"];

        DeityNode* deity = new DeityNode(name, id, title);
        registerNode(deity);

        deity->loadFromJson(deityData);
        pantheon.push_back(deity);
    }
}

void ReligionController::createTemplesFromJson(const nlohmann::json& templesData)
{
    for (const auto& templeData : templesData) {
        std::string id = templeData["id"];
        std::string name = templeData["name"];
        std::string location = templeData["location"];
        std::string deityId = templeData["deityId"];

        TempleNode* temple = new TempleNode(id, name, location, deityId);
        registerNode(temple);

        temple->loadFromJson(templeData);
        temples.push_back(temple);
    }
}

void ReligionController::createRitualsFromJson(const nlohmann::json& ritualsData)
{
    for (const auto& ritualData : ritualsData) {
        std::string id = ritualData["id"];
        std::string name = ritualData["name"];
        std::string deityId = ritualData["deityId"];

        RitualNode* ritual = new RitualNode(id, name, deityId);
        registerNode(ritual);

        ritual->loadFromJson(ritualData);
        rituals.push_back(ritual);
    }
}

void ReligionController::createBlessingsFromJson(const nlohmann::json& blessingsData)
{
    for (const auto& blessingData : blessingsData) {
        std::string id = blessingData["id"];
        std::string name = blessingData["name"];
        std::string deityId = blessingData["deityId"];
        int tier = blessingData["tier"];

        BlessingNode* blessing = new BlessingNode(
            id,
            name,
            deityId,
            static_cast<BlessingNode::BlessingTier>(tier));
        registerNode(blessing);

        blessing->loadFromJson(blessingData);
        blessings.push_back(blessing);
    }
}

void ReligionController::createReligiousQuestsFromJson(const nlohmann::json& questsData)
{
    for (const auto& questData : questsData) {
        std::string id = questData["id"];
        std::string deityId = questData["deityId"];

        ReligiousQuestNode* quest = new ReligiousQuestNode(id, deityId);
        registerNode(quest);

        quest->loadFromJson(questData);
        religiousQuests.push_back(quest);
    }
}

void ReligionController::setupReligionHierarchy()
{
    // Add all deities to the religion root
    for (auto* deity : pantheon) {
        religionRoot->addChild(deity);

        // Set up blessing connections
        for (auto* blessing : blessings) {
            if (blessing->deityId == deity->deityId) {
                deity->availableBlessings.push_back(blessing);
            }
        }

        // Set up temple connections
        for (auto* temple : temples) {
            if (temple->deityId == deity->deityId) {
                deity->temples.push_back(temple);

                // Add rituals to temples
                for (auto* ritual : rituals) {
                    if (ritual->deityId == deity->deityId) {
                        temple->availableRituals.push_back(ritual);
                    }
                }
            }
        }

        // Create prayer node for each deity
        PrayerNode* prayerNode = new PrayerNode("Prayer_" + deity->deityId, deity->deityId);
        registerNode(prayerNode);
        prayerNode->prayerDescription = "You kneel in prayer before the altar of " + deity->deityName + ".";
        deity->addChild(prayerNode);

        // Set up return transitions
        prayerNode->addTransition(
            [](const TAInput& input) {
                return input.type == "prayer_action" && std::get<std::string>(input.parameters.at("action")) == "finish";
            },
            deity, "Return to deity");
    }

    // Set up temple exit transitions
    for (auto* temple : temples) {
        // Find corresponding region/location
        temple->addTransition(
            [](const TAInput& input) {
                return input.type == "temple_action" && std::get<std::string>(input.parameters.at("action")) == "leave";
            },
            religionRoot, "Exit temple");
    }

    // Set up ritual transitions
    for (auto* ritual : rituals) {
        // Find corresponding temple
        for (auto* temple : temples) {
            if (temple->deityId == ritual->deityId) {
                ritual->addTransition(
                    [](const TAInput& input) {
                        return input.type == "ritual_action" && std::get<std::string>(input.parameters.at("action")) == "back";
                    },
                    temple, "Return to temple");
                break;
            }
        }
    }

    // Set up deity return transitions
    for (auto* deity : pantheon) {
        deity->addTransition(
            [](const TAInput& input) {
                return input.type == "deity_action" && std::get<std::string>(input.parameters.at("action")) == "back";
            },
            religionRoot, "Return to pantheon");
    }
}