#pragma once

#include <map>
#include <string>

// Game world state structure
struct WorldState {
    std::map<std::string, std::string> locationStates;
    std::map<std::string, std::string> factionStates;
    std::map<std::string, bool> worldFlags;
    int daysPassed = 0;
    std::string currentSeason = "spring";

    WorldState();
    bool hasFlag(const std::string& flag) const;
    std::string getLocationState(const std::string& location) const;
    std::string getFactionState(const std::string& faction) const;
    void setLocationState(const std::string& location, const std::string& state);
    void setFactionState(const std::string& faction, const std::string& state);
    void setWorldFlag(const std::string& flag, bool value);
    void advanceDay();
};