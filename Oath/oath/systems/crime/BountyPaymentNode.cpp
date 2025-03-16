// BountyPaymentNode.cpp
#include "BountyPaymentNode.hpp"
#include "../../core/TAInput.hpp"
#include "CrimeLawConfig.hpp"
#include <iostream>
#include <random>

BountyPaymentNode::BountyPaymentNode(const std::string& name)
    : CrimeSystemNode(name)
{
}

void BountyPaymentNode::onEnter(GameContext* context)
{
    CrimeLawContext* lawContext = getLawContext(context);
    std::string region = getCurrentRegion(context);

    int bounty = lawContext->criminalRecord.getBounty(region);

    std::cout << "You are at the bounty office in " << region << "." << std::endl;
    std::cout << "Your current bounty is " << bounty << " gold." << std::endl;

    // Show list of crimes
    auto crimes = lawContext->criminalRecord.getUnpaidCrimes(region);
    if (!crimes.empty()) {
        std::cout << "\nYour unpaid crimes in this region:" << std::endl;
        for (size_t i = 0; i < crimes.size(); i++) {
            std::cout << i + 1 << ". " << crimes[i].getDescription() << std::endl;
        }
    }
}

std::vector<TAAction> BountyPaymentNode::getAvailableActions()
{
    std::vector<TAAction> actions;

    actions.push_back({ "pay_full", "Pay full bounty", []() -> TAInput {
                           return { "bounty_action", { { "action", std::string("pay_full") } } };
                       } });

    actions.push_back({ "negotiate", "Attempt to negotiate", []() -> TAInput {
                           return { "bounty_action", { { "action", std::string("negotiate") } } };
                       } });

    actions.push_back({ "leave", "Leave bounty office", []() -> TAInput {
                           return { "bounty_action", { { "action", std::string("leave") } } };
                       } });

    return actions;
}

bool BountyPaymentNode::evaluateTransition(const TAInput& input, TANode*& outNextNode)
{
    if (input.type == "bounty_action") {
        std::string action = std::get<std::string>(input.parameters.at("action"));

        if (action == "pay_full") {
            bool paid = payFullBounty(nullptr); // Context would be passed in real implementation

            // Find the right transition
            for (const auto& rule : transitionRules) {
                if ((paid && rule.description.find("payment_success") != std::string::npos) || (!paid && rule.description.find("payment_failure") != std::string::npos)) {
                    outNextNode = rule.targetNode;
                    return true;
                }
            }
        } else if (action == "negotiate") {
            bool success = negotiateBounty(nullptr); // Context would be passed

            // Find the right transition
            for (const auto& rule : transitionRules) {
                if ((success && rule.description.find("negotiate_success") != std::string::npos) || (!success && rule.description.find("negotiate_failure") != std::string::npos)) {
                    outNextNode = rule.targetNode;
                    return true;
                }
            }
        } else if (action == "leave") {
            // Find the exit transition
            for (const auto& rule : transitionRules) {
                if (rule.description.find("leave") != std::string::npos) {
                    outNextNode = rule.targetNode;
                    return true;
                }
            }
        }
    }

    return CrimeSystemNode::evaluateTransition(input, outNextNode);
}

bool BountyPaymentNode::payFullBounty(GameContext* context)
{
    CrimeLawContext* lawContext = getLawContext(context);
    std::string region = getCurrentRegion(context);

    int bounty = lawContext->criminalRecord.getBounty(region);

    // Check if player has enough gold
    bool hasEnoughGold = true; // In real implementation, check player gold
    if (context) {
        // For example:
        // hasEnoughGold = (context->playerGold >= bounty);
    }

    if (hasEnoughGold) {
        // Deduct gold
        if (context) {
            // context->playerGold -= bounty;
        }

        // Clear bounty
        lawContext->criminalRecord.payBounty(region, bounty, context);

        std::cout << "You've paid your bounty of " << bounty << " gold." << std::endl;
        std::cout << "Your criminal record in " << region << " has been cleared." << std::endl;
        return true;
    } else {
        std::cout << "You don't have enough gold to pay your bounty." << std::endl;
        return false;
    }
}

bool BountyPaymentNode::negotiateBounty(GameContext* context)
{
    CrimeLawContext* lawContext = getLawContext(context);
    std::string region = getCurrentRegion(context);
    const auto& config = crimeLawConfig["bountyNegotiationConfig"];

    // Negotiate based on speech/charisma skill
    int negotiateChance = config["baseChance"].get<int>();

    if (context) {
        // Add speech skill bonus
        auto speechIt = context->playerStats.skills.find("speech");
        if (speechIt != context->playerStats.skills.end()) {
            negotiateChance += speechIt->second * config["speechMultiplier"].get<int>();
        }

        // Add charisma bonus
        negotiateChance += (context->playerStats.charisma - config["charismaBaseValue"].get<int>()) * config["charismaMultiplier"].get<int>();
    }

    // Criminal reputation affects negotiation
    int criminalRep = lawContext->criminalRecord.getReputation(region);
    negotiateChance += criminalRep / config["reputationDivisor"].get<int>(); // Better reputation helps

    // Ensure reasonable bounds
    negotiateChance = std::max(config["minChance"].get<int>(),
        std::min(negotiateChance, config["maxChance"].get<int>()));

    // Random check
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 100);

    bool success = dis(gen) <= negotiateChance;

    if (success) {
        int originalBounty = lawContext->criminalRecord.getBounty(region);
        int discountPercent = config["discountPercent"].get<int>();
        int discountedBounty = originalBounty * (100 - discountPercent) / 100;

        std::cout << "Through skillful negotiation, you've reduced your bounty from "
                  << originalBounty << " to " << discountedBounty << " gold." << std::endl;

        // Check if player has enough gold for discounted bounty
        bool hasEnoughGold = true; // In real implementation, check player gold
        if (context) {
            // For example:
            // hasEnoughGold = (context->playerGold >= discountedBounty);
        }

        if (hasEnoughGold) {
            // Deduct gold
            if (context) {
                // context->playerGold -= discountedBounty;
            }

            // Clear bounty
            lawContext->criminalRecord.payBounty(region, discountedBounty, context);

            std::cout << "You've paid your reduced bounty of " << discountedBounty << " gold." << std::endl;
            std::cout << "Your criminal record in " << region << " has been cleared." << std::endl;
            return true;
        } else {
            std::cout << "Even with the discount, you don't have enough gold." << std::endl;
            return false;
        }
    } else {
        std::cout << "The official isn't impressed by your negotiation attempt." << std::endl;
        std::cout << "\"Pay the full bounty or face the consequences!\"" << std::endl;
        return false;
    }
}