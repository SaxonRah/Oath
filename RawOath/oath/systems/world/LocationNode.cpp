#include <LocationNode.hpp>

bool LocationNode::AccessCondition::check(const GameContext& context) const
{
    if (type == "item") {
        return context.playerInventory.hasItem(target, value);
    } else if (type == "skill") {
        return context.playerStats.hasSkill(target, value);
    } else if (type == "faction") {
        return context.playerStats.hasFactionReputation(target, value);
    } else if (type == "worldflag") {
        return context.worldState.hasFlag(target);
    }
    return false;
}

LocationNode::LocationNode(const std::string& name, const std::string& location,
    const std::string& initialState)
    : TANode(name)
    , locationName(location)
    , currentState(initialState)
{
}

bool LocationNode::canAccess(const GameContext& context) const
{
    for (const auto& condition : accessConditions) {
        if (!condition.check(context)) {
            return false;
        }
    }
    return true;
}

void LocationNode::onEnter(GameContext* context)
{
    std::cout << "Arrived at " << locationName << std::endl;

    // Show description based on current state
    if (stateDescriptions.find(currentState) != stateDescriptions.end()) {
        std::cout << stateDescriptions.at(currentState) << std::endl;
    } else {
        std::cout << description << std::endl;
    }

    // Update world state
    if (context) {
        context->worldState.setLocationState(locationName, currentState);
    }

    // List NPCs
    if (!npcs.empty()) {
        std::cout << "People here:" << std::endl;
        for (const auto& npc : npcs) {
            std::cout << "- " << npc->name << std::endl;
        }
    }

    // List activities
    if (!activities.empty()) {
        std::cout << "Available activities:" << std::endl;
        for (const auto& activity : activities) {
            std::cout << "- " << activity->nodeName << std::endl;
        }
    }
}

std::vector<TAAction> LocationNode::getAvailableActions()
{
    std::vector<TAAction> actions = TANode::getAvailableActions();

    // Add NPC interaction actions
    for (size_t i = 0; i < npcs.size(); i++) {
        actions.push_back({ "talk_to_npc_" + std::to_string(i),
            "Talk to " + npcs[i]->name, [this, i]() -> TAInput {
                return { "location_action",
                    { { "action", std::string("talk") },
                        { "npc_index", static_cast<int>(i) } } };
            } });
    }

    // Add activity actions
    for (size_t i = 0; i < activities.size(); i++) {
        actions.push_back({ "do_activity_" + std::to_string(i),
            "Do " + activities[i]->nodeName,
            [this, i]() -> TAInput {
                return { "location_action",
                    { { "action", std::string("activity") },
                        { "activity_index", static_cast<int>(i) } } };
            } });
    }

    return actions;
}