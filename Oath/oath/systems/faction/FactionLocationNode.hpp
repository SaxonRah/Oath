// systems/faction/FactionLocationNode.hpp
#ifndef OATH_FACTION_LOCATION_NODE_HPP
#define OATH_FACTION_LOCATION_NODE_HPP

#include "data/GameContext.hpp"
#include "systems/world/LocationNode.hpp"


#include <map>
#include <string>


namespace oath {

// Faction-specific location with access control
class FactionLocationNode : public LocationNode {
public:
    std::string controllingFactionId;
    int minReputationRequired;
    int minRankRequired;
    bool restrictAccess;
    std::map<std::string, float> servicePriceModifiers; // Affects prices for different services

    FactionLocationNode(const std::string& name, const std::string& location,
        const std::string& faction, int minReputation = 0, int minRank = 0);

    bool canAccess(const GameContext& context) override;
    void onEnter(GameContext* context) override;
    float getPriceModifier(const std::string& serviceType, GameContext* context) const;

private:
    // Helper function to find the faction system
    FactionSystemNode* findFactionSystem(const GameContext* context) const;
};

} // namespace oath

#endif // OATH_FACTION_LOCATION_NODE_HPP