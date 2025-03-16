// TheftNode.hpp
#pragma once

#include "../../core/TAAction.hpp"
#include "CrimeSystemNode.hpp"
#include <vector>

// Forward declarations
struct TAInput;
class TANode;

// Theft action node
class TheftNode : public CrimeSystemNode {
public:
    TheftNode(const std::string& name);

    void onEnter(GameContext* context) override;
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;
};