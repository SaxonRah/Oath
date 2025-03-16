#pragma once

#include "../../core/TANode.hpp"
#include "SpellDesign.hpp"
#include <vector>

class SpellExaminationNode : public TANode {
public:
    std::vector<SpellDesign*>& knownSpells;
    std::vector<SpellDesign*> availableSpells;

    SpellExaminationNode(const std::string& name, std::vector<SpellDesign*>& playerSpells);

    void onEnter(GameContext* context) override;

    void listAvailableSpells();
    void examineSpell(int spellIndex);
    bool learnSpell(int spellIndex, GameContext* context);

    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;
};