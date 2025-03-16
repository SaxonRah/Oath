#include "NPCInfoNode.hpp"
#include "../../data/GameContext.hpp"
#include "RelationshipConfig.hpp"
#include <iostream>

NPCInfoNode::NPCInfoNode(const std::string& name, NPCRelationshipManager* manager)
    : TANode(name)
    , relationshipManager(manager)
    , currentNPCId("")
{
}

void NPCInfoNode::setCurrentNPC(const std::string& npcId)
{
    currentNPCId = npcId;
}

void NPCInfoNode::onEnter(GameContext* context)
{
    TANode::onEnter(context);

    if (currentNPCId.empty()) {
        std::cout << "No NPC selected for viewing." << std::endl;
        return;
    }

    RelationshipNPC* npc = relationshipManager->getNPC(currentNPCId);
    if (!npc) {
        std::cout << "Selected NPC not found in database." << std::endl;
        return;
    }

    RelationshipConfig& config = RelationshipConfig::getInstance();

    // Display basic NPC info
    std::cout << "===== NPC Information =====" << std::endl;
    std::cout << "Name: " << npc->name << std::endl;
    std::cout << "Occupation: " << npc->occupation << std::endl;
    std::cout << "Age: " << npc->age << std::endl;
    std::cout << "Gender: " << npc->gender << std::endl;
    std::cout << "Race: " << npc->race << std::endl;
    std::cout << "Faction: " << npc->faction << std::endl;
    std::cout << "Home: " << npc->homeLocation << std::endl;

    // Relationship information
    std::string relationshipDesc = relationshipManager->getRelationshipDescription(currentNPCId);
    int relationshipValue = relationshipManager->getRelationshipValue(currentNPCId);
    std::cout << "\nRelationship: " << relationshipDesc << " (" << relationshipValue << ")" << std::endl;

    // Only show personality traits if relationship is good enough
    if (relationshipValue >= config.getFriendlyThreshold()) {
        std::cout << "\nPersonality traits:" << std::endl;
        for (const auto& trait : npc->personalityTraits) {
            std::cout << " - " << config.getPersonalityTraitString(trait) << std::endl;
        }
    }

    // Show gift preferences if relationship is good
    if (relationshipValue >= config.getCloseThreshold()) {
        std::cout << "\nGift preferences:" << std::endl;
        for (const auto& [category, preference] : npc->giftPreferences) {
            if (preference > 0.5f) {
                std::cout << " - Likes " << config.getGiftCategoryString(category) << std::endl;
            } else if (preference < -0.5f) {
                std::cout << " - Dislikes " << config.getGiftCategoryString(category) << std::endl;
            }
        }
    }

    // Show favorite items only to very close friends/partners
    if (relationshipValue >= config.getIntimateThreshold()) {
        if (!npc->favoriteItems.empty()) {
            std::cout << "\nFavorite items:" << std::endl;
            for (const auto& item : npc->favoriteItems) {
                std::cout << " - " << item << std::endl;
            }
        }
    }

    // Show daily schedule if friends or better
    if (relationshipValue >= config.getFriendlyThreshold()) {
        std::cout << "\nTypical daily schedule:" << std::endl;
        for (const auto& entry : npc->weekdaySchedule) {
            std::cout << " - " << entry.startHour << ":00 to " << entry.endHour
                      << ":00: " << entry.activity << " at " << entry.location << std::endl;
        }
    }
}

std::vector<TAAction> NPCInfoNode::getAvailableActions()
{
    std::vector<TAAction> actions = TANode::getAvailableActions();

    actions.push_back({ "return_to_interaction", "Return to interaction",
        [this]() -> TAInput {
            return { "info_action", { { "action", std::string("return") } } };
        } });

    return actions;
}