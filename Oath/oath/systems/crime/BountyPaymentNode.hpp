// BountyPaymentNode.hpp
#pragma once

#include "../../core/TAAction.hpp"
#include "CrimeSystemNode.hpp"
#include <vector>

// Forward declarations
struct TAInput;
class TANode;

// Bounty payment node
class BountyPaymentNode : public CrimeSystemNode {
public:
    BountyPaymentNode(const std::string& name);

    void onEnter(GameContext* context) override;
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;
    bool payFullBounty(GameContext* context);
    bool negotiateBounty(GameContext* context);
};