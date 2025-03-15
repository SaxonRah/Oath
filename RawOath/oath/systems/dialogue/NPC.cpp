#include "NPC.hpp"

#include <iostream>

NPC::NPC(const std::string& npcName, const std::string& desc)
    : name(npcName)
    , description(desc)
    , rootDialogue(nullptr)
    , currentDialogue(nullptr)
{
}

void NPC::startDialogue(GameContext* context)
{
    if (rootDialogue) {
        currentDialogue = rootDialogue;
        currentDialogue->onEnter(context);
    }
}

bool NPC::processResponse(int responseIndex, GameContext* context)
{
    if (!currentDialogue)
        return false;

    if (responseIndex >= 0 && responseIndex < static_cast<int>(currentDialogue->responses.size())) {
        auto& response = currentDialogue->responses[responseIndex];

        // Check if requirement is met
        if (!response.requirement(*context)) {
            std::cout << "You cannot select that response." << std::endl;
            return false;
        }

        // Execute effect
        if (response.effect) {
            response.effect(context);
        }

        // Move to next dialogue node
        currentDialogue = dynamic_cast<DialogueNode*>(response.targetNode);
        if (currentDialogue) {
            currentDialogue->onEnter(context);
            return true;
        }
    }

    return false;
}
