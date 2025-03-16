#pragma once

#include "../../core/TAController.hpp"
#include "NPCInfoNode.hpp"
#include "NPCInteractionNode.hpp"
#include "NPCRelationshipManager.hpp"
#include "RelationshipBrowserNode.hpp"


class RelationshipSystemController {
private:
    NPCRelationshipManager relationshipManager;
    TAController* controller;

    // Nodes for the relationship system
    RelationshipBrowserNode* browserNode;
    NPCInteractionNode* interactionNode;
    NPCInfoNode* infoNode;

public:
    RelationshipSystemController(TAController* gameController);

    // Helper method to access relationship manager
    NPCRelationshipManager* getRelationshipManager();

    // Update time of day - call this when game time advances
    void updateTimeOfDay(int day, int hour);

    // Process events like NPC birthdays, special occasions
    void processSpecialEvents(int day);

    // Save/load relationship system
    bool saveRelationshipSystem(const std::string& filename);
    bool loadRelationshipSystem(const std::string& filename);
};