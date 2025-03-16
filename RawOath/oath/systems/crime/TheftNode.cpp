// TheftNode.cpp
#include "TheftNode.hpp"
#include "../../core/TAInput.hpp"
#include "CrimeLawConfig.hpp"
#include <iostream>

TheftNode::TheftNode(const std::string& name)
    : CrimeSystemNode(name)
{
}

void TheftNode::onEnter(GameContext* context)
{
    std::cout << "You are considering stealing something..." << std::endl;

    // Show potential theft targets based on current location
    std::cout << "Potential targets:" << std::endl;

    for (size_t i = 0; i < crimeLawConfig["theftTargets"].size(); i++) {
        const auto& target = crimeLawConfig["theftTargets"][i];
        std::cout << i + 1 << ". " << target["name"].get<std::string>()
                  << " (" << (target["severity"].get<int>() <= 3 ? "Low" : (target["severity"].get<int>() <= 6 ? "Medium" : "High"))
                  << " risk)" << std::endl;
    }
}

std::vector<TAAction> TheftNode::getAvailableActions()
{
    std::vector<TAAction> actions;

    // Generate actions for each theft target from config
    for (const auto& target : crimeLawConfig["theftTargets"]) {
        std::string id = target["id"];
        std::string name = target["name"];

        actions.push_back({ "steal_" + id,
            "Steal " + name,
            [id]() -> TAInput {
                return { "theft_action", { { "target", id } } };
            } });
    }

    actions.push_back({ "cancel_theft", "Cancel theft attempt", []() -> TAInput {
                           return { "theft_action", { { "target", std::string("cancel") } } };
                       } });

    return actions;
}

bool TheftNode::evaluateTransition(const TAInput& input, TANode*& outNextNode)
{
    if (input.type == "theft_action") {
        std::string target = std::get<std::string>(input.parameters.at("target"));

        // Find the target node in the transitions
        for (const auto& rule : transitionRules) {
            if (rule.description.find(target) != std::string::npos) {
                outNextNode = rule.targetNode;
                return true;
            }
        }
    }

    return CrimeSystemNode::evaluateTransition(input, outNextNode);
}