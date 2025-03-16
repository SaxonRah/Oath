#include "MountEquipmentShopNode.hpp"
#include "../../core/TAAction.hpp"
#include "../../core/TAInput.hpp"
#include "../../data/GameContext.hpp"
#include "MountEquipment.hpp"
#include <iostream>


MountEquipmentShopNode::MountEquipmentShopNode(const std::string& name, const std::string& shop)
    : TANode(name)
    , shopName(shop)
{
}

void MountEquipmentShopNode::onEnter(GameContext* context)
{
    std::cout << "Welcome to " << shopName << "!" << std::endl;
    std::cout << "We offer the finest equipment for your mount." << std::endl;

    if (availableEquipment.empty()) {
        std::cout << "Unfortunately, we're out of stock at the moment." << std::endl;
        return;
    }

    std::cout << "\nAvailable Equipment:" << std::endl;
    for (size_t i = 0; i < availableEquipment.size(); i++) {
        MountEquipment* equip = availableEquipment[i];
        std::cout << i + 1 << ". " << equip->name << " - " << equip->price << " gold" << std::endl;
        std::cout << "   " << equip->description << std::endl;

        // Show stat modifiers
        if (!equip->statModifiers.empty()) {
            std::cout << "   Effects: ";
            bool first = true;
            for (const auto& [stat, mod] : equip->statModifiers) {
                if (!first)
                    std::cout << ", ";
                std::cout << stat << " " << (mod >= 0 ? "+" : "") << mod;
                first = false;
            }
            std::cout << std::endl;
        }
    }
}

std::vector<TAAction> MountEquipmentShopNode::getAvailableActions()
{
    std::vector<TAAction> actions = TANode::getAvailableActions();

    // Buy equipment action
    if (!availableEquipment.empty()) {
        actions.push_back({ "buy_equipment", "Purchase equipment",
            [this]() -> TAInput {
                return { "shop_action", { { "action", std::string("buy") } } };
            } });
    }

    // Sell equipment action
    actions.push_back({ "sell_equipment", "Sell equipment",
        [this]() -> TAInput {
            return { "shop_action", { { "action", std::string("sell") } } };
        } });

    // Repair equipment action
    actions.push_back({ "repair_equipment", "Repair equipment",
        [this]() -> TAInput {
            return { "shop_action", { { "action", std::string("repair") } } };
        } });

    // Exit shop action
    actions.push_back({ "exit_shop", "Leave shop",
        [this]() -> TAInput {
            return { "shop_action", { { "action", std::string("exit") } } };
        } });

    return actions;
}

bool MountEquipmentShopNode::evaluateTransition(const TAInput& input, TANode*& outNextNode)
{
    if (input.type == "shop_action") {
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

        // Other actions would stay in same node
        outNextNode = this;
        return true;
    }

    return TANode::evaluateTransition(input, outNextNode);
}