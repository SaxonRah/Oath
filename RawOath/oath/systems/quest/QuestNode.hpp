#pragma once

#include "../../core/TAAction.hpp"
#include "../../core/TANode.hpp"
#include "../../data/GameContext.hpp"

#include <functional>
#include <string>
#include <vector>

// Forward declaration
class TANode;
struct TAAction;
struct GameContext;

// Quest-specific node implementation
class QuestNode : public TANode {
public:
    // Quest state (Available, Active, Completed, Failed)
    std::string questState;

    // Quest details
    std::string questTitle;
    std::string questDescription;

    // Rewards for completion
    struct QuestReward {
        std::string type;
        int amount;
        std::string itemId;
    };
    std::vector<QuestReward> rewards;

    // Requirements to access this quest
    struct QuestRequirement {
        std::string type; // skill, item, faction, etc.
        std::string target; // skill name, item id, faction name, etc.
        int value; // required value

        bool check(const GameContext& context) const;
    };
    std::vector<QuestRequirement> requirements;

    QuestNode(const std::string& name);

    // Process player action and return next state
    bool processAction(const std::string& playerAction, TANode*& outNextNode);

    // Check if player can access this quest
    bool canAccess(const GameContext& context) const;

    // Activate child quests when this node is entered
    void onEnter(GameContext* context) override;

    // Award rewards when completing quest
    void onExit(GameContext* context) override;

    // Get available actions specific to quests
    std::vector<TAAction> getAvailableActions() override;
};