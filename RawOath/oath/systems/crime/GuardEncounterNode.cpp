// GuardEncounterNode.cpp
#include "GuardEncounterNode.hpp"
#include "../../core/TAInput.hpp"
#include "CrimeLawConfig.hpp"
#include <iostream>

GuardEncounterNode::GuardEncounterNode(const std::string& name)
    : CrimeSystemNode(name)
{
}

void GuardEncounterNode::onEnter(GameContext* context)
{
    CrimeLawContext* lawContext = getLawContext(context);
    std::string region = getCurrentRegion(context);

    // Check if actually wanted
    if (!lawContext->criminalRecord.isWanted(region)) {
        std::cout << "Guard looks at you but doesn't seem interested." << std::endl;
        return;
    }

    int bounty = lawContext->criminalRecord.getBounty(region);
    int guardResponse = determineGuardResponse(context);

    switch (guardResponse) {
    case 0: // ARREST
        std::cout << "\"Stop right there, criminal scum! You have committed crimes against our region.\"" << std::endl;
        std::cout << "\"Your bounty is " << bounty << " gold. Pay your fine or serve your sentence.\"" << std::endl;
        break;
    case 1: // ATTACK
        std::cout << "\"You're a wanted criminal! Surrender or die!\"" << std::endl;
        std::cout << "The guards draw their weapons and prepare to attack!" << std::endl;
        break;
    case 2: // FINE
        std::cout << "\"You've broken the law. Pay a fine of " << bounty << " gold.\"" << std::endl;
        break;
    case 3: // WARN
        std::cout << "\"We're keeping an eye on you. Don't cause any trouble.\"" << std::endl;
        break;
    default:
        std::cout << "The guard glances at you suspiciously but says nothing." << std::endl;
    }
}

int GuardEncounterNode::determineGuardResponse(GameContext* context)
{
    CrimeLawContext* lawContext = getLawContext(context);
    std::string region = getCurrentRegion(context);

    int bounty = lawContext->criminalRecord.getBounty(region);
    int criminalRep = lawContext->criminalRecord.getReputation(region);

    // Determine response based on bounty and reputation
    if (bounty > 1000 || criminalRep < -50) {
        return 1; // ATTACK for serious criminals
    } else if (bounty > 500 || criminalRep < -30) {
        return 0; // ARREST for moderate criminals
    } else if (bounty > 100) {
        return 2; // FINE for minor criminals
    } else {
        return 3; // WARN for petty criminals
    }
}

std::vector<TAAction> GuardEncounterNode::getAvailableActions()
{
    std::vector<TAAction> actions;

    actions.push_back({ "pay_bounty", "Pay bounty", []() -> TAInput {
                           return { "guard_response", { { "action", std::string("pay") } } };
                       } });

    actions.push_back({ "resist_arrest", "Resist arrest", []() -> TAInput {
                           return { "guard_response", { { "action", std::string("resist") } } };
                       } });

    actions.push_back({ "surrender", "Surrender", []() -> TAInput {
                           return { "guard_response", { { "action", std::string("surrender") } } };
                       } });

    actions.push_back({ "flee", "Attempt to flee", []() -> TAInput {
                           return { "guard_response", { { "action", std::string("flee") } } };
                       } });

    return actions;
}

bool GuardEncounterNode::evaluateTransition(const TAInput& input, TANode*& outNextNode)
{
    if (input.type == "guard_response") {
        std::string action = std::get<std::string>(input.parameters.at("action"));

        // Find the transition for this action
        for (const auto& rule : transitionRules) {
            if (rule.description.find(action) != std::string::npos) {
                outNextNode = rule.targetNode;
                return true;
            }
        }
    }

    return CrimeSystemNode::evaluateTransition(input, outNextNode);
}