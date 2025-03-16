#include "systems/religion/RitualNode.hpp"
#include "core/TAAction.hpp"
#include "core/TAInput.hpp"
#include "systems/religion/DeityNode.hpp"
#include <iostream>
#include <nlohmann/json.hpp>

RitualNode::RitualNode(const std::string& id, const std::string& name, const std::string& deity)
    : TANode(name)
    , ritualId(id)
    , ritualName(name)
    , deityId(deity)
    , favorRequirement(0)
    , requiresHolyDay(false)
    , requiresPrimaryDeity(false)
    , goldCost(0)
    , favorReward(10)
    , skillBoost(0)
    , blessingDuration(0)
    , complexity(MODERATE)
{
}

void RitualNode::loadFromJson(const nlohmann::json& ritualData)
{
    ritualDescription = ritualData["description"];
    favorRequirement = ritualData["favorRequirement"];
    requiresHolyDay = ritualData["requiresHolyDay"];
    requiresPrimaryDeity = ritualData["requiresPrimaryDeity"];
    goldCost = ritualData["goldCost"];
    favorReward = ritualData["favorReward"];
    skillBoost = ritualData["skillBoost"];
    skillAffected = ritualData["skillAffected"];
    blessingGranted = ritualData["blessingGranted"];
    blessingDuration = ritualData["blessingDuration"];
    complexity = static_cast<RitualComplexity>(ritualData["complexity"]);

    // Load item requirements
    itemRequirements.clear();
    for (auto& [item, quantity] : ritualData["itemRequirements"].items()) {
        itemRequirements[item] = quantity;
    }
}

void RitualNode::onEnter(ReligiousGameContext* context)
{
    std::cout << "=== " << ritualName << " Ritual ===" << std::endl;
    std::cout << ritualDescription << std::endl;

    if (context) {
        // Check if ritual has been completed before
        bool completed = context->religiousStats.hasCompletedRitual(ritualId);
        if (completed) {
            std::cout << "You have performed this ritual before." << std::endl;
        }

        // Show requirements
        std::cout << "\nRequirements:" << std::endl;

        if (favorRequirement > 0) {
            std::cout << "- Minimum " << favorRequirement << " favor with deity" << std::endl;
            if (context->religiousStats.deityFavor[deityId] < favorRequirement) {
                std::cout << "  (You don't have enough favor)" << std::endl;
            }
        }

        if (requiresHolyDay) {
            std::cout << "- Must be performed on deity's holy day" << std::endl;
            DeityNode* deity = findDeityNode(context);
            if (deity && !deity->isHolyDay(context)) {
                std::cout << "  (Today is not the holy day)" << std::endl;
            }
        }

        if (requiresPrimaryDeity) {
            std::cout << "- Deity must be your primary deity" << std::endl;
            if (context->religiousStats.primaryDeity != deityId) {
                std::cout << "  (This is not your primary deity)" << std::endl;
            }
        }

        for (const auto& [item, quantity] : itemRequirements) {
            std::cout << "- " << quantity << "x " << item << std::endl;
            if (!context->playerInventory.hasItem(item, quantity)) {
                std::cout << "  (You don't have enough)" << std::endl;
            }
        }

        if (goldCost > 0) {
            std::cout << "- " << goldCost << " gold donation" << std::endl;
            // Check gold in inventory...
        }

        // Show rewards
        std::cout << "\nRewards:" << std::endl;
        std::cout << "- " << favorReward << " favor with deity" << std::endl;

        if (skillBoost > 0 && !skillAffected.empty()) {
            std::cout << "- +" << skillBoost << " to " << skillAffected << " skill" << std::endl;
        }

        if (!blessingGranted.empty() && blessingDuration > 0) {
            std::cout << "- " << blessingGranted << " blessing (" << blessingDuration << " days)" << std::endl;
        }
    }
}

DeityNode* RitualNode::findDeityNode(ReligiousGameContext* context) const
{
    // This would need to be implemented to find the deity node from the controller
    // For now, return nullptr as a placeholder
    return nullptr;
}

bool RitualNode::canPerformRitual(ReligiousGameContext* context) const
{
    if (!context)
        return false;

    // Check favor requirement
    if (context->religiousStats.deityFavor[deityId] < favorRequirement) {
        return false;
    }

    // Check if holy day is required
    if (requiresHolyDay) {
        DeityNode* deity = findDeityNode(context);
        if (!deity || !deity->isHolyDay(context)) {
            return false;
        }
    }

    // Check if primary deity is required
    if (requiresPrimaryDeity && context->religiousStats.primaryDeity != deityId) {
        return false;
    }

    // Check item requirements
    for (const auto& [item, quantity] : itemRequirements) {
        if (!context->playerInventory.hasItem(item, quantity)) {
            return false;
        }
    }

    // Check gold (would need to be implemented)

    return true;
}

bool RitualNode::performRitual(ReligiousGameContext* context)
{
    if (!context)
        return false;

    if (!canPerformRitual(context)) {
        std::cout << "You cannot perform this ritual. Some requirements are not met." << std::endl;
        return false;
    }

    // Consume required items
    for (const auto& [item, quantity] : itemRequirements) {
        context->playerInventory.removeItem(item, quantity);
    }

    // Consume gold (would need to be implemented)

    // Grant favor reward
    context->religiousStats.changeFavor(deityId, favorReward);
    std::cout << "You have gained " << favorReward << " favor with the deity." << std::endl;

    // Grant skill boost if applicable
    if (skillBoost > 0 && !skillAffected.empty()) {
        context->playerStats.improveSkill(skillAffected, skillBoost);
        std::cout << "Your " << skillAffected << " skill has improved by " << skillBoost << "." << std::endl;
    }

    // Grant blessing if applicable
    if (!blessingGranted.empty() && blessingDuration > 0) {
        context->religiousStats.addBlessing(blessingGranted, blessingDuration);
        std::cout << "You have received the " << blessingGranted << " blessing for " << blessingDuration << " days." << std::endl;
    }

    // Mark ritual as completed
    context->religiousStats.markRitualCompleted(ritualId);

    std::cout << "The ritual has been successfully completed." << std::endl;

    // Ritual effects on the world (could trigger world events)
    switch (complexity) {
    case GRAND:
        std::cout << "The very fabric of reality seems to shift in response to your grand ritual." << std::endl;
        // Implement world-changing effects
        break;
    case ELABORATE:
        std::cout << "The power of your ritual resonates throughout the region." << std::endl;
        // Implement regional effects
        break;
    case COMPLEX:
        std::cout << "The deity's attention has been drawn to your devotion." << std::endl;
        // Special deity interactions
        break;
    case MODERATE:
        std::cout << "You feel the power of the ritual washing over you." << std::endl;
        break;
    case SIMPLE:
        std::cout << "A simple but effective communion with the divine." << std::endl;
        break;
    }

    return true;
}

std::vector<TAAction> RitualNode::getAvailableActions()
{
    std::vector<TAAction> actions = TANode::getAvailableActions();

    // Add ritual actions
    actions.push_back({ "perform_ritual",
        "Perform the ritual",
        [this]() -> TAInput {
            return { "ritual_action", { { "action", std::string("perform") } } };
        } });

    actions.push_back({ "back_to_temple",
        "Return to temple",
        [this]() -> TAInput {
            return { "ritual_action", { { "action", std::string("back") } } };
        } });

    return actions;
}

bool RitualNode::evaluateTransition(const TAInput& input, TANode*& outNextNode)
{
    if (input.type == "ritual_action") {
        std::string action = std::get<std::string>(input.parameters.at("action"));

        if (action == "back") {
            // Return to temple node
            for (const auto& rule : transitionRules) {
                if (rule.description == "Return to temple") {
                    outNextNode = rule.targetNode;
                    return true;
                }
            }
        } else {
            // For other actions (perform), stay on same node
            outNextNode = this;
            return true;
        }
    }

    return TANode::evaluateTransition(input, outNextNode);
}