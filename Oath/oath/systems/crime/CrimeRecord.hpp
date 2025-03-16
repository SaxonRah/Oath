// CrimeRecord.hpp
#pragma once

#include <ctime>
#include <string>

// Record of a committed crime
struct CrimeRecord {

    std::string type; // Type of crime (from CrimeType)
    std::string region; // Region where crime was committed
    std::string location; // Specific location
    bool witnessed; // Whether the crime was witnessed
    int severity; // 1-10 scale of severity
    int bounty; // Gold bounty assigned
    time_t timestamp; // When the crime was committed
    bool paid; // Whether the bounty has been paid

    CrimeRecord(const std::string& crimeType, const std::string& crimeRegion, const std::string& crimeLocation,
        bool wasWitnessed, int crimeSeverity);

    int calculateBounty() const;
    std::string getDescription() const;
};