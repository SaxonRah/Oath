// JailNode.hpp
#pragma once

#include "../../core/TAAction.hpp"
#include "CrimeSystemNode.hpp"
#include <vector>

// Forward declarations
struct TAInput;
class TANode;

// Jail node - handles jail sentences
class JailNode : public CrimeSystemNode {
public:
    JailNode(const std::string& name);

    void onEnter(GameContext* context) override;
    void confiscateItems(GameContext* context);
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;
    void serveTime(GameContext* context);
    bool attemptEscape(GameContext* context);
};