// systems/economy/BusinessInvestment.hpp
#pragma once

#include <nlohmann/json.hpp>
#include <random>
#include <string>


using json = nlohmann::json;

// Investment opportunity in a business
struct BusinessInvestment {
    std::string id;
    std::string name;
    std::string description;
    std::string marketId; // Market where this business is located
    int initialCost; // Cost to invest
    float riskLevel; // 0.0 to 1.0 (higher = riskier)
    float returnRate; // Base return rate (higher for riskier investments)
    float marketSensitivity; // How much market conditions affect this business (0.0 to 2.0)
    int playerInvestment; // Amount player has invested
    int daysSinceLastPayout; // Days since last profit payout
    int payoutInterval; // Days between profit payouts
    bool isActive; // If the player has invested

    BusinessInvestment();

    // Load from JSON
    static BusinessInvestment fromJson(const json& j);

    // Calculate expected profit
    int calculateExpectedProfit(float marketMultiplier) const;

    // Calculate actual profit (with random variation)
    int calculateActualProfit(float marketMultiplier) const;

    // Process a day passing
    bool advanceDay(float marketMultiplier, int& profit);
};
