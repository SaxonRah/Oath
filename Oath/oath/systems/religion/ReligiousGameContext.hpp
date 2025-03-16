#pragma once

#include "data/GameContext.hpp"
#include "systems/religion/ReligiousStats.hpp"
#include <map>
#include <string>

// Forward declaration
namespace nlohmann {
class json;
}

class ReligiousGameContext : public GameContext {
public:
    ReligiousStats religiousStats;
    std::map<std::string, std::string> templeJournal; // Records temple visits and rituals
    std::map<int, std::string> holyDayCalendar; // Days of the year for holy celebrations
    std::map<int, std::string> holyDayDeities; // Map days to deity IDs

    ReligiousGameContext();
    void loadHolyDays(const nlohmann::json& holyDayData);
    int getCurrentDayOfYear() const;
    bool isHolyDay() const;
    std::string getCurrentHolyDay() const;
    std::string getDeityOfCurrentHolyDay() const;
};