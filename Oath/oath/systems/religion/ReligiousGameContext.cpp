#include "systems/religion/ReligiousGameContext.hpp"
#include <nlohmann/json.hpp>
#include <string>

ReligiousGameContext::ReligiousGameContext()
    : GameContext()
{
}

void ReligiousGameContext::loadHolyDays(const nlohmann::json& holyDayData)
{
    for (auto& [dayStr, holyDay] : holyDayData.items()) {
        int day = std::stoi(dayStr);
        holyDayCalendar[day] = holyDay["name"];
        holyDayDeities[day] = holyDay["deityId"];
    }
}

int ReligiousGameContext::getCurrentDayOfYear() const
{
    return (worldState.daysPassed % 365) + 1; // 1-365
}

bool ReligiousGameContext::isHolyDay() const
{
    return holyDayCalendar.find(getCurrentDayOfYear()) != holyDayCalendar.end();
}

std::string ReligiousGameContext::getCurrentHolyDay() const
{
    auto it = holyDayCalendar.find(getCurrentDayOfYear());
    if (it != holyDayCalendar.end()) {
        return it->second;
    }
    return "";
}

std::string ReligiousGameContext::getDeityOfCurrentHolyDay() const
{
    auto it = holyDayDeities.find(getCurrentDayOfYear());
    if (it != holyDayDeities.end()) {
        return it->second;
    }
    return "";
}