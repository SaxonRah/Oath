// PickpocketNode.hpp
#pragma once

#include "../../core/TAAction.hpp"
#include "CrimeSystemNode.hpp"
#include <nlohmann/json.hpp>
#include <vector>

// Forward declarations
struct TAInput;
class TANode;

// Pickpocketing node - For stealing from NPCs
class PickpocketNode : public CrimeSystemNode {
public:
    PickpocketNode(const std::string& name);

    void onEnter(GameContext* context) override;
    std::vector<nlohmann::json> getPickpocketTargets(GameContext* context);
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;
    void attemptPickpocket(GameContext* context, const std::string& target, int difficulty);
};