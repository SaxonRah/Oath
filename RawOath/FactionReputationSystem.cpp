#include <algorithm>
#include <ctime>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <queue>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

// Forward declarations from RawOathFull.cpp
class TANode;
class TAController;
class Inventory;
class NPC;
class Recipe;
class GameContext;
class QuestNode;
class DialogueNode;
class SkillNode;
class ClassNode;
class CraftingNode;
class LocationNode;
class TimeNode;
struct NodeID;
struct TAInput;
struct TAAction;
struct TATransitionRule;
struct CharacterStats;
struct WorldState;

// New faction-specific classes and structures
struct FactionRelationship {
    std::string factionA;
    std::string factionB;
    int relationValue; // -100 to 100
    std::string relationState; // "war", "hostile", "unfriendly", "neutral", "friendly", "allied"

    FactionRelationship(const std::string& a, const std::string& b, int value = 0)
        : factionA(a)
        , factionB(b)
        , relationValue(value)
    {
        updateState();
    }

    void updateState()
    {
        if (relationValue <= -75)
            relationState = "war";
        else if (relationValue <= -50)
            relationState = "hostile";
        else if (relationValue <= -20)
            relationState = "unfriendly";
        else if (relationValue < 20)
            relationState = "neutral";
        else if (relationValue < 50)
            relationState = "friendly";
        else if (relationValue < 75)
            relationState = "cordial";
        else
            relationState = "allied";
    }

    void changeRelation(int amount)
    {
        relationValue = std::max(-100, std::min(100, relationValue + amount));
        updateState();
    }
};

class Faction {
public:
    std::string id;
    std::string name;
    std::string description;
    std::vector<std::string> territories; // Region IDs this faction controls
    std::vector<std::string> leaders; // NPC IDs of faction leaders
    std::set<std::string> members; // NPC IDs of faction members
    std::map<std::string, int> resourceInfluence; // Economic impact on resources
    std::map<std::string, int> skillBonuses; // Skill bonuses for faction members
    std::string primaryCulture; // Cultural background
    std::string politicalAlignment; // Political stance
    std::string dominantReligion; // Main religious alignment

    // Faction state management
    std::string currentState; // "stable", "growing", "declining", "at_war", "in_crisis"
    int economicPower; // 0-100
    int militaryPower; // 0-100
    int politicalInfluence; // 0-100

    // Player-specific data
    int playerRank; // 0-10, 0 = not a member
    int playerReputation; // -100 to 100
    std::string playerReputationState; // "exalted", "revered", "honored", "friendly", "neutral", "unfriendly", "hostile", "hated"
    bool playerKnown; // Does player know about this faction
    std::vector<std::string> completedQuests; // Quests completed for this faction

    // Special faction features
    std::set<std::string> specialPrivileges; // Special access granted at certain ranks
    std::map<int, std::string> rankTitles; // Titles for different ranks
    std::vector<std::string> specialLocations; // Faction-specific locations

    Faction(const std::string& factionId, const std::string& factionName)
        : id(factionId)
        , name(factionName)
        , currentState("stable")
        , economicPower(50)
        , militaryPower(50)
        , politicalInfluence(50)
        , playerRank(0)
        , playerReputation(0)
        , playerReputationState("neutral")
        , playerKnown(false)
    {
        // Initialize rank titles
        rankTitles[0] = "Outsider";
        rankTitles[1] = "Initiate";
        rankTitles[2] = "Associate";
        rankTitles[3] = "Member";
        rankTitles[4] = "Trusted";
        rankTitles[5] = "Guardian";
        rankTitles[6] = "Defender";
        rankTitles[7] = "Protector";
        rankTitles[8] = "Elder";
        rankTitles[9] = "Master";
        rankTitles[10] = "Grandmaster";
    }

    void updatePlayerReputationState()
    {
        if (playerReputation <= -90)
            playerReputationState = "hated";
        else if (playerReputation <= -70)
            playerReputationState = "despised";
        else if (playerReputation <= -30)
            playerReputationState = "hostile";
        else if (playerReputation <= -10)
            playerReputationState = "unfriendly";
        else if (playerReputation < 10)
            playerReputationState = "neutral";
        else if (playerReputation < 30)
            playerReputationState = "friendly";
        else if (playerReputation < 70)
            playerReputationState = "honored";
        else if (playerReputation < 90)
            playerReputationState = "revered";
        else
            playerReputationState = "exalted";
    }

    void changeReputation(int amount)
    {
        playerReputation = std::max(-100, std::min(100, playerReputation + amount));
        updatePlayerReputationState();
    }

    bool canAdvanceRank() const
    {
        // Requirements for each rank advancement
        switch (playerRank) {
        case 0: // Outsider to Initiate
            return playerReputation >= 10;
        case 1: // Initiate to Associate
            return playerReputation >= 20 && completedQuests.size() >= 2;
        case 2: // Associate to Member
            return playerReputation >= 30 && completedQuests.size() >= 5;
        case 3: // Member to Trusted
            return playerReputation >= 40 && completedQuests.size() >= 10;
        case 4: // Trusted to Guardian
            return playerReputation >= 50 && completedQuests.size() >= 15;
        case 5: // Guardian to Defender
            return playerReputation >= 60 && completedQuests.size() >= 20;
        case 6: // Defender to Protector
            return playerReputation >= 70 && completedQuests.size() >= 25;
        case 7: // Protector to Elder
            return playerReputation >= 80 && completedQuests.size() >= 30;
        case 8: // Elder to Master
            return playerReputation >= 90 && completedQuests.size() >= 40;
        case 9: // Master to Grandmaster
            return playerReputation >= 100 && completedQuests.size() >= 50;
        default:
            return false;
        }
    }

    bool tryAdvanceRank()
    {
        if (canAdvanceRank() && playerRank < 10) {
            playerRank++;

            // Unlock new privileges based on rank
            if (playerRank == 3) { // Member
                specialPrivileges.insert("faction_equipment_access");
            }
            if (playerRank == 5) { // Guardian
                specialPrivileges.insert("faction_housing_access");
            }
            if (playerRank == 7) { // Protector
                specialPrivileges.insert("faction_special_quests");
            }
            if (playerRank == 10) { // Grandmaster
                specialPrivileges.insert("faction_leadership_council");
            }

            return true;
        }
        return false;
    }

    float getTradeModifier() const
    {
        // Calculate price discount/premium based on reputation
        if (playerReputation <= -70)
            return 1.5f; // 50% markup
        else if (playerReputation <= -30)
            return 1.25f; // 25% markup
        else if (playerReputation <= -10)
            return 1.1f; // 10% markup
        else if (playerReputation < 10)
            return 1.0f; // standard prices
        else if (playerReputation < 30)
            return 0.95f; // 5% discount
        else if (playerReputation < 70)
            return 0.9f; // 10% discount
        else if (playerReputation < 90)
            return 0.85f; // 15% discount
        else
            return 0.75f; // 25% discount
    }

    std::string getCurrentRankTitle() const
    {
        auto it = rankTitles.find(playerRank);
        if (it != rankTitles.end()) {
            return it->second;
        }
        return "Unknown";
    }

    std::string getNextRankTitle() const
    {
        if (playerRank >= 10) {
            return "Maximum Rank Achieved";
        }

        auto it = rankTitles.find(playerRank + 1);
        if (it != rankTitles.end()) {
            return it->second;
        }
        return "Unknown";
    }

    std::string getReputationDescription() const
    {
        return playerReputationState;
    }
};

// Main faction system node
class FactionSystemNode : public TANode {
public:
    std::map<std::string, Faction> factions;
    std::map<std::string, std::map<std::string, FactionRelationship>> factionRelations;

    FactionSystemNode(const std::string& name)
        : TANode(name)
    {
    }

    void addFaction(const Faction& faction)
    {
        factions[faction.id] = faction;

        // Initialize relationships with existing factions
        for (const auto& [otherId, otherFaction] : factions) {
            if (otherId != faction.id) {
                // Default relationship is neutral (0)
                factionRelations[faction.id][otherId] = FactionRelationship(faction.id, otherId);
                factionRelations[otherId][faction.id] = FactionRelationship(otherId, faction.id);
            }
        }
    }

    bool adjustFactionRelation(const std::string& factionA, const std::string& factionB, int amount)
    {
        if (factions.find(factionA) == factions.end() || factions.find(factionB) == factions.end()) {
            return false;
        }

        factionRelations[factionA][factionB].changeRelation(amount);
        factionRelations[factionB][factionA].changeRelation(amount);
        return true;
    }

    int getFactionRelation(const std::string& factionA, const std::string& factionB) const
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

    std::string getFactionRelationState(const std::string& factionA, const std::string& factionB) const
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

    // Change player reputation with a faction
    bool changePlayerReputation(const std::string& factionId, int amount, GameContext* context = nullptr)
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

    // Apply reputation changes to other factions based on their relationships
    void applyReputationRippleEffects(const std::string& primaryFactionId, int primaryAmount)
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

    void onEnter(GameContext* context) override
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

    std::vector<TAAction> getAvailableActions() override
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

        return actions;
    }

    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override
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
            }
        }

        return TANode::evaluateTransition(input, outNextNode);
    }

    void displayFactionDetails(const std::string& factionId)
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

                    int reqRep = 0;
                    int reqQuests = 0;

                    switch (faction.playerRank) {
                    case 0:
                        reqRep = 10;
                        reqQuests = 0;
                        break;
                    case 1:
                        reqRep = 20;
                        reqQuests = 2;
                        break;
                    case 2:
                        reqRep = 30;
                        reqQuests = 5;
                        break;
                    case 3:
                        reqRep = 40;
                        reqQuests = 10;
                        break;
                    case 4:
                        reqRep = 50;
                        reqQuests = 15;
                        break;
                    case 5:
                        reqRep = 60;
                        reqQuests = 20;
                        break;
                    case 6:
                        reqRep = 70;
                        reqQuests = 25;
                        break;
                    case 7:
                        reqRep = 80;
                        reqQuests = 30;
                        break;
                    case 8:
                        reqRep = 90;
                        reqQuests = 40;
                        break;
                    case 9:
                        reqRep = 100;
                        reqQuests = 50;
                        break;
                    }

                    std::cout << "- Reputation: " << faction.playerReputation << "/" << reqRep << std::endl;
                    std::cout << "- Completed quests: " << faction.completedQuests.size() << "/" << reqQuests << std::endl;
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

    void displayFactionRelations()
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

    void advanceFactionRank(const std::string& factionId)
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
};

// Faction-specific quest node
class FactionQuestNode : public QuestNode {
public:
    std::string factionId;
    int reputationReward;
    bool rankAdvancement;
    std::map<std::string, int> reputationEffectsOnOtherFactions;

    FactionQuestNode(const std::string& name, const std::string& faction)
        : QuestNode(name)
        , factionId(faction)
        , reputationReward(10)
        , rankAdvancement(false)
    {
    }

    void onExit(GameContext* context) override
    {
        // Call base class implementation for standard rewards
        QuestNode::onExit(context);

        // If this is a completed quest (accepting state), give faction rewards
        if (isAcceptingState && context) {
            // Update the faction system if found
            FactionSystemNode* factionSystem = findFactionSystem(context);
            if (factionSystem) {
                // Add this quest to completed quests list
                auto it = factionSystem->factions.find(factionId);
                if (it != factionSystem->factions.end()) {
                    Faction& faction = it->second;
                    faction.completedQuests.push_back(nodeName);

                    // Apply reputation reward
                    factionSystem->changePlayerReputation(factionId, reputationReward, context);

                    // Apply reputation effects on other factions
                    for (const auto& [otherFaction, amount] : reputationEffectsOnOtherFactions) {
                        factionSystem->changePlayerReputation(otherFaction, amount, context);
                    }

                    // Check for rank advancement if enabled
                    if (rankAdvancement && faction.canAdvanceRank()) {
                        if (faction.tryAdvanceRank()) {
                            std::cout << "Your service has earned you promotion to "
                                      << faction.getCurrentRankTitle() << " in the "
                                      << faction.name << "." << std::endl;
                        }
                    }
                }
            }
        }
    }

private:
    // Helper function to find the faction system
    FactionSystemNode* findFactionSystem(GameContext* context)
    {
        // This would depend on how you access the faction system from the context
        // In a real implementation, this might be stored in the TAController

        // Placeholder - in real code, you'd get this from your controller
        return nullptr;
    }
};

// Faction-related dialogue node
class FactionDialogueNode : public DialogueNode {
public:
    std::string factionId;
    int reputationEffect;

    FactionDialogueNode(const std::string& name, const std::string& speaker,
        const std::string& text, const std::string& faction = "",
        int repEffect = 0)
        : DialogueNode(name, speaker, text)
        , factionId(faction)
        , reputationEffect(repEffect)
    {
    }

    void onEnter(GameContext* context) override
    {
        // Call base implementation to display dialogue
        DialogueNode::onEnter(context);

        // Apply reputation effect if applicable
        if (!factionId.empty() && reputationEffect != 0 && context) {
            // Update the faction system if found
            FactionSystemNode* factionSystem = findFactionSystem(context);
            if (factionSystem) {
                factionSystem->changePlayerReputation(factionId, reputationEffect, context);
            }
        }
    }

private:
    // Helper function to find the faction system
    FactionSystemNode* findFactionSystem(GameContext* context)
    {
        // This would depend on how you access the faction system from the context
        // In a real implementation, this might be stored in the TAController

        // Placeholder - in real code, you'd get this from your controller
        return nullptr;
    }
};

// Faction-specific location with access control
class FactionLocationNode : public LocationNode {
public:
    std::string controllingFactionId;
    int minReputationRequired;
    int minRankRequired;
    bool restrictAccess;
    std::map<std::string, float> servicePriceModifiers; // Affects prices for different services

    FactionLocationNode(const std::string& name, const std::string& location,
        const std::string& faction, int minReputation = 0, int minRank = 0)
        : LocationNode(name, location)
        , controllingFactionId(faction)
        , minReputationRequired(minReputation)
        , minRankRequired(minRank)
        , restrictAccess(true)
    {
    }

    bool canAccess(const GameContext& context) override
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

    void onEnter(GameContext* context) override
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

    float getPriceModifier(const std::string& serviceType, GameContext* context) const
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

private:
    // Helper function to find the faction system
    FactionSystemNode* findFactionSystem(const GameContext* context) const
    {
        // Placeholder - in real code, you'd get this from your controller
        return nullptr;
    }
};

// Political shift event that changes faction relations
class FactionPoliticalShiftEvent {
public:
    std::string name;
    std::string description;
    std::map<std::string, int> factionPowerShifts; // Changes to economic/military/political power
    std::map<std::pair<std::string, std::string>, int> relationShifts; // Changes to relations
    std::function<bool(const GameContext&)> condition;
    bool hasOccurred;
    int daysTillNextCheck;

    FactionPoliticalShiftEvent(const std::string& eventName, const std::string& eventDesc)
        : name(eventName)
        , description(eventDesc)
        , hasOccurred(false)
        , daysTillNextCheck(0)
    {
        // Default condition always returns true
        condition = [](const GameContext&) { return true; };
    }

    bool checkAndExecute(GameContext* context, FactionSystemNode* factionSystem)
    {
        if (hasOccurred || !factionSystem || !context) {
            return false;
        }

        // Check if event conditions are met
        if (condition(*context)) {
            std::cout << "=== Political Shift: " << name << " ===" << std::endl;
            std::cout << description << std::endl;

            // Apply power shifts to affected factions
            for (const auto& [factionId, powerChange] : factionPowerShifts) {
                auto it = factionSystem->factions.find(factionId);
                if (it != factionSystem->factions.end()) {
                    Faction& faction = it->second;

                    // Decide which power types are affected
                    // This implementation assumes even distribution, but you could specify which
                    int economicChange = powerChange / 3;
                    int militaryChange = powerChange / 3;
                    int politicalChange = powerChange - economicChange - militaryChange;

                    faction.economicPower = std::max(0, std::min(100, faction.economicPower + economicChange));
                    faction.militaryPower = std::max(0, std::min(100, faction.militaryPower + militaryChange));
                    faction.politicalInfluence = std::max(0, std::min(100, faction.politicalInfluence + politicalChange));

                    // Update faction state based on new power levels
                    updateFactionState(faction);

                    std::cout << faction.name << " has been "
                              << (powerChange > 0 ? "strengthened" : "weakened")
                              << " by this event." << std::endl;
                }
            }

            // Apply relation shifts
            for (const auto& [factionPair, relationChange] : relationShifts) {
                const std::string& factionA = factionPair.first;
                const std::string& factionB = factionPair.second;

                if (factionSystem->adjustFactionRelation(factionA, factionB, relationChange)) {
                    std::string relationState = factionSystem->getFactionRelationState(factionA, factionB);

                    std::cout << "Relations between " << factionSystem->factions[factionA].name
                              << " and " << factionSystem->factions[factionB].name
                              << " have " << (relationChange > 0 ? "improved" : "deteriorated")
                              << " to " << relationState << "." << std::endl;

                    // If relations reach war or alliance levels, this is significant
                    if (relationState == "war") {
                        std::cout << "WAR has broken out between "
                                  << factionSystem->factions[factionA].name << " and "
                                  << factionSystem->factions[factionB].name << "!" << std::endl;

                        // Update faction states to reflect war
                        auto itA = factionSystem->factions.find(factionA);
                        auto itB = factionSystem->factions.find(factionB);

                        if (itA != factionSystem->factions.end()) {
                            itA->second.currentState = "at_war";
                        }

                        if (itB != factionSystem->factions.end()) {
                            itB->second.currentState = "at_war";
                        }
                    } else if (relationState == "allied") {
                        std::cout << "A formal alliance has formed between "
                                  << factionSystem->factions[factionA].name << " and "
                                  << factionSystem->factions[factionB].name << "!" << std::endl;
                    }
                }
            }

            hasOccurred = true;
            return true;
        }

        return false;
    }

private:
    void updateFactionState(Faction& faction)
    {
        // Average the three power indicators
        int avgPower = (faction.economicPower + faction.militaryPower + faction.politicalInfluence) / 3;

        // Determine state based on average power
        if (faction.currentState == "at_war") {
            // War state persists unless explicitly changed
            return;
        } else if (avgPower < 20) {
            faction.currentState = "in_crisis";
        } else if (avgPower < 40) {
            faction.currentState = "declining";
        } else if (avgPower < 60) {
            faction.currentState = "stable";
        } else if (avgPower < 80) {
            faction.currentState = "growing";
        } else {
            faction.currentState = "flourishing";
        }
    }
};

// Main function with demonstration of faction system
int main()
{
    // Create TAController
    TAController controller;

    // Create Faction System Node
    FactionSystemNode* factionSystem = dynamic_cast<FactionSystemNode*>(
        controller.createNode<FactionSystemNode>("FactionSystem"));

    //----------------------------------------
    // FACTION DEFINITIONS
    //----------------------------------------
    std::cout << "Creating factions..." << std::endl;

    // Create Merchants Guild faction
    Faction merchantsGuild("merchants_guild", "Merchants Guild");
    merchantsGuild.description = "A powerful association of traders and merchants that controls much of the kingdom's commerce.";
    merchantsGuild.economicPower = 80;
    merchantsGuild.militaryPower = 30;
    merchantsGuild.politicalInfluence = 60;
    merchantsGuild.primaryCulture = "Mixed";
    merchantsGuild.politicalAlignment = "Neutral/Pragmatic";
    merchantsGuild.dominantReligion = "Diverse";
    merchantsGuild.territories = { "Market District", "Harbor", "Caravan Routes" };
    merchantsGuild.leaders = { "Guildmaster Harmon", "Treasurer Selina" };
    merchantsGuild.skillBonuses["trading"] = 2;
    merchantsGuild.skillBonuses["persuasion"] = 1;
    merchantsGuild.playerKnown = true; // Player knows this faction from the start

    // Create Noble Houses faction
    Faction nobleHouses("noble_houses", "Noble Houses");
    nobleHouses.description = "The aristocratic families that have ruled the kingdom for generations, maintaining their power through politics and tradition.";
    nobleHouses.economicPower = 70;
    nobleHouses.militaryPower = 50;
    nobleHouses.politicalInfluence = 90;
    nobleHouses.primaryCulture = "High Kingdom";
    nobleHouses.politicalAlignment = "Conservative";
    nobleHouses.dominantReligion = "Traditional Faith";
    nobleHouses.territories = { "Noble Quarter", "Palace District", "Countryside Estates" };
    nobleHouses.leaders = { "Lord Bastian", "Lady Elara" };
    nobleHouses.skillBonuses["etiquette"] = 2;
    nobleHouses.skillBonuses["politics"] = 2;
    nobleHouses.playerKnown = true; // Player knows this faction from the start

    // Create City Guard faction
    Faction cityGuard("city_guard", "City Guard");
    cityGuard.description = "The law enforcement organization responsible for maintaining order in the city.";
    cityGuard.economicPower = 40;
    cityGuard.militaryPower = 60;
    cityGuard.politicalInfluence = 50;
    cityGuard.primaryCulture = "Urban";
    cityGuard.politicalAlignment = "Lawful";
    cityGuard.dominantReligion = "Various";
    cityGuard.territories = { "Guard Posts", "City Walls", "Prison" };
    cityGuard.leaders = { "Captain Marcus", "Lieutenant Thorne" };
    cityGuard.skillBonuses["combat"] = 1;
    cityGuard.skillBonuses["investigation"] = 1;
    cityGuard.playerKnown = true; // Player knows this faction from the start

    // Create Mages Guild faction
    Faction magesGuild("mages_guild", "Mages Guild");
    magesGuild.description = "A scholarly organization dedicated to the study and regulation of magic throughout the kingdom.";
    magesGuild.economicPower = 50;
    magesGuild.militaryPower = 40;
    magesGuild.politicalInfluence = 70;
    magesGuild.primaryCulture = "Academic";
    magesGuild.politicalAlignment = "Progressive";
    magesGuild.dominantReligion = "Arcane Philosophy";
    magesGuild.territories = { "Mage Tower", "Academy", "Magical Study Sites" };
    magesGuild.leaders = { "Archmage Lyra", "Master Wizard Toren" };
    magesGuild.skillBonuses["magic"] = 3;
    magesGuild.skillBonuses["alchemy"] = 1;
    magesGuild.playerKnown = true; // Player knows this faction from the start

    // Create Thieves' Guild faction
    Faction thievesGuild("thieves_guild", "Shadow Network");
    thievesGuild.description = "A secretive organization of thieves, smugglers, and information brokers operating in the shadows of society.";
    thievesGuild.economicPower = 60;
    thievesGuild.militaryPower = 30;
    thievesGuild.politicalInfluence = 40;
    thievesGuild.primaryCulture = "Criminal";
    thievesGuild.politicalAlignment = "Chaotic";
    thievesGuild.dominantReligion = "Cult of Shadows";
    thievesGuild.territories = { "Undercity", "Sewers", "Shadowy Corners" };
    thievesGuild.leaders = { "The Phantom", "Silvertongue" };
    thievesGuild.skillBonuses["stealth"] = 3;
    thievesGuild.skillBonuses["lockpicking"] = 2;
    thievesGuild.playerKnown = false; // Player must discover this faction

    // Create Forest Wardens faction
    Faction forestWardens("forest_wardens", "Forest Wardens");
    forestWardens.description = "Protectors of the ancient forests, balancing nature's needs with those of civilization.";
    forestWardens.economicPower = 30;
    forestWardens.militaryPower = 50;
    forestWardens.politicalInfluence = 30;
    forestWardens.primaryCulture = "Sylvan";
    forestWardens.politicalAlignment = "Neutral";
    forestWardens.dominantReligion = "Nature Worship";
    forestWardens.territories = { "Great Forest", "Sacred Groves", "Ranger Outposts" };
    forestWardens.leaders = { "Elder Tristan", "Ranger Captain Faye" };
    forestWardens.skillBonuses["survival"] = 3;
    forestWardens.skillBonuses["archery"] = 2;
    forestWardens.playerKnown = false; // Player must discover this faction

    // Create Desert Nomads faction
    Faction desertNomads("desert_nomads", "Desert Nomads");
    desertNomads.description = "Proud wanderers of the great desert, masters of survival in the harshest environment.";
    desertNomads.economicPower = 40;
    desertNomads.militaryPower = 60;
    desertNomads.politicalInfluence = 20;
    desertNomads.primaryCulture = "Nomadic";
    desertNomads.politicalAlignment = "Traditional";
    desertNomads.dominantReligion = "Sun and Stars Faith";
    desertNomads.territories = { "Desert Oases", "Caravan Routes", "Hidden Valleys" };
    desertNomads.leaders = { "Sheikh Amir", "Caravan Master Zara" };
    desertNomads.skillBonuses["survival"] = 2;
    desertNomads.skillBonuses["riding"] = 2;
    desertNomads.playerKnown = false; // Player must discover this faction

    // Add factions to the faction system
    factionSystem->addFaction(merchantsGuild);
    factionSystem->addFaction(nobleHouses);
    factionSystem->addFaction(cityGuard);
    factionSystem->addFaction(magesGuild);
    factionSystem->addFaction(thievesGuild);
    factionSystem->addFaction(forestWardens);
    factionSystem->addFaction(desertNomads);

    //----------------------------------------
    // INITIAL FACTION RELATIONS SETUP
    //----------------------------------------
    std::cout << "Setting up initial faction relationships..." << std::endl;

    // Merchants Guild relations
    factionSystem->adjustFactionRelation("merchants_guild", "noble_houses", 30); // Cordial business relations
    factionSystem->adjustFactionRelation("merchants_guild", "city_guard", 50); // Strong cooperation
    factionSystem->adjustFactionRelation("merchants_guild", "mages_guild", 10); // Limited business
    factionSystem->adjustFactionRelation("merchants_guild", "thieves_guild", -60); // Hostile due to theft
    factionSystem->adjustFactionRelation("merchants_guild", "forest_wardens", 10); // Trade for forest goods
    factionSystem->adjustFactionRelation("merchants_guild", "desert_nomads", 40); // Important trade partners

    // Noble Houses relations
    factionSystem->adjustFactionRelation("noble_houses", "city_guard", 70); // Control the guard
    factionSystem->adjustFactionRelation("noble_houses", "mages_guild", 20); // Cautious respect
    factionSystem->adjustFactionRelation("noble_houses", "thieves_guild", -80); // Despise criminals
    factionSystem->adjustFactionRelation("noble_houses", "forest_wardens", -10); // Land disputes
    factionSystem->adjustFactionRelation("noble_houses", "desert_nomads", -20); // Cultural differences

    // City Guard relations
    factionSystem->adjustFactionRelation("city_guard", "mages_guild", 30); // Magical assistance
    factionSystem->adjustFactionRelation("city_guard", "thieves_guild", -90); // Sworn enemies
    factionSystem->adjustFactionRelation("city_guard", "forest_wardens", 20); // Mutual respect
    factionSystem->adjustFactionRelation("city_guard", "desert_nomads", 0); // Neutral

    // Mages Guild relations
    factionSystem->adjustFactionRelation("mages_guild", "thieves_guild", -40); // Magical items stolen
    factionSystem->adjustFactionRelation("mages_guild", "forest_wardens", 50); // Magical nature interest
    factionSystem->adjustFactionRelation("mages_guild", "desert_nomads", 10); // Some magical trade

    // Thieves Guild relations
    factionSystem->adjustFactionRelation("thieves_guild", "forest_wardens", -20); // Some forest thefts
    factionSystem->adjustFactionRelation("thieves_guild", "desert_nomads", 20); // Smuggling partners

    // Forest Wardens relations
    factionSystem->adjustFactionRelation("forest_wardens", "desert_nomads", 30); // Respect for nature

    //----------------------------------------
    // POLITICAL EVENTS SETUP
    //----------------------------------------
    std::cout << "Setting up political events..." << std::endl;

    // Create some political shift events
    FactionPoliticalShiftEvent merchantScandal("Merchant Corruption Scandal",
        "A major corruption scandal has rocked the Merchants Guild, with several high-ranking members implicated in tax evasion and bribery of officials.");
    merchantScandal.factionPowerShifts["merchants_guild"] = -15;
    merchantScandal.factionPowerShifts["noble_houses"] = 5;
    merchantScandal.factionPowerShifts["city_guard"] = 3;
    merchantScandal.relationShifts[{ "merchants_guild", "noble_houses" }] = -20;
    merchantScandal.relationShifts[{ "merchants_guild", "city_guard" }] = -15;
    merchantScandal.condition = [](const GameContext& context) {
        // This would check for specific game conditions
        // For demo purposes, we'll just return true
        return true;
    };

    FactionPoliticalShiftEvent magicalDiscovery("Major Magical Discovery",
        "The Mages Guild has announced a breakthrough in magical research, boosting their prestige and influence throughout the kingdom.");
    magicalDiscovery.factionPowerShifts["mages_guild"] = 20;
    magicalDiscovery.factionPowerShifts["noble_houses"] = 5;
    magicalDiscovery.relationShifts[{ "mages_guild", "noble_houses" }] = 15;
    magicalDiscovery.relationShifts[{ "mages_guild", "merchants_guild" }] = 10;
    magicalDiscovery.condition = [](const GameContext& context) {
        // This would check for specific game conditions
        return true;
    };

    FactionPoliticalShiftEvent forestTreaty("Forest Protection Treaty",
        "After years of disputes, the Forest Wardens and Noble Houses have reached an agreement on sustainable forest use and conservation areas.");
    forestTreaty.factionPowerShifts["forest_wardens"] = 15;
    forestTreaty.factionPowerShifts["noble_houses"] = 10;
    forestTreaty.relationShifts[{ "forest_wardens", "noble_houses" }] = 35;
    forestTreaty.relationShifts[{ "forest_wardens", "merchants_guild" }] = 20;
    forestTreaty.condition = [](const GameContext& context) {
        // Would check for specific quest completions or player actions
        return true;
    };

    FactionPoliticalShiftEvent thievesWarNobleTrade("Trade Route Ambush Campaign",
        "The Shadow Network has begun systematically targeting Noble Houses' trade caravans, pushing tensions to the breaking point.");
    thievesWarNobleTrade.factionPowerShifts["thieves_guild"] = 10;
    thievesWarNobleTrade.factionPowerShifts["noble_houses"] = -15;
    thievesWarNobleTrade.factionPowerShifts["merchants_guild"] = -5;
    thievesWarNobleTrade.relationShifts[{ "thieves_guild", "noble_houses" }] = -25;
    thievesWarNobleTrade.relationShifts[{ "noble_houses", "city_guard" }] = 15;
    thievesWarNobleTrade.condition = [](const GameContext& context) {
        // Check if thieves guild and noble houses are already hostile
        return true;
    };

    FactionPoliticalShiftEvent desertTradeAlliance("Desert-Merchant Trade Alliance",
        "The Desert Nomads and Merchants Guild have established a formal trade alliance, creating a new silk and spice route that promises great wealth for both sides.");
    desertTradeAlliance.factionPowerShifts["desert_nomads"] = 20;
    desertTradeAlliance.factionPowerShifts["merchants_guild"] = 15;
    desertTradeAlliance.relationShifts[{ "desert_nomads", "merchants_guild" }] = 30;
    desertTradeAlliance.condition = [](const GameContext& context) {
        // Check for player facilitation or time passage
        return true;
    };

    //----------------------------------------
    // FACTION QUEST EXAMPLES
    //----------------------------------------
    std::cout << "Creating faction quests..." << std::endl;

    // Merchants Guild - Trading competition quest
    FactionQuestNode* merchantQuest = dynamic_cast<FactionQuestNode*>(
        controller.createNode<FactionQuestNode>("MerchantTradingCompetition", "merchants_guild"));
    merchantQuest->questTitle = "The Grand Trading Competition";
    merchantQuest->questDescription = "The Merchants Guild is holding its annual trading competition. Prove your mercantile skills by turning a small investment into the largest profit possible.";
    merchantQuest->reputationReward = 15;
    merchantQuest->requirements.push_back({ "skill", "trading", 2 });
    merchantQuest->rewards.push_back({ "gold", 300, "" });
    merchantQuest->rewards.push_back({ "item", 1, "merchant_signet" });

    // Noble Houses - Court intrigue quest
    FactionQuestNode* nobleQuest = dynamic_cast<FactionQuestNode*>(
        controller.createNode<FactionQuestNode>("CourtIntrigue", "noble_houses"));
    nobleQuest->questTitle = "Whispers at Court";
    nobleQuest->questDescription = "Lord Bastian suspects a rival noble is plotting against him. Investigate the court intrigue and uncover the truth behind these rumors.";
    nobleQuest->reputationReward = 20;
    nobleQuest->requirements.push_back({ "skill", "persuasion", 3 });
    nobleQuest->requirements.push_back({ "faction", "noble_houses", 10 }); // Minimum reputation
    nobleQuest->rewards.push_back({ "gold", 200, "" });
    nobleQuest->rewards.push_back({ "faction", 10, "noble_houses" });
    nobleQuest->reputationEffectsOnOtherFactions["merchants_guild"] = 5; // Helping nobles improves merchant relations slightly
    nobleQuest->reputationEffectsOnOtherFactions["thieves_guild"] = -10; // But thieves don't like noble supporters

    // Thieves Guild - Heist quest
    FactionQuestNode* thievesQuest = dynamic_cast<FactionQuestNode*>(
        controller.createNode<FactionQuestNode>("TheGreatHeist", "thieves_guild"));
    thievesQuest->questTitle = "The Great Heist";
    thievesQuest->questDescription = "The Shadow Network is planning a major heist at a noble's estate. Your role would be to create a distraction while others enter the vault.";
    thievesQuest->reputationReward = 25;
    thievesQuest->rankAdvancement = true;
    thievesQuest->requirements.push_back({ "skill", "stealth", 4 });
    thievesQuest->requirements.push_back({ "faction", "thieves_guild", 20 }); // Need substantial reputation
    thievesQuest->rewards.push_back({ "gold", 500, "" });
    thievesQuest->rewards.push_back({ "item", 1, "shadow_cloak" });
    thievesQuest->reputationEffectsOnOtherFactions["noble_houses"] = -30; // Nobles will hate you
    thievesQuest->reputationEffectsOnOtherFactions["city_guard"] = -20; // Guards will be suspicious

    //----------------------------------------
    // FACTION LOCATIONS
    //----------------------------------------
    std::cout << "Creating faction locations..." << std::endl;

    // Merchants Guild Hall
    FactionLocationNode* merchantsGuildHall = dynamic_cast<FactionLocationNode*>(
        controller.createNode<FactionLocationNode>("MerchantsGuildHall", "Merchants Guild Hall", "merchants_guild"));
    merchantsGuildHall->description = "A grand building with ornate columns and gold-plated doors, serving as the headquarters of the Merchants Guild.";
    merchantsGuildHall->minReputationRequired = -10; // Anyone not hostile can enter the public areas
    merchantsGuildHall->minRankRequired = 0;
    merchantsGuildHall->restrictAccess = true;
    merchantsGuildHall->servicePriceModifiers["trading_license"] = 1.0f;
    merchantsGuildHall->servicePriceModifiers["market_stall_rental"] = 1.0f;

    // Noble Quarter
    FactionLocationNode* nobleQuarter = dynamic_cast<FactionLocationNode*>(
        controller.createNode<FactionLocationNode>("NobleQuarter", "Noble Quarter", "noble_houses"));
    nobleQuarter->description = "The wealthiest district in the city, featuring magnificent manors, private gardens, and guard patrols.";
    nobleQuarter->minReputationRequired = 0; // Neutral or better reputation required
    nobleQuarter->minRankRequired = 0;
    nobleQuarter->restrictAccess = true;
    nobleQuarter->servicePriceModifiers["luxury_goods"] = 1.2f; // Higher prices here

    // Mage Tower
    FactionLocationNode* mageTower = dynamic_cast<FactionLocationNode*>(
        controller.createNode<FactionLocationNode>("MageTower", "Mage Tower", "mages_guild"));
    mageTower->description = "A tall, mystical tower with glowing runes and floating platforms, home to the Mages Guild.";
    mageTower->minReputationRequired = 10; // Need some reputation to enter
    mageTower->minRankRequired = 1; // Must be at least an initiate
    mageTower->restrictAccess = true;
    mageTower->servicePriceModifiers["spell_scrolls"] = 0.9f; // Guild members get discounts
    mageTower->servicePriceModifiers["enchanting"] = 0.9f;

    // Thieves Den
    FactionLocationNode* thievesDen = dynamic_cast<FactionLocationNode*>(
        controller.createNode<FactionLocationNode>("ThievesDen", "The Rat's Nest Tavern", "thieves_guild"));
    thievesDen->description = "A seedy tavern with a secret basement where the Shadow Network conducts its business.";
    thievesDen->minReputationRequired = 20; // Need significant reputation
    thievesDen->minRankRequired = 2; // Must be at least an associate
    thievesDen->restrictAccess = true;
    thievesDen->servicePriceModifiers["fence_items"] = 0.8f; // Good rates for stolen goods
    thievesDen->servicePriceModifiers["lockpicks"] = 0.7f;

    // Register the faction system
    controller.setSystemRoot("FactionSystem", factionSystem);

    //----------------------------------------
    // DEMONSTRATION
    //----------------------------------------
    std::cout << "\n=== FACTION SYSTEM DEMONSTRATION ===\n"
              << std::endl;

    // Initialize the player with some faction reputation
    controller.gameContext.playerStats.factionReputation["merchants_guild"] = 15;
    controller.gameContext.playerStats.factionReputation["noble_houses"] = 5;
    controller.gameContext.playerStats.factionReputation["city_guard"] = 10;
    controller.gameContext.playerStats.factionReputation["mages_guild"] = 0;

    // Update faction system with these initial values
    factionSystem->factions["merchants_guild"].playerReputation = 15;
    factionSystem->factions["merchants_guild"].playerReputationState = "friendly";
    factionSystem->factions["noble_houses"].playerReputation = 5;
    factionSystem->factions["noble_houses"].playerReputationState = "neutral";
    factionSystem->factions["city_guard"].playerReputation = 10;
    factionSystem->factions["city_guard"].playerReputationState = "friendly";
    factionSystem->factions["mages_guild"].playerReputation = 0;
    factionSystem->factions["mages_guild"].playerReputationState = "neutral";

    // Join Merchants Guild
    std::cout << "You decide to join the Merchants Guild..." << std::endl;
    factionSystem->factions["merchants_guild"].playerRank = 1;
    factionSystem->factions["merchants_guild"].updatePlayerReputationState();
    std::cout << "You are now an " << factionSystem->factions["merchants_guild"].getCurrentRankTitle()
              << " of the " << factionSystem->factions["merchants_guild"].name << "." << std::endl;

    // Display faction system
    controller.processInput("FactionSystem", {});

    // View Merchants Guild details
    std::cout << "\nExamining the Merchants Guild in detail..." << std::endl;
    TAInput viewMerchantsInput = {
        "faction_action",
        { { "action", std::string("view") }, { "faction_id", std::string("merchants_guild") } }
    };
    controller.processInput("FactionSystem", viewMerchantsInput);

    // Complete a quest for the Merchants Guild and gain reputation
    std::cout << "\nCompleting a trading mission for the Merchants Guild..." << std::endl;
    factionSystem->changePlayerReputation("merchants_guild", 10, &controller.gameContext);
    factionSystem->factions["merchants_guild"].completedQuests.push_back("Minor Trade Route");

    // Check if can advance rank
    std::cout << "\nChecking for rank advancement possibilities..." << std::endl;
    if (factionSystem->factions["merchants_guild"].canAdvanceRank()) {
        TAInput advanceRankInput = {
            "faction_action",
            { { "action", std::string("advance_rank") }, { "faction_id", std::string("merchants_guild") } }
        };
        controller.processInput("FactionSystem", advanceRankInput);
    } else {
        std::cout << "You need more reputation or completed quests to advance in the Merchants Guild." << std::endl;
    }

    // Discover the Thieves Guild
    std::cout << "\nThrough your adventures, you discover the existence of the Shadow Network..." << std::endl;
    factionSystem->factions["thieves_guild"].playerKnown = true;
    factionSystem->factions["thieves_guild"].playerReputation = -10; // Initial suspicion
    factionSystem->factions["thieves_guild"].updatePlayerReputationState();

    // View faction relations
    std::cout << "\nExamining the political landscape of the kingdom..." << std::endl;
    TAInput viewRelationsInput = {
        "faction_action",
        { { "action", std::string("view_relations") } }
    };
    controller.processInput("FactionSystem", viewRelationsInput);

    // Simulate a political event
    std::cout << "\nTension rises between factions..." << std::endl;
    thievesWarNobleTrade.checkAndExecute(&controller.gameContext, factionSystem);

    // Help City Guard capture thieves - positive reputation with guard, negative with thieves
    std::cout << "\nYou help the City Guard apprehend several Shadow Network operatives..." << std::endl;
    factionSystem->changePlayerReputation("city_guard", 15, &controller.gameContext);
    factionSystem->changePlayerReputation("thieves_guild", -20, &controller.gameContext);

    // This should trigger ripple effects
    std::cout << "\nYour actions have consequences throughout the faction system..." << std::endl;

    // View updated faction relations
    std::cout << "\nThe political landscape has shifted due to recent events..." << std::endl;
    controller.processInput("FactionSystem", viewRelationsInput);

    // Another political event - Desert Trade Alliance
    std::cout << "\nMonths pass, and new alliances form..." << std::endl;
    desertTradeAlliance.checkAndExecute(&controller.gameContext, factionSystem);

    // Discover Desert Nomads faction through this event
    factionSystem->factions["desert_nomads"].playerKnown = true;
    factionSystem->factions["desert_nomads"].playerReputation = 5; // Slight positive due to merchant association
    factionSystem->factions["desert_nomads"].updatePlayerReputationState();

    // Final faction status check
    std::cout << "\nCurrent faction standings after all events:" << std::endl;
    controller.processInput("FactionSystem", {});

    // Save state demonstration
    std::cout << "\nSaving faction system state..." << std::endl;
    controller.saveState("faction_save.dat");
    std::cout << "Faction system state saved successfully." << std::endl;

    // Example of how certain actions could modify shop prices
    float merchantDiscount = factionSystem->factions["merchants_guild"].getTradeModifier();
    std::cout << "\nAs a member of the Merchants Guild with a reputation of "
              << factionSystem->factions["merchants_guild"].playerReputation
              << ", you receive a price modifier of " << merchantDiscount
              << " when trading with guild merchants." << std::endl;

    int itemBasePrice = 100;
    int actualPrice = static_cast<int>(itemBasePrice * merchantDiscount);
    std::cout << "An item normally costing " << itemBasePrice
              << " gold would cost you " << actualPrice << " gold." << std::endl;

    // Show how faction status affects quest availability
    std::cout << "\nAvailable faction quests based on your standing:" << std::endl;

    if (factionSystem->factions["merchants_guild"].playerReputation >= 10) {
        std::cout << "- Merchants Guild: The Grand Trading Competition" << std::endl;
    }

    if (factionSystem->factions["noble_houses"].playerReputation >= 10) {
        std::cout << "- Noble Houses: Whispers at Court" << std::endl;
    } else {
        std::cout << "- Noble Houses: No quests available (insufficient reputation)" << std::endl;
    }

    if (factionSystem->factions["thieves_guild"].playerReputation >= 20) {
        std::cout << "- Shadow Network: The Great Heist" << std::endl;
    } else {
        std::cout << "- Shadow Network: No quests available (insufficient reputation)" << std::endl;
    }

    // Add this pause before exiting
    std::cout << "\nPress Enter to exit...";
    std::cin.get();

    return 0;
}