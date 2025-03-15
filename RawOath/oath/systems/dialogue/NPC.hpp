#pragma once

#include "../../data/GameContext.hpp"
#include "DialogueNode.hpp"

#include <map>
#include <string>

class DialogueNode; // Forward declaration

// NPC class for dialogue interactions
class NPC {
public:
    std::string name;
    std::string description;
    DialogueNode* rootDialogue;
    DialogueNode* currentDialogue;
    std::map<std::string, DialogueNode*> dialogueNodes;

    // Relationship with player
    int relationshipValue = 0;

    NPC(const std::string& npcName, const std::string& desc);

    void startDialogue(GameContext* context);
    bool processResponse(int responseIndex, GameContext* context);
};