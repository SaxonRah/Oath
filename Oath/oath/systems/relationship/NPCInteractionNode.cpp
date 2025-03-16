#include "NPCInteractionNode.hpp"
#include "../../data/GameContext.hpp"
#include "RelationshipConfig.hpp"
#include <iostream>


NPCInteractionNode::NPCInteractionNode(const std::string& name, NPCRelationshipManager* manager)
    : TANode(name)
    , relationshipManager(manager)
    , currentNPCId("")
{
}

void NPCInteractionNode::setCurrentNPC(const std::string& npcId)
{
    currentNPCId = npcId;
}

void NPCInteractionNode::onEnter(GameContext* context)
{
    TANode::onEnter(context);

    if (currentNPCId.empty()) {
        std::cout << "No NPC selected for interaction." << std::endl;
        return;
    }

    RelationshipNPC* npc = relationshipManager->getNPC(currentNPCId);
    if (!npc) {
        std::cout << "Selected NPC not found in database." << std::endl;
        return;
    }

    std::string relationshipDesc = relationshipManager->getRelationshipDescription(currentNPCId);
    int relationshipValue = relationshipManager->getRelationshipValue(currentNPCId);

    std::cout << "Interacting with " << npc->name << " (" << npc->occupation << ")" << std::endl;
    std::cout << "Current relationship: " << relationshipDesc << " (" << relationshipValue << ")" << std::endl;
}

std::vector<TAAction> NPCInteractionNode::getAvailableActions()
{
    std::vector<TAAction> actions = TANode::getAvailableActions();

    if (currentNPCId.empty()) {
        return actions;
    }

    RelationshipNPC* npc = relationshipManager->getNPC(currentNPCId);
    if (!npc) {
        return actions;
    }

    RelationshipConfig& config = RelationshipConfig::getInstance();

    // Get relationship value to determine available actions
    int value = relationshipManager->getRelationshipValue(currentNPCId);
    RelationshipType type = relationshipManager->getRelationshipType(currentNPCId);

    // Basic actions available to everyone
    actions.push_back({ "talk_to_npc", "Talk with " + npc->name,
        [this]() -> TAInput {
            return { "npc_action", { { "action", std::string("talk") } } };
        } });

    actions.push_back({ "give_gift", "Give a gift to " + npc->name,
        [this]() -> TAInput {
            return { "npc_action", { { "action", std::string("give_gift") } } };
        } });

    // Only available for neutral+ relationships
    if (value >= config.getNeutralThreshold()) {
        actions.push_back({ "ask_about_self", "Ask about " + npc->name,
            [this]() -> TAInput {
                return { "npc_action", { { "action", std::string("ask_about") } } };
            } });
    }

    // Only available for friendly+ relationships
    if (value >= config.getFriendlyThreshold()) {
        actions.push_back({ "request_help", "Ask for help",
            [this]() -> TAInput {
                return { "npc_action", { { "action", std::string("request_help") } } };
            } });
    }

    // Only available for close friends or better
    if (value >= config.getCloseThreshold()) {
        actions.push_back({ "personal_request", "Make personal request",
            [this]() -> TAInput {
                return { "npc_action", { { "action", std::string("personal_request") } } };
            } });
    }

    // Romantic options only available if not enemies/rivals and not already in a relationship
    if (value >= config.getFriendlyThreshold() && type != RelationshipType::Enemy && type != RelationshipType::Rival && type != RelationshipType::Spouse && type != RelationshipType::Partner) {

        actions.push_back({ "flirt", "Flirt with " + npc->name,
            [this]() -> TAInput {
                return { "npc_action", { { "action", std::string("flirt") } } };
            } });
    }

    // Marriage proposal - only if already partners
    if (type == RelationshipType::Partner) {
        actions.push_back({ "propose", "Propose marriage to " + npc->name,
            [this]() -> TAInput {
                return { "npc_action", { { "action", std::string("propose") } } };
            } });
    }

    return actions;
}

bool NPCInteractionNode::evaluateTransition(const TAInput& input, TANode*& outNextNode)
{
    if (input.type != "npc_action") {
        return TANode::evaluateTransition(input, outNextNode);
    }

    std::string action = std::get<std::string>(input.parameters.at("action"));
    RelationshipNPC* npc = relationshipManager->getNPC(currentNPCId);

    if (!npc) {
        return false;
    }

    RelationshipConfig& config = RelationshipConfig::getInstance();

    // Process different interaction types
    if (action == "talk") {
        // Get available topics based on relationship
        std::vector<std::string> topics;
        for (const auto& topic : npc->conversationTopics) {
            topics.push_back(topic);
        }

        // Add generic topics if we don't have enough
        if (topics.empty()) {
            topics = { "weather", "town_news", "personal", "work" };
        }

        std::cout << "What would you like to talk about with " << npc->name << "?" << std::endl;
        for (size_t i = 0; i < topics.size(); i++) {
            std::cout << i + 1 << ". " << topics[i] << std::endl;
        }

        // In a real implementation, we'd get user input here
        int topicIndex = 0; // For demonstration

        if (topicIndex >= 0 && topicIndex < static_cast<int>(topics.size())) {
            std::string topic = topics[topicIndex];
            bool isPositive = true; // Would be based on player's dialogue choices

            relationshipManager->handleConversation(currentNPCId, topic, isPositive);

            if (isPositive) {
                std::cout << npc->name << " seems to enjoy talking about this topic." << std::endl;
            } else {
                std::cout << npc->name << " doesn't seem interested in this topic." << std::endl;
            }
        }
    } else if (action == "give_gift") {
        // Simulate gift selection (would be UI-based in real game)
        std::string itemId = "silver_necklace";
        GiftCategory category = GiftCategory::Jewelry;
        int itemValue = 50;

        bool giftAccepted = relationshipManager->giveGift(currentNPCId, itemId, category, itemValue);

        if (giftAccepted) {
            float reaction = npc->getGiftReaction(itemId, category);

            if (reaction >= config.getFavoriteGiftMultiplier()) {
                std::cout << npc->name << " loves this gift!" << std::endl;
            } else if (reaction >= config.getLikedGiftMultiplier()) {
                std::cout << npc->name << " likes this gift." << std::endl;
            } else if (reaction <= config.getHatedGiftMultiplier()) {
                std::cout << npc->name << " hates this gift!" << std::endl;
            } else if (reaction <= config.getDislikedGiftMultiplier()) {
                std::cout << npc->name << " doesn't like this gift much." << std::endl;
            } else {
                std::cout << npc->name << " accepts your gift politely." << std::endl;
            }
        } else {
            std::cout << "You've already given " << npc->name << " a gift recently." << std::endl;
        }
    } else if (action == "ask_about") {
        std::cout << npc->name << " tells you a bit about their life:" << std::endl;
        std::cout << "They work as a " << npc->occupation << " and live in " << npc->homeLocation << "." << std::endl;

        // Share some personal details based on relationship level
        int value = relationshipManager->getRelationshipValue(currentNPCId);
        if (value >= config.getCloseThreshold()) {
            std::cout << "They share some personal details about their past and aspirations." << std::endl;
            relationshipManager->changeRelationship(currentNPCId, 2);
        } else if (value >= config.getFriendlyThreshold()) {
            std::cout << "They tell you about their current projects and interests." << std::endl;
            relationshipManager->changeRelationship(currentNPCId, 1);
        } else {
            std::cout << "They share basic information, but remain somewhat guarded." << std::endl;
        }
    } else if (action == "flirt") {
        int value = relationshipManager->getRelationshipValue(currentNPCId);
        bool receptive = false;

        // Check if NPC is receptive to romance (based on relationship and personality)
        if (value >= config.getCloseThreshold()) {
            receptive = true;
        } else if (value >= config.getFriendlyThreshold()) {
            // More likely if the NPC has romantic personality trait
            receptive = npc->personalityTraits.find(PersonalityTrait::Romantic) != npc->personalityTraits.end();
        }

        if (receptive) {
            std::cout << npc->name << " responds positively to your flirtation." << std::endl;
            relationshipManager->changeRelationship(currentNPCId, 3);

            // If relationship is already strong, potentially advance to romantic interest
            if (value >= config.getIntimateThreshold()) {
                relationshipManager->changeRelationshipType(currentNPCId, RelationshipType::RomanticInterest);
                std::cout << npc->name << " seems to be developing romantic feelings for you." << std::endl;
            }
        } else {
            std::cout << npc->name << " politely deflects your advances." << std::endl;
            if (value < config.getFriendlyThreshold()) {
                relationshipManager->changeRelationship(currentNPCId, -1);
            }
        }
    } else if (action == "propose") {
        int value = relationshipManager->getRelationshipValue(currentNPCId);

        // NPC will accept if relationship is very high
        if (value >= config.getIntimateThreshold()) {
            bool accepted = relationshipManager->changeRelationshipType(currentNPCId, RelationshipType::Spouse);

            if (accepted) {
                std::cout << npc->name << " joyfully accepts your proposal!" << std::endl;
                relationshipManager->changeRelationship(currentNPCId, 20);
            } else {
                std::cout << "Something prevents " << npc->name << " from accepting your proposal." << std::endl;
            }
        } else {
            std::cout << npc->name << " isn't ready for that level of commitment yet." << std::endl;
            relationshipManager->changeRelationship(currentNPCId, -5);
        }
    } else if (action == "request_help") {
        int value = relationshipManager->getRelationshipValue(currentNPCId);

        if (value >= config.getFriendlyThreshold()) {
            std::cout << npc->name << " agrees to help you." << std::endl;

            // The level of help would depend on relationship strength
            if (value >= config.getIntimateThreshold()) {
                std::cout << "They are willing to go to great lengths to assist you." << std::endl;
            } else if (value >= config.getCloseThreshold()) {
                std::cout << "They offer significant assistance." << std::endl;
            } else {
                std::cout << "They provide basic help." << std::endl;
            }
        } else {
            std::cout << npc->name << " declines to help at this time." << std::endl;
        }
    }

    // Stay in the same node after interaction
    outNextNode = this;
    return true;
}