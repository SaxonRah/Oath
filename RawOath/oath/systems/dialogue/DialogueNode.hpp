#pragma once

#include "../../core/TANode.hpp"
#include "../../data/GameContext.hpp"

#include <functional>
#include <string>
#include <vector>

class NPC; // Forward declaration

// Dialogue node for conversation trees
class DialogueNode : public TANode {
public:
    // The text to display for this dialogue node
    std::string speakerName;
    std::string dialogueText;

    // Response options
    struct DialogueResponse {
        std::string text;
        std::function<bool(const GameContext&)> requirement;
        TANode* targetNode;
        std::function<void(GameContext*)> effect;

        DialogueResponse(
            const std::string& responseText, TANode* target,
            std::function<bool(const GameContext&)> req =
                [](const GameContext&) { return true; },
            std::function<void(GameContext*)> eff = [](GameContext*) {});
    };
    std::vector<DialogueResponse> responses;

    // Optional effect to run when this dialogue is shown
    std::function<void(GameContext*)> onShowEffect;

    DialogueNode(const std::string& name, const std::string& speaker,
        const std::string& text);

    void addResponse(
        const std::string& text, TANode* target,
        std::function<bool(const GameContext&)> requirement =
            [](const GameContext&) { return true; },
        std::function<void(GameContext*)> effect = [](GameContext*) {});

    void onEnter(GameContext* context) override;

    // Get available dialogue responses
    std::vector<TAAction> getAvailableActions() override;

    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;
};