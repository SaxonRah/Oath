#include "systems/religion/PrayerNode.hpp"
#include "core/TAAction.hpp"
#include "core/TAInput.hpp"
#include "systems/religion/DeityNode.hpp"
#include <iostream>

PrayerNode::PrayerNode(const std::string& name, const std::string& deity)
    : TANode(name)
    , deityId(deity)
{
    // Initialize prayer results for different favor levels
    initializePrayerResults();
}

void PrayerNode::initializePrayerResults()
{
    // For each prayer type, define results at different favor thresholds
    // Example for GUIDANCE prayers:
    prayerResults[GUIDANCE][75] = { // Exalted (75+ favor)
        "The deity speaks directly to your mind, offering crystal clear guidance.",
        { { "wisdom", 2 } },
        "divine_insight",
        24,
        false
    };

    prayerResults[GUIDANCE][50] = { // Honored (50+ favor)
        "A vision appears before you, showing a clear path forward.",
        { { "wisdom", 1 } },
        "spiritual_clarity",
        12,
        false
    };

    prayerResults[GUIDANCE][10] = { // Friendly (10+ favor)
        "You receive a vague but helpful nudge in the right direction.",
        {},
        "minor_insight",
        6,
        false
    };

    prayerResults[GUIDANCE][0] = { // Neutral (0+ favor)
        "Your prayer is acknowledged, but no clear answer comes.",
        {},
        "",
        0,
        false
    };

    prayerResults[GUIDANCE][-25] = { // Disliked or worse
        "Your prayer seems to echo emptily. No guidance comes.",
        {},
        "",
        0,
        false
    };

    // Similarly define results for other prayer types...
}

void PrayerNode::onEnter(ReligiousGameContext* context)
{
    std::cout << "=== Prayer to the ";

    if (context) {
        DeityNode* deity = findDeityNode(context);
        if (deity) {
            std::cout << deity->deityName << " ===" << std::endl;
        } else {
            std::cout << "Deity ===" << std::endl;
        }
    } else {
        std::cout << "Deity ===" << std::endl;
    }

    std::cout << prayerDescription << std::endl;

    if (context) {
        int favor = context->religiousStats.deityFavor[deityId];
        bool isPrimaryDeity = (context->religiousStats.primaryDeity == deityId);

        std::cout << "\nYour favor: " << favor;
        if (isPrimaryDeity) {
            std::cout << " (Primary Deity)";
        }
        std::cout << std::endl;

        // Check for holy day
        DeityNode* deity = findDeityNode(context);
        if (deity && deity->isHolyDay(context)) {
            std::cout << "Today is this deity's holy day. Your prayers will be more effective." << std::endl;
        }
    }

    std::cout << "\nWhat would you like to pray for?" << std::endl;
}

DeityNode* PrayerNode::findDeityNode(ReligiousGameContext* context) const
{
    // This would need to be implemented to find the deity node from the controller
    // For now, return nullptr as a placeholder
    return nullptr;
}

PrayerNode::PrayerResult PrayerNode::getPrayerOutcome(ReligiousGameContext* context, PrayerType type)
{
    if (!context)
        return {}; // Return empty result if no context

    int favor = context->religiousStats.deityFavor[deityId];
    bool isHolyDay = false;

    // Check for holy day bonus
    DeityNode* deity = findDeityNode(context);
    if (deity) {
        isHolyDay = deity->isHolyDay(context);
    }

    // Apply holy day bonus
    if (isHolyDay) {
        favor += 15; // Temporary bonus for outcome calculation
    }

    // Find the appropriate result based on favor
    if (favor >= 75)
        return prayerResults[type][75];
    if (favor >= 50)
        return prayerResults[type][50];
    if (favor >= 10)
        return prayerResults[type][10];
    if (favor >= 0)
        return prayerResults[type][0];
    return prayerResults[type][-25];
}

void PrayerNode::performPrayer(ReligiousGameContext* context, PrayerType type)
{
    if (!context)
        return;

    // Small devotion increase for praying
    context->religiousStats.addDevotion(deityId, 1);

    // Get prayer outcome
    PrayerResult result = getPrayerOutcome(context, type);

    // Display outcome
    std::cout << "\n"
              << result.description << std::endl;

    // Apply stat effects
    for (const auto& [stat, value] : result.statEffects) {
        if (stat == "strength")
            context->playerStats.strength += value;
        else if (stat == "dexterity")
            context->playerStats.dexterity += value;
        else if (stat == "constitution")
            context->playerStats.constitution += value;
        else if (stat == "intelligence")
            context->playerStats.intelligence += value;
        else if (stat == "wisdom")
            context->playerStats.wisdom += value;
        else if (stat == "charisma")
            context->playerStats.charisma += value;

        if (value != 0) {
            std::cout << "Your " << stat << " has " << (value > 0 ? "increased" : "decreased")
                      << " by " << std::abs(value) << "." << std::endl;
        }
    }

    // Grant blessing if applicable
    if (!result.blessingGranted.empty() && result.blessingDuration > 0) {
        context->religiousStats.addBlessing(result.blessingGranted, result.blessingDuration);
        std::cout << "You have received the " << result.blessingGranted << " blessing for "
                  << result.blessingDuration << " days." << std::endl;
    }

    // Remove curse if applicable
    if (result.curseRemoved) {
        // Would need curse system integration
        std::cout << "You feel a weight lift from your shoulders as a curse is removed." << std::endl;
    }
}

std::vector<TAAction> PrayerNode::getAvailableActions()
{
    std::vector<TAAction> actions = TANode::getAvailableActions();

    // Add prayer actions for different types
    actions.push_back({ "pray_guidance",
        "Pray for guidance",
        [this]() -> TAInput {
            return { "prayer_action", { { "action", std::string("pray") }, { "type", std::string("guidance") } } };
        } });

    actions.push_back({ "pray_blessing",
        "Pray for blessing",
        [this]() -> TAInput {
            return { "prayer_action", { { "action", std::string("pray") }, { "type", std::string("blessing") } } };
        } });

    actions.push_back({ "pray_protection",
        "Pray for protection",
        [this]() -> TAInput {
            return { "prayer_action", { { "action", std::string("pray") }, { "type", std::string("protection") } } };
        } });

    actions.push_back({ "pray_healing",
        "Pray for healing",
        [this]() -> TAInput {
            return { "prayer_action", { { "action", std::string("pray") }, { "type", std::string("healing") } } };
        } });

    actions.push_back({ "pray_strength",
        "Pray for strength",
        [this]() -> TAInput {
            return { "prayer_action", { { "action", std::string("pray") }, { "type", std::string("strength") } } };
        } });

    actions.push_back({ "pray_fortune",
        "Pray for fortune",
        [this]() -> TAInput {
            return { "prayer_action", { { "action", std::string("pray") }, { "type", std::string("fortune") } } };
        } });

    actions.push_back({ "pray_forgiveness",
        "Pray for forgiveness",
        [this]() -> TAInput {
            return { "prayer_action", { { "action", std::string("pray") }, { "type", std::string("forgiveness") } } };
        } });

    actions.push_back({ "return_to_deity",
        "Finish prayer",
        [this]() -> TAInput {
            return { "prayer_action", { { "action", std::string("finish") } } };
        } });

    return actions;
}

bool PrayerNode::evaluateTransition(const TAInput& input, TANode*& outNextNode)
{
    if (input.type == "prayer_action") {
        std::string action = std::get<std::string>(input.parameters.at("action"));

        if (action == "pray") {
            // Process prayer but stay on same node
            outNextNode = this;
            return true;
        } else if (action == "finish") {
            // Return to deity view
            for (const auto& rule : transitionRules) {
                if (rule.description == "Return to deity") {
                    outNextNode = rule.targetNode;
                    return true;
                }
            }
        }
    }

    return TANode::evaluateTransition(input, outNextNode);
}