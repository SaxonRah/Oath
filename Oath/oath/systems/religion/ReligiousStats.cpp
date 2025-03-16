#include "systems/religion/ReligiousStats.hpp"
#include <algorithm>

ReligiousStats::ReligiousStats()
{
    primaryDeity = ""; // No primary deity initially
}

void ReligiousStats::changeFavor(const std::string& deity, int amount)
{
    if (deityFavor.find(deity) != deityFavor.end()) {
        deityFavor[deity] += amount;

        // Favor is capped between -100 and 100
        deityFavor[deity] = std::max(-100, std::min(100, deityFavor[deity]));
    }
}

void ReligiousStats::addDevotion(const std::string& deity, int points)
{
    if (deityDevotion.find(deity) != deityDevotion.end()) {
        deityDevotion[deity] += points;
        // Every 10 devotion points adds 1 favor
        changeFavor(deity, points / 10);
    }
}

bool ReligiousStats::hasMinimumFavor(const std::string& deity, int minimumFavor) const
{
    auto it = deityFavor.find(deity);
    return it != deityFavor.end() && it->second >= minimumFavor;
}

bool ReligiousStats::hasBlessingActive(const std::string& blessing) const
{
    return activeBlessing.find(blessing) != activeBlessing.end();
}

void ReligiousStats::addBlessing(const std::string& blessing, int duration)
{
    activeBlessing.insert(blessing);
    blessingDuration[blessing] = duration;
}

void ReligiousStats::removeBlessing(const std::string& blessing)
{
    activeBlessing.erase(blessing);
    blessingDuration.erase(blessing);
}

void ReligiousStats::updateBlessings()
{
    std::set<std::string> expiredBlessings;

    // Decrement duration and identify expired blessings
    for (auto& [blessing, duration] : blessingDuration) {
        duration--;
        if (duration <= 0) {
            expiredBlessings.insert(blessing);
        }
    }

    // Remove expired blessings
    for (const auto& blessing : expiredBlessings) {
        removeBlessing(blessing);
    }
}

void ReligiousStats::setPrimaryDeity(const std::string& deity)
{
    if (deityFavor.find(deity) != deityFavor.end()) {
        // Remove alignment from current primary deity
        if (!primaryDeity.empty()) {
            deityAlignment[primaryDeity] = false;
        }

        primaryDeity = deity;
        deityAlignment[deity] = true;

        // Give an initial favor boost when choosing a primary deity
        changeFavor(deity, 10);
    }
}

bool ReligiousStats::hasCompletedRitual(const std::string& ritualId) const
{
    return completedRituals.find(ritualId) != completedRituals.end();
}

void ReligiousStats::markRitualCompleted(const std::string& ritualId)
{
    completedRituals.insert(ritualId);
}

void ReligiousStats::initializeDeities(const std::vector<std::string>& deityIds)
{
    // Initialize with neutral favor for all deities
    for (const auto& id : deityIds) {
        deityFavor[id] = 0;
        deityDevotion[id] = 0;
        deityAlignment[id] = false;
    }
}