// systems/economy/TradeCommodity.hpp
#pragma once

#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

// Trade good commodity that can be tracked for economic simulation
struct TradeCommodity {
    std::string id;
    std::string name;
    float basePrice;
    float currentPrice;
    int supply; // Local supply level
    int demand; // Local demand level
    int baseSupply; // Reference supply level
    int baseDemand; // Reference demand level
    std::string origin; // Primary region where this commodity is produced
    bool isLuxury; // Luxury goods have higher price elasticity
    float volatility; // How much prices fluctuate (0.0 - 1.0)

    // Load from JSON
    static TradeCommodity fromJson(const json& j);

    // Calculate price based on supply/demand ratios
    void updatePrice();

    // Apply a market shock (war, natural disaster, etc.)
    void applyShock(float supplyShock, float demandShock);
};
