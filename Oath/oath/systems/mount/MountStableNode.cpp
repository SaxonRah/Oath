#include "MountStableNode.hpp"
#include "../../core/TAAction.hpp"
#include "../../core/TAInput.hpp"
#include "../../data/GameContext.hpp"
#include "Mount.hpp"
#include "MountStable.hpp"
#include <iostream>


MountStableNode::MountStableNode(const std::string& name, MountStable* targetStable)
    : TANode(name)
    , stable(targetStable)
{
}

void MountStableNode::onEnter(GameContext* context)
{
    std::cout << "Welcome to " << stable->name << " Stables!" << std::endl;
    std::cout << "Currently housing " << stable->stabledMounts.size() << " of "
              << stable->capacity << " available stalls." << std::endl;

    // List stabled mounts
    if (!stable->stabledMounts.empty()) {
        std::cout << "\nYour stabled mounts:" << std::endl;
        for (size_t i = 0; i < stable->stabledMounts.size(); i++) {
            Mount* mount = stable->stabledMounts[i];
            std::cout << i + 1 << ". " << mount->getStateDescription() << std::endl;
        }
        std::cout << "\nDaily care cost: " << stable->getDailyCost() << " gold" << std::endl;
    }

    // List mounts for sale
    if (!stable->availableForPurchase.empty()) {
        std::cout << "\nMounts available for purchase:" << std::endl;
        for (size_t i = 0; i < stable->availableForPurchase.size(); i++) {
            Mount* mount = stable->availableForPurchase[i];
            int price = calculatePrice(mount);
            std::cout << i + 1 << ". " << mount->name << " (" << mount->breed->name
                      << ") - " << price << " gold" << std::endl;
        }
    }
}

int MountStableNode::calculatePrice(Mount* mount) const
{
    if (!mount)
        return 0;

    int basePrice = 200; // Base price for any mount

    // Adjust for breed
    if (mount->breed) {
        basePrice += (mount->breed->baseSpeed - 100) * 2;
        basePrice += (mount->breed->baseStamina - 100) * 2;
        basePrice += (mount->breed->baseTrainability - 50) * 3;

        // Premium for special abilities
        if (mount->breed->naturalSwimmer)
            basePrice += 100;
        if (mount->breed->naturalJumper)
            basePrice += 100;
        if (mount->breed->naturalClimber)
            basePrice += 200;
    }

    // Adjust for training level
    basePrice += mount->stats.training * 5;

    // Adjust for special abilities
    if (mount->stats.canJump)
        basePrice += 50;
    if (mount->stats.canSwim)
        basePrice += 50;
    if (mount->stats.canClimb)
        basePrice += 100;

    // Adjust for age (prime age is worth more)
    int ageModifier = 0;
    if (mount->age < 24) { // Young
        ageModifier = -50;
    } else if (mount->age > 120) { // Old
        ageModifier = -100;
    } else if (mount->age >= 36 && mount->age <= 84) { // Prime age
        ageModifier = 50;
    }
    basePrice += ageModifier;

    return basePrice;
}

std::vector<TAAction> MountStableNode::getAvailableActions()
{
    std::vector<TAAction> actions = TANode::getAvailableActions();

    // Add stable-specific actions

    // Retrieve a mount action
    if (!stable->stabledMounts.empty()) {
        actions.push_back({ "retrieve_mount", "Retrieve a mount from the stable",
            [this]() -> TAInput {
                return { "stable_action", { { "action", std::string("retrieve") } } };
            } });
    }

    // Buy a mount action
    if (!stable->availableForPurchase.empty()) {
        actions.push_back({ "buy_mount", "Purchase a new mount",
            [this]() -> TAInput {
                return { "stable_action", { { "action", std::string("buy") } } };
            } });
    }

    // Stable a mount action (if player has an active mount)
    actions.push_back({ "stable_mount", "Leave your mount at the stable",
        [this]() -> TAInput {
            return { "stable_action", { { "action", std::string("stable") } } };
        } });

    // Train mount action
    if (!stable->stabledMounts.empty()) {
        actions.push_back({ "train_mount", "Train one of your stabled mounts",
            [this]() -> TAInput {
                return { "stable_action", { { "action", std::string("train") } } };
            } });
    }

    // Exit action
    actions.push_back({ "exit_stable", "Leave the stable",
        [this]() -> TAInput {
            return { "stable_action", { { "action", std::string("exit") } } };
        } });

    return actions;
}

bool MountStableNode::evaluateTransition(const TAInput& input, TANode*& outNextNode)
{
    if (input.type == "stable_action") {
        std::string action = std::get<std::string>(input.parameters.at("action"));

        // Handle different stable actions
        if (action == "exit") {
            // Find the exit transition
            for (const auto& rule : transitionRules) {
                if (rule.description == "Exit") {
                    outNextNode = rule.targetNode;
                    return true;
                }
            }
        }
        // Other actions would be processed in the game logic
        // and would likely stay in the same node
        outNextNode = this;
        return true;
    }

    return TANode::evaluateTransition(input, outNextNode);
}