
// WeatherCondition.hpp
#pragma once

#include <functional>
#include <random>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "../../data/GameContext.hpp"
#include "WeatherTypes.hpp"

// Weather condition specific data
class WeatherCondition {
public:
    WeatherType type;
    WeatherIntensity intensity;
    std::vector<WeatherEffect> activeEffects;
    std::string description;

    // Special events that can occur in this weather
    struct WeatherEvent {
        std::string name;
        std::string description;
        double probability;
        std::function<void(GameContext*)> effect;
    };
    std::vector<WeatherEvent> possibleEvents;

    // Constructor
    WeatherCondition(WeatherType t = WeatherType::Clear,
        WeatherIntensity i = WeatherIntensity::None,
        const std::string& desc = "Clear skies");

    // Check if a specific effect is active
    bool hasEffect(WeatherEffect effect) const;

    // Apply weather effects to game context
    void applyEffects(GameContext* context);

    // Roll for possible weather events
    void checkForEvents(GameContext* context, std::mt19937& rng);

    // Get movement speed modifier based on weather
    float getMovementModifier(const nlohmann::json& weatherData) const;

    // Get visibility range modifier based on weather
    float getVisibilityModifier(const nlohmann::json& weatherData) const;

    // Get combat modifier based on weather
    float getCombatModifier(const nlohmann::json& weatherData) const;
};
