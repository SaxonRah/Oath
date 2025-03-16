// WeatherTypes.cpp
#include "WeatherTypes.hpp"

WeatherType stringToWeatherType(const std::string& type)
{
    if (type == "Clear")
        return WeatherType::Clear;
    if (type == "Cloudy")
        return WeatherType::Cloudy;
    if (type == "Foggy")
        return WeatherType::Foggy;
    if (type == "Rainy")
        return WeatherType::Rainy;
    if (type == "Stormy")
        return WeatherType::Stormy;
    if (type == "Snowy")
        return WeatherType::Snowy;
    if (type == "Blizzard")
        return WeatherType::Blizzard;
    if (type == "SandStorm")
        return WeatherType::SandStorm;

    // Default
    return WeatherType::Clear;
}

WeatherIntensity stringToWeatherIntensity(const std::string& intensity)
{
    if (intensity == "None")
        return WeatherIntensity::None;
    if (intensity == "Light")
        return WeatherIntensity::Light;
    if (intensity == "Moderate")
        return WeatherIntensity::Moderate;
    if (intensity == "Heavy")
        return WeatherIntensity::Heavy;
    if (intensity == "Severe")
        return WeatherIntensity::Severe;

    // Default
    return WeatherIntensity::None;
}

std::string weatherTypeToString(WeatherType type)
{
    switch (type) {
    case WeatherType::Clear:
        return "Clear";
    case WeatherType::Cloudy:
        return "Cloudy";
    case WeatherType::Foggy:
        return "Foggy";
    case WeatherType::Rainy:
        return "Rainy";
    case WeatherType::Stormy:
        return "Stormy";
    case WeatherType::Snowy:
        return "Snowy";
    case WeatherType::Blizzard:
        return "Blizzard";
    case WeatherType::SandStorm:
        return "SandStorm";
    default:
        return "Unknown";
    }
}

std::string weatherIntensityToString(WeatherIntensity intensity)
{
    switch (intensity) {
    case WeatherIntensity::None:
        return "None";
    case WeatherIntensity::Light:
        return "Light";
    case WeatherIntensity::Moderate:
        return "Moderate";
    case WeatherIntensity::Heavy:
        return "Heavy";
    case WeatherIntensity::Severe:
        return "Severe";
    default:
        return "Unknown";
    }
}