#pragma once

#include "../../core/TANode.hpp"
#include "NPCRelationshipManager.hpp"

class NPCInfoNode : public TANode {
private:
    NPCRelationshipManager* relationshipManager;
    std::string currentNPCId;

public:
    NPCInfoNode(const std::string& name, NPCRelationshipManager* manager);
    void setCurrentNPC(const std::string& npcId);
    void onEnter(GameContext* context) override;
    std::vector<TAAction> getAvailableActions() override;
};