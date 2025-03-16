// systems/faction/FactionQuestNode.cpp
#include "FactionQuestNode.hpp"
#include "FactionSystemNode.hpp"
#include "core/TAController.hpp"
#include <iostream>

namespace oath {

FactionQuestNode::FactionQuestNode(const std::string& name, const std::string& faction)
    : QuestNode(name)
    , factionId(faction)
    , reputationReward(10)
    , rankAdvancement(false)
{
}

void FactionQuestNode::onExit(GameContext* context)
{
    // Call base class implementation for standard rewards
    QuestNode::onExit(context);

    // If this is a completed quest (accepting state), give faction rewards
    if (isAcceptingState && context) {
        // Update the faction system if found
        FactionSystemNode* factionSystem = findFactionSystem(context);
        if (factionSystem) {
            // Add this quest to completed quests list
            auto it = factionSystem->factions.find(factionId);
            if (it != factionSystem->factions.end()) {
                Faction& faction = it->second;
                faction.completedQuests.push_back(nodeName);

                // Apply reputation reward
                factionSystem->changePlayerReputation(factionId, reputationReward, context);

                // Apply reputation effects on other factions
                for (const auto& [otherFaction, amount] : reputationEffectsOnOtherFactions) {
                    factionSystem->changePlayerReputation(otherFaction, amount, context);
                }

                // Check for rank advancement if enabled
                if (rankAdvancement && faction.canAdvanceRank()) {
                    if (faction.tryAdvanceRank()) {
                        std::cout << "Your service has earned you promotion to "
                                  << faction.getCurrentRankTitle() << " in the "
                                  << faction.name << "." << std::endl;
                    }
                }
            }
        }
    }
}

FactionSystemNode* FactionQuestNode::findFactionSystem(GameContext* context)
{
    // This implementation would depend on how you access the faction system from the context
    // For example, you might get it from the controller using a system lookup
    if (context && context->controller) {
        return static_cast<FactionSystemNode*>(context->controller->getSystemNode("FactionSystem"));
    }
    return nullptr;
}

} // namespace oath