// systems/faction/FactionDialogueNode.cpp
#include "FactionDialogueNode.hpp"
#include "FactionSystemNode.hpp"
#include "core/TAController.hpp"
#include <iostream>

namespace oath {

FactionDialogueNode::FactionDialogueNode(const std::string& name, const std::string& speaker,
    const std::string& text, const std::string& faction,
    int repEffect)
    : DialogueNode(name, speaker, text)
    , factionId(faction)
    , reputationEffect(repEffect)
{
}

void FactionDialogueNode::onEnter(GameContext* context)
{
    // Call base implementation to display dialogue
    DialogueNode::onEnter(context);

    // Apply reputation effect if applicable
    if (!factionId.empty() && reputationEffect != 0 && context) {
        // Update the faction system if found
        FactionSystemNode* factionSystem = findFactionSystem(context);
        if (factionSystem) {
            factionSystem->changePlayerReputation(factionId, reputationEffect, context);
        }
    }
}

FactionSystemNode* FactionDialogueNode::findFactionSystem(GameContext* context)
{
    // This implementation would depend on how you access the faction system from the context
    // For example, you might get it from the controller using a system lookup
    if (context && context->controller) {
        return static_cast<FactionSystemNode*>(context->controller->getSystemNode("FactionSystem"));
    }
    return nullptr;
}

} // namespace oath