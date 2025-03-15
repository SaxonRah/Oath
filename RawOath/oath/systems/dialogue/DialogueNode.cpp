#include "DialogueNode.hpp"

DialogueNode::DialogueResponse::DialogueResponse(
    const std::string& responseText, TANode* target,
    std::function<bool(const GameContext&)> req,
    std::function<void(GameContext*)> eff)
    : text(responseText)
    , requirement(req)
    , targetNode(target)
    , effect(eff)
{
}

DialogueNode::DialogueNode(const std::string& name, const std::string& speaker,
    const std::string& text)
    : TANode(name)
    , speakerName(speaker)
    , dialogueText(text)
{
}

void DialogueNode::addResponse(
    const std::string& text, TANode* target,
    std::function<bool(const GameContext&)> requirement,
    std::function<void(GameContext*)> effect)
{
    responses.push_back(DialogueResponse(text, target, requirement, effect));
}

void DialogueNode::onEnter(GameContext* context)
{
    // Display the dialogue
    std::cout << speakerName << ": " << dialogueText << std::endl;

    // Run any effects
    if (onShowEffect && context) {
        onShowEffect(context);
    }

    // Store in dialogue history
    if (context) {
        context->dialogueHistory[nodeName] = dialogueText;
    }
}

std::vector<TAAction> DialogueNode::getAvailableActions()
{
    std::vector<TAAction> actions;

    for (size_t i = 0; i < responses.size(); i++) {
        actions.push_back(
            { "response_" + std::to_string(i), responses[i].text,
                [this, i]() -> TAInput {
                    return { "dialogue_response", { { "index", static_cast<int>(i) } } };
                } });
    }

    return actions;
}

bool DialogueNode::evaluateTransition(const TAInput& input, TANode*& outNextNode)
{
    if (input.type == "dialogue_response") {
        int index = std::get<int>(input.parameters.at("index"));
        if (index >= 0 && index < static_cast<int>(responses.size())) {
            outNextNode = responses[index].targetNode;
            return true;
        }
    }
    return false;
}