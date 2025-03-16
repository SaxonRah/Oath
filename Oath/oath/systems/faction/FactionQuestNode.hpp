// systems/faction/FactionQuestNode.hpp
#ifndef OATH_FACTION_QUEST_NODE_HPP
#define OATH_FACTION_QUEST_NODE_HPP

#include "data/GameContext.hpp"
#include "systems/quest/QuestNode.hpp"


#include <map>
#include <string>


namespace oath {

// Faction-specific quest node
class FactionQuestNode : public QuestNode {
public:
    std::string factionId;
    int reputationReward;
    bool rankAdvancement;
    std::map<std::string, int> reputationEffectsOnOtherFactions;

    FactionQuestNode(const std::string& name, const std::string& faction);
    void onExit(GameContext* context) override;

private:
    // Helper function to find the faction system
    FactionSystemNode* findFactionSystem(GameContext* context);
};

} // namespace oath

#endif // OATH_FACTION_QUEST_NODE_HPP