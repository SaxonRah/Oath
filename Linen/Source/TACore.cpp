#include "TACore.hpp"

void TANode::addChild(TANode* child)
{
    if (child) {
        childNodes.push_back(child);
    }
}

void TANode::onEnter(GameContext* context)
{
    // Base implementation does nothing
}

void TANode::onExit(GameContext* context)
{
    // Base implementation does nothing
}

std::vector<TAAction> TANode::getAvailableActions()
{
    return {}; // Base implementation returns no actions
}

bool TANode::evaluateTransition(const TAInput& input, TANode*& outNextNode)
{
    // Check each transition rule
    for (const auto& rule : transitionRules) {
        if (rule.condition(input)) {
            outNextNode = rule.targetNode;
            return true;
        }
    }
    return false;
}

void TANode::addTransition(
    const std::function<bool(const TAInput&)>& condition,
    TANode* target,
    const std::string& description)
{

    transitionRules.push_back({ condition,
        target,
        description });
}
