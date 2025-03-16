#include "systems/religion/DeityNode.hpp"
#include "core/TAAction.hpp"
#include "core/TAInput.hpp"
#include "systems/religion/BlessingNode.hpp"
#include "systems/religion/PrayerNode.hpp"
#include "systems/religion/TempleNode.hpp"
#include <iostream>
#include <nlohmann/json.hpp>

DeityNode::DeityNode(const std::string& name, const std::string& id, const std::string& title)
    : TANode(name)
    , deityId(id)
    , deityName(name)
    , deityTitle(title)
{
}

void DeityNode::loadFromJson(const nlohmann::json& deityData)
{
    deityDescription = deityData["description"];
    deityDomain = deityData["domain"];
    alignmentRequirement = deityData["alignment"];

    // Load opposing deities
    opposingDeities.clear();
    for (const auto& opposing : deityData["opposingDeities"]) {
        opposingDeities.push_back(opposing);
    }

    // Load favored actions
    favoredActions.clear();
    for (const auto& action : deityData["favoredActions"]) {
        favoredActions.push_back({ action["action"],
            action["favorChange"],
            action["description"] });
    }

    // Load disfavored actions
    disfavoredActions.clear();
    for (const auto& action : deityData["disfavoredActions"]) {
        disfavoredActions.push_back({ action["action"],
            action["favorChange"],
            action["description"] });
    }
}

void DeityNode::onEnter(ReligiousGameContext* context)
{
    std::cout << "=== " << deityName << ", " << deityTitle << " ===" << std::endl;
    std::cout << deityDescription << std::endl;
    std::cout << "Domain: " << deityDomain << std::endl;
    std::cout << "Alignment: " << alignmentRequirement << std::endl;

    if (context) {
        int favor = context->religiousStats.deityFavor[deityId];
        int devotion = context->religiousStats.deityDevotion[deityId];
        bool isPrimary = (context->religiousStats.primaryDeity == deityId);

        std::cout << "\nYour standing with " << deityName << ":" << std::endl;
        std::cout << "Favor: " << favor << " (" << getFavorLevel(favor) << ")" << std::endl;
        std::cout << "Devotion: " << devotion << std::endl;

        if (isPrimary) {
            std::cout << "This is your primary deity." << std::endl;
        }

        if (isHolyDay(context) && context->religiousStats.hasMinimumFavor(deityId, -10)) {
            std::cout << "\nToday is a holy day for " << deityName << "!" << std::endl;
            std::cout << "Special blessings and rituals are available." << std::endl;
        }
    }

    std::cout << "\nTemples:" << std::endl;
    if (temples.empty()) {
        std::cout << "No known temples." << std::endl;
    } else {
        for (const auto& temple : temples) {
            std::cout << "- " << temple->templeName << " in " << temple->templeLocation << std::endl;
        }
    }
}

bool DeityNode::isHolyDay(ReligiousGameContext* context) const
{
    if (!context)
        return false;
    return context->getDeityOfCurrentHolyDay() == deityId;
}

std::string DeityNode::getFavorLevel(int favor) const
{
    if (favor >= 90)
        return "Exalted";
    if (favor >= 75)
        return "Revered";
    if (favor >= 50)
        return "Honored";
    if (favor >= 25)
        return "Friendly";
    if (favor >= 10)
        return "Liked";
    if (favor >= -10)
        return "Neutral";
    if (favor >= -25)
        return "Disliked";
    if (favor >= -50)
        return "Unfriendly";
    if (favor >= -75)
        return "Hostile";
    return "Hated";
}

bool DeityNode::canGrantBlessing(ReligiousGameContext* context, const std::string& blessingId) const
{
    if (!context)
        return false;

    // Check if player has enough favor
    int favor = context->religiousStats.deityFavor[deityId];

    // Basic blessings require at least 10 favor
    if (favor < 10)
        return false;

    // Greater blessings require at least 50 favor
    if (blessingId.find("greater_") != std::string::npos && favor < 50)
        return false;

    // Divine blessings require at least 75 favor and primary deity status
    if (blessingId.find("divine_") != std::string::npos) {
        if (favor < 75)
            return false;
        if (context->religiousStats.primaryDeity != deityId)
            return false;
    }

    return true;
}

void DeityNode::processDevotionAction(ReligiousGameContext* context, const std::string& actionType)
{
    if (!context)
        return;

    // Process action based on deity's favored/disfavored actions
    for (const auto& action : favoredActions) {
        if (action.action == actionType) {
            context->religiousStats.changeFavor(deityId, action.favorChange);
            context->religiousStats.addDevotion(deityId, action.favorChange > 0 ? action.favorChange : 0);
            std::cout << action.description << std::endl;
            std::cout << "Your favor with " << deityName << " has increased by " << action.favorChange << "." << std::endl;

            // Apply opposing deity effects
            for (const auto& opposingDeity : opposingDeities) {
                int opposingPenalty = action.favorChange / 2;
                if (opposingPenalty > 0) {
                    context->religiousStats.changeFavor(opposingDeity, -opposingPenalty);
                    std::cout << "Your favor with the opposing deity has decreased." << std::endl;
                }
            }
            return;
        }
    }

    for (const auto& action : disfavoredActions) {
        if (action.action == actionType) {
            context->religiousStats.changeFavor(deityId, action.favorChange);
            std::cout << action.description << std::endl;
            std::cout << "Your favor with " << deityName << " has decreased by " << -action.favorChange << "." << std::endl;
            return;
        }
    }
}

std::vector<TAAction> DeityNode::getAvailableActions()
{
    std::vector<TAAction> actions = TANode::getAvailableActions();

    // Add deity-specific actions
    actions.push_back({ "select_as_primary",
        "Select as primary deity",
        [this]() -> TAInput {
            return { "deity_action", { { "action", std::string("set_primary") }, { "deity", deityId } } };
        } });

    actions.push_back({ "pray_to_deity",
        "Pray to " + deityName,
        [this]() -> TAInput {
            return { "deity_action", { { "action", std::string("pray") }, { "deity", deityId } } };
        } });

    actions.push_back({ "view_blessings",
        "View available blessings",
        [this]() -> TAInput {
            return { "deity_action", { { "action", std::string("view_blessings") }, { "deity", deityId } } };
        } });

    actions.push_back({ "view_temples",
        "Find temples",
        [this]() -> TAInput {
            return { "deity_action", { { "action", std::string("view_temples") }, { "deity", deityId } } };
        } });

    actions.push_back({ "back_to_pantheon",
        "Return to pantheon view",
        [this]() -> TAInput {
            return { "deity_action", { { "action", std::string("back") } } };
        } });

    return actions;
}

bool DeityNode::evaluateTransition(const TAInput& input, TANode*& outNextNode)
{
    if (input.type == "deity_action") {
        std::string action = std::get<std::string>(input.parameters.at("action"));

        if (action == "set_primary") {
            // Stay on same node but update primary deity
            outNextNode = this;
            return true;
        } else if (action == "pray") {
            // Find prayer node if available
            for (TANode* child : childNodes) {
                if (dynamic_cast<PrayerNode*>(child)) {
                    outNextNode = child;
                    return true;
                }
            }
            // If no specific prayer node, stay on same node
            outNextNode = this;
            return true;
        } else if (action == "view_blessings") {
            // Find a blessing node to transition to
            if (!availableBlessings.empty()) {
                outNextNode = availableBlessings[0]; // Go to first blessing view
                return true;
            }
            // Stay on same node if no blessings
            outNextNode = this;
            return true;
        } else if (action == "view_temples") {
            // Find a temple to transition to
            if (!temples.empty()) {
                outNextNode = temples[0]; // Go to first temple
                return true;
            }
            // Stay on same node if no temples
            outNextNode = this;
            return true;
        } else if (action == "back") {
            // Return to parent/pantheon node
            for (const auto& rule : transitionRules) {
                if (rule.description == "Return to pantheon") {
                    outNextNode = rule.targetNode;
                    return true;
                }
            }
        }
    }

    return TANode::evaluateTransition(input, outNextNode);
}