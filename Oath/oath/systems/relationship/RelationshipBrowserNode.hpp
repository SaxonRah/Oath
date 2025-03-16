#pragma once

#include "../../core/TANode.hpp"
#include "NPCInfoNode.hpp"
#include "NPCInteractionNode.hpp"
#include "NPCRelationshipManager.hpp"


class RelationshipBrowserNode : public TANode {
private:
    NPCRelationshipManager* relationshipManager;
    std::vector<std::string> currentNPCList;
    NPCInteractionNode* interactionNode;
    NPCInfoNode* infoNode;

public:
    // Filter type for browsing
    enum class BrowseFilter {
        All,
        Friends,
        RomanticRelations,
        FamilyMembers,
        Acquaintances,
        Rivals
    };

    BrowseFilter currentFilter;

    RelationshipBrowserNode(const std::string& name, NPCRelationshipManager* manager,
        NPCInteractionNode* interaction, NPCInfoNode* info);

    void onEnter(GameContext* context) override;
    void updateNPCList();
    void displayNPCList();
    std::string getFilterName(BrowseFilter filter);
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;
};