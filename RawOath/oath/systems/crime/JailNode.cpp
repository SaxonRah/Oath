// JailNode.cpp
#include "JailNode.hpp"
#include "../../core/TAInput.hpp"
#include "CrimeLawConfig.hpp"
#include "CrimeType.hpp"
#include <iostream>
#include <random>


JailNode::JailNode(const std::string& name)
    : CrimeSystemNode(name)
{
}

void JailNode::onEnter(GameContext* context)
{
    CrimeLawContext* lawContext = getLawContext(context);
    std::string region = getCurrentRegion(context);

    // Calculate sentence if not already set
    if (lawContext->jailSentencesByRegion[region] <= 0) {
        lawContext->jailSentencesByRegion[region] = lawContext->calculateJailSentence(region);
    }

    int sentence = lawContext->jailSentencesByRegion[region];

    // Setup jail state
    lawContext->currentJailRegion = region;
    lawContext->currentJailDays = sentence;
    lawContext->inJail = true;

    // Confiscate stolen items
    confiscateItems(context);

    std::cout << "You've been thrown in jail in " << region << " for " << sentence << " days." << std::endl;
    std::cout << "Your stolen items have been confiscated." << std::endl;
}

void JailNode::confiscateItems(GameContext* context)
{
    if (!context)
        return;

    CrimeLawContext* lawContext = getLawContext(context);
    lawContext->confiscatedItems = Inventory(); // Clear previous

    // Copy stolen items to confiscated inventory and remove from player
    std::vector<Item> stolenItems;
    for (const auto& item : context->playerInventory.items) {
        if (item.type == "stolen") {
            lawContext->confiscatedItems.addItem(item);
            stolenItems.push_back(item);
        }
    }

    // Remove from player inventory
    for (const auto& item : stolenItems) {
        context->playerInventory.removeItem(item.id, item.quantity);
    }
}

std::vector<TAAction> JailNode::getAvailableActions()
{
    std::vector<TAAction> actions;

    actions.push_back({ "serve_time", "Serve your sentence", []() -> TAInput {
                           return { "jail_action", { { "action", std::string("serve") } } };
                       } });

    actions.push_back({ "attempt_escape", "Attempt to escape", []() -> TAInput {
                           return { "jail_action", { { "action", std::string("escape") } } };
                       } });

    return actions;
}

bool JailNode::evaluateTransition(const TAInput& input, TANode*& outNextNode)
{
    if (input.type == "jail_action") {
        std::string action = std::get<std::string>(input.parameters.at("action"));

        if (action == "serve") {
            serveTime(nullptr); // Context not needed here

            // Find the transition for serving time
            for (const auto& rule : transitionRules) {
                if (rule.description.find("serve") != std::string::npos) {
                    outNextNode = rule.targetNode;
                    return true;
                }
            }
        } else if (action == "escape") {
            bool escaped = attemptEscape(nullptr); // Context not needed

            // Find appropriate transition based on escape success
            for (const auto& rule : transitionRules) {
                if (escaped && rule.description.find("escape_success") != std::string::npos) {
                    outNextNode = rule.targetNode;
                    return true;
                } else if (!escaped && rule.description.find("escape_failure") != std::string::npos) {
                    outNextNode = rule.targetNode;
                    return true;
                }
            }
        }
    }

    return CrimeSystemNode::evaluateTransition(input, outNextNode);
}

void JailNode::serveTime(GameContext* context)
{
    CrimeLawContext* lawContext = getLawContext(context);

    // Skip time forward
    if (context) {
        // This would integrate with your time system
        // For example, advancing the day by the jail sentence
        // context->worldState.advanceDay(lawContext->currentJailDays);
        std::cout << "Time passes... " << lawContext->currentJailDays << " days later." << std::endl;
    }

    // Clear criminal record for this region
    lawContext->criminalRecord.serveJailSentence(lawContext->currentJailRegion, lawContext->currentJailDays);

    // Reset jail state
    lawContext->inJail = false;
    lawContext->currentJailDays = 0;
    lawContext->jailSentencesByRegion[lawContext->currentJailRegion] = 0;

    std::cout << "You've served your sentence and are released from jail." << std::endl;
    std::cout << "Your criminal record in " << lawContext->currentJailRegion << " has been cleared." << std::endl;
}

bool JailNode::attemptEscape(GameContext* context)
{
    // Chance to escape based on config and skills
    const auto& config = crimeLawConfig["jailEscapeConfig"];
    int escapeChance = config["baseChance"].get<int>();

    if (context) {
        // Add stealth skill bonus
        auto stealthIt = context->playerStats.skills.find("stealth");
        if (stealthIt != context->playerStats.skills.end()) {
            escapeChance += stealthIt->second * config["stealthMultiplier"].get<int>();
        }

        // Add lockpicking skill bonus
        auto lockpickIt = context->playerStats.skills.find("lockpicking");
        if (lockpickIt != context->playerStats.skills.end()) {
            escapeChance += lockpickIt->second * config["lockpickingMultiplier"].get<int>();
        }
    }

    // Longer sentences are harder to escape from
    CrimeLawContext* lawContext = getLawContext(context);
    escapeChance -= lawContext->currentJailDays / config["sentencePenaltyDivisor"].get<int>();

    // Ensure reasonable bounds
    escapeChance = std::max(config["minChance"].get<int>(),
        std::min(escapeChance, config["maxChance"].get<int>()));

    // Random check
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 100);

    bool escaped = dis(gen) <= escapeChance;

    if (escaped) {
        std::cout << "You've successfully escaped from jail!" << std::endl;

        // Update record - escaping increases bounty and makes guards more aggressive
        CrimeLawContext* lawContext = getLawContext(context);

        // Add prison break crime
        CrimeRecord breakout(CrimeType::PRISON_BREAK(), lawContext->currentJailRegion, "Jail", true, 8);
        lawContext->criminalRecord.addCrime(breakout);

        // Reset jail state but keep criminal record
        lawContext->inJail = false;
        lawContext->currentJailDays = 0;

        // Guards will now be more aggressive
        lawContext->guardSuspicion[lawContext->currentJailRegion] = 100;
    } else {
        std::cout << "Your escape attempt has failed! The guards caught you." << std::endl;

        // Increase sentence for attempted escape
        CrimeLawContext* lawContext = getLawContext(context);
        int sentenceIncrease = config["sentenceIncrease"].get<int>();
        lawContext->currentJailDays += sentenceIncrease;
        lawContext->jailSentencesByRegion[lawContext->currentJailRegion] += sentenceIncrease;

        std::cout << "Your sentence has been extended by " << sentenceIncrease << " days." << std::endl;
    }

    return escaped;
}