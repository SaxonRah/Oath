// systems/economy/TradeRoute.cpp

#include "TradeRoute.hpp"
#include <algorithm>

TradeRoute TradeRoute::fromJson(const json& j)
{
    TradeRoute route;
    route.id = j["id"];
    route.name = j["name"];
    route.sourceMarket = j["sourceMarket"];
    route.destinationMarket = j["destinationMarket"];
    route.distance = j["distance"];
    route.dangerLevel = j["dangerLevel"];
    route.isActive = true; // Start active
    route.travelDays = std::max(1, (int)(route.distance / 20.0f)); // 20 distance units per day

    if (j.contains("tradedGoods") && j["tradedGoods"].is_array()) {
        for (const auto& good : j["tradedGoods"]) {
            route.tradedGoods.push_back(good);
        }
    }

    return route;
}

float TradeRoute::getTransportCostMultiplier() const
{
    return 1.0f + (distance * 0.01f) + (dangerLevel * 0.2f);
}

bool TradeRoute::checkDisruption(float randomValue) const
{
    return randomValue < dangerLevel * 0.1f;
}
