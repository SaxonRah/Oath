// CriminalRecord.hpp
#pragma once

#include "CrimeRecord.hpp"
#include <map>
#include <string>
#include <vector>

// Forward declarations
struct GameContext;

// Tracks the player's criminal status across different regions
class CriminalRecord {
public:
    std::vector<CrimeRecord> crimes;
    std::map<std::string, int> totalBountyByRegion;
    std::map<std::string, int> reputationByRegion; // Criminal reputation (-100 to 100, lower is worse)
    std::map<std::string, bool> wantedStatus; // Whether player is actively wanted by guards

    // Add a new crime to the record
    void addCrime(const CrimeRecord& crime);

    // Pay bounty for a specific region
    bool payBounty(const std::string& region, int goldAmount, GameContext* context);

    // Check if player is wanted in the given region
    bool isWanted(const std::string& region) const;

    // Get total bounty for a region
    int getBounty(const std::string& region) const;

    // Get criminal reputation for a region
    int getReputation(const std::string& region) const;

    // Get a list of unpaid crimes in a region
    std::vector<CrimeRecord> getUnpaidCrimes(const std::string& region) const;

    // Serves jail time and clears some crimes based on time served
    void serveJailSentence(const std::string& region, int days);
};