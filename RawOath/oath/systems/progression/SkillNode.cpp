#include <SkillNode.hpp>

bool SkillNode::SkillRequirement::check(const GameContext& context) const
{
    if (type == "skill") {
        return context.playerStats.hasSkill(target, level);
    } else if (type == "item") {
        return context.playerInventory.hasItem(target, level);
    } else if (type == "knowledge") {
        return context.playerStats.hasKnowledge(target);
    }
    return false;
}

void SkillNode::SkillEffect::apply(GameContext* context) const
{
    if (!context)
        return;

    if (type == "stat") {
        if (target == "strength")
            context->playerStats.strength += value;
        else if (target == "dexterity")
            context->playerStats.dexterity += value;
        else if (target == "constitution")
            context->playerStats.constitution += value;
        else if (target == "intelligence")
            context->playerStats.intelligence += value;
        else if (target == "wisdom")
            context->playerStats.wisdom += value;
        else if (target == "charisma")
            context->playerStats.charisma += value;
    } else if (type == "skill") {
        context->playerStats.improveSkill(target, value);
    } else if (type == "ability") {
        context->playerStats.unlockAbility(target);
    }
}

bool SkillNode::SkillCost::canPay(const GameContext& context) const
{
    if (type == "item") {
        return context.playerInventory.hasItem(itemId, amount);
    }
    // Other types would be checked here
    return true;
}

void SkillNode::SkillCost::pay(GameContext* context) const
{
    if (!context)
        return;

    if (type == "item") {
        context->playerInventory.removeItem(itemId, amount);
    }
    // Other payment types would be handled here
}

SkillNode::SkillNode(const std::string& name, const std::string& skill,
    int initialLevel, int max)
    : TANode(name)
    , skillName(skill)
    , description("")
    , level(initialLevel)
    , maxLevel(max)
{
}

bool SkillNode::canLearn(const GameContext& context) const
{
    // Check all requirements
    for (const auto& req : requirements) {
        if (!req.check(context)) {
            return false;
        }
    }

    // Check costs
    for (const auto& cost : costs) {
        if (!cost.canPay(context)) {
            return false;
        }
    }

    return level < maxLevel;
}

void SkillNode::learnSkill(GameContext* context)
{
    if (!context || !canLearn(*context)) {
        return;
    }

    // Pay costs
    for (const auto& cost : costs) {
        cost.pay(context);
    }

    // Apply effects
    for (const auto& effect : effects) {
        effect.apply(context);
    }

    // Increase level
    level++;

    // Update player stats
    context->playerStats.improveSkill(skillName, 1);

    // If reached max level, mark as accepting state
    if (level >= maxLevel) {
        isAcceptingState = true;
    }

    std::cout << "Learned " << skillName << " (Level " << level << "/"
              << maxLevel << ")" << std::endl;
}

void SkillNode::onEnter(GameContext* context)
{
    std::cout << "Viewing skill: " << skillName << " (Level " << level << "/"
              << maxLevel << ")" << std::endl;
    std::cout << description << std::endl;

    if (context && canLearn(*context)) {
        std::cout << "This skill can be learned/improved." << std::endl;
    }
}

std::vector<TAAction> SkillNode::getAvailableActions()
{
    std::vector<TAAction> actions = TANode::getAvailableActions();

    // Add learn skill action if not at max level
    if (level < maxLevel) {
        actions.push_back(
            { "learn_skill", "Learn/Improve " + skillName, [this]() -> TAInput {
                 return { "skill_action",
                     { { "action", std::string("learn") }, { "skill", skillName } } };
             } });
    }

    return actions;
}

bool SkillNode::evaluateTransition(const TAInput& input, TANode*& outNextNode)
{
    if (input.type == "skill_action") {
        std::string action = std::get<std::string>(input.parameters.at("action"));
        if (action == "learn") {
            // Stay in same node after learning
            outNextNode = this;
            return true;
        }
    }

    return TANode::evaluateTransition(input, outNextNode);
}