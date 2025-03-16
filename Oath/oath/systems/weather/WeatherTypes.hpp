// WeatherTypes.hpp
#pragma once

#include <string>

// Weather state enumeration
enum class WeatherType {
    Clear,
    Cloudy,
    Foggy,
    Rainy,
    Stormy,
    Snowy,
    Blizzard,
    SandStorm // For desert regions
};

// Weather intensity level
enum class WeatherIntensity {
    None,
    Light,
    Moderate,
    Heavy,
    Severe
};

// Weather effect types
enum class WeatherEffect {
    None,
    ReducedVisibility,
    SlowMovement,
    DamageOverTime,
    BonusToSkill,
    PenaltyToSkill,
    SpecialEncounter
};

// Helper functions to convert string to enum and vice versa
WeatherType stringToWeatherType(const std::string& type);
WeatherIntensity stringToWeatherIntensity(const std::string& intensity);
std::string weatherTypeToString(WeatherType type);
std::string weatherIntensityToString(WeatherIntensity intensity);
