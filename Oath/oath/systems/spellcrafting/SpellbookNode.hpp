#pragma once

#include "../../core/TANode.hpp"
#include "SpellDesign.hpp"
#include <vector>

class SpellbookNode : public TANode {
public:
    std::vector<SpellDesign*>& knownSpells;

    SpellbookNode(const std::string& name, std::vector<SpellDesign*>& playerSpells);

    void onEnter(GameContext* context) override;

    void listSpells();
    void examineSpell(int spellIndex);
    void toggleFavorite(int spellIndex);
    bool castSpell(int spellIndex, GameContext* context);

    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;
};