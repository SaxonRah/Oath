#pragma once

#include "../../core/TANode.hpp"
#include "../../data/GameContext.hpp"

#include <string>
#include <vector>

class QuestNode : public TANode {
public:
    // Quest state
    std::string questState;

    // Quest details
    std::string questTitle;
    std::string questDescription;

    // Reward structure
    struct QuestReward {
        std::string type;
        int amount;
        std::string itemId;
    };
    std::vector<QuestReward> rewards;

    // Requirement structure
    struct QuestRequirement {
        std::string type;
        std::string target;
        int value;

        bool check(const GameContext& context) const;
    };
    std::vector<QuestRequirement> requirements;

    // Methods
    QuestNode(const std::string& name);
    // ...other methods as in original implementation
};