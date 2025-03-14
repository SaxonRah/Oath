// System_FactionReputation.cpp
#include <algorithm>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <queue>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

// Using nlohmann json for modern C++ JSON handling
using json = nlohmann::json;

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
    std::map<std::string, int> relationThresholds; // Thresholds for different relation states

    FactionRelationship(const std::string& a, const std::string& b, int value = 0)
        : factionA(a)
        , factionB(b)
        , relationValue(value)
    {
        // Default thresholds if not loaded from JSON
        relationThresholds = {
            { "war", -75 },
            { "hostile", -50 },
            { "unfriendly", -20 },
            { "neutral", 20 },
            { "friendly", 50 },
            { "cordial", 75 },
            { "allied", 100 }
        };
        updateState();
    }

    void loadThresholds(const json& thresholds)
    {
        if (thresholds.is_object()) {
            for (auto& [state, value] : thresholds.items()) {
                relationThresholds[state] = value;
            }
        }
        // After loading, update the state based on new thresholds
        updateState();
    }

    void updateState()
    {
        // Sort thresholds by value
        std::vector<std::pair<std::string, int>> sortedThresholds;
        for (const auto& [state, value] : relationThresholds) {
            sortedThresholds.push_back({ state, value });
        }

        std::sort(sortedThresholds.begin(), sortedThresholds.end(),
            [](const auto& a, const auto& b) { return a.second < b.second; });

        // Find appropriate state based on current relation value
        for (auto it = sortedThresholds.rbegin(); it != sortedThresholds.rend(); ++it) {
            if (relationValue >= it->second) {
                relationState = it->first;
                return;
            }
        }

        // Default to the lowest state if no match found
        if (!sortedThresholds.empty()) {
            relationState = sortedThresholds[0].first;
        } else {
            relationState = "neutral"; // Fallback
        }
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

    // Reputation state thresholds
    std::map<std::string, int> reputationThresholds;

    // Rank requirements
    std::map<int, std::pair<int, int>> rankRequirements; // rank -> {min_reputation, min_quests}

    // Rank privilege unlocks
    std::map<int, std::string> rankPrivileges;

    Faction()
        : currentState("stable")
        , economicPower(50)
        , militaryPower(50)
        , politicalInfluence(50)
        , playerRank(0)
        , playerReputation(0)
        , playerReputationState("neutral")
        , playerKnown(false)
    {
        // Initialize default reputation thresholds if not loaded from JSON
        reputationThresholds = {
            { "hated", -100 },
            { "despised", -70 },
            { "hostile", -30 },
            { "unfriendly", -10 },
            { "neutral", 10 },
            { "friendly", 30 },
            { "honored", 70 },
            { "revered", 90 },
            { "exalted", 100 }
        };

        // Default rank titles if not loaded from JSON
        rankTitles = {
            { 0, "Outsider" },
            { 1, "Initiate" },
            { 2, "Associate" },
            { 3, "Member" },
            { 4, "Trusted" },
            { 5, "Guardian" },
            { 6, "Defender" },
            { 7, "Protector" },
            { 8, "Elder" },
            { 9, "Master" },
            { 10, "Grandmaster" }
        };

        // Default rank requirements if not loaded from JSON
        rankRequirements = {
            { 0, { 10, 0 } }, // From Outsider to Initiate
            { 1, { 20, 2 } }, // From Initiate to Associate
            { 2, { 30, 5 } }, // From Associate to Member
            { 3, { 40, 10 } }, // From Member to Trusted
            { 4, { 50, 15 } }, // From Trusted to Guardian
            { 5, { 60, 20 } }, // From Guardian to Defender
            { 6, { 70, 25 } }, // From Defender to Protector
            { 7, { 80, 30 } }, // From Protector to Elder
            { 8, { 90, 40 } }, // From Elder to Master
            { 9, { 100, 50 } } // From Master to Grandmaster
        };

        // Default rank privileges if not loaded from JSON
        rankPrivileges = {
            { 3, "faction_equipment_access" },
            { 5, "faction_housing_access" },
            { 7, "faction_special_quests" },
            { 10, "faction_leadership_council" }
        };
    }

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
        // Same default initializations as the default constructor
        reputationThresholds = {
            { "hated", -100 },
            { "despised", -70 },
            { "hostile", -30 },
            { "unfriendly", -10 },
            { "neutral", 10 },
            { "friendly", 30 },
            { "honored", 70 },
            { "revered", 90 },
            { "exalted", 100 }
        };

        rankTitles = {
            { 0, "Outsider" },
            { 1, "Initiate" },
            { 2, "Associate" },
            { 3, "Member" },
            { 4, "Trusted" },
            { 5, "Guardian" },
            { 6, "Defender" },
            { 7, "Protector" },
            { 8, "Elder" },
            { 9, "Master" },
            { 10, "Grandmaster" }
        };

        rankRequirements = {
            { 0, { 10, 0 } }, // From Outsider to Initiate
            { 1, { 20, 2 } }, // From Initiate to Associate
            { 2, { 30, 5 } }, // From Associate to Member
            { 3, { 40, 10 } }, // From Member to Trusted
            { 4, { 50, 15 } }, // From Trusted to Guardian
            { 5, { 60, 20 } }, // From Guardian to Defender
            { 6, { 70, 25 } }, // From Defender to Protector
            { 7, { 80, 30 } }, // From Protector to Elder
            { 8, { 90, 40 } }, // From Elder to Master
            { 9, { 100, 50 } } // From Master to Grandmaster
        };

        rankPrivileges = {
            { 3, "faction_equipment_access" },
            { 5, "faction_housing_access" },
            { 7, "faction_special_quests" },
            { 10, "faction_leadership_council" }
        };
    }

    // Load faction from JSON
    static Faction fromJson(const json& j)
    {
        Faction faction;

        // Required fields
        faction.id = j.value("id", "unknown");
        faction.name = j.value("name", "Unknown Faction");

        // Optional fields with defaults
        faction.description = j.value("description", "");
        faction.currentState = j.value("currentState", "stable");
        faction.economicPower = j.value("economicPower", 50);
        faction.militaryPower = j.value("militaryPower", 50);
        faction.politicalInfluence = j.value("politicalInfluence", 50);
        faction.primaryCulture = j.value("primaryCulture", "");
        faction.politicalAlignment = j.value("politicalAlignment", "");
        faction.dominantReligion = j.value("dominantReligion", "");
        faction.playerKnown = j.value("playerKnown", false);

        // Array fields
        if (j.contains("territories") && j["territories"].is_array()) {
            for (const auto& territory : j["territories"]) {
                faction.territories.push_back(territory);
            }
        }

        if (j.contains("leaders") && j["leaders"].is_array()) {
            for (const auto& leader : j["leaders"]) {
                faction.leaders.push_back(leader);
            }
        }

        if (j.contains("specialLocations") && j["specialLocations"].is_array()) {
            for (const auto& location : j["specialLocations"]) {
                faction.specialLocations.push_back(location);
            }
        }

        // Object fields
        if (j.contains("skillBonuses") && j["skillBonuses"].is_object()) {
            for (auto& [skill, bonus] : j["skillBonuses"].items()) {
                faction.skillBonuses[skill] = bonus;
            }
        }

        return faction;
    }

    // Convert faction to JSON for saving
    json toJson() const
    {
        json j;

        // Basic properties
        j["id"] = id;
        j["name"] = name;
        j["description"] = description;
        j["currentState"] = currentState;
        j["economicPower"] = economicPower;
        j["militaryPower"] = militaryPower;
        j["politicalInfluence"] = politicalInfluence;
        j["primaryCulture"] = primaryCulture;
        j["politicalAlignment"] = politicalAlignment;
        j["dominantReligion"] = dominantReligion;
        j["playerKnown"] = playerKnown;
        j["playerRank"] = playerRank;
        j["playerReputation"] = playerReputation;
        j["playerReputationState"] = playerReputationState;

        // Arrays
        j["territories"] = territories;
        j["leaders"] = leaders;
        j["specialLocations"] = specialLocations;
        j["completedQuests"] = completedQuests;

        // Convert members set to array
        j["members"] = json::array();
        for (const auto& member : members) {
            j["members"].push_back(member);
        }

        // Convert special privileges set to array
        j["specialPrivileges"] = json::array();
        for (const auto& privilege : specialPrivileges) {
            j["specialPrivileges"].push_back(privilege);
        }

        // Objects
        j["skillBonuses"] = skillBonuses;
        j["resourceInfluence"] = resourceInfluence;

        // Convert maps with integer keys to string-keyed objects
        j["rankTitles"] = json::object();
        for (const auto& [rank, title] : rankTitles) {
            j["rankTitles"][std::to_string(rank)] = title;
        }

        return j;
    }

    void updatePlayerReputationState()
    {
        // Sort thresholds by value
        std::vector<std::pair<std::string, int>> sortedThresholds;
        for (const auto& [state, value] : reputationThresholds) {
            sortedThresholds.push_back({ state, value });
        }

        std::sort(sortedThresholds.begin(), sortedThresholds.end(),
            [](const auto& a, const auto& b) { return a.second < b.second; });

        // Find appropriate state based on current reputation value
        for (auto it = sortedThresholds.rbegin(); it != sortedThresholds.rend(); ++it) {
            if (playerReputation >= it->second) {
                playerReputationState = it->first;
                return;
            }
        }

        // Default to the lowest state if no match found
        if (!sortedThresholds.empty()) {
            playerReputationState = sortedThresholds[0].first;
        } else {
            playerReputationState = "neutral"; // Fallback
        }
    }

    void changeReputation(int amount)
    {
        playerReputation = std::max(-100, std::min(100, playerReputation + amount));
        updatePlayerReputationState();
    }

    bool canAdvanceRank() const
    {
        if (playerRank >= 10) {
            return false; // Already at max rank
        }

        auto it = rankRequirements.find(playerRank);
        if (it == rankRequirements.end()) {
            return false; // Requirements not found
        }

        const auto& [requiredRep, requiredQuests] = it->second;
        return playerReputation >= requiredRep && completedQuests.size() >= requiredQuests;
    }

    bool tryAdvanceRank()
    {
        if (canAdvanceRank() && playerRank < 10) {
            playerRank++;

            // Check for new privileges based on rank
            auto it = rankPrivileges.find(playerRank);
            if (it != rankPrivileges.end()) {
                specialPrivileges.insert(it->second);
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
    std::string jsonFilePath;

    FactionSystemNode(const std::string& name, const std::string& configFile = "FactionReputation.json")
        : TANode(name)
        , jsonFilePath(configFile)
    {
        loadFromJson();
    }

    // Load faction system data from JSON file
    void loadFromJson()
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
            std::cerr << "Error loading faction data: " << e.what() << std::endl;
        }
    }

    // Save faction system data to JSON file
    bool saveToJson(const std::string& outputPath = "")
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

    void addFaction(const Faction& faction)
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

    bool adjustFactionRelation(const std::string& factionA, const std::string& factionB, int amount)
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
            } else if (action == "save") {
                saveToJson();
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
    std::vector<std::tuple<std::string, std::string, int>> relationShifts; // Changes to relations (factionA, factionB, amount)
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

    // Create from JSON
    static FactionPoliticalShiftEvent fromJson(const json& j)
    {
        FactionPoliticalShiftEvent event(
            j.value("name", "Unnamed Event"),
            j.value("description", "No description"));

        // Load faction power shifts
        if (j.contains("factionPowerShifts") && j["factionPowerShifts"].is_object()) {
            for (auto& [factionId, shift] : j["factionPowerShifts"].items()) {
                event.factionPowerShifts[factionId] = shift;
            }
        }

        // Load relation shifts
        if (j.contains("relationShifts") && j["relationShifts"].is_array()) {
            for (const auto& shift : j["relationShifts"]) {
                if (shift.contains("factionA") && shift.contains("factionB") && shift.contains("amount")) {
                    std::string factionA = shift["factionA"];
                    std::string factionB = shift["factionB"];
                    int amount = shift["amount"];
                    event.relationShifts.push_back(std::make_tuple(factionA, factionB, amount));
                }
            }
        }

        return event;
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
            for (const auto& [factionA, factionB, relationChange] : relationShifts) {
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

            // Save the updated faction system
            factionSystem->saveToJson();

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

    // Create Faction System Node with path to config file
    FactionSystemNode* factionSystem = dynamic_cast<FactionSystemNode*>(
        controller.createNode<FactionSystemNode>("FactionSystem", "FactionReputation.json"));

    // Load political events from JSON
    std::vector<FactionPoliticalShiftEvent> politicalEvents;
    try {
        std::ifstream file("FactionReputation.json");
        if (file.is_open()) {
            json j;
            file >> j;
            file.close();

            if (j.contains("politicalEvents") && j["politicalEvents"].is_array()) {
                for (const auto& eventJson : j["politicalEvents"]) {
                    politicalEvents.push_back(FactionPoliticalShiftEvent::fromJson(eventJson));
                }
                std::cout << "Loaded " << politicalEvents.size() << " political events from JSON" << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error loading political events: " << e.what() << std::endl;
    }

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
    auto it_merchants = factionSystem->factions.find("merchants_guild");
    if (it_merchants != factionSystem->factions.end()) {
        it_merchants->second.playerReputation = 15;
        it_merchants->second.updatePlayerReputationState();
    }

    auto it_nobles = factionSystem->factions.find("noble_houses");
    if (it_nobles != factionSystem->factions.end()) {
        it_nobles->second.playerReputation = 5;
        it_nobles->second.updatePlayerReputationState();
    }

    auto it_guard = factionSystem->factions.find("city_guard");
    if (it_guard != factionSystem->factions.end()) {
        it_guard->second.playerReputation = 10;
        it_guard->second.updatePlayerReputationState();
    }

    auto it_mages = factionSystem->factions.find("mages_guild");
    if (it_mages != factionSystem->factions.end()) {
        it_mages->second.playerReputation = 0;
        it_mages->second.updatePlayerReputationState();
    }

    // Join Merchants Guild
    std::cout << "You decide to join the Merchants Guild..." << std::endl;
    if (it_merchants != factionSystem->factions.end()) {
        it_merchants->second.playerRank = 1;
        it_merchants->second.updatePlayerReputationState();
        std::cout << "You are now an " << it_merchants->second.getCurrentRankTitle()
                  << " of the " << it_merchants->second.name << "." << std::endl;
    }

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
    if (it_merchants != factionSystem->factions.end()) {
        it_merchants->second.completedQuests.push_back("Minor Trade Route");
    }

    // Check if can advance rank
    std::cout << "\nChecking for rank advancement possibilities..." << std::endl;
    if (it_merchants != factionSystem->factions.end() && it_merchants->second.canAdvanceRank()) {
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
    auto it_thieves = factionSystem->factions.find("thieves_guild");
    if (it_thieves != factionSystem->factions.end()) {
        it_thieves->second.playerKnown = true;
        it_thieves->second.playerReputation = -10; // Initial suspicion
        it_thieves->second.updatePlayerReputationState();
    }

    // View faction relations
    std::cout << "\nExamining the political landscape of the kingdom..." << std::endl;
    TAInput viewRelationsInput = {
        "faction_action",
        { { "action", std::string("view_relations") } }
    };
    controller.processInput("FactionSystem", viewRelationsInput);

    // Simulate a political event
    std::cout << "\nTension rises between factions..." << std::endl;
    if (!politicalEvents.empty()) {
        // Find the "Trade Route Ambush Campaign" event or use the first one
        auto eventIt = std::find_if(politicalEvents.begin(), politicalEvents.end(),
            [](const FactionPoliticalShiftEvent& event) {
                return event.name == "Trade Route Ambush Campaign";
            });

        if (eventIt != politicalEvents.end()) {
            eventIt->checkAndExecute(&controller.gameContext, factionSystem);
        } else {
            politicalEvents[0].checkAndExecute(&controller.gameContext, factionSystem);
        }
    }

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
    auto eventIt = std::find_if(politicalEvents.begin(), politicalEvents.end(),
        [](const FactionPoliticalShiftEvent& event) {
            return event.name == "Desert-Merchant Trade Alliance";
        });

    if (eventIt != politicalEvents.end()) {
        eventIt->checkAndExecute(&controller.gameContext, factionSystem);
    }

    // Discover Desert Nomads faction through this event
    auto it_nomads = factionSystem->factions.find("desert_nomads");
    if (it_nomads != factionSystem->factions.end()) {
        it_nomads->second.playerKnown = true;
        it_nomads->second.playerReputation = 5; // Slight positive due to merchant association
        it_nomads->second.updatePlayerReputationState();
    }

    // Final faction status check
    std::cout << "\nCurrent faction standings after all events:" << std::endl;
    controller.processInput("FactionSystem", {});

    // Save state demonstration
    std::cout << "\nSaving faction system state..." << std::endl;
    factionSystem->saveToJson("faction_save.json");
    std::cout << "Faction system state saved successfully." << std::endl;

    // Example of how certain actions could modify shop prices
    if (it_merchants != factionSystem->factions.end()) {
        float merchantDiscount = it_merchants->second.getTradeModifier();
        std::cout << "\nAs a member of the Merchants Guild with a reputation of "
                  << it_merchants->second.playerReputation
                  << ", you receive a price modifier of " << merchantDiscount
                  << " when trading with guild merchants." << std::endl;

        int itemBasePrice = 100;
        int actualPrice = static_cast<int>(itemBasePrice * merchantDiscount);
        std::cout << "An item normally costing " << itemBasePrice
                  << " gold would cost you " << actualPrice << " gold." << std::endl;
    }

    // Show how faction status affects quest availability
    std::cout << "\nAvailable faction quests based on your standing:" << std::endl;

    if (it_merchants != factionSystem->factions.end() && it_merchants->second.playerReputation >= 10) {
        std::cout << "- Merchants Guild: The Grand Trading Competition" << std::endl;
    }

    if (it_nobles != factionSystem->factions.end() && it_nobles->second.playerReputation >= 10) {
        std::cout << "- Noble Houses: Whispers at Court" << std::endl;
    } else if (it_nobles != factionSystem->factions.end()) {
        std::cout << "- Noble Houses: No quests available (insufficient reputation)" << std::endl;
    }

    if (it_thieves != factionSystem->factions.end() && it_thieves->second.playerReputation >= 20) {
        std::cout << "- Shadow Network: The Great Heist" << std::endl;
    } else if (it_thieves != factionSystem->factions.end()) {
        std::cout << "- Shadow Network: No quests available (insufficient reputation)" << std::endl;
    }

    // Add this pause before exiting
    std::cout << "\nPress Enter to exit...";
    std::cin.get();

    return 0;
}