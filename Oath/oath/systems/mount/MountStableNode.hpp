#pragma once

#include "../../core/TANode.hpp"
#include <string>

class MountStable;
class GameContext;
struct TAInput;
struct TAAction;

// Mount stable interaction node
class MountStableNode : public TANode {
public:
    MountStable* stable;

    MountStableNode(const std::string& name, MountStable* targetStable);
    void onEnter(GameContext* context) override;
    int calculatePrice(Mount* mount) const;
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;
};