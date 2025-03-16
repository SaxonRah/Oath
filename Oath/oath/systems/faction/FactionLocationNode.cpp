// systems/faction/FactionLocationNode.cpp
#include "FactionLocationNode.hpp"
#include "FactionSystemNode.hpp"
#include "core/TAController.hpp"
#include <iostream>

namespace oath {

FactionLocationNode::FactionLocationNode(const std::string& name, const std::string& location,
    const std::string& faction, int minReputation, int minRank)
    : LocationNode(name, location)
    , controllingFactionId(faction)
    , minReputationRequired(minReputation)
    , minRankRequired(minRank)
    , restrictAccess(true)
{
}

bool FactionLocationNode::canAccess(const GameContext& context)
{
    // Call base implementation for standard access rules
    if (!LocationNode::canAccess(context)) {
        return false;
    }

    // If no access restriction, allow entry
    if (!restrictAccess) {
        return true;
    }

    // Check faction access requirements
    FactionSystemNode* factionSystem = findFactionSystem(&context);
    if (factionSystem) {
        auto it = factionSystem->factions.find(controllingFactionId);
        if (it != factionSystem->factions.end()) {
            const Faction& faction = it->second;

            // Check reputation requirement
            if (faction.playerReputation < minReputationRequired) {
                return false;
            }

            // Check rank requirement
            if (faction.playerRank < minRankRequired) {
                return false;
            }

            // Access granted
            return true;
        }
    }

    // Default to base behavior if faction system not found
    return true;
}

void FactionLocationNode::onEnter(GameContext* context)
{
    // Call base implementation to display standard location info
    LocationNode::onEnter(context);

    // Add faction-specific description
    FactionSystemNode* factionSystem = findFactionSystem(context);
    if (factionSystem) {
        auto it = factionSystem->factions.find(controllingFactionId);
        if (it != factionSystem->factions.end()) {
            const Faction& faction = it->second;

            std::cout << "This location is controlled by the " << faction.name << "." << std::endl;

            if (faction.playerRank > 0) {
                std::cout << "As a " << faction.getCurrentRankTitle() << " of the "
                          << faction.name << ", you receive certain privileges here." << std::endl;

                // Display price modifications
                float priceModifier = faction.getTradeModifier();
                if (priceModifier < 1.0f) {
                    int discount = static_cast<int>((1.0f - priceModifier) * 100);
                    std::cout << "You receive a " << discount << "% discount on goods and services." << std::endl;
                }
            }
        }
    }
}

float FactionLocationNode::getPriceModifier(const std::string& serviceType, GameContext* context) const
{
    float baseModifier = 1.0f;

    // Apply faction-specific price modifier
    FactionSystemNode* factionSystem = findFactionSystem(context);
    if (factionSystem) {
        auto it = factionSystem->factions.find(controllingFactionId);
        if (it != factionSystem->factions.end()) {
            const Faction& faction = it->second;
            baseModifier = faction.getTradeModifier();
        }
    }

    // Apply service-specific modifier
    auto it = servicePriceModifiers.find(serviceType);
    if (it != servicePriceModifiers.end()) {
        baseModifier *= it->second;
    }

    return baseModifier;
}

FactionSystemNode* FactionLocationNode::findFactionSystem(const GameContext* context) const
{
    // This implementation would depend on how you access the faction system from the context
    // For example, you might get it from the controller using a system lookup
    if (context && context->controller) {
        return static_cast<FactionSystemNode*>(context->controller->getSystemNode("FactionSystem"));
    }
    return nullptr;
}

} // namespace oath