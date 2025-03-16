// systems/economy/EconomicSystemNode.hpp
#pragma once

#include "../../core/TAAction.hpp"
#include "../../core/TAInput.hpp"
#include "../../core/TANode.hpp"
#include "../../data/GameContext.hpp"
#include "EconomicEvent.hpp"
#include "Market.hpp"
#include "TradeRoute.hpp"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

using json = nlohmann::json;

// Economic system manager node that controls all markets and trade
class EconomicSystemNode : public TANode {
public:
    std::vector<Market*> markets;
    std::vector<TradeRoute> tradeRoutes;
    std::vector<EconomicEvent> activeEvents;
    std::vector<EconomicEvent> potentialEvents;
    int daysSinceLastEvent;
    float globalEconomicMultiplier;
    json configData; // Store the loaded JSON configuration

    EconomicSystemNode(const std::string& name);
    ~EconomicSystemNode();

    void loadConfigFromJson();
    void onEnter(GameContext* context) override;
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;

    // Create and register a new market
    Market* createMarket(const std::string& id, const std::string& name, MarketType type);

    // Add a trade route between markets
    void addTradeRoute(const std::string& id, const std::string& name,
        const std::string& sourceMarketId, const std::string& destMarketId,
        float distance, float danger);

    // Add a potential economic event
    void addPotentialEvent(const EconomicEvent& event);

    // Process market supplies based on trade routes
    void processTradeRoutes();

    // Process economic events
    void processEconomicEvents();

    // Simulate one economic day
    void simulateEconomicDay();

private:
    void displayMarkets();
    void displayTradeRoutes();
    void displayEconomicEvents();
    Market* findMarketById(const std::string& marketId);
};