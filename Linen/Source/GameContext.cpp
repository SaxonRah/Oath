#include "GameContext.hpp"

GameContext::GameContext()
    : systemManager(nullptr)
{
    // Initialize player stats
    playerStats.strength = 10;
    playerStats.dexterity = 10;
    playerStats.constitution = 10;
    playerStats.intelligence = 10;
    playerStats.wisdom = 10;
    playerStats.charisma = 10;

    // Initialize health
    healthContext.playerHealth.currentHealth = 100.0f;
    healthContext.playerHealth.maxHealth = 100.0f;
    healthContext.playerHealth.currentMana = 100.0f;
    healthContext.playerHealth.maxMana = 100.0f;
    healthContext.playerHealth.naturalHealRate = 1.0f;

    // Initialize economy
    economyContext.playerGold = 1000;

    // Initialize world state
    worldState.daysPassed = 0;
    worldState.currentSeason = "spring";

    // Initialize starting location
    worldState.setLocationState("village", "current");
}

std::string GameContext::getCurrentRegion() const
{
    for (const auto& [location, state] : locationStates) {
        if (state == "current") {
            return location;
        }
    }
    return "unknown";
}

int GameContext::getTimeOfDay() const
{
    return (daysPassed * 24) % 24; // Simple time calculation
}