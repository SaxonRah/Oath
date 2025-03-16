// systems/faction/FactionSystemNode.hpp
#ifndef OATH_FACTION_SYSTEM_NODE_HPP
#define OATH_FACTION_SYSTEM_NODE_HPP

#include "Faction.hpp"
#include "FactionRelationship.hpp"
#include "core/TAAction.hpp"
#include "core/TAInput.hpp"
#include "core/TANode.hpp"
#include "data/GameContext.hpp"


#include <map>
#include <string>
#include <vector>


namespace oath {

// Main faction system node
class FactionSystemNode : public TANode {
public:
    std::map<std::string, Faction> factions;
    std::map<std::string, std::map<std::string, FactionRelationship>> factionRelations;
    std::string jsonFilePath;

    FactionSystemNode(const std::string& name, const std::string& configFile = "resources/config/FactionReputation.json");

    // Load faction system data from JSON file
    void loadFromJson();

    // Save faction system data to JSON file
    bool saveToJson(const std::string& outputPath = "");

    void addFaction(const Faction& faction);
    bool adjustFactionRelation(const std::string& factionA, const std::string& factionB, int amount);
    int getFactionRelation(const std::string& factionA, const std::string& factionB) const;
    std::string getFactionRelationState(const std::string& factionA, const std::string& factionB) const;

    // Change player reputation with a faction
    bool changePlayerReputation(const std::string& factionId, int amount, GameContext* context = nullptr);

    // Apply reputation changes to other factions based on their relationships
    void applyReputationRippleEffects(const std::string& primaryFactionId, int primaryAmount);

    void onEnter(GameContext* context) override;
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;

    void displayFactionDetails(const std::string& factionId);
    void displayFactionRelations();
    void advanceFactionRank(const std::string& factionId);
};

} // namespace oath

#endif // OATH_FACTION_SYSTEM_NODE_HPP