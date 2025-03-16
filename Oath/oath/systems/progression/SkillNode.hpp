#pragma once

#include "../../core/TAAction.hpp"
#include "../../core/TAInput.hpp"
#include "../../core/TANode.hpp"
#include "../../data/GameContext.hpp"

#include <functional>
#include <string>
#include <vector>

// Forward declaration
class TANode;
struct TAAction;
struct TAInput;
struct GameContext;

// Skill node for character progression
class SkillNode : public TANode {
public:
    std::string skillName;
    std::string description;
    int level;
    int maxLevel;

    // Requirements to unlock this skill
    struct SkillRequirement {
        std::string type; // "skill", "item", "quest", etc.
        std::string target; // skill name, item id, quest id, etc.
        int level;

        bool check(const GameContext& context) const;
    };
    std::vector<SkillRequirement> requirements;

    // Effects when this skill is learned or improved
    struct SkillEffect {
        std::string type;
        std::string target;
        int value;

        void apply(GameContext* context) const;
    };
    std::vector<SkillEffect> effects;

    // Cost to learn this skill
    struct SkillCost {
        std::string type; // "points", "gold", "item", etc.
        std::string itemId; // if type is "item"
        int amount;

        bool canPay(const GameContext& context) const;
        void pay(GameContext* context) const;
    };
    std::vector<SkillCost> costs;

    SkillNode(const std::string& name, const std::string& skill,
        int initialLevel = 0, int max = 5);

    bool canLearn(const GameContext& context) const;
    void learnSkill(GameContext* context);
    void onEnter(GameContext* context) override;
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;
};