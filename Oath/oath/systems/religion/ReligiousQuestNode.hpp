#pragma once

#include "systems/quest/QuestNode.hpp"
#include "systems/religion/ReligiousGameContext.hpp"
#include <map>
#include <string>

// Forward declaration
class DeityNode;

class ReligiousQuestNode : public QuestNode {
public:
    std::string deityId;
    int favorReward;
    int devotionReward;

    // Effects on the world when completed
    std::map<std::string, std::string> worldStateChanges;

    ReligiousQuestNode(const std::string& name, const std::string& deity);
    void loadFromJson(const nlohmann::json& questData);
    void onEnter(GameContext* baseContext) override;
    void onExit(GameContext* baseContext) override;
    DeityNode* findDeityNode(ReligiousGameContext* context) const;
};