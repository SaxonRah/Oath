#pragma once

#include <map>
#include <string>

// WorldState holds data about the game world
class WorldState {
public:
    // Time tracking
    int daysPassed = 0;
    std::string currentSeason = "spring";
    int timeOfDay = 8; // 0-23 hours

    // State tracking
    std::map<std::string, std::string> locationStates;
    std::map<std::string, std::string> factionStates;
    std::map<std::string, bool> worldFlags;

    WorldState()
    {
        // Initialize with defaults
        locationStates["current_region"] = "city";
        locationStates["current_weather"] = "clear";

        factionStates["traders"] = "neutral";
        factionStates["nobles"] = "neutral";
        factionStates["thieves"] = "neutral";
        factionStates["mages"] = "neutral";

        worldFlags["war_active"] = false;
        worldFlags["plague_spreading"] = false;
    }

    // Location methods
    std::string getCurrentRegion() const
    {
        auto it = locationStates.find("current_region");
        return it != locationStates.end() ? it->second : "unknown";
    }

    std::string getLocationState(const std::string& location) const
    {
        auto it = locationStates.find(location);
        return it != locationStates.end() ? it->second : "unknown";
    }

    void setLocationState(const std::string& location, const std::string& state)
    {
        locationStates[location] = state;
    }

    // Faction methods
    std::string getFactionState(const std::string& faction) const
    {
        auto it = factionStates.find(faction);
        return it != factionStates.end() ? it->second : "unknown";
    }

    void setFactionState(const std::string& faction, const std::string& state)
    {
        factionStates[faction] = state;
    }

    // Flag methods
    bool hasFlag(const std::string& flag) const
    {
        auto it = worldFlags.find(flag);
        return it != worldFlags.end() && it->second;
    }

    void setWorldFlag(const std::string& flag, bool value)
    {
        worldFlags[flag] = value;
    }

    // Time methods
    void advanceDay()
    {
        daysPassed++;

        // Update season every 90 days
        if (daysPassed % 90 == 0) {
            if (currentSeason == "spring")
                currentSeason = "summer";
            else if (currentSeason == "summer")
                currentSeason = "autumn";
            else if (currentSeason == "autumn")
                currentSeason = "winter";
            else if (currentSeason == "winter")
                currentSeason = "spring";
        }
    }

    void advanceHour(int hours = 1)
    {
        timeOfDay = (timeOfDay + hours) % 24;
    }

    int getTimeOfDay() const
    {
        return timeOfDay;
    }

    std::string getTimeOfDayString() const
    {
        if (timeOfDay >= 5 && timeOfDay < 12)
            return "morning";
        else if (timeOfDay >= 12 && timeOfDay < 17)
            return "afternoon";
        else if (timeOfDay >= 17 && timeOfDay < 21)
            return "evening";
        else
            return "night";
    }
};