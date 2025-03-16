// TheftExecutionNode.cpp
#include "TheftExecutionNode.hpp"
#include "../../data/GameContext.hpp"
#include "CrimeLawConfig.hpp"
#include "CrimeType.hpp"
#include <iostream>
#include <random>

TheftExecutionNode::TheftExecutionNode(const std::string& name, const std::string& target)
    : CrimeSystemNode(name)
    , theftTarget(target)
{
    // Find target in the config
    for (const auto& t : crimeLawConfig["theftTargets"]) {
        if (t["id"] == target) {
            theftValue = t["value"].get<int>();
            theftSeverity = t["severity"].get<int>();
            break;
        }
    }
}

void TheftExecutionNode::onEnter(GameContext* context)
{
    std::cout << "Attempting to steal " << theftTarget << "..." << std::endl;

    // Roll for theft success
    bool success = attemptTheft(context);

    if (success) {
        std::cout << "Theft successful! You've stolen " << theftTarget << " worth " << theftValue << " gold." << std::endl;

        // Add stolen goods to inventory
        if (context) {
            std::string itemId = "stolen_" + theftTarget;
            std::string itemName = "Stolen " + theftTarget;
            context->playerInventory.addItem({ itemId, itemName, "stolen", theftValue, 1 });
        }

        // Record the crime
        commitCrime(context, CrimeType::THEFT(), theftSeverity, 0);
    } else {
        std::cout << "Theft failed! You couldn't steal the " << theftTarget << "." << std::endl;

        // Check if caught
        CrimeLawContext* lawContext = getLawContext(context);
        std::string region = getCurrentRegion(context);

        bool caught = (lawContext->guardSuspicion[region] > 50);
        if (caught) {
            std::cout << "You've been caught attempting theft!" << std::endl;
            commitCrime(context, CrimeType::THEFT(), theftSeverity / 2, -20); // Caught in the act
        } else {
            std::cout << "No one noticed your failed attempt." << std::endl;
        }
    }
}

bool TheftExecutionNode::attemptTheft(GameContext* context)
{
    // Base chance based on theft difficulty
    int successChance = 70 - (theftSeverity * 10); // Harder thefts are less likely

    // Adjust based on player skills
    if (context) {
        // Add stealth skill bonus
        auto stealthIt = context->playerStats.skills.find("stealth");
        if (stealthIt != context->playerStats.skills.end()) {
            successChance += stealthIt->second * 3;
        }

        // Add pickpocket/theft skill bonus
        auto theftIt = context->playerStats.skills.find("pickpocket");
        if (theftIt != context->playerStats.skills.end()) {
            successChance += theftIt->second * 5;
        }
    }

    // Ensure reasonable bounds
    successChance = std::max(5, std::min(successChance, 95));

    // Random check
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 100);

    return dis(gen) <= successChance;
}