// systems/faction/FactionSystemNode.cpp
#include "FactionSystemNode.hpp"
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>

namespace oath {

FactionSystemNode::FactionSystemNode(const std::string& name, const std::string& configFile)
    : TANode(name)
    , jsonFilePath(configFile)
{
    loadFromJson();
}

void FactionSystemNode::loadFromJson()
{
    // Clear existing data
    factions.clear();
    factionRelations.clear();

    try {
        // Check if file exists
        if (!std::filesystem::exists(jsonFilePath)) {
            std::cerr << "Error: Faction config file not found: " << jsonFilePath << std::endl;
            return;
        }

        // Open and parse JSON file
        std::ifstream file(jsonFilePath);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open faction config file: " << jsonFilePath << std::endl;
            return;
        }

        json j;
        file >> j;
        file.close();

        // Load global configuration
        json rankTitlesJson;
        json rankRequirementsJson;
        json specialPrivilegesJson;
        json reputationStatesJson;
        json relationStatesJson;

        if (j.contains("rankTitles") && j["rankTitles"].is_object()) {
            rankTitlesJson = j["rankTitles"];
        }

        if (j.contains("rankRequirements") && j["rankRequirements"].is_object()) {
            rankRequirementsJson = j["rankRequirements"];
        }

        if (j.contains("specialPrivileges") && j["specialPrivileges"].is_object()) {
            specialPrivilegesJson = j["specialPrivileges"];
        }

        if (j.contains("reputationStates") && j["reputationStates"].is_object()) {
            reputationStatesJson = j["reputationStates"];
        }

        if (j.contains("relationStates") && j["relationStates"].is_object()) {
            relationStatesJson = j["relationStates"];
        }

        // Load factions
        if (j.contains("factions") && j["factions"].is_object()) {
            for (auto& [factionId, factionData] : j["factions"].items()) {
                Faction faction = Faction::fromJson(factionData);

                // Apply global configurations
                if (!rankTitlesJson.is_null()) {
                    faction.rankTitles.clear();
                    for (auto& [rankStr, title] : rankTitlesJson.items()) {
                        int rank = std::stoi(rankStr);
                        faction.rankTitles[rank] = title;
                    }
                }

                if (!rankRequirementsJson.is_null()) {
                    faction.rankRequirements.clear();
                    for (auto& [rankStr, requirements] : rankRequirementsJson.items()) {
                        int rank = std::stoi(rankStr);
                        int reputation = requirements["reputation"];
                        int quests = requirements["quests"];
                        faction.rankRequirements[rank] = { reputation, quests };
                    }
                }

                if (!reputationStatesJson.is_null()) {
                    faction.reputationThresholds.clear();
                    for (auto& [state, threshold] : reputationStatesJson.items()) {
                        faction.reputationThresholds[state] = threshold;
                    }
                    faction.updatePlayerReputationState();
                }

                factions[factionId] = faction;
            }
        }

        // Load faction relations
        if (j.contains("factionRelations") && j["factionRelations"].is_object()) {
            for (auto& [factionA, relations] : j["factionRelations"].items()) {
                if (relations.is_object()) {
                    for (auto& [factionB, value] : relations.items()) {
                        // Create the relationship with value from JSON
                        FactionRelationship relationship(factionA, factionB, value);

                        // Apply global relation thresholds if available
                        if (!relationStatesJson.is_null()) {
                            relationship.loadThresholds(relationStatesJson);
                        }

                        // Store the relationship
                        factionRelations[factionA][factionB] = relationship;
                    }
                }
            }
        }

        std::cout << "Successfully loaded faction data from: " << jsonFilePath << std::endl;
        std::cout << "Loaded " << factions.size() << " factions and their relationships." << std::endl;

    } catch (const json::exception& e) {
        std::cerr << "JSON parsing error: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        // systems/faction/FactionSystemNode.cpp (continued)
        std::cerr << "Error loading faction data: " << e.what() << std::endl;
    }
}

bool FactionSystemNode::saveToJson(const std::string& outputPath)
{
    try {
        std::string savePath = outputPath.empty() ? jsonFilePath : outputPath;

        json j;

        // Save factions
        j["factions"] = json::object();
        for (const auto& [id, faction] : factions) {
            j["factions"][id] = faction.toJson();
        }

        // Save faction relations
        j["factionRelations"] = json::object();
        for (const auto& [factionA, relations] : factionRelations) {
            j["factionRelations"][factionA] = json::object();
            for (const auto& [factionB, relation] : relations) {
                j["factionRelations"][factionA][factionB] = relation.relationValue;
            }
        }

        // Write to file
        std::ofstream file(savePath);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file for writing: " << savePath << std::endl;
            return false;
        }

        file << std::setw(4) << j << std::endl;
        file.close();

        std::cout << "Successfully saved faction data to: " << savePath << std::endl;
        return true;

    } catch (const std::exception& e) {
        std::cerr << "Error saving faction data: " << e.what() << std::endl;
        return false;
    }
}

void FactionSystemNode::addFaction(const Faction& faction)
{
    factions[faction.id] = faction;

    // Initialize relationships with existing factions
    for (const auto& [otherId, otherFaction] : factions) {
        if (otherId != faction.id) {
            // Default relationship is neutral (0)
            FactionRelationship relationship(faction.id, otherId);

            // Apply global relation thresholds if available from a faction that has them
            if (!factionRelations.empty() && !factionRelations.begin()->second.empty()) {
                auto& someRelation = factionRelations.begin()->second.begin()->second;
                relationship.relationThresholds = someRelation.relationThresholds;
                relationship.updateState();
            }

            factionRelations[faction.id][otherId] = relationship;
            factionRelations[otherId][faction.id] = FactionRelationship(otherId, faction.id);
        }
    }
}

bool FactionSystemNode::adjustFactionRelation(const std::string& factionA, const std::string& factionB, int amount)
{
    if (factions.find(factionA) == factions.end() || factions.find(factionB) == factions.end()) {
        return false;
    }

    // Make sure the relation maps exist
    if (factionRelations.find(factionA) == factionRelations.end()) {
        factionRelations[factionA] = std::map<std::string, FactionRelationship>();
    }
    if (factionRelations[factionA].find(factionB) == factionRelations[factionA].end()) {
        factionRelations[factionA][factionB] = FactionRelationship(factionA, factionB);
    }

    if (factionRelations.find(factionB) == factionRelations.end()) {
        factionRelations[factionB] = std::map<std::string, FactionRelationship>();
    }
    if (factionRelations[factionB].find(factionA) == factionRelations[factionB].end()) {
        factionRelations[factionB][factionA] = FactionRelationship(factionB, factionA);
    }

    factionRelations[factionA][factionB].changeRelation(amount);
    factionRelations[factionB][factionA].changeRelation(amount);
    return true;
}

int FactionSystemNode::getFactionRelation(const std::string& factionA, const std::string& factionB) const
{
    auto factionAIt = factionRelations.find(factionA);
    if (factionAIt == factionRelations.end()) {
        return 0;
    }

    auto relationIt = factionAIt->second.find(factionB);
    if (relationIt == factionAIt->second.end()) {
        return 0;
    }

    return relationIt->second.relationValue;
}

std::string FactionSystemNode::getFactionRelationState(const std::string& factionA, const std::string& factionB) const
{
    auto factionAIt = factionRelations.find(factionA);
    if (factionAIt == factionRelations.end()) {
        return "unknown";
    }

    auto relationIt = factionAIt->second.find(factionB);
    if (relationIt == factionAIt->second.end()) {
        return "unknown";
    }

    return relationIt->second.relationState;
}

bool FactionSystemNode::changePlayerReputation(const std::string& factionId, int amount, GameContext* context)
{
    auto it = factions.find(factionId);
    if (it == factions.end()) {
        return false;
    }

    int oldRep = it->second.playerReputation;
    it->second.changeReputation(amount);

    // Mark faction as known to player if not already
    if (!it->second.playerKnown) {
        it->second.playerKnown = true;
    }

    // Update game context if provided
    if (context) {
        context->playerStats.changeFactionRep(factionId, amount);
    }

    // Apply reputation effects to rival/ally factions
    applyReputationRippleEffects(factionId, amount);

    // Log the change
    std::cout << "Reputation with " << it->second.name << " changed: "
              << oldRep << " -> " << it->second.playerReputation
              << " (" << (amount >= 0 ? "+" : "") << amount << ")" << std::endl;

    return true;
}

void FactionSystemNode::applyReputationRippleEffects(const std::string& primaryFactionId, int primaryAmount)
{
    // Skip if very small change
    if (abs(primaryAmount) < 5) {
        return;
    }

    // Check relations with all other factions
    for (const auto& [otherId, otherFaction] : factions) {
        // Skip the primary faction
        if (otherId == primaryFactionId) {
            continue;
        }

        int relation = getFactionRelation(primaryFactionId, otherId);

        // For strong positive or negative relations, reputation changes ripple
        if (abs(relation) >= 50) {
            // Calculate ripple amount - opposite for enemies, same for allies
            int rippleAmount = 0;

            if (relation >= 50) {
                // Allies - smaller same-direction effect
                rippleAmount = primaryAmount / 2;
            } else if (relation <= -50) {
                // Enemies - smaller opposite-direction effect
                rippleAmount = -primaryAmount / 3;
            }

            // Apply the ripple effect if significant
            if (abs(rippleAmount) >= 1) {
                auto it = factions.find(otherId);
                if (it != factions.end()) {
                    int oldRep = it->second.playerReputation;
                    it->second.changeReputation(rippleAmount);

                    // Log the ripple effect
                    std::cout << "Reputation ripple effect on " << it->second.name
                              << " due to relation with " << factions[primaryFactionId].name
                              << ": " << oldRep << " -> " << it->second.playerReputation
                              << " (" << (rippleAmount >= 0 ? "+" : "") << rippleAmount << ")" << std::endl;
                }
            }
        }
    }
}

void FactionSystemNode::onEnter(GameContext* context)
{
    std::cout << "=== Faction System ===" << std::endl;
    std::cout << "Known factions: " << std::endl;

    for (const auto& [id, faction] : factions) {
        if (faction.playerKnown) {
            std::cout << "- " << faction.name << " (" << faction.playerReputationState << ")" << std::endl;
            if (faction.playerRank > 0) {
                std::cout << "  Rank: " << faction.getCurrentRankTitle() << " (" << faction.playerRank << "/10)" << std::endl;
            }
        }
    }
}

std::vector<TAAction> FactionSystemNode::getAvailableActions()
{
    std::vector<TAAction> actions = TANode::getAvailableActions();

    // Add faction-specific actions
    int index = 0;
    for (const auto& [id, faction] : factions) {
        if (faction.playerKnown) {
            // View faction details
            actions.push_back({ "view_faction_" + id,
                "View details about " + faction.name,
                [this, id]() -> TAInput {
                    return {
                        "faction_action",
                        { { "action", std::string("view") },
                            { "faction_id", id } }
                    };
                } });

            // Check rank advancement if member
            if (faction.playerRank > 0 && faction.playerRank < 10 && faction.canAdvanceRank()) {
                actions.push_back({ "advance_rank_" + id,
                    "Advance rank in " + faction.name + " to " + faction.getNextRankTitle(),
                    [this, id]() -> TAInput {
                        return {
                            "faction_action",
                            { { "action", std::string("advance_rank") },
                                { "faction_id", id } }
                        };
                    } });
            }
        }
        index++;
    }

    // Add faction relationship view
    actions.push_back({ "view_faction_relations",
        "View relationships between factions",
        [this]() -> TAInput {
            return {
                "faction_action",
                { { "action", std::string("view_relations") } }
            };
        } });

    // Add save action
    actions.push_back({ "save_faction_data",
        "Save faction system data",
        [this]() -> TAInput {
            return {
                "faction_action",
                { { "action", std::string("save") } }
            };
        } });

    return actions;
}

bool FactionSystemNode::evaluateTransition(const TAInput& input, TANode*& outNextNode)
{
    if (input.type == "faction_action") {
        std::string action = std::get<std::string>(input.parameters.at("action"));

        if (action == "view") {
            std::string factionId = std::get<std::string>(input.parameters.at("faction_id"));
            displayFactionDetails(factionId);
            outNextNode = this; // Stay in faction system
            return true;
        } else if (action == "advance_rank") {
            std::string factionId = std::get<std::string>(input.parameters.at("faction_id"));
            advanceFactionRank(factionId);
            outNextNode = this; // Stay in faction system
            return true;
        } else if (action == "view_relations") {
            displayFactionRelations();
            outNextNode = this; // Stay in faction system
            return true;
        } else if (action == "save") {
            saveToJson();
            outNextNode = this; // Stay in faction system
            return true;
        }
    }

    return TANode::evaluateTransition(input, outNextNode);
}

void FactionSystemNode::displayFactionDetails(const std::string& factionId)
{
    auto it = factions.find(factionId);
    if (it == factions.end()) {
        std::cout << "Faction not found." << std::endl;
        return;
    }

    const Faction& faction = it->second;

    std::cout << "=== " << faction.name << " ===" << std::endl;
    std::cout << faction.description << std::endl;
    std::cout << std::endl;

    std::cout << "Current State: " << faction.currentState << std::endl;
    std::cout << "Economic Power: " << faction.economicPower << "/100" << std::endl;
    std::cout << "Military Strength: " << faction.militaryPower << "/100" << std::endl;
    std::cout << "Political Influence: " << faction.politicalInfluence << "/100" << std::endl;
    std::cout << std::endl;

    std::cout << "Your Reputation: " << faction.playerReputation << " (" << faction.playerReputationState << ")" << std::endl;

    if (faction.playerRank > 0) {
        std::cout << "Your Rank: " << faction.getCurrentRankTitle() << " (" << faction.playerRank << "/10)" << std::endl;

        if (faction.playerRank < 10) {
            std::cout << "Next Rank: " << faction.getNextRankTitle() << std::endl;

            if (faction.canAdvanceRank()) {
                std::cout << "You are eligible for promotion!" << std::endl;
            } else {
                std::cout << "Requirements for next rank:" << std::endl;

                auto reqIt = faction.rankRequirements.find(faction.playerRank);
                if (reqIt != faction.rankRequirements.end()) {
                    const auto& [reqRep, reqQuests] = reqIt->second;
                    std::cout << "- Reputation: " << faction.playerReputation << "/" << reqRep << std::endl;
                    std::cout << "- Completed quests: " << faction.completedQuests.size() << "/" << reqQuests << std::endl;
                }
            }
        } else {
            std::cout << "You have achieved the highest rank in this faction!" << std::endl;
        }

        if (!faction.specialPrivileges.empty()) {
            std::cout << "Your special privileges:" << std::endl;
            for (const auto& privilege : faction.specialPrivileges) {
                std::cout << "- " << privilege << std::endl;
            }
        }
    } else {
        std::cout << "You are not a member of this faction." << std::endl;

        if (faction.playerReputation >= 10) {
            std::cout << "You are eligible to join this faction." << std::endl;
        }
    }

    std::cout << std::endl;

    // Show territories
    if (!faction.territories.empty()) {
        std::cout << "Territories:" << std::endl;
        for (const auto& territory : faction.territories) {
            std::cout << "- " << territory << std::endl;
        }
        std::cout << std::endl;
    }

    // Show faction leaders if known
    if (!faction.leaders.empty()) {
        std::cout << "Known leaders:" << std::endl;
        for (const auto& leader : faction.leaders) {
            std::cout << "- " << leader << std::endl;
        }
        std::cout << std::endl;
    }

    // Show faction allies and enemies
    std::cout << "Relations with other factions:" << std::endl;
    for (const auto& [otherId, otherFaction] : factions) {
        if (otherId != factionId && otherFaction.playerKnown) {
            std::string relationState = getFactionRelationState(factionId, otherId);
            std::cout << "- " << otherFaction.name << ": " << relationState << std::endl;
        }
    }
}

void FactionSystemNode::displayFactionRelations()
{
    std::cout << "=== Faction Relationships ===" << std::endl;

    // Build a list of known factions
    std::vector<std::string> knownFactionIds;
    for (const auto& [id, faction] : factions) {
        if (faction.playerKnown) {
            knownFactionIds.push_back(id);
        }
    }

    // Output header row
    std::cout << std::setw(20) << "Faction"
              << " | ";
    for (const auto& id : knownFactionIds) {
        std::cout << std::setw(10) << factions[id].name << " | ";
    }
    std::cout << std::endl;

    // Output separator
    std::cout << std::setfill('-') << std::setw(20) << ""
              << "-|-";
    for (const auto& id : knownFactionIds) {
        std::cout << std::setw(10) << ""
                  << "-|-";
    }
    std::cout << std::setfill(' ') << std::endl;

    // Output each faction's relations
    for (const auto& rowId : knownFactionIds) {
        std::cout << std::setw(20) << factions[rowId].name << " | ";

        for (const auto& colId : knownFactionIds) {
            if (rowId == colId) {
                std::cout << std::setw(10) << "---"
                          << " | ";
            } else {
                std::string relation = getFactionRelationState(rowId, colId);
                std::cout << std::setw(10) << relation << " | ";
            }
        }
        std::cout << std::endl;
    }
}

void FactionSystemNode::advanceFactionRank(const std::string& factionId)
{
    auto it = factions.find(factionId);
    if (it == factions.end()) {
        std::cout << "Faction not found." << std::endl;
        return;
    }

    Faction& faction = it->second;

    if (faction.tryAdvanceRank()) {
        std::cout << "Congratulations! You have been promoted to "
                  << faction.getCurrentRankTitle() << " in the "
                  << faction.name << "." << std::endl;

        // Check for newly unlocked privileges
        if (faction.specialPrivileges.find("faction_equipment_access") != faction.specialPrivileges.end() && faction.playerRank == 3) {
            std::cout << "You now have access to faction equipment and supplies." << std::endl;
        }

        if (faction.specialPrivileges.find("faction_housing_access") != faction.specialPrivileges.end() && faction.playerRank == 5) {
            std::cout << "You now have access to faction housing and secure storage." << std::endl;
        }

        if (faction.specialPrivileges.find("faction_special_quests") != faction.specialPrivileges.end() && faction.playerRank == 7) {
            std::cout << "You now have access to special high-ranking faction quests." << std::endl;
        }

        if (faction.specialPrivileges.find("faction_leadership_council") != faction.specialPrivileges.end() && faction.playerRank == 10) {
            std::cout << "You have achieved the highest rank and joined the leadership council!" << std::endl;
        }
    } else {
        std::cout << "You are not eligible for promotion in " << faction.name << "." << std::endl;
    }
}

} // namespace oath