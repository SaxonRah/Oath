// systems/economy/MarketNode.cpp

#include "MarketNode.hpp"
#include <iomanip>
#include <iostream>
#include <random>


MarketNode::MarketNode(const std::string& name, Market* linkedMarket)
    : TANode(name)
    , market(linkedMarket)
{
    if (market) {
        availableDialogueOptions = {
            "Buy Items",
            "Sell Items",
            "Browse Commodities",
            "Haggle",
            "Exit"
        };
    }
}

void MarketNode::onEnter(GameContext* context)
{
    std::cout << "Entered " << market->name << " (" << marketTypeToString(market->type) << ")" << std::endl;
    std::cout << "Shopkeeper: " << market->ownerName << std::endl;

    // Different greetings based on relation
    float relation = market->getRelationToPlayer();
    if (relation > 50.0f) {
        std::cout << "\"Ah, my favorite customer! Welcome back! What can I get for you today?\"" << std::endl;
    } else if (relation > 0.0f) {
        std::cout << "\"Welcome to my shop. Feel free to browse my wares.\"" << std::endl;
    } else if (relation > -50.0f) {
        std::cout << "\"What do you want? I have goods to sell if you have coin.\"" << std::endl;
    } else {
        std::cout << "\"I'll do business with you, but don't expect any special treatment.\"" << std::endl;
    }

    // Show available options
    std::cout << "\nAvailable options:" << std::endl;
    for (size_t i = 0; i < availableDialogueOptions.size(); i++) {
        std::cout << (i + 1) << ". " << availableDialogueOptions[i] << std::endl;
    }
}

std::vector<TAAction> MarketNode::getAvailableActions()
{
    std::vector<TAAction> actions;

    for (size_t i = 0; i < availableDialogueOptions.size(); i++) {
        actions.push_back({ "market_option_" + std::to_string(i),
            availableDialogueOptions[i],
            [this, i]() -> TAInput {
                return { "market_action", { { "action", std::string("option") }, { "index", static_cast<int>(i) } } };
            } });
    }

    return actions;
}

bool MarketNode::evaluateTransition(const TAInput& input, TANode*& outNextNode)
{
    if (input.type == "market_action") {
        std::string action = std::get<std::string>(input.parameters.at("action"));

        if (action == "option") {
            int index = std::get<int>(input.parameters.at("index"));

            if (index >= 0 && index < static_cast<int>(availableDialogueOptions.size())) {
                handleMarketOption(index, outNextNode);
                return true;
            }
        } else if (action == "buy") {
            // Handle buying specific item
            std::string itemId = std::get<std::string>(input.parameters.at("item_id"));
            int quantity = std::get<int>(input.parameters.at("quantity"));
            handleBuyItem(itemId, quantity);
            outNextNode = this; // Stay in market
            return true;
        } else if (action == "sell") {
            // Handle selling specific item
            std::string itemId = std::get<std::string>(input.parameters.at("item_id"));
            int quantity = std::get<int>(input.parameters.at("quantity"));
            handleSellItem(itemId, quantity);
            outNextNode = this; // Stay in market
            return true;
        } else if (action == "haggle") {
            // Handle haggling
            handleHaggle();
            outNextNode = this; // Stay in market
            return true;
        } else if (action == "exit") {
            // Find and use an exit transition if available
            for (const auto& rule : transitionRules) {
                if (rule.description == "Exit") {
                    outNextNode = rule.targetNode;
                    return true;
                }
            }
        }
    }

    return TANode::evaluateTransition(input, outNextNode);
}

void MarketNode::handleMarketOption(int option, TANode*& outNextNode)
{
    std::string optionName = availableDialogueOptions[option];

    if (optionName == "Buy Items") {
        displayBuyOptions();
        outNextNode = this; // Stay in shop
    } else if (optionName == "Sell Items") {
        displaySellOptions();
        outNextNode = this; // Stay in shop
    } else if (optionName == "Browse Commodities") {
        displayCommodities();
        outNextNode = this; // Stay in shop
    } else if (optionName == "Haggle") {
        handleHaggle();
        outNextNode = this; // Stay in shop
    } else if (optionName == "Exit") {
        std::cout << "Shopkeeper: \"Come back again!\"" << std::endl;
        // Find and use an exit transition
        for (const auto& rule : transitionRules) {
            if (rule.description == "Exit") {
                outNextNode = rule.targetNode;
                return;
            }
        }
        // Fallback if no exit transition found
        outNextNode = this;
    }
}

void MarketNode::displayBuyOptions()
{
    std::cout << "Items for sale:" << std::endl;
    std::cout << "------------------------------------------" << std::endl;
    std::cout << std::left << std::setw(30) << "Item" << std::setw(10) << "Price" << std::setw(10) << "Quantity" << std::endl;
    std::cout << "------------------------------------------" << std::endl;

    // Show merchant inventory
    for (size_t i = 0; i < market->inventory.items.size(); i++) {
        const auto& item = market->inventory.items[i];
        std::cout << std::left << std::setw(30) << item.name
                  << std::setw(10) << market->calculateBuyPrice(item, 50) // Assuming barter skill of 50
                  << std::setw(10) << item.quantity << std::endl;
    }

    std::cout << "\nTo buy an item, use the 'buy [item_name] [quantity]' command." << std::endl;
}

void MarketNode::displaySellOptions()
{
    // TODO: In a real implementation, this would access the player inventory from the game context
    std::cout << "Your inventory items that this merchant might buy:" << std::endl;
    std::cout << "------------------------------------------" << std::endl;
    std::cout << std::left << std::setw(30) << "Item" << std::setw(10) << "Value" << std::setw(10) << "Quantity" << std::endl;
    std::cout << "------------------------------------------" << std::endl;

    // Placeholder for player inventory display
    std::cout << std::left << std::setw(30) << "Iron Dagger"
              << std::setw(10) << "15"
              << std::setw(10) << "1" << std::endl;

    std::cout << std::left << std::setw(30) << "Leather Strips"
              << std::setw(10) << "2"
              << std::setw(10) << "5" << std::endl;

    std::cout << "\nTo sell an item, use the 'sell [item_name] [quantity]' command." << std::endl;
}

void MarketNode::displayCommodities()
{
    std::cout << "Trade commodities at " << market->name << ":" << std::endl;
    std::cout << "------------------------------------------" << std::endl;
    std::cout << std::left << std::setw(20) << "Commodity"
              << std::setw(10) << "Price"
              << std::setw(10) << "Supply"
              << std::setw(10) << "Demand" << std::endl;
    std::cout << "------------------------------------------" << std::endl;

    for (const auto& commodity : market->commodities) {
        std::cout << std::left << std::setw(20) << commodity.name
                  << std::setw(10) << std::fixed << std::setprecision(2) << commodity.currentPrice
                  << std::setw(10) << commodity.supply
                  << std::setw(10) << commodity.demand << std::endl;
    }

    std::cout << "\nCommodity prices change based on supply and demand, and can be affected by trade routes and economic events." << std::endl;
}

void MarketNode::handleHaggle()
{
    // TODO: In a real implementation, this would check player skills from the game context
    int persuasionSkill = 50; // Placeholder value
    float successChance = (float)persuasionSkill / (persuasionSkill + market->haggleSkillLevel);

    std::cout << "You attempt to haggle with " << market->ownerName << "..." << std::endl;

    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    if (dist(gen) < successChance) {
        // Success
        std::cout << "Your haggling was successful! Prices are temporarily improved." << std::endl;
        market->improveRelation(2.0f);
        // This would set a temporary price modifier in the actual implementation
    } else {
        // Failure
        std::cout << "The merchant is not impressed with your haggling attempt." << std::endl;
        market->worsenRelation(1.0f);
    }
}

void MarketNode::handleBuyItem(const std::string& itemId, int quantity)
{
    // This is a simplified version - a real implementation would:
    // 1. Find the item in the merchant's inventory
    // 2. Calculate total cost based on player's barter skill
    // 3. Check if player has enough gold
    // 4. Transfer the item from merchant to player
    // 5. Transfer gold from player to merchant

    std::cout << "You bought " << quantity << "x " << itemId << "." << std::endl;
    std::cout << "This would normally transfer the item and deduct gold." << std::endl;
}

void MarketNode::handleSellItem(const std::string& itemId, int quantity)
{
    // This is a simplified version - a real implementation would:
    // 1. Find the item in the player's inventory
    // 2. Calculate total value based on player's barter skill
    // 3. Transfer the item from player to merchant
    // 4. Transfer gold from merchant to player
    // 5. Record the item as player-sold for potential buyback

    std::cout << "You sold " << quantity << "x " << itemId << "." << std::endl;
    std::cout << "This would normally transfer the item and add gold." << std::endl;

    // Record for buyback
    market->recordPlayerSoldItem(itemId, quantity);
}
