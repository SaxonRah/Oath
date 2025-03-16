// systems/faction/Faction.cpp
#include "Faction.hpp"
#include <algorithm>

namespace oath {

Faction::Faction()
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

Faction::Faction(const std::string& factionId, const std::string& factionName)
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

Faction Faction::fromJson(const json& j)
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

json Faction::toJson() const
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

void Faction::updatePlayerReputationState()
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

void Faction::changeReputation(int amount)
{
    playerReputation = std::max(-100, std::min(100, playerReputation + amount));
    updatePlayerReputationState();
}

bool Faction::canAdvanceRank() const
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

bool Faction::tryAdvanceRank()
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

float Faction::getTradeModifier() const
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

std::string Faction::getCurrentRankTitle() const
{
    auto it = rankTitles.find(playerRank);
    if (it != rankTitles.end()) {
        return it->second;
    }
    return "Unknown";
}

std::string Faction::getNextRankTitle() const
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

std::string Faction::getReputationDescription() const
{
    return playerReputationState;
}

} // namespace oath