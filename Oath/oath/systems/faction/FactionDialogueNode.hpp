// systems/faction/FactionDialogueNode.hpp
#ifndef OATH_FACTION_DIALOGUE_NODE_HPP
#define OATH_FACTION_DIALOGUE_NODE_HPP

#include "data/GameContext.hpp"
#include "systems/dialogue/DialogueNode.hpp"


#include <string>

namespace oath {

// Faction-related dialogue node
class FactionDialogueNode : public DialogueNode {
public:
    std::string factionId;
    int reputationEffect;

    FactionDialogueNode(const std::string& name, const std::string& speaker,
        const std::string& text, const std::string& faction = "",
        int repEffect = 0);
    void onEnter(GameContext* context) override;

private:
    // Helper function to find the faction system
    FactionSystemNode* findFactionSystem(GameContext* context);
};

} // namespace oath

#endif // OATH_FACTION_DIALOGUE_NODE_HPP