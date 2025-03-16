#pragma once

#include "../../core/TANode.hpp"
#include "NPCRelationshipManager.hpp"

class NPCInteractionNode : public TANode {
private:
    NPCRelationshipManager* relationshipManager;
    std::string currentNPCId;

public:
    NPCInteractionNode(const std::string& name, NPCRelationshipManager* manager);
    void setCurrentNPC(const std::string& npcId);
    void onEnter(GameContext* context) override;
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;
};