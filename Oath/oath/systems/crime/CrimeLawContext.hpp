// CrimeLawContext.hpp
#pragma once

#include "../../data/Inventory.hpp"
#include "CrimeLawConfig.hpp"
#include "CriminalRecord.hpp"
#include <map>
#include <string>

// Extend GameContext to include criminal record
struct CrimeLawContext {
    CriminalRecord criminalRecord;

    // Existing guard status
    std::map<std::string, bool> guardAlerted;
    std::map<std::string, int> guardSuspicion; // 0-100
    std::map<std::string, int> jailSentencesByRegion; // Days of jail time

    // Jail properties
    int currentJailDays = 0;
    std::string currentJailRegion = "";
    bool inJail = false;

    // Items confiscated during arrest
    Inventory confiscatedItems;

    CrimeLawContext();

    // Calculate jail sentence based on crimes
    int calculateJailSentence(const std::string& region);
};