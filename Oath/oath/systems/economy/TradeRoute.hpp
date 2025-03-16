// systems/economy/TradeRoute.hpp
#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <vector>


using json = nlohmann::json;

// Trade route connecting two markets with specific goods
struct TradeRoute {
    std::string id;
    std::string name;
    std::string sourceMarket;
    std::string destinationMarket;
    std::vector<std::string> tradedGoods;
    float distance; // Affects transport cost and time
    float dangerLevel; // Affects risk of disruption (0.0 - 1.0)
    bool isActive; // Can be disrupted by events
    int travelDays; // Days required for goods to travel

    // Load from JSON
    static TradeRoute fromJson(const json& j);

    // Calculate transport cost multiplier
    float getTransportCostMultiplier() const;

    // Check if route is disrupted by random event
    bool checkDisruption(float randomValue) const;
};