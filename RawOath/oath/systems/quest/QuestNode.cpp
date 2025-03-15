#include <QuestNode.hpp>

bool QuestNode::QuestRequirement::check(const GameContext& context) const
{
    if (type == "skill") {
        return context.playerStats.hasSkill(target, value);
    } else if (type == "item") {
        return context.playerInventory.hasItem(target, value);
    } else if (type == "faction") {
        return context.playerStats.hasFactionReputation(target, value);
    } else if (type == "knowledge") {
        return context.playerStats.hasKnowledge(target);
    } else if (type == "worldflag") {
        return context.worldState.hasFlag(target);
    }
    return false;
}

QuestNode::QuestNode(const std::string& name)
    : TANode(name)
    , questState("Available")
{
}

bool QuestNode::processAction(const std::string& playerAction, TANode*& outNextNode)
{
    return evaluateTransition({ "action", { { "name", playerAction } } },
        outNextNode);
}

bool QuestNode::canAccess(const GameContext& context) const
{
    for (const auto& req : requirements) {
        if (!req.check(context)) {
            return false;
        }
    }
    return true;
}

void QuestNode::onEnter(GameContext* context)
{
    // Mark quest as active
    questState = "Active";

    // Activate all child quests/objectives
    for (TANode* childNode : childNodes) {
        if (auto* questChild = dynamic_cast<QuestNode*>(childNode)) {
            questChild->questState = "Available";
        }
    }

    // Update quest journal
    if (context) {
        context->questJournal[nodeName] = "Active";
    }

    std::cout << "Quest activated: " << questTitle << std::endl;
    std::cout << questDescription << std::endl;
}

void QuestNode::onExit(GameContext* context)
{
    // Only award rewards if moving to a completion state
    if (isAcceptingState) {
        questState = "Completed";

        if (context) {
            context->questJournal[nodeName] = "Completed";

            // Award rewards to player
            std::cout << "Quest completed: " << questTitle << std::endl;
            std::cout << "Rewards:" << std::endl;

            for (const auto& reward : rewards) {
                if (reward.type == "experience") {
                    std::cout << "  " << reward.amount << " experience points"
                              << std::endl;
                } else if (reward.type == "gold") {
                    std::cout << "  " << reward.amount << " gold coins" << std::endl;
                } else if (reward.type == "item") {
                    context->playerInventory.addItem(Item(reward.itemId, reward.itemId,
                        "quest_reward", 1,
                        reward.amount));
                    std::cout << "  " << reward.amount << "x " << reward.itemId
                              << std::endl;
                } else if (reward.type == "faction") {
                    context->playerStats.changeFactionRep(reward.itemId, reward.amount);
                    std::cout << "  " << reward.amount << " reputation with "
                              << reward.itemId << std::endl;
                } else if (reward.type == "skill") {
                    context->playerStats.improveSkill(reward.itemId, reward.amount);
                    std::cout << "  " << reward.amount << " points in " << reward.itemId
                              << " skill" << std::endl;
                }
            }
        }
    } else if (questState == "Failed") {
        if (context) {
            context->questJournal[nodeName] = "Failed";
        }
        std::cout << "Quest failed: " << questTitle << std::endl;
    }
}

std::vector<TAAction> QuestNode::getAvailableActions()
{
    std::vector<TAAction> actions = TANode::getAvailableActions();

    // Add quest-specific actions
    actions.push_back(
        { "abandon_quest", "Abandon this quest", []() -> TAInput {
             return { "quest_action", { { "action", std::string("abandon") } } };
         } });

    return actions;
}