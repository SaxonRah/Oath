#include "RelationshipSystemController.hpp"
#include <functional>

RelationshipSystemController::RelationshipSystemController(TAController* gameController)
    : controller(gameController)
{
    // Create the necessary nodes
    interactionNode = dynamic_cast<NPCInteractionNode*>(
        controller->createNode<NPCInteractionNode>("NPCInteraction", &relationshipManager));

    infoNode = dynamic_cast<NPCInfoNode*>(
        controller->createNode<NPCInfoNode>("NPCInfo", &relationshipManager));

    browserNode = dynamic_cast<RelationshipBrowserNode*>(
        controller->createNode<RelationshipBrowserNode>(
            "RelationshipBrowser", &relationshipManager, interactionNode, infoNode));

    // Set up transitions
    interactionNode->addTransition(
        [](const TAInput& input) {
            return input.type == "npc_action" && std::get<std::string>(input.parameters.at("action")) == "view_info";
        },
        infoNode, "View NPC information");

    interactionNode->addTransition(
        [](const TAInput& input) {
            return input.type == "npc_action" && std::get<std::string>(input.parameters.at("action")) == "return_to_browser";
        },
        browserNode, "Return to relationship browser");

    infoNode->addTransition(
        [](const TAInput& input) {
            return input.type == "info_action" && std::get<std::string>(input.parameters.at("action")) == "return";
        },
        interactionNode, "Return to interaction");

    // Register the relationship system root node
    controller->setSystemRoot("RelationshipSystem", browserNode);
}

NPCRelationshipManager* RelationshipSystemController::getRelationshipManager()
{
    return &relationshipManager;
}

void RelationshipSystemController::updateTimeOfDay(int day, int hour)
{
    // Move NPCs according to their schedules

    // Check for daily relationship decay
    static int lastProcessedDay = -1;
    if (day > lastProcessedDay) {
        relationshipManager.advanceDay();
        lastProcessedDay = day;
    }
}

void RelationshipSystemController::processSpecialEvents(int day)
{
    // This would check for birthdays, anniversaries, etc.
    // and potentially update relationships or trigger special interactions
}

bool RelationshipSystemController::saveRelationshipSystem(const std::string& filename)
{
    return relationshipManager.saveRelationships(filename);
}

bool RelationshipSystemController::loadRelationshipSystem(const std::string& filename)
{
    return relationshipManager.loadRelationships(filename);
}