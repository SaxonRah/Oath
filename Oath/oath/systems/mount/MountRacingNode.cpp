#include "MountRacingNode.hpp"
#include "../../core/TAAction.hpp"
#include "../../core/TAInput.hpp"
#include "../../data/GameContext.hpp"
#include "Mount.hpp"
#include "MountStats.hpp"
#include <algorithm>
#include <iostream>
#include <nlohmann/json.hpp>
#include <random>


MountRacingNode::MountRacingNode(const std::string& name, const std::string& track, float length, int diff)
    : TANode(name)
    , trackName(track)
    , trackLength(length)
    , difficulty(diff)
    , entryFee(diff * 2)
    , firstPrize(diff * 10)
    , secondPrize(diff * 5)
    , thirdPrize(diff * 2)
{
}

MountRacingNode* MountRacingNode::createFromJson(const std::string& name, const nlohmann::json& j)
{
    std::string trackName = j["name"];
    float trackLength = j["length"];
    int difficulty = j["difficulty"];

    MountRacingNode* node = new MountRacingNode(name, trackName, trackLength, difficulty);

    // Set prizes
    if (j.contains("prizes") && j["prizes"].is_object()) {
        node->firstPrize = j["prizes"]["first"];
        node->secondPrize = j["prizes"]["second"];
        node->thirdPrize = j["prizes"]["third"];
    }

    // Set entry fee
    node->entryFee = j.value("entryFee", difficulty * 2);

    // Generate competitors using name lists from JSON
    if (j.contains("competitorNames") && j.contains("competitorLastNames")) {
        std::vector<std::string> names = j["competitorNames"];
        std::vector<std::string> lastNames = j["competitorLastNames"];
        node->generateCompetitors(5, names, lastNames, difficulty);
    } else {
        node->generateCompetitors(5);
    }

    return node;
}

void MountRacingNode::generateCompetitors(int count, const std::vector<std::string>& names,
    const std::vector<std::string>& lastNames, int baseDifficulty)
{
    // Use provided names or defaults
    std::vector<std::string> competitorNames = names;
    std::vector<std::string> competitorLastNames = lastNames;

    // Default names if none provided
    if (competitorNames.empty()) {
        competitorNames = {
            "Thunder", "Lightning", "Shadow", "Storm", "Arrow",
            "Wind", "Blaze", "Whisper", "Flash", "Midnight"
        };
    }

    if (competitorLastNames.empty()) {
        competitorLastNames = {
            "Runner", "Galloper", "Dasher", "Swift", "Racer",
            "Hooves", "Striker", "Chaser", "Bolt", "Charge"
        };
    }

    // Random generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> nameDist(0, competitorNames.size() - 1);
    std::uniform_int_distribution<> lastNameDist(0, competitorLastNames.size() - 1);

    // Stats distribution based on difficulty
    int baseValue = baseDifficulty ? 80 + baseDifficulty / 2 : 80 + difficulty / 2;

    std::uniform_int_distribution<> speedVar(-20, 20);
    std::uniform_int_distribution<> staminaVar(-20, 20);
    std::uniform_int_distribution<> skillVar(-20, 20);

    // Generate competitors
    competitors.clear();
    for (int i = 0; i < count; i++) {
        RaceCompetitor comp;
        comp.name = competitorNames[nameDist(gen)] + " " + competitorLastNames[lastNameDist(gen)];
        comp.speed = baseValue + speedVar(gen);
        comp.stamina = baseValue + staminaVar(gen);
        comp.skill = baseValue + skillVar(gen);

        competitors.push_back(comp);
    }
}

void MountRacingNode::onEnter(GameContext* context)
{
    std::cout << "Welcome to the " << trackName << " Racing Track!" << std::endl;
    std::cout << "Track Length: " << trackLength << " units" << std::endl;
    std::cout << "Difficulty: " << difficulty << "/100" << std::endl;
    std::cout << "Entry Fee: " << entryFee << " gold" << std::endl;
    std::cout << "Prizes: 1st - " << firstPrize << " gold, 2nd - "
              << secondPrize << " gold, 3rd - " << thirdPrize << " gold" << std::endl;

    std::cout << "\nToday's Competitors:" << std::endl;
    for (const auto& comp : competitors) {
        std::cout << "- " << comp.name << std::endl;
    }
}

std::vector<MountRacingNode::RaceResult> MountRacingNode::simulateRace(Mount* playerMount)
{
    if (!playerMount)
        return {};

    std::vector<RaceResult> results;
    std::random_device rd;
    std::mt19937 gen(rd());

    // Add player mount to results
    RaceResult playerResult;
    playerResult.name = playerMount->name + " (You)";

    // Calculate player's time
    MountStats stats = playerMount->getEffectiveStats();

    // Base speed affects time directly
    float baseTimePlayer = trackLength / (stats.getEffectiveSpeed() * 0.1f);

    // Stamina affects endurance - worse stamina = more slowdown
    float staminaFactorPlayer = 1.0f + (stats.maxStamina - stats.stamina) * 0.005f;

    // Training in racing skill helps optimize performance
    float skillFactorPlayer = 1.0f - (stats.specialTraining.at("racing") * 0.002f);

    // Random factor (±10%)
    std::uniform_real_distribution<> randFactorDist(0.9f, 1.1f);
    float randomFactorPlayer = randFactorDist(gen);

    // Final time calculation
    playerResult.time = baseTimePlayer * staminaFactorPlayer * skillFactorPlayer * randomFactorPlayer;

    // Calculate time for each NPC competitor
    for (const auto& comp : competitors) {
        RaceResult npcResult;
        npcResult.name = comp.name;

        float baseTimeNPC = trackLength / (comp.speed * 0.1f);
        float staminaFactorNPC = 1.0f + ((100 - comp.stamina) * 0.005f);
        float skillFactorNPC = 1.0f - (comp.skill * 0.002f);
        float randomFactorNPC = randFactorDist(gen);

        npcResult.time = baseTimeNPC * staminaFactorNPC * skillFactorNPC * randomFactorNPC;

        results.push_back(npcResult);
    }

    // Add player result
    results.push_back(playerResult);

    // Sort by time (ascending)
    std::sort(results.begin(), results.end(),
        [](const RaceResult& a, const RaceResult& b) {
            return a.time < b.time;
        });

    // Assign positions
    for (size_t i = 0; i < results.size(); i++) {
        results[i].position = i + 1;
    }

    // Fatigue the mount based on race intensity
    playerMount->stats.useStamina(trackLength * 0.5f);
    playerMount->stats.fatigue += trackLength * 0.3f;
    if (playerMount->stats.fatigue > 100)
        playerMount->stats.fatigue = 100;

    return results;
}

std::vector<TAAction> MountRacingNode::getAvailableActions()
{
    std::vector<TAAction> actions = TANode::getAvailableActions();

    // Enter race action
    actions.push_back({ "enter_race", "Enter the race",
        [this]() -> TAInput {
            return { "race_action", { { "action", std::string("enter") } } };
        } });

    // View competitors action
    actions.push_back({ "view_competitors", "Study your competition",
        [this]() -> TAInput {
            return { "race_action", { { "action", std::string("view") } } };
        } });

    // Practice on track action
    actions.push_back({ "practice", "Practice on the track",
        [this]() -> TAInput {
            return { "race_action", { { "action", std::string("practice") } } };
        } });

    // Exit racing area action
    actions.push_back({ "exit_racing", "Leave racing area",
        [this]() -> TAInput {
            return { "race_action", { { "action", std::string("exit") } } };
        } });

    return actions;
}

bool MountRacingNode::evaluateTransition(const TAInput& input, TANode*& outNextNode)
{
    if (input.type == "race_action") {
        std::string action = std::get<std::string>(input.parameters.at("action"));

        if (action == "exit") {
            // Find the exit transition
            for (const auto& rule : transitionRules) {
                if (rule.description == "Exit") {
                    outNextNode = rule.targetNode;
                    return true;
                }
            }
        }

        // Other actions stay in same node
        outNextNode = this;
        return true;
    }

    return TANode::evaluateTransition(input, outNextNode);
}