#include "RelationshipBrowserNode.hpp"
#include "../../data/GameContext.hpp"
#include <iostream>


RelationshipBrowserNode::RelationshipBrowserNode(const std::string& name, NPCRelationshipManager* manager,
    NPCInteractionNode* interaction, NPCInfoNode* info)
    : TANode(name)
    , relationshipManager(manager)
    , interactionNode(interaction)
    , infoNode(info)
    , currentFilter(BrowseFilter::All)
{
}

void RelationshipBrowserNode::onEnter(GameContext* context)
{
    TANode::onEnter(context);

    std::cout << "==== Relationship Browser ====" << std::endl;

    // Apply current filter
    updateNPCList();

    // Display NPCs
    displayNPCList();
}

void RelationshipBrowserNode::updateNPCList()
{
    currentNPCList.clear();
    RelationshipConfig& config = RelationshipConfig::getInstance();

    // Get all NPCs with relationships
    for (const auto& [npcId, _] : relationshipManager->getPlayerRelationships()) {
        RelationshipType type = relationshipManager->getRelationshipType(npcId);
        bool includeNPC = false;

        switch (currentFilter) {
        case BrowseFilter::All:
            includeNPC = true;
            break;
        case BrowseFilter::Friends:
            includeNPC = (type == RelationshipType::Friend || type == RelationshipType::CloseFriend || type == RelationshipType::BestFriend);
            break;
        case BrowseFilter::RomanticRelations:
            includeNPC = (type == RelationshipType::RomanticInterest || type == RelationshipType::Partner || type == RelationshipType::Spouse);
            break;
        case BrowseFilter::FamilyMembers:
            includeNPC = (type == RelationshipType::Family);
            break;
        case BrowseFilter::Acquaintances:
            includeNPC = (type == RelationshipType::Acquaintance);
            break;
        case BrowseFilter::Rivals:
            includeNPC = (type == RelationshipType::Rival || type == RelationshipType::Enemy);
            break;
        }

        if (includeNPC) {
            currentNPCList.push_back(npcId);
        }
    }
}

void RelationshipBrowserNode::displayNPCList()
{
    std::cout << "Current filter: " << getFilterName(currentFilter) << std::endl;

    if (currentNPCList.empty()) {
        std::cout << "No relationships found with this filter." << std::endl;
        return;
    }

    std::cout << "Select an NPC to interact with:" << std::endl;
    for (size_t i = 0; i < currentNPCList.size(); i++) {
        RelationshipNPC* npc = relationshipManager->getNPC(currentNPCList[i]);
        std::string name = npc ? npc->name : currentNPCList[i];
        std::string relationshipDesc = relationshipManager->getRelationshipDescription(currentNPCList[i]);

        std::cout << i + 1 << ". " << name << " - " << relationshipDesc << std::endl;
    }
}

std::string RelationshipBrowserNode::getFilterName(BrowseFilter filter)
{
    switch (filter) {
    case BrowseFilter::All:
        return "All Relationships";
    case BrowseFilter::Friends:
        return "Friends";
    case BrowseFilter::RomanticRelations:
        return "Romantic Relations";
    case BrowseFilter::FamilyMembers:
        return "Family Members";
    case BrowseFilter::Acquaintances:
        return "Acquaintances";
    case BrowseFilter::Rivals:
        return "Rivals & Enemies";
    default:
        return "Unknown";
    }
}

std::vector<TAAction> RelationshipBrowserNode::getAvailableActions()
{
    std::vector<TAAction> actions = TANode::getAvailableActions();

    // Add filter options
    actions.push_back({ "filter_all", "Show All Relationships",
        [this]() -> TAInput {
            return { "browser_action", { { "action", std::string("filter") }, { "filter", std::string("all") } } };
        } });

    actions.push_back({ "filter_friends", "Show Friends Only",
        [this]() -> TAInput {
            return { "browser_action", { { "action", std::string("filter") }, { "filter", std::string("friends") } } };
        } });

    actions.push_back({ "filter_romantic", "Show Romantic Relations",
        [this]() -> TAInput {
            return { "browser_action", { { "action", std::string("filter") }, { "filter", std::string("romantic") } } };
        } });

    actions.push_back({ "filter_family", "Show Family Members",
        [this]() -> TAInput {
            return { "browser_action", { { "action", std::string("filter") }, { "filter", std::string("family") } } };
        } });

    // Add selection options for each NPC in the current list
    for (size_t i = 0; i < currentNPCList.size(); i++) {
        RelationshipNPC* npc = relationshipManager->getNPC(currentNPCList[i]);
        std::string name = npc ? npc->name : currentNPCList[i];

        actions.push_back({ "select_npc_" + std::to_string(i), "Select " + name,
            [this, i]() -> TAInput {
                return { "browser_action", { { "action", std::string("select") }, { "index", static_cast<int>(i) } } };
            } });
    }

    return actions;
}

bool RelationshipBrowserNode::evaluateTransition(const TAInput& input, TANode*& outNextNode)
{
    if (input.type != "browser_action") {
        return TANode::evaluateTransition(input, outNextNode);
    }

    std::string action = std::get<std::string>(input.parameters.at("action"));

    if (action == "filter") {
        std::string filterStr = std::get<std::string>(input.parameters.at("filter"));

        if (filterStr == "all") {
            currentFilter = BrowseFilter::All;
        } else if (filterStr == "friends") {
            currentFilter = BrowseFilter::Friends;
        } else if (filterStr == "romantic") {
            currentFilter = BrowseFilter::RomanticRelations;
        } else if (filterStr == "family") {
            currentFilter = BrowseFilter::FamilyMembers;
        } else if (filterStr == "acquaintances") {
            currentFilter = BrowseFilter::Acquaintances;
        } else if (filterStr == "rivals") {
            currentFilter = BrowseFilter::Rivals;
        }

        updateNPCList();
        displayNPCList();

        // Stay in browser node after filtering
        outNextNode = this;
        return true;
    } else if (action == "select") {
        int index = std::get<int>(input.parameters.at("index"));

        if (index >= 0 && index < static_cast<int>(currentNPCList.size())) {
            std::string selectedNpcId = currentNPCList[index];

            // Set the selected NPC for the interaction node
            interactionNode->setCurrentNPC(selectedNpcId);
            infoNode->setCurrentNPC(selectedNpcId);

            // Transition to interaction node
            outNextNode = interactionNode;
            return true;
        }
    }

    return false;
}