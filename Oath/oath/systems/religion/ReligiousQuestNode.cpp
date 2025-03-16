#include "systems/religion/ReligiousQuestNode.hpp"
#include "systems/religion/DeityNode.hpp"
#include <iostream>
#include <nlohmann/json.hpp>

ReligiousQuestNode::ReligiousQuestNode(const std::string& name, const std::string& deity)
    : QuestNode(name)
    , deityId(deity)
    , favorReward(20)
    , devotionReward(10)
{
}

void ReligiousQuestNode::loadFromJson(const nlohmann::json& questData)
{
    questTitle = questData["title"];
    questDescription = questData["description"];
    favorReward = questData["favorReward"];
    devotionReward = questData["devotionReward"];

    // Load rewards
    rewards.clear();
    for (const auto& reward : questData["rewards"]) {
        rewards.push_back({ reward["type"],
            reward["amount"],
            reward["description"] });
    }

    // Load world state changes
    worldStateChanges.clear();
    for (auto& [key, value] : questData["worldStateChanges"].items()) {
        worldStateChanges[key] = value;
    }
}

void ReligiousQuestNode::onEnter(GameContext* baseContext)
{
    QuestNode::onEnter(baseContext);

    // Additional religious quest info
    ReligiousGameContext* context = dynamic_cast<ReligiousGameContext*>(baseContext);
    if (context) {
        std::cout << "\nThis is a religious quest for ";

        DeityNode* deity = findDeityNode(context);
        if (deity) {
            std::cout << deity->deityName << ", " << deity->deityTitle << "." << std::endl;
        } else {
            std::cout << "a deity." << std::endl;
        }

        std::cout << "Favor reward: " << favorReward << std::endl;
        std::cout << "Devotion reward: " << devotionReward << std::endl;

        // Show current favor
        int currentFavor = context->religiousStats.deityFavor[deityId];
        std::cout << "Your current favor: " << currentFavor << std::endl;
    }
}

void ReligiousQuestNode::onExit(GameContext* baseContext)
{
    QuestNode::onExit(baseContext);

    // Religious-specific rewards when quest is completed
    if (isAcceptingState) {
        ReligiousGameContext* context = dynamic_cast<ReligiousGameContext*>(baseContext);
        if (context) {
            // Award favor and devotion
            context->religiousStats.changeFavor(deityId, favorReward);
            context->religiousStats.addDevotion(deityId, devotionReward);

            std::cout << "You have gained " << favorReward << " favor and " << devotionReward
                      << " devotion with the deity." << std::endl;

            // Apply world state changes
            for (const auto& [key, value] : worldStateChanges) {
                context->worldState.setWorldFlag(key, value == "true");
                std::cout << "The world has changed in response to your actions." << std::endl;
            }
        }
    }
}

DeityNode* ReligiousQuestNode::findDeityNode(ReligiousGameContext* context) const
{
    // This would need to be implemented to find the deity node from the controller
    // For now, return nullptr as a placeholder
    return nullptr;
}