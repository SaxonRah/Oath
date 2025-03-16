// WeatherCondition.cpp
#include "WeatherCondition.hpp"
#include <algorithm>
#include <iostream>

WeatherCondition::WeatherCondition(WeatherType t, WeatherIntensity i, const std::string& desc)
    : type(t)
    , intensity(i)
    , description(desc)
{
}

bool WeatherCondition::hasEffect(WeatherEffect effect) const
{
    return std::find(activeEffects.begin(), activeEffects.end(), effect) != activeEffects.end();
}

void WeatherCondition::applyEffects(GameContext* context)
{
    if (!context)
        return;

    // Apply appropriate effects based on weather type and intensity
    switch (type) {
    case WeatherType::Rainy:
    case WeatherType::Stormy:
        // Slow movement in rain/storms
        if (intensity >= WeatherIntensity::Moderate) {
            // Store movement penalty in context
            context->worldState.setWorldFlag("weather_slows_movement", true);
        }
        break;

    case WeatherType::Foggy:
        // Reduce visibility
        if (intensity >= WeatherIntensity::Light) {
            // Store visibility reduction in context
            context->worldState.setWorldFlag("weather_reduces_visibility", true);
        }
        break;

    case WeatherType::Snowy:
    case WeatherType::Blizzard:
        // Severe movement penalty and possible damage
        context->worldState.setWorldFlag("weather_slows_movement", true);
        if (intensity >= WeatherIntensity::Heavy) {
            // Cold damage if not properly equipped
            if (!context->playerInventory.hasItem("warm_cloak") && !context->playerInventory.hasItem("fur_armor")) {
                context->worldState.setWorldFlag("weather_causes_damage", true);
            }
        }
        break;

    case WeatherType::SandStorm:
        // Severe movement penalty and visibility reduction
        context->worldState.setWorldFlag("weather_slows_movement", true);
        context->worldState.setWorldFlag("weather_reduces_visibility", true);
        if (intensity >= WeatherIntensity::Heavy) {
            // Damage if not properly equipped
            if (!context->playerInventory.hasItem("desert_garb") && !context->playerInventory.hasItem("face_covering")) {
                context->worldState.setWorldFlag("weather_causes_damage", true);
            }
        }
        break;

    default:
        // Clear weather has no negative effects
        context->worldState.setWorldFlag("weather_slows_movement", false);
        context->worldState.setWorldFlag("weather_reduces_visibility", false);
        context->worldState.setWorldFlag("weather_causes_damage", false);
        break;
    }

    // Apply skill modifiers based on weather
    if (type == WeatherType::Clear) {
        // Bonus to perception in clear weather
        context->playerStats.skills["perception"] += 5;
    } else if (type == WeatherType::Stormy || type == WeatherType::Blizzard || type == WeatherType::SandStorm) {
        // Penalty to ranged combat in bad weather
        context->playerStats.skills["archery"] -= 10;
    }
}

void WeatherCondition::checkForEvents(GameContext* context, std::mt19937& rng)
{
    if (!context || possibleEvents.empty())
        return;

    std::uniform_real_distribution<double> dist(0.0, 1.0);

    for (const auto& event : possibleEvents) {
        if (dist(rng) < event.probability) {
            // Event triggered!
            std::cout << "Weather event occurred: " << event.name << std::endl;
            std::cout << event.description << std::endl;

            // Apply the event effect
            event.effect(context);

            // Only trigger one event at a time
            break;
        }
    }
}

float WeatherCondition::getMovementModifier(const nlohmann::json& weatherData) const
{
    // Get movement speed modifier from JSON (1.0 = normal speed)
    std::string typeStr = weatherTypeToString(type);
    std::string intensityStr = weatherIntensityToString(intensity);

    // Check if we have specific data for this type and intensity
    if (weatherData.contains("movementModifiers") && weatherData["movementModifiers"].contains(typeStr)) {
        const auto& typeData = weatherData["movementModifiers"][typeStr];
        if (typeData.contains(intensityStr)) {
            return typeData[intensityStr].get<float>();
        } else if (typeData.contains("default")) {
            return typeData["default"].get<float>();
        }
    }

    // Default value if nothing found
    return 1.0f;
}

float WeatherCondition::getVisibilityModifier(const nlohmann::json& weatherData) const
{
    // Get visibility modifier from JSON (1.0 = normal visibility)
    std::string typeStr = weatherTypeToString(type);
    std::string intensityStr = weatherIntensityToString(intensity);

    // Check if we have specific data for this type and intensity
    if (weatherData.contains("visibilityModifiers") && weatherData["visibilityModifiers"].contains(typeStr)) {
        const auto& typeData = weatherData["visibilityModifiers"][typeStr];
        if (typeData.contains(intensityStr)) {
            return typeData[intensityStr].get<float>();
        } else if (typeData.contains("default")) {
            return typeData["default"].get<float>();
        }
    }

    // Default value if nothing found
    return 1.0f;
}

float WeatherCondition::getCombatModifier(const nlohmann::json& weatherData) const
{
    // Get combat effectiveness modifier from JSON (1.0 = normal effectiveness)
    std::string typeStr = weatherTypeToString(type);
    std::string intensityStr = weatherIntensityToString(intensity);

    // Check if we have specific data for this type and intensity
    if (weatherData.contains("combatModifiers") && weatherData["combatModifiers"].contains(typeStr)) {
        const auto& typeData = weatherData["combatModifiers"][typeStr];
        if (typeData.contains(intensityStr)) {
            return typeData[intensityStr].get<float>();
        } else if (typeData.contains("default")) {
            return typeData["default"].get<float>();
        }
    }

    // Default value if nothing found
    return 1.0f;
}
