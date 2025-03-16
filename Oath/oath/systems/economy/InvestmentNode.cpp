// systems/economy/InvestmentNode.cpp

#include "InvestmentNode.hpp"
#include "EconomicSystemNode.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>

InvestmentNode::InvestmentNode(const std::string& name, EconomicSystemNode* econSystem)
    : TANode(name)
    , economicSystem(econSystem)
{
    // Load investments from JSON if available
    loadInvestmentsFromJson();
}

void InvestmentNode::loadInvestmentsFromJson()
{
    try {
        // Open and parse the JSON file
        std::ifstream file("resources/json/economy.json");
        if (!file.is_open()) {
            std::cerr << "Failed to open economy.json for investments" << std::endl;
            return;
        }

        json data;
        file >> data;
        file.close();

        // Load investments
        if (data.contains("investments") && data["investments"].is_array()) {
            for (const auto& investmentData : data["investments"]) {
                investments.push_back(BusinessInvestment::fromJson(investmentData));
            }

            std::cout << "Loaded " << investments.size() << " investment opportunities from JSON." << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error loading investments from JSON: " << e.what() << std::endl;
    }
}

void InvestmentNode::onEnter(GameContext* context)
{
    std::cout << "Business Investment Management" << std::endl;
    std::cout << "-----------------------------" << std::endl;
    std::cout << "You can invest in businesses to generate passive income." << std::endl;
    std::cout << "The returns depend on the business type, risk level, and market conditions." << std::endl;

    displayInvestments();
}

std::vector<TAAction> InvestmentNode::getAvailableActions()
{
    std::vector<TAAction> actions;

    actions.push_back({ "view_investments", "View Your Investments",
        [this]() -> TAInput {
            return { "investment_action", { { "action", std::string("view_investments") } } };
        } });

    actions.push_back({ "view_opportunities", "View Investment Opportunities",
        [this]() -> TAInput {
            return { "investment_action", { { "action", std::string("view_opportunities") } } };
        } });

    actions.push_back({ "invest", "Invest in a Business",
        [this]() -> TAInput {
            return { "investment_action", { { "action", std::string("invest") } } };
        } });

    actions.push_back({ "collect_profits", "Collect Investment Profits",
        [this]() -> TAInput {
            return { "investment_action", { { "action", std::string("collect_profits") } } };
        } });

    actions.push_back({ "exit", "Exit Investment Manager",
        [this]() -> TAInput {
            return { "investment_action", { { "action", std::string("exit") } } };
        } });

    return actions;
}

bool InvestmentNode::evaluateTransition(const TAInput& input, TANode*& outNextNode)
{
    if (input.type == "investment_action") {
        std::string action = std::get<std::string>(input.parameters.at("action"));

        if (action == "view_investments") {
            displayInvestments();
            outNextNode = this;
            return true;
        } else if (action == "view_opportunities") {
            displayOpportunities();
            outNextNode = this;
            return true;
        } else if (action == "invest") {
            handleInvesting();
            outNextNode = this;
            return true;
        } else if (action == "collect_profits") {
            collectProfits();
            outNextNode = this;
            return true;
        } else if (action == "exit") {
            // Find and use an exit transition
            for (const auto& rule : transitionRules) {
                if (rule.description == "Exit") {
                    outNextNode = rule.targetNode;
                    return true;
                }
            }
            // Fallback
            outNextNode = this;
            return true;
        }
    }

    return TANode::evaluateTransition(input, outNextNode);
}

void InvestmentNode::addInvestmentOpportunity(const BusinessInvestment& investment)
{
    investments.push_back(investment);
}

void InvestmentNode::advanceDay(GameContext* context)
{
    if (!context)
        return;

    int totalProfit = 0;

    for (auto& investment : investments) {
        int profit = 0;

        if (investment.advanceDay(economicSystem->globalEconomicMultiplier, profit)) {
            std::cout << "Your investment in " << investment.name << " has generated "
                      << profit << " gold in profits!" << std::endl;
            totalProfit += profit;
        }
    }

    if (totalProfit > 0) {
        // Add gold to player inventory in a real implementation
        std::cout << "Total profit from all investments: " << totalProfit << " gold" << std::endl;
    }
}

void InvestmentNode::displayInvestments()
{
    bool hasInvestments = false;

    std::cout << "Your Active Investments:" << std::endl;
    std::cout << "-----------------------------------------------------------" << std::endl;
    std::cout << std::left << std::setw(25) << "Business"
              << std::setw(15) << "Investment"
              << std::setw(15) << "Risk Level"
              << std::setw(15) << "Next Payout" << std::endl;
    std::cout << "-----------------------------------------------------------" << std::endl;

    for (const auto& investment : investments) {
        if (investment.isActive) {
            hasInvestments = true;
            std::cout << std::left << std::setw(25) << investment.name
                      << std::setw(15) << investment.playerInvestment
                      << std::setw(15) << (investment.riskLevel < 0.3f ? "Low" : (investment.riskLevel < 0.7f ? "Medium" : "High"))
                      << std::setw(15) << (investment.payoutInterval - investment.daysSinceLastPayout)
                      << " days" << std::endl;
        }
    }

    if (!hasInvestments) {
        std::cout << "You don't have any active investments." << std::endl;
    }
}

void InvestmentNode::displayOpportunities()
{
    std::cout << "Available Investment Opportunities:" << std::endl;
    std::cout << "-----------------------------------------------------------" << std::endl;
    std::cout << std::left << std::setw(25) << "Business"
              << std::setw(15) << "Initial Cost"
              << std::setw(15) << "Risk Level"
              << std::setw(15) << "Return Rate" << std::endl;
    std::cout << "-----------------------------------------------------------" << std::endl;

    for (size_t i = 0; i < investments.size(); i++) {
        const auto& investment = investments[i];
        if (!investment.isActive) {
            std::cout << std::left << std::setw(25) << investment.name
                      << std::setw(15) << investment.initialCost
                      << std::setw(15) << (investment.riskLevel < 0.3f ? "Low" : (investment.riskLevel < 0.7f ? "Medium" : "High"))
                      << std::setw(15) << std::fixed << std::setprecision(1)
                      << (investment.returnRate * 100.0f) << "%" << std::endl;
        }
    }
}

void InvestmentNode::handleInvesting()
{
    // In a real implementation, this would select from available investments
    // and process the gold transaction

    std::cout << "Which business would you like to invest in? (Enter index or name)" << std::endl;

    // Example placeholder response
    std::cout << "You've invested in The Golden Goblet Tavern for 500 gold." << std::endl;
    std::cout << "You'll receive returns every 7 days." << std::endl;

    // For demonstration, activate the first inactive investment
    for (auto& investment : investments) {
        if (!investment.isActive) {
            investment.isActive = true;
            investment.playerInvestment = investment.initialCost;
            break;
        }
    }
}

void InvestmentNode::collectProfits()
{
    std::cout << "Your profits have been automatically added to your gold." << std::endl;
    std::cout << "The next payout will occur when the investment interval completes." << std::endl;
}