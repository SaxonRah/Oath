// PickpocketNode.cpp
#include "PickpocketNode.hpp"
#include "../../core/TAInput.hpp"
#include "CrimeLawConfig.hpp"
#include "CrimeType.hpp"
#include <iostream>
#include <nlohmann/json.hpp>
#include <random>

PickpocketNode::PickpocketNode(const std::string& name)
    : CrimeSystemNode(name)
{
}

void PickpocketNode::onEnter(GameContext* context)
{
    std::cout << "You scan the area for potential pickpocketing targets..." << std::endl;

    // Get list of NPCs in current location
    std::vector<nlohmann::json> targets = getPickpocketTargets(context);

    if (targets.empty()) {
        std::cout << "There's no one suitable to pickpocket here." << std::endl;
        return;
    }

    std::cout << "Potential targets:" << std::endl;
    for (size_t i = 0; i < targets.size(); i++) {
        std::cout << i + 1 << ". " << targets[i]["name"].get<std::string>() << std::endl;
    }
}

std::vector<nlohmann::json> PickpocketNode::getPickpocketTargets(GameContext* context)
{
    // In a real implementation, this would get NPCs from current location
    // For now, just return the targets from the config
    return crimeLawConfig["pickpocketTargets"].get<std::vector<nlohmann::json>>();
}

std::vector<TAAction> PickpocketNode::getAvailableActions()
{
    std::vector<TAAction> actions;

    // Generate actions based on the targets from the config
    for (const auto& target : crimeLawConfig["pickpocketTargets"]) {
        std::string id = target["id"];
        std::string name = target["name"];
        int difficulty = target["difficulty"];

        actions.push_back({ "pickpocket_" + id,
            "Pickpocket " + name,
            [id, difficulty]() -> TAInput {
                return { "pickpocket_action", { { "target", id }, { "difficulty", difficulty } } };
            } });
    }

    actions.push_back({ "cancel", "Cancel pickpocketing", []() -> TAInput {
                           return { "pickpocket_action", { { "target", std::string("cancel") } } };
                       } });

    return actions;
}

bool PickpocketNode::evaluateTransition(const TAInput& input, TANode*& outNextNode)
{
    if (input.type == "pickpocket_action") {
        std::string target = std::get<std::string>(input.parameters.at("target"));

        if (target == "cancel") {
            // Find the cancel transition
            for (const auto& rule : transitionRules) {
                if (rule.description.find("cancel") != std::string::npos) {
                    outNextNode = rule.targetNode;
                    return true;
                }
            }
        } else {
            // Attempt pickpocketing
            int difficulty = std::get<int>(input.parameters.at("difficulty"));
            attemptPickpocket(nullptr, target, difficulty); // Context would be passed

            // Use the success/failure transition
            // For example, if we want to stay in the same node:
            outNextNode = this;
            return true;
        }
    }

    return CrimeSystemNode::evaluateTransition(input, outNextNode);
}

void PickpocketNode::attemptPickpocket(GameContext* context, const std::string& target, int difficulty)
{
    // Base chance based on difficulty
    int successChance = 80 - (difficulty * 8); // 1-10 difficulty scale

    // Adjust based on player skills
    if (context) {
        // Add stealth skill bonus
        auto stealthIt = context->playerStats.skills.find("stealth");
        if (stealthIt != context->playerStats.skills.end()) {
            successChance += stealthIt->second * 2;
        }

        // Add pickpocket skill bonus
        auto pickpocketIt = context->playerStats.skills.find("pickpocket");
        if (pickpocketIt != context->playerStats.skills.end()) {
            successChance += pickpocketIt->second * 4;
        }
    }

    // Ensure reasonable bounds
    successChance = std::max(5, std::min(successChance, 95));

    // Random check
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 100);

    bool success = dis(gen) <= successChance;

    // Find target config
    nlohmann::json targetConfig;
    for (const auto& t : crimeLawConfig["pickpocketTargets"]) {
        if (t["id"] == target) {
            targetConfig = t;
            break;
        }
    }

    if (success) {
        std::cout << "You successfully pickpocket the " << targetConfig["name"].get<std::string>() << "!" << std::endl;

        // Determine loot based on target
        int minGold = targetConfig["gold"]["min"].get<int>();
        int maxGold = targetConfig["gold"]["max"].get<int>();
        int goldRange = maxGold - minGold;
        int gold = minGold + (rand() % (goldRange + 1));

        std::string itemLoot = targetConfig["loot"].get<std::string>();

        std::cout << "You find " << gold << " gold";
        if (!itemLoot.empty()) {
            std::cout << " and a " << itemLoot;

            // Add to inventory
            if (context) {
                context->playerInventory.addItem({ itemLoot, itemLoot, "stolen", 0, 1 });
            }
        }
        std::cout << "." << std::endl;

        // Record the crime
        commitCrime(context, CrimeType::PICKPOCKETING(), difficulty / 2, 10);

        // Improve skill with successful pickpocketing
        if (context) {
            if (rand() % 100 < crimeLawConfig["crimeConfig"]["skillImprovementChance"].get<int>()) {
                context->playerStats.improveSkill("pickpocket", 1);
                std::cout << "Your pickpocketing skill has improved!" << std::endl;
            }
        }
    } else {
        std::cout << "You fail to pickpocket the " << targetConfig["name"].get<std::string>() << "!" << std::endl;

        // Determine if caught
        int detectionChance = difficulty * 10;

        // Random check for detection
        bool caught = dis(gen) <= detectionChance;

        if (caught) {
            std::cout << "\"Hey! This person is trying to rob me!\"" << std::endl;
            std::cout << "You've been caught pickpocketing!" << std::endl;

            // Record the crime as witnessed
            commitCrime(context, CrimeType::PICKPOCKETING(), difficulty / 2, -20);

            CrimeLawContext* lawContext = getLawContext(context);
            std::string region = getCurrentRegion(context);
            lawContext->guardAlerted[region] = true;
        } else {
            std::cout << "Fortunately, no one noticed your attempt." << std::endl;
        }
    }
}