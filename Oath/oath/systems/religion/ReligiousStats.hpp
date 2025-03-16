#pragma once

#include <map>
#include <set>
#include <string>
#include <vector>

class ReligiousStats {
public:
    std::map<std::string, int> deityFavor; // Favor level with each deity
    std::map<std::string, int> deityDevotion; // Devotion points spent
    std::map<std::string, bool> deityAlignment; // If true, player is aligned with deity
    std::string primaryDeity; // Current primary deity
    std::set<std::string> completedRituals; // Completed ritual IDs
    std::set<std::string> activeBlessing; // Active blessing effects
    std::map<std::string, int> blessingDuration; // Remaining time on blessings

    ReligiousStats();
    void changeFavor(const std::string& deity, int amount);
    void addDevotion(const std::string& deity, int points);
    bool hasMinimumFavor(const std::string& deity, int minimumFavor) const;
    bool hasBlessingActive(const std::string& blessing) const;
    void addBlessing(const std::string& blessing, int duration);
    void removeBlessing(const std::string& blessing);
    void updateBlessings();
    void setPrimaryDeity(const std::string& deity);
    bool hasCompletedRitual(const std::string& ritualId) const;
    void markRitualCompleted(const std::string& ritualId);
    void initializeDeities(const std::vector<std::string>& deityIds);
};