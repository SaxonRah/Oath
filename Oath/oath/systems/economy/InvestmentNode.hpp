// systems/economy/InvestmentNode.hpp
#pragma once

#include "../../core/TAAction.hpp"
#include "../../core/TAInput.hpp"
#include "../../core/TANode.hpp"
#include "../../data/GameContext.hpp"
#include "BusinessInvestment.hpp"
#include <string>
#include <vector>


// Forward declaration
class EconomicSystemNode;

// Node for managing business investments
class InvestmentNode : public TANode {
public:
    std::vector<BusinessInvestment> investments;
    EconomicSystemNode* economicSystem;

    InvestmentNode(const std::string& name, EconomicSystemNode* econSystem);

    void loadInvestmentsFromJson();
    void onEnter(GameContext* context) override;
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;

    // Add a new investment opportunity
    void addInvestmentOpportunity(const BusinessInvestment& investment);

    // Process a day passing for all investments
    void advanceDay(GameContext* context);

private:
    void displayInvestments();
    void displayOpportunities();
    void handleInvesting();
    void collectProfits();
};
