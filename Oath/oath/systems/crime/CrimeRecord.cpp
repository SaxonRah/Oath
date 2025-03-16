// CrimeRecord.cpp
#include "CrimeRecord.hpp"
#include "CrimeLawConfig.hpp"
#include "CrimeType.hpp"
#include <cctype>

CrimeRecord::CrimeRecord(const std::string& crimeType, const std::string& crimeRegion, const std::string& crimeLocation,
    bool wasWitnessed, int crimeSeverity)
    : type(crimeType)
    , region(crimeRegion)
    , location(crimeLocation)
    , witnessed(wasWitnessed)
    , severity(crimeSeverity)
    , bounty(calculateBounty())
    , timestamp(std::time(nullptr))
    , paid(false)
{
}

int CrimeRecord::calculateBounty() const
{
    // Base bounty by crime type
    int baseBounty = crimeLawConfig["baseBounties"].contains(type) ? crimeLawConfig["baseBounties"][type].get<int>() : 0;

    // Adjust by severity
    int adjustedBounty = baseBounty * severity / 5;

    // Adjust if witnessed
    if (witnessed)
        adjustedBounty *= crimeLawConfig["crimeConfig"]["bountyWitnessMultiplier"].get<int>();

    return adjustedBounty;
}

std::string CrimeRecord::getDescription() const
{
    std::string desc = type;
    // Capitalize first letter
    if (!desc.empty()) {
        desc[0] = std::toupper(desc[0]);
    }

    desc += " in " + location + ", " + region;
    if (witnessed) {
        desc += " (Witnessed)";
    } else {
        desc += " (Unwitnessed)";
    }

    desc += " - Bounty: " + std::to_string(bounty) + " gold";
    if (paid) {
        desc += " (Paid)";
    }

    return desc;
}