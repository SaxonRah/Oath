// systems/economy/MarketNode.hpp
#pragma once

#include "../../core/TAAction.hpp"
#include "../../core/TAInput.hpp"
#include "../../core/TANode.hpp"
#include "../../data/GameContext.hpp"
#include "Market.hpp"
#include <string>
#include <vector>

// Node for representing a merchant market in the tree automata system
class MarketNode : public TANode {
public:
    Market* market;
    std::vector<std::string> availableDialogueOptions;

    MarketNode(const std::string& name, Market* linkedMarket);

    void onEnter(GameContext* context) override;
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;

private:
    void handleMarketOption(int option, TANode*& outNextNode);
    void displayBuyOptions();
    void displaySellOptions();
    void displayCommodities();
    void handleHaggle();
    void handleBuyItem(const std::string& itemId, int quantity);
    void handleSellItem(const std::string& itemId, int quantity);
};
