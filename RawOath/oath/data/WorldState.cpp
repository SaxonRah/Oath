#include <WorldState.hpp>

WorldState::WorldState()
{
    // Initialize some locations
    locationStates["village"] = "peaceful";
    locationStates["forest"] = "wild";
    locationStates["mountain"] = "unexplored";
    locationStates["castle"] = "occupied";

    // Initialize faction states
    factionStates["villagers"] = "normal";
    factionStates["bandits"] = "aggressive";
    factionStates["merchants"] = "traveling";

    // Initialize world flags
    worldFlags["war_active"] = false;
    worldFlags["plague_spreading"] = false;
    worldFlags["dragons_returned"] = false;
}

bool WorldState::hasFlag(const std::string& flag) const
{
    auto it = worldFlags.find(flag);
    return it != worldFlags.end() && it->second;
}

std::string WorldState::getLocationState(const std::string& location) const
{
    auto it = locationStates.find(location);
    return (it != locationStates.end()) ? it->second : "unknown";
}

std::string WorldState::getFactionState(const std::string& faction) const
{
    auto it = factionStates.find(faction);
    return (it != factionStates.end()) ? it->second : "unknown";
}

void WorldState::setLocationState(const std::string& location, const std::string& state)
{
    locationStates[location] = state;
}

void WorldState::setFactionState(const std::string& faction, const std::string& state)
{
    factionStates[faction] = state;
}

void WorldState::setWorldFlag(const std::string& flag, bool value)
{
    worldFlags[flag] = value;
}

void WorldState::advanceDay()
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