// CriminalRecord.cpp
#include "CriminalRecord.hpp"

#include "CrimeLawConfig.hpp"
#include <algorithm>

void CriminalRecord::addCrime(const CrimeRecord& crime)
{
    crimes.push_back(crime);

    // Update bounty for the region
    if (!crime.paid) {
        totalBountyByRegion[crime.region] += crime.bounty;
    }

    // Update criminal reputation
    int repLoss = crime.severity * crimeLawConfig["crimeConfig"]["repLossBaseFactor"].get<int>();
    if (crime.witnessed)
        repLoss *= crimeLawConfig["crimeConfig"]["witnessRepMultiplier"].get<int>();

    if (reputationByRegion.find(crime.region) == reputationByRegion.end()) {
        reputationByRegion[crime.region] = 0; // Start at neutral
    }

    reputationByRegion[crime.region] -= repLoss;
    if (reputationByRegion[crime.region] < -100) {
        reputationByRegion[crime.region] = -100; // Cap at -100
    }

    // Update wanted status based on the crime
    if (crime.witnessed && crime.severity > crimeLawConfig["crimeConfig"]["wantedThreshold"].get<int>()) {
        wantedStatus[crime.region] = true;
    }
}

bool CriminalRecord::payBounty(const std::string& region, int goldAmount, GameContext* context)
{
    if (goldAmount < totalBountyByRegion[region]) {
        return false; // Not enough gold
    }

    // Mark all crimes as paid in this region
    for (auto& crime : crimes) {
        if (crime.region == region && !crime.paid) {
            crime.paid = true;
        }
    }

    // Reset bounty for the region
    totalBountyByRegion[region] = 0;

    // Reset wanted status
    wantedStatus[region] = false;

    // Improve reputation slightly
    if (reputationByRegion.find(region) != reputationByRegion.end()) {
        reputationByRegion[region] += 10;
        if (reputationByRegion[region] > 100) {
            reputationByRegion[region] = 100;
        }
    }

    return true;
}

bool CriminalRecord::isWanted(const std::string& region) const
{
    auto it = wantedStatus.find(region);
    return it != wantedStatus.end() && it->second;
}

int CriminalRecord::getBounty(const std::string& region) const
{
    auto it = totalBountyByRegion.find(region);
    return (it != totalBountyByRegion.end()) ? it->second : 0;
}

int CriminalRecord::getReputation(const std::string& region) const
{
    auto it = reputationByRegion.find(region);
    return (it != reputationByRegion.end()) ? it->second : 0;
}

std::vector<CrimeRecord> CriminalRecord::getUnpaidCrimes(const std::string& region) const
{
    std::vector<CrimeRecord> unpaid;
    for (const auto& crime : crimes) {
        if (crime.region == region && !crime.paid) {
            unpaid.push_back(crime);
        }
    }
    return unpaid;
}

void CriminalRecord::serveJailSentence(const std::string& region, int days)
{
    // Mark all crimes as paid in this region
    for (auto& crime : crimes) {
        if (crime.region == region && !crime.paid) {
            crime.paid = true;
        }
    }

    // Reset bounty for the region
    totalBountyByRegion[region] = 0;

    // Reset wanted status
    wantedStatus[region] = false;

    // Improve reputation based on days served
    if (reputationByRegion.find(region) != reputationByRegion.end()) {
        reputationByRegion[region] += days * crimeLawConfig["jailConfig"]["repGainPerDay"].get<int>();
        if (reputationByRegion[region] > 100) {
            reputationByRegion[region] = 100;
        }
    }
}