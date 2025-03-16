// WeatherSystemNode.hpp
#pragma once

#include <map>
#include <random>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "../../core/TAAction.hpp"
#include "../../core/TAController.hpp"
#include "../../core/TAInput.hpp"
#include "../../core/TANode.hpp"
#include "../../data/GameContext.hpp"
#include "../world/RegionNode.hpp"
#include "../world/TimeNode.hpp"

#include "WeatherCondition.hpp"

class TAController;
class RegionNode;
struct GameContext;
struct TAInput;
struct TAAction;

// Main weather system node
class WeatherSystemNode : public TANode {
public:
    // JSON configuration
    nlohmann::json weatherConfig;

    // Add this member variable to track the current region within the weather system
    std::string currentRegionName = "default";

    // Current weather state for each region
    std::map<std::string, WeatherCondition> regionalWeather;

    // Current global weather (for areas not in a specific region)
    WeatherCondition globalWeather;

    // RNG for weather simulation
    std::mt19937 rng;

    // Hours until next weather check
    int hoursUntilWeatherChange;

    // Weather forecast (predictions for next few days)
    struct WeatherForecast {
        int dayOffset;
        WeatherType predictedType;
        WeatherIntensity predictedIntensity;
        float accuracy; // 0.0 to 1.0, how likely this forecast is correct
    };
    std::vector<WeatherForecast> weatherForecast;

    // Constructor
    WeatherSystemNode(const std::string& name);

    // Load weather configuration from JSON file
    void loadWeatherConfig();

    // Generate weather predictions for the next few days
    void generateWeatherForecasts();

    // Update weather when time advances
    void updateWeather(GameContext* context, int hoursElapsed);

    // Determine the appropriate weather for a region based on season and region type
    WeatherCondition determineRegionalWeather(const std::string& region,
        const std::string& regionType,
        const std::string& season);

    // Apply appropriate weather effects when entering a region
    void onRegionEnter(GameContext* context, const std::string& regionName);

    // Get the current weather for a specific region
    WeatherCondition getCurrentWeather(const std::string& regionName) const;

    // Get the current weather description for a region
    std::string getWeatherDescription(const std::string& regionName) const;

    // Generate a descriptive weather report
    std::string getWeatherReport(const std::string& regionName) const;

    // Get the forecast for upcoming days
    std::string getWeatherForecast() const;

    // Override TANode methods
    void onEnter(GameContext* context) override;
    void onExit(GameContext* context) override;

    // Generate available actions for the weather system
    std::vector<TAAction> getAvailableActions() override;

    // Handle transitions
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;

    // Save and load weather system state
    void serialize(std::ofstream& file) const override;
    bool deserialize(std::ifstream& file) override;
};

// Function to initialize the weather system and register it with the controller
void initializeWeatherSystem(TAController& controller);

// Hook to update weather when time advances in the time system
void hookWeatherToTimeSystem(TAController& controller);

// Function to demonstrate weather system functionality
void demonstrateWeatherSystem(TAController& controller);
