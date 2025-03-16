// CrimeLawContext.cpp
#include "CrimeLawContext.hpp"

#include "CrimeType.hpp"
#include <algorithm>

CrimeLawContext::CrimeLawContext()
{
    // Initialize with default values from config
    for (const auto& region : crimeLawConfig["regions"]) {
        guardAlerted[region] = false;
        guardSuspicion[region] = 0;
        jailSentencesByRegion[region] = 0;
    }
}

int CrimeLawContext::calculateJailSentence(const std::string& region)
{
    int sentence = 0;
    auto crimes = criminalRecord.getUnpaidCrimes(region);

    for (const auto& crime : crimes) {
        if (crime.type == CrimeType::MURDER()) {
            sentence += crimeLawConfig["jailConfig"]["murderDaysPerPoint"].get<int>() * crime.severity;
        } else if (crime.type == CrimeType::ASSAULT()) {
            sentence += crimeLawConfig["jailConfig"]["assaultDaysPerPoint"].get<int>() * crime.severity;
        } else if (crime.type == CrimeType::THEFT() || crime.type == CrimeType::PICKPOCKETING()) {
            sentence += crimeLawConfig["jailConfig"]["theftDaysPerPoint"].get<int>() * crime.severity;
        } else {
            sentence += crimeLawConfig["jailConfig"]["minorCrimeDaysPerPoint"].get<int>() * crime.severity;
        }
    }

    // Cap at reasonable values
    return std::min(sentence, crimeLawConfig["jailConfig"]["maxJailSentence"].get<int>());
}