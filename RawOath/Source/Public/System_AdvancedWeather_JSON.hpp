// System_AdvancedWeather_JSON.hpp
#ifndef SYSTEM_ADVANCED_WEATHER_JSON_HPP
#define SYSTEM_ADVANCED_WEATHER_JSON_HPP

#include <algorithm>
#include <ctime>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <queue>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

// Forward declarations of main RawOathFull classes (simplified)
class TANode;
class TAController;
class GameContext;
class RegionNode;
class WorldState;
struct TAInput;
struct TAAction;

//----------------------------------------
// WEATHER SYSTEM
//----------------------------------------

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

// Weather condition specific data
struct WeatherCondition {
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
    float getMovementModifier(const json& weatherData) const;

    // Get visibility range modifier based on weather
    float getVisibilityModifier(const json& weatherData) const;

    // Get combat modifier based on weather
    float getCombatModifier(const json& weatherData) const;
};

// Main weather system node
class WeatherSystemNode : public TANode {
private:
    // JSON configuration
    json weatherConfig;

public:
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

//----------------------------------------
// INTEGRATION WITH MAIN GAME SYSTEMS
//----------------------------------------

// Function to initialize the weather system and register it with the controller
void initializeWeatherSystem(TAController& controller);

// Hook to update weather when time advances in the time system
void hookWeatherToTimeSystem(TAController& controller);

// Function to demonstrate weather system functionality
void demonstrateWeatherSystem(TAController& controller);

//----------------------------------------
// MAIN FUNCTION
//----------------------------------------

// This would be added to the main function in your existing code
void addWeatherSystemToMain();

#endif // SYSTEM_ADVANCED_WEATHER_JSON_HPP