// systems/economy/TradeCommodity.cpp

#include "TradeCommodity.hpp"
#include <algorithm>
#include <cmath>

TradeCommodity TradeCommodity::fromJson(const json& j)
{
    TradeCommodity commodity;
    commodity.id = j["id"];
    commodity.name = j["name"];
    commodity.basePrice = j["basePrice"];
    commodity.currentPrice = j["basePrice"]; // Start at base price
    commodity.supply = j["supply"];
    commodity.demand = j["demand"];
    commodity.baseSupply = j["baseSupply"];
    commodity.baseDemand = j["baseDemand"];
    commodity.origin = j["origin"];
    commodity.isLuxury = j["isLuxury"];
    commodity.volatility = j["volatility"];

    return commodity;
}

void TradeCommodity::updatePrice()
{
    float supplyRatio = (supply > 0) ? (float)baseSupply / supply : 2.0f;
    float demandRatio = (float)demand / baseDemand;

    // Price is affected by supply/demand dynamics
    float marketFactor = demandRatio * supplyRatio;

    // Luxury goods have higher price elasticity
    if (isLuxury) {
        marketFactor = std::pow(marketFactor, 1.5);
    }

    // Calculate new price with limited volatility
    float targetPrice = basePrice * marketFactor;
    float maxChange = basePrice * volatility;

    // Limit price change to volatility
    if (targetPrice > currentPrice + maxChange) {
        currentPrice += maxChange;
    } else if (targetPrice < currentPrice - maxChange) {
        currentPrice -= maxChange;
    } else {
        currentPrice = targetPrice;
    }

    // Ensure price never goes below 10% of base
    currentPrice = std::max(currentPrice, basePrice * 0.1f);
}

void TradeCommodity::applyShock(float supplyShock, float demandShock)
{
    supply = std::max(1, (int)(supply * supplyShock));
    demand = std::max(1, (int)(demand * demandShock));
    updatePrice();
}