#include "systems/religion/BlessingNode.hpp"
#include "core/TAAction.hpp"
#include "core/TAInput.hpp"
#include <iostream>
#include <nlohmann/json.hpp>

BlessingNode::BlessingNode(const std::string& id, const std::string& name, const std::string& deity, BlessingTier t)
    : TANode(name)
    , blessingId(id)
    , blessingName(name)
    , deityId(deity)
    , tier(t)
{
    // Set default values based on tier
    switch (tier) {
    case MINOR:
        favorRequirement = 10;
        duration = 7;
        break;
    case MODERATE:
        favorRequirement = 25;
        duration = 14;
        break;
    case MAJOR:
        favorRequirement = 50;
        duration = 21;
        break;
    case GREATER:
        favorRequirement = 75;
        duration = 30;
        break;
    case DIVINE:
        favorRequirement = 90;
        duration = 60;
        break;
    }
}

void BlessingNode::loadFromJson(const nlohmann::json& blessingData)
{
    blessingDescription = blessingData["description"];
    favorRequirement = blessingData["favorRequirement"];
    duration = blessingData["duration"];

    // Load effects
    effects.clear();
    for (const auto& effect : blessingData["effects"]) {
        effects.push_back({ effect["type"],
            effect["target"],
            effect["magnitude"],
            effect["description"] });
    }
}

void BlessingNode::onEnter(ReligiousGameContext* context)
{
    std::cout << "=== " << blessingName << " ===\n"
              << std::endl;
    std::cout << blessingDescription << std::endl;

    std::cout << "\nTier: " << getTierName() << std::endl;
    std::cout << "Duration: " << duration << " days" << std::endl;
    std::cout << "Favor Required: " << favorRequirement << std::endl;

    std::cout << "\nEffects:" << std::endl;
    for (const auto& effect : effects) {
        std::cout << "- " << effect.description << std::endl;
    }

    if (context) {
        bool hasBlessing = context->religiousStats.hasBlessingActive(blessingId);
        int currentFavor = context->religiousStats.deityFavor[deityId];

        if (hasBlessing) {
            int remainingDuration = context->religiousStats.blessingDuration[blessingId];
            std::cout << "\nThis blessing is currently active. " << remainingDuration << " days remaining." << std::endl;
        }

        if (currentFavor < favorRequirement) {
            std::cout << "\nYou need " << (favorRequirement - currentFavor) << " more favor to receive this blessing." << std::endl;
        }
    }
}

std::string BlessingNode::getTierName() const
{
    switch (tier) {
    case MINOR:
        return "Minor";
    case MODERATE:
        return "Moderate";
    case MAJOR:
        return "Major";
    case GREATER:
        return "Greater";
    case DIVINE:
        return "Divine";
    default:
        return "Unknown";
    }
}

bool BlessingNode::canReceiveBlessing(ReligiousGameContext* context) const
{
    if (!context)
        return false;

    // Check if player has enough favor
    int favor = context->religiousStats.deityFavor[deityId];
    if (favor < favorRequirement) {
        return false;
    }

    // Divine blessings require primary deity status
    if (tier == DIVINE && context->religiousStats.primaryDeity != deityId) {
        return false;
    }

    return true;
}

bool BlessingNode::grantBlessing(ReligiousGameContext* context)
{
    if (!context)
        return false;

    if (!canReceiveBlessing(context)) {
        std::cout << "You cannot receive this blessing. You need more favor with the deity." << std::endl;

        if (tier == DIVINE && context->religiousStats.primaryDeity != deityId) {
            std::cout << "Divine blessings can only be granted by your primary deity." << std::endl;
        }

        return false;
    }

    // Add blessing to active blessings
    context->religiousStats.addBlessing(blessingId, duration);

    // Apply immediate effects
    for (const auto& effect : effects) {
        if (effect.type == "stat") {
            if (effect.target == "strength")
                context->playerStats.strength += effect.magnitude;
            else if (effect.target == "dexterity")
                context->playerStats.dexterity += effect.magnitude;
            else if (effect.target == "constitution")
                context->playerStats.constitution += effect.magnitude;
            else if (effect.target == "intelligence")
                context->playerStats.intelligence += effect.magnitude;
            else if (effect.target == "wisdom")
                context->playerStats.wisdom += effect.magnitude;
            else if (effect.target == "charisma")
                context->playerStats.charisma += effect.magnitude;
        } else if (effect.type == "skill") {
            context->playerStats.improveSkill(effect.target, effect.magnitude);
        }
    }

    std::cout << "The " << blessingName << " blessing has been granted to you for " << duration << " days." << std::endl;

    // Small favor cost for receiving the blessing
    int favorCost = tier * 2;
    context->religiousStats.changeFavor(deityId, -favorCost);
    std::cout << "You have spent " << favorCost << " favor to receive this blessing." << std::endl;

    return true;
}

std::vector<TAAction> BlessingNode::getAvailableActions()
{
    std::vector<TAAction> actions = TANode::getAvailableActions();

    // Add blessing actions
    actions.push_back({ "request_blessing",
        "Request this blessing",
        [this]() -> TAInput {
            return { "blessing_action", { { "action", std::string("request") } } };
        } });

    actions.push_back({ "return_to_deity",
        "Return to deity view",
        [this]() -> TAInput {
            return { "blessing_action", { { "action", std::string("back") } } };
        } });

    return actions;
}