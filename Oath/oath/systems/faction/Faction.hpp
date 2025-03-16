// systems/faction/Faction.hpp
#pragma once

#include <map>
#include <nlohmann/json.hpp>
#include <set>
#include <string>
#include <vector>

using json = nlohmann::json;

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

    Faction();
    Faction(const std::string& factionId, const std::string& factionName);

    // Load faction from JSON
    static Faction fromJson(const json& j);

    // Convert faction to JSON for saving
    json toJson() const;

    void updatePlayerReputationState();
    void changeReputation(int amount);
    bool canAdvanceRank() const;
    bool tryAdvanceRank();
    float getTradeModifier() const;
    std::string getCurrentRankTitle() const;
    std::string getNextRankTitle() const;
    std::string getReputationDescription() const;
};
