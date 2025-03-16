// GuardEncounterNode.hpp
#pragma once

#include "../../core/TAAction.hpp"
#include "CrimeSystemNode.hpp"
#include <vector>

// Forward declarations
struct TAInput;
class TANode;

// Guard encounter node - handles guard interactions when wanted
class GuardEncounterNode : public CrimeSystemNode {
public:
    GuardEncounterNode(const std::string& name);

    void onEnter(GameContext* context) override;
    int determineGuardResponse(GameContext* context);
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;
};