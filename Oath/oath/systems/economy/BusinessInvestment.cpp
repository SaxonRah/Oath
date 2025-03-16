// systems/economy/BusinessInvestment.cpp

#include "BusinessInvestment.hpp"
#include <algorithm>

BusinessInvestment::BusinessInvestment()
    : initialCost(100)
    , riskLevel(0.5f)
    , returnRate(0.05f)
    , marketSensitivity(1.0f)
    , playerInvestment(0)
    , daysSinceLastPayout(0)
    , payoutInterval(7)
    , isActive(false)
{
}

BusinessInvestment BusinessInvestment::fromJson(const json& j)
{
    BusinessInvestment investment;
    investment.id = j["id"];
    investment.name = j["name"];
    investment.description = j["description"];
    investment.marketId = j["marketId"];
    investment.initialCost = j["initialCost"];
    investment.riskLevel = j["riskLevel"];
    investment.returnRate = j["returnRate"];
    investment.marketSensitivity = j["marketSensitivity"];
    investment.payoutInterval = j["payoutInterval"];

    return investment;
}

int BusinessInvestment::calculateExpectedProfit(float marketMultiplier) const
{
    float effectiveRate = returnRate * (1.0f + (marketMultiplier - 1.0f) * marketSensitivity);
    return (int)(playerInvestment * effectiveRate);
}

int BusinessInvestment::calculateActualProfit(float marketMultiplier) const
{
    int expectedProfit = calculateExpectedProfit(marketMultiplier);

    // Add random variation based on risk
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::normal_distribution<float> variation(1.0f, riskLevel * 0.5f);

    float randomFactor = variation(gen);
    randomFactor = std::max(0.1f, randomFactor); // Never lose more than 90%

    return (int)(expectedProfit * randomFactor);
}

bool BusinessInvestment::advanceDay(float marketMultiplier, int& profit)
{
    if (!isActive)
        return false;

    daysSinceLastPayout++;

    if (daysSinceLastPayout >= payoutInterval) {
        profit = calculateActualProfit(marketMultiplier);
        daysSinceLastPayout = 0;
        return true;
    }

    return false;
}
