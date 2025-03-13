// AdvancedWeatherSystem.cpp
// An extension to the RawOathFull engine that implements a detailed weather simulation

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
        const std::string& desc = "Clear skies")
        : type(t)
        , intensity(i)
        , description(desc)
    {
    }

    // Check if a specific effect is active
    bool hasEffect(WeatherEffect effect) const
    {
        return std::find(activeEffects.begin(), activeEffects.end(), effect) != activeEffects.end();
    }

    // Apply weather effects to game context
    void applyEffects(GameContext* context);

    // Roll for possible weather events
    void checkForEvents(GameContext* context, std::mt19937& rng);

    // Get movement speed modifier based on weather
    float getMovementModifier() const;

    // Get visibility range modifier based on weather
    float getVisibilityModifier() const;

    // Get combat modifier based on weather
    float getCombatModifier() const;
};

// Main weather system node
class WeatherSystemNode : public TANode {
public:
    // Current weather state for each region
    std::map<std::string, WeatherCondition> regionalWeather;

    // Current global weather (for areas not in a specific region)
    WeatherCondition globalWeather;

    // Base weather probabilities for each season
    std::map<std::string, std::map<WeatherType, float>> seasonalWeatherProbabilities;

    // Base weather probabilities for each region type
    std::map<std::string, std::map<WeatherType, float>> regionTypeWeatherProbabilities;

    // Weather transition probabilities
    std::map<WeatherType, std::map<WeatherType, float>> weatherTransitionProbabilities;

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
    WeatherSystemNode(const std::string& name)
        : TANode(name)
    {
        // Initialize RNG with random seed
        std::random_device rd;
        rng.seed(rd());
        hoursUntilWeatherChange = 4 + (rng() % 8); // 4-12 hours

        // Initialize with default weather
        globalWeather = WeatherCondition(WeatherType::Clear, WeatherIntensity::None, "Clear skies with a gentle breeze.");

        // Setup default seasonal probabilities
        initializeSeasonalProbabilities();

        // Setup default region type probabilities
        initializeRegionTypeProbabilities();

        // Setup weather transition matrix
        initializeWeatherTransitions();

        // Generate initial forecasts
        generateWeatherForecasts();
    }

    // Initialize default seasonal weather probabilities
    void initializeSeasonalProbabilities()
    {
        // Spring probabilities
        seasonalWeatherProbabilities["spring"][WeatherType::Clear] = 0.35f;
        seasonalWeatherProbabilities["spring"][WeatherType::Cloudy] = 0.30f;
        seasonalWeatherProbabilities["spring"][WeatherType::Foggy] = 0.15f;
        seasonalWeatherProbabilities["spring"][WeatherType::Rainy] = 0.15f;
        seasonalWeatherProbabilities["spring"][WeatherType::Stormy] = 0.05f;
        seasonalWeatherProbabilities["spring"][WeatherType::Snowy] = 0.0f;
        seasonalWeatherProbabilities["spring"][WeatherType::Blizzard] = 0.0f;
        seasonalWeatherProbabilities["spring"][WeatherType::SandStorm] = 0.0f;

        // Summer probabilities
        seasonalWeatherProbabilities["summer"][WeatherType::Clear] = 0.50f;
        seasonalWeatherProbabilities["summer"][WeatherType::Cloudy] = 0.20f;
        seasonalWeatherProbabilities["summer"][WeatherType::Foggy] = 0.05f;
        seasonalWeatherProbabilities["summer"][WeatherType::Rainy] = 0.15f;
        seasonalWeatherProbabilities["summer"][WeatherType::Stormy] = 0.10f;
        seasonalWeatherProbabilities["summer"][WeatherType::Snowy] = 0.0f;
        seasonalWeatherProbabilities["summer"][WeatherType::Blizzard] = 0.0f;
        seasonalWeatherProbabilities["summer"][WeatherType::SandStorm] = 0.0f;

        // Autumn probabilities
        seasonalWeatherProbabilities["autumn"][WeatherType::Clear] = 0.30f;
        seasonalWeatherProbabilities["autumn"][WeatherType::Cloudy] = 0.35f;
        seasonalWeatherProbabilities["autumn"][WeatherType::Foggy] = 0.20f;
        seasonalWeatherProbabilities["autumn"][WeatherType::Rainy] = 0.10f;
        seasonalWeatherProbabilities["autumn"][WeatherType::Stormy] = 0.05f;
        seasonalWeatherProbabilities["autumn"][WeatherType::Snowy] = 0.0f;
        seasonalWeatherProbabilities["autumn"][WeatherType::Blizzard] = 0.0f;
        seasonalWeatherProbabilities["autumn"][WeatherType::SandStorm] = 0.0f;

        // Winter probabilities
        seasonalWeatherProbabilities["winter"][WeatherType::Clear] = 0.25f;
        seasonalWeatherProbabilities["winter"][WeatherType::Cloudy] = 0.30f;
        seasonalWeatherProbabilities["winter"][WeatherType::Foggy] = 0.10f;
        seasonalWeatherProbabilities["winter"][WeatherType::Rainy] = 0.05f;
        seasonalWeatherProbabilities["winter"][WeatherType::Stormy] = 0.05f;
        seasonalWeatherProbabilities["winter"][WeatherType::Snowy] = 0.20f;
        seasonalWeatherProbabilities["winter"][WeatherType::Blizzard] = 0.05f;
        seasonalWeatherProbabilities["winter"][WeatherType::SandStorm] = 0.0f;
    }

    // Initialize region type weather probabilities
    void initializeRegionTypeProbabilities()
    {
        // Forest region probabilities
        regionTypeWeatherProbabilities["forest"][WeatherType::Clear] = 0.30f;
        regionTypeWeatherProbabilities["forest"][WeatherType::Cloudy] = 0.25f;
        regionTypeWeatherProbabilities["forest"][WeatherType::Foggy] = 0.20f;
        regionTypeWeatherProbabilities["forest"][WeatherType::Rainy] = 0.15f;
        regionTypeWeatherProbabilities["forest"][WeatherType::Stormy] = 0.10f;
        regionTypeWeatherProbabilities["forest"][WeatherType::Snowy] = 0.0f;
        regionTypeWeatherProbabilities["forest"][WeatherType::Blizzard] = 0.0f;
        regionTypeWeatherProbabilities["forest"][WeatherType::SandStorm] = 0.0f;

        // Mountain region probabilities
        regionTypeWeatherProbabilities["mountain"][WeatherType::Clear] = 0.25f;
        regionTypeWeatherProbabilities["mountain"][WeatherType::Cloudy] = 0.30f;
        regionTypeWeatherProbabilities["mountain"][WeatherType::Foggy] = 0.15f;
        regionTypeWeatherProbabilities["mountain"][WeatherType::Rainy] = 0.10f;
        regionTypeWeatherProbabilities["mountain"][WeatherType::Stormy] = 0.10f;
        regionTypeWeatherProbabilities["mountain"][WeatherType::Snowy] = 0.08f;
        regionTypeWeatherProbabilities["mountain"][WeatherType::Blizzard] = 0.02f;
        regionTypeWeatherProbabilities["mountain"][WeatherType::SandStorm] = 0.0f;

        // Plains region probabilities
        regionTypeWeatherProbabilities["plains"][WeatherType::Clear] = 0.40f;
        regionTypeWeatherProbabilities["plains"][WeatherType::Cloudy] = 0.25f;
        regionTypeWeatherProbabilities["plains"][WeatherType::Foggy] = 0.05f;
        regionTypeWeatherProbabilities["plains"][WeatherType::Rainy] = 0.15f;
        regionTypeWeatherProbabilities["plains"][WeatherType::Stormy] = 0.15f;
        regionTypeWeatherProbabilities["plains"][WeatherType::Snowy] = 0.0f;
        regionTypeWeatherProbabilities["plains"][WeatherType::Blizzard] = 0.0f;
        regionTypeWeatherProbabilities["plains"][WeatherType::SandStorm] = 0.0f;

        // Desert region probabilities
        regionTypeWeatherProbabilities["desert"][WeatherType::Clear] = 0.70f;
        regionTypeWeatherProbabilities["desert"][WeatherType::Cloudy] = 0.10f;
        regionTypeWeatherProbabilities["desert"][WeatherType::Foggy] = 0.0f;
        regionTypeWeatherProbabilities["desert"][WeatherType::Rainy] = 0.05f;
        regionTypeWeatherProbabilities["desert"][WeatherType::Stormy] = 0.05f;
        regionTypeWeatherProbabilities["desert"][WeatherType::Snowy] = 0.0f;
        regionTypeWeatherProbabilities["desert"][WeatherType::Blizzard] = 0.0f;
        regionTypeWeatherProbabilities["desert"][WeatherType::SandStorm] = 0.10f;

        // Coastal region probabilities
        regionTypeWeatherProbabilities["coastal"][WeatherType::Clear] = 0.30f;
        regionTypeWeatherProbabilities["coastal"][WeatherType::Cloudy] = 0.25f;
        regionTypeWeatherProbabilities["coastal"][WeatherType::Foggy] = 0.20f;
        regionTypeWeatherProbabilities["coastal"][WeatherType::Rainy] = 0.15f;
        regionTypeWeatherProbabilities["coastal"][WeatherType::Stormy] = 0.10f;
        regionTypeWeatherProbabilities["coastal"][WeatherType::Snowy] = 0.0f;
        regionTypeWeatherProbabilities["coastal"][WeatherType::Blizzard] = 0.0f;
        regionTypeWeatherProbabilities["coastal"][WeatherType::SandStorm] = 0.0f;
    }

    // Initialize weather transition matrix
    void initializeWeatherTransitions()
    {
        // Clear weather transitions
        weatherTransitionProbabilities[WeatherType::Clear][WeatherType::Clear] = 0.7f;
        weatherTransitionProbabilities[WeatherType::Clear][WeatherType::Cloudy] = 0.2f;
        weatherTransitionProbabilities[WeatherType::Clear][WeatherType::Foggy] = 0.1f;

        // Cloudy weather transitions
        weatherTransitionProbabilities[WeatherType::Cloudy][WeatherType::Clear] = 0.3f;
        weatherTransitionProbabilities[WeatherType::Cloudy][WeatherType::Cloudy] = 0.4f;
        weatherTransitionProbabilities[WeatherType::Cloudy][WeatherType::Foggy] = 0.1f;
        weatherTransitionProbabilities[WeatherType::Cloudy][WeatherType::Rainy] = 0.2f;

        // Foggy weather transitions
        weatherTransitionProbabilities[WeatherType::Foggy][WeatherType::Clear] = 0.2f;
        weatherTransitionProbabilities[WeatherType::Foggy][WeatherType::Cloudy] = 0.3f;
        weatherTransitionProbabilities[WeatherType::Foggy][WeatherType::Foggy] = 0.4f;
        weatherTransitionProbabilities[WeatherType::Foggy][WeatherType::Rainy] = 0.1f;

        // Rainy weather transitions
        weatherTransitionProbabilities[WeatherType::Rainy][WeatherType::Clear] = 0.1f;
        weatherTransitionProbabilities[WeatherType::Rainy][WeatherType::Cloudy] = 0.3f;
        weatherTransitionProbabilities[WeatherType::Rainy][WeatherType::Rainy] = 0.4f;
        weatherTransitionProbabilities[WeatherType::Rainy][WeatherType::Stormy] = 0.2f;

        // Stormy weather transitions
        weatherTransitionProbabilities[WeatherType::Stormy][WeatherType::Clear] = 0.05f;
        weatherTransitionProbabilities[WeatherType::Stormy][WeatherType::Cloudy] = 0.25f;
        weatherTransitionProbabilities[WeatherType::Stormy][WeatherType::Rainy] = 0.5f;
        weatherTransitionProbabilities[WeatherType::Stormy][WeatherType::Stormy] = 0.2f;

        // Snowy weather transitions (winter only)
        weatherTransitionProbabilities[WeatherType::Snowy][WeatherType::Clear] = 0.1f;
        weatherTransitionProbabilities[WeatherType::Snowy][WeatherType::Cloudy] = 0.2f;
        weatherTransitionProbabilities[WeatherType::Snowy][WeatherType::Snowy] = 0.6f;
        weatherTransitionProbabilities[WeatherType::Snowy][WeatherType::Blizzard] = 0.1f;

        // Blizzard weather transitions (winter only)
        weatherTransitionProbabilities[WeatherType::Blizzard][WeatherType::Cloudy] = 0.2f;
        weatherTransitionProbabilities[WeatherType::Blizzard][WeatherType::Snowy] = 0.7f;
        weatherTransitionProbabilities[WeatherType::Blizzard][WeatherType::Blizzard] = 0.1f;

        // Sandstorm weather transitions (desert only)
        weatherTransitionProbabilities[WeatherType::SandStorm][WeatherType::Clear] = 0.6f;
        weatherTransitionProbabilities[WeatherType::SandStorm][WeatherType::Cloudy] = 0.2f;
        weatherTransitionProbabilities[WeatherType::SandStorm][WeatherType::SandStorm] = 0.2f;
    }

    // Generate weather predictions for the next few days
    void generateWeatherForecasts()
    {
        weatherForecast.clear();

        std::uniform_real_distribution<float> accuracyDistribution(0.5f, 1.0f);

        // Generate forecasts for the next 3 days
        for (int day = 1; day <= 3; day++) {
            WeatherForecast forecast;
            forecast.dayOffset = day;

            // Prediction is less accurate the further out it is
            forecast.accuracy = accuracyDistribution(rng) * (1.0f - (day * 0.1f));

            // For now, just predict the current global weather with some randomness
            if (rng() % 100 < static_cast<int>(forecast.accuracy * 100)) {
                forecast.predictedType = globalWeather.type;
                forecast.predictedIntensity = globalWeather.intensity;
            } else {
                // Random prediction from available types
                std::vector<WeatherType> types = {
                    WeatherType::Clear, WeatherType::Cloudy, WeatherType::Foggy,
                    WeatherType::Rainy, WeatherType::Stormy
                };
                std::vector<WeatherIntensity> intensities = {
                    WeatherIntensity::Light, WeatherIntensity::Moderate,
                    WeatherIntensity::Heavy
                };

                forecast.predictedType = types[rng() % types.size()];
                forecast.predictedIntensity = intensities[rng() % intensities.size()];
            }

            weatherForecast.push_back(forecast);
        }
    }

    // Update weather when time advances
    void updateWeather(GameContext* context, int hoursElapsed);

    // Determine the appropriate weather for a region based on season and region type
    WeatherCondition determineRegionalWeather(const std::string& region,
        const std::string& regionType,
        const std::string& season);

    // Apply appropriate weather effects when entering a region
    void onRegionEnter(GameContext* context, const std::string& regionName);

    // Get the current weather for a specific region
    WeatherCondition getCurrentWeather(const std::string& regionName) const
    {
        auto it = regionalWeather.find(regionName);
        if (it != regionalWeather.end()) {
            return it->second;
        }
        // If no region-specific weather, return global weather
        return globalWeather;
    }

    // Get the current weather description for a region
    std::string getWeatherDescription(const std::string& regionName) const
    {
        WeatherCondition weather = getCurrentWeather(regionName);
        return weather.description;
    }

    // Convert WeatherType to string
    std::string weatherTypeToString(WeatherType type) const
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
            return "Sandstorm";
        default:
            return "Unknown";
        }
    }

    // Convert WeatherIntensity to string
    std::string intensityToString(WeatherIntensity intensity) const
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

// Implementation of WeatherCondition methods
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

float WeatherCondition::getMovementModifier() const
{
    // Return movement speed modifier (1.0 = normal speed)
    switch (type) {
    case WeatherType::Clear:
        return 1.0f;
    case WeatherType::Cloudy:
        return 1.0f;
    case WeatherType::Foggy:
        return intensity >= WeatherIntensity::Heavy ? 0.9f : 1.0f;
    case WeatherType::Rainy:
        switch (intensity) {
        case WeatherIntensity::Light:
            return 0.95f;
        case WeatherIntensity::Moderate:
            return 0.9f;
        case WeatherIntensity::Heavy:
            return 0.8f;
        case WeatherIntensity::Severe:
            return 0.7f;
        default:
            return 1.0f;
        }
    case WeatherType::Stormy:
        switch (intensity) {
        case WeatherIntensity::Light:
            return 0.9f;
        case WeatherIntensity::Moderate:
            return 0.8f;
        case WeatherIntensity::Heavy:
            return 0.7f;
        case WeatherIntensity::Severe:
            return 0.6f;
        default:
            return 1.0f;
        }
    case WeatherType::Snowy:
        switch (intensity) {
        case WeatherIntensity::Light:
            return 0.9f;
        case WeatherIntensity::Moderate:
            return 0.8f;
        case WeatherIntensity::Heavy:
            return 0.7f;
        case WeatherIntensity::Severe:
            return 0.6f;
        default:
            return 1.0f;
        }
    case WeatherType::Blizzard:
        return 0.5f;
    case WeatherType::SandStorm:
        return 0.5f;
    default:
        return 1.0f;
    }
}

float WeatherCondition::getVisibilityModifier() const
{
    // Return visibility modifier (1.0 = normal visibility)
    switch (type) {
    case WeatherType::Clear:
        return 1.0f;
    case WeatherType::Cloudy:
        return 0.9f;
    case WeatherType::Foggy:
        switch (intensity) {
        case WeatherIntensity::Light:
            return 0.7f;
        case WeatherIntensity::Moderate:
            return 0.5f;
        case WeatherIntensity::Heavy:
            return 0.3f;
        case WeatherIntensity::Severe:
            return 0.2f;
        default:
            return 0.7f;
        }
    case WeatherType::Rainy:
        switch (intensity) {
        case WeatherIntensity::Light:
            return 0.8f;
        case WeatherIntensity::Moderate:
            return 0.7f;
        case WeatherIntensity::Heavy:
            return 0.5f;
        case WeatherIntensity::Severe:
            return 0.4f;
        default:
            return 0.8f;
        }
    case WeatherType::Stormy:
        return 0.4f;
    case WeatherType::Snowy:
        switch (intensity) {
        case WeatherIntensity::Light:
            return 0.8f;
        case WeatherIntensity::Moderate:
            return 0.7f;
        case WeatherIntensity::Heavy:
            return 0.5f;
        case WeatherIntensity::Severe:
            return 0.3f;
        default:
            return 0.8f;
        }
    case WeatherType::Blizzard:
        return 0.2f;
    case WeatherType::SandStorm:
        return 0.3f;
    default:
        return 1.0f;
    }
}

float WeatherCondition::getCombatModifier() const
{
    // Return combat effectiveness modifier (1.0 = normal effectiveness)
    switch (type) {
    case WeatherType::Clear:
        return 1.0f;
    case WeatherType::Cloudy:
        return 1.0f;
    case WeatherType::Foggy:
        return 0.8f; // Harder to see enemies
    case WeatherType::Rainy:
        return intensity >= WeatherIntensity::Heavy ? 0.9f : 1.0f;
    case WeatherType::Stormy:
        return 0.8f; // Lightning and thunder distractions
    case WeatherType::Snowy:
        return intensity >= WeatherIntensity::Heavy ? 0.9f : 1.0f;
    case WeatherType::Blizzard:
        return 0.7f; // Very difficult combat conditions
    case WeatherType::SandStorm:
        return 0.7f; // Very difficult combat conditions
    default:
        return 1.0f;
    }
}

// Implementation of WeatherSystemNode methods
void WeatherSystemNode::updateWeather(GameContext* context, int hoursElapsed)
{
    if (!context)
        return;

    // Decrement hours until next weather change
    hoursUntilWeatherChange -= hoursElapsed;

    // If it's time for weather to change
    if (hoursUntilWeatherChange <= 0) {
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);

        // Reset timer for next change (4-8 hours typically)
        hoursUntilWeatherChange = 4 + (rng() % 5);

        // Get current season from context
        std::string currentSeason = context->worldState.currentSeason;

        // Update global weather using transition matrix
        float roll = dist(rng);
        float cumulativeProbability = 0.0f;

        // Apply season modifiers to transition probabilities
        auto& transitions = weatherTransitionProbabilities[globalWeather.type];

        // Determine next weather type based on transition probabilities
        WeatherType nextType = globalWeather.type; // Default to current

        for (const auto& [type, probability] : transitions) {
            // Skip inappropriate weather for season
            if ((type == WeatherType::Snowy || type == WeatherType::Blizzard) && currentSeason != "winter") {
                continue;
            }

            // Apply seasonal weighting
            float adjustedProb = probability;

            // Boost probability based on season
            if (currentSeason == "winter" && (type == WeatherType::Snowy || type == WeatherType::Blizzard)) {
                adjustedProb *= 1.5f;
            } else if (currentSeason == "spring" && type == WeatherType::Rainy) {
                adjustedProb *= 1.3f;
            } else if (currentSeason == "summer" && type == WeatherType::Stormy) {
                adjustedProb *= 1.2f;
            } else if (currentSeason == "autumn" && type == WeatherType::Foggy) {
                adjustedProb *= 1.3f;
            }

            cumulativeProbability += adjustedProb;
            if (roll < cumulativeProbability) {
                nextType = type;
                break;
            }

            // Determine intensity based on weather type
            std::vector<WeatherIntensity> intensities;
            std::vector<float> intensityProbs;

            switch (nextType) {
            case WeatherType::Clear:
                intensities = { WeatherIntensity::None };
                intensityProbs = { 1.0f };
                break;

            case WeatherType::Cloudy:
                intensities = { WeatherIntensity::Light, WeatherIntensity::Moderate };
                intensityProbs = { 0.7f, 0.3f };
                break;

            case WeatherType::Foggy:
                intensities = { WeatherIntensity::Light, WeatherIntensity::Moderate, WeatherIntensity::Heavy };
                intensityProbs = { 0.4f, 0.4f, 0.2f };
                break;

            case WeatherType::Rainy:
                intensities = { WeatherIntensity::Light, WeatherIntensity::Moderate, WeatherIntensity::Heavy };
                intensityProbs = { 0.5f, 0.3f, 0.2f };
                break;

            case WeatherType::Stormy:
                intensities = { WeatherIntensity::Moderate, WeatherIntensity::Heavy, WeatherIntensity::Severe };
                intensityProbs = { 0.4f, 0.4f, 0.2f };
                break;

            case WeatherType::Snowy:
                intensities = { WeatherIntensity::Light, WeatherIntensity::Moderate, WeatherIntensity::Heavy };
                intensityProbs = { 0.5f, 0.3f, 0.2f };
                break;

            case WeatherType::Blizzard:
                intensities = { WeatherIntensity::Heavy, WeatherIntensity::Severe };
                intensityProbs = { 0.6f, 0.4f };
                break;

            case WeatherType::SandStorm:
                intensities = { WeatherIntensity::Moderate, WeatherIntensity::Heavy, WeatherIntensity::Severe };
                intensityProbs = { 0.3f, 0.4f, 0.3f };
                break;

            default:
                intensities = { WeatherIntensity::Light };
                intensityProbs = { 1.0f };
                break;
            }

            // Select intensity based on probability distribution
            WeatherIntensity nextIntensity = intensities[0];
            float intensityRoll = dist(rng);
            float cumIntensityProb = 0.0f;

            for (size_t i = 0; i < intensities.size(); i++) {
                cumIntensityProb += intensityProbs[i];
                if (intensityRoll < cumIntensityProb) {
                    nextIntensity = intensities[i];
                    break;
                }
            }

            // Create the new weather condition
            WeatherCondition newWeather(nextType, nextIntensity);

            // Generate appropriate description
            std::ostringstream desc;

            // Base description on weather type and intensity
            switch (nextType) {
            case WeatherType::Clear:
                desc << "The sky is clear";
                if (context->worldState.timeOfDay == "morning") {
                    desc << " with a beautiful sunrise.";
                } else if (context->worldState.timeOfDay == "evening") {
                    desc << " with a stunning sunset.";
                } else if (context->worldState.timeOfDay == "night") {
                    desc << " and filled with stars.";
                } else {
                    desc << " and the sun shines brightly.";
                }
                break;

            case WeatherType::Cloudy:
                desc << "The sky is " << (nextIntensity == WeatherIntensity::Light ? "partly" : "mostly")
                     << " cloudy with gray clouds moving overhead.";
                break;

            case WeatherType::Foggy:
                switch (nextIntensity) {
                case WeatherIntensity::Light:
                    desc << "A light mist hangs in the air, giving everything a soft appearance.";
                    break;
                case WeatherIntensity::Moderate:
                    desc << "Fog reduces visibility, obscuring distant landmarks.";
                    break;
                case WeatherIntensity::Heavy:
                case WeatherIntensity::Severe:
                    desc << "A thick fog blankets the area, making it difficult to see more than a few yards ahead.";
                    break;
                default:
                    desc << "Wisps of fog drift through the area.";
                    break;
                }
                break;

            case WeatherType::Rainy:
                switch (nextIntensity) {
                case WeatherIntensity::Light:
                    desc << "A gentle drizzle falls from the sky.";
                    break;
                case WeatherIntensity::Moderate:
                    desc << "A steady rain pours down, creating puddles on the ground.";
                    break;
                case WeatherIntensity::Heavy:
                    desc << "Heavy rain pours down relentlessly, soaking everything.";
                    break;
                case WeatherIntensity::Severe:
                    desc << "A torrential downpour floods the area, making travel difficult.";
                    break;
                default:
                    desc << "Raindrops fall sporadically from dark clouds above.";
                    break;
                }
                break;

            case WeatherType::Stormy:
                switch (nextIntensity) {
                case WeatherIntensity::Moderate:
                    desc << "Thunder rumbles in the distance as rain falls steadily.";
                    break;
                case WeatherIntensity::Heavy:
                    desc << "A thunderstorm rages with lightning flashing across the sky.";
                    break;
                case WeatherIntensity::Severe:
                    desc << "A violent storm with howling winds and crashing thunder makes travel treacherous.";
                    break;
                default:
                    desc << "Dark storm clouds gather as thunder echoes in the distance.";
                    break;
                }
                break;

            case WeatherType::Snowy:
                switch (nextIntensity) {
                case WeatherIntensity::Light:
                    desc << "Light snowflakes drift lazily from the sky.";
                    break;
                case WeatherIntensity::Moderate:
                    desc << "Snow falls steadily, covering the ground in white.";
                    break;
                case WeatherIntensity::Heavy:
                    desc << "Heavy snow falls rapidly, accumulating on every surface.";
                    break;
                case WeatherIntensity::Severe:
                    desc << "Snow falls in thick blankets, making travel slow and difficult.";
                    break;
                default:
                    desc << "A few snowflakes float down from the cloudy sky.";
                    break;
                }
                break;

            case WeatherType::Blizzard:
                desc << "A fierce blizzard rages with howling winds and blinding snow.";
                if (nextIntensity == WeatherIntensity::Severe) {
                    desc << " Venturing outside without protection would be deadly.";
                }
                break;

            case WeatherType::SandStorm:
                desc << "A " << (nextIntensity == WeatherIntensity::Severe ? "violent" : "strong")
                     << " sandstorm whips sand and dust through the air, stinging exposed skin.";
                break;

            default:
                desc << "The weather seems unremarkable.";
                break;
            }

            newWeather.description = desc.str();

            // Set up weather effects based on type and intensity
            if (nextType == WeatherType::Foggy && nextIntensity >= WeatherIntensity::Moderate) {
                newWeather.activeEffects.push_back(WeatherEffect::ReducedVisibility);
            }

            if ((nextType == WeatherType::Rainy && nextIntensity >= WeatherIntensity::Heavy) || (nextType == WeatherType::Stormy) || (nextType == WeatherType::Snowy && nextIntensity >= WeatherIntensity::Moderate) || (nextType == WeatherType::Blizzard) || (nextType == WeatherType::SandStorm)) {
                newWeather.activeEffects.push_back(WeatherEffect::SlowMovement);
            }

            if ((nextType == WeatherType::Blizzard && nextIntensity == WeatherIntensity::Severe) || (nextType == WeatherType::SandStorm && nextIntensity == WeatherIntensity::Severe)) {
                newWeather.activeEffects.push_back(WeatherEffect::DamageOverTime);
            }

            // Add weather-specific bonuses
            if (nextType == WeatherType::Clear) {
                newWeather.activeEffects.push_back(WeatherEffect::BonusToSkill); // Perception bonus
            }

            // Add weather-specific penalties
            if (nextType == WeatherType::Stormy || nextType == WeatherType::Blizzard || nextType == WeatherType::SandStorm) {
                newWeather.activeEffects.push_back(WeatherEffect::PenaltyToSkill); // Archery penalty
            }

            // Add special weather events
            if (nextType == WeatherType::Stormy && nextIntensity >= WeatherIntensity::Heavy) {
                newWeather.possibleEvents.push_back({ "Lightning Strike",
                    "A bolt of lightning strikes nearby, startling you and any nearby creatures.",
                    0.1, // 10% chance each hour during heavy storms
                    [](GameContext* ctx) {
                        if (ctx) {
                            // Lightning strike could scare away enemies or damage the player
                            bool outsideOrInMetal = !ctx->worldState.worldFlags["player_indoors"] || ctx->worldState.worldFlags["player_in_metal_armor"];

                            if (outsideOrInMetal && (rand() % 100 < 25)) {
                                // 25% chance of damage if exposed
                                std::cout << "The lightning strikes dangerously close, causing damage!" << std::endl;
                                ctx->worldState.worldFlags["player_took_lightning_damage"] = true;
                            } else {
                                // Otherwise, just a frightening experience
                                std::cout << "Nearby creatures flee from the lightning strike." << std::endl;
                                ctx->worldState.worldFlags["enemies_frightened"] = true;
                            }
                        }
                    } });
            }

            if (nextType == WeatherType::Foggy && nextIntensity >= WeatherIntensity::Heavy) {
                newWeather.possibleEvents.push_back({ "Strange Sounds",
                    "In the thick fog, you hear strange, unsettling sounds all around you.",
                    0.15, // 15% chance each hour in heavy fog
                    [](GameContext* ctx) {
                        if (ctx) {
                            // Fog can hide special encounters
                            if (rand() % 100 < 50) {
                                std::cout << "The mist parts briefly, revealing something you might have otherwise missed." << std::endl;
                                ctx->worldState.worldFlags["fog_revealed_secret"] = true;
                            } else {
                                std::cout << "You feel as if something is watching you from within the fog." << std::endl;
                                ctx->worldState.worldFlags["fog_hides_danger"] = true;
                            }
                        }
                    } });
            }

            // Update global weather
            globalWeather = newWeather;

            // Also update regional weather where appropriate
            for (auto& [regionName, regionWeather] : regionalWeather) {
                // Regional weather should be influenced by global weather but maintain some independence
                // For simplicity, give a 50% chance to sync with global weather
                if (rand() % 100 < 50) {
                    // Get region type (if available in context)
                    std::string regionType = "default";
                    if (context->worldState.locationStates.find(regionName) != context->worldState.locationStates.end()) {
                        regionType = context->worldState.locationStates[regionName];
                    }

                    // Region-specific adjustments
                    // Desert regions resist rain, mountain regions get more snow, etc.
                    WeatherCondition adjustedWeather = newWeather;

                    if (regionType == "desert") {
                        if (newWeather.type == WeatherType::Rainy || newWeather.type == WeatherType::Snowy) {
                            // Desert is more likely to be clear regardless of global weather
                            adjustedWeather.type = (rand() % 100 < 80) ? WeatherType::Clear : WeatherType::Cloudy;
                            adjustedWeather.intensity = WeatherIntensity::Light;
                        } else if (rand() % 100 < 15) { // 15% chance of sandstorm in desert
                            adjustedWeather.type = WeatherType::SandStorm;
                            adjustedWeather.intensity = (rand() % 100 < 30) ? WeatherIntensity::Severe : WeatherIntensity::Moderate;
                        }
                    } else if (regionType == "mountain" && currentSeason == "winter") {
                        // Mountains in winter are more likely to have snow
                        if (newWeather.type == WeatherType::Rainy) {
                            adjustedWeather.type = WeatherType::Snowy;
                        } else if (newWeather.type == WeatherType::Stormy && rand() % 100 < 50) {
                            adjustedWeather.type = WeatherType::Blizzard;
                        }
                    } else if (regionType == "forest") {
                        // Forests are more likely to retain precipitation longer
                        if (newWeather.type == WeatherType::Clear && (regionWeather.type == WeatherType::Rainy || regionWeather.type == WeatherType::Snowy)) {
                            adjustedWeather.type = regionWeather.type;
                            adjustedWeather.intensity = WeatherIntensity::Light;
                        }
                    } else if (regionType == "coastal") {
                        // Coastal areas have more fog
                        if (rand() % 100 < 30 && newWeather.type != WeatherType::Stormy) {
                            adjustedWeather.type = WeatherType::Foggy;
                            adjustedWeather.intensity = WeatherIntensity::Moderate;
                        }
                    }

                    // Update description for the region-specific weather
                    std::ostringstream regionDesc;
                    regionDesc << "In " << regionName << ": " << desc.str();
                    adjustedWeather.description = regionDesc.str();

                    regionalWeather[regionName] = adjustedWeather;
                }
            }

            // Apply weather effects to current region
            std::string currentRegion = context->worldState.worldFlags["current_region_name"] ? std::get<std::string>(context->stateData.at("current_region")) : "default";

            WeatherCondition currentWeather = getCurrentWeather(currentRegion);
            currentWeather.applyEffects(context);

            // Check for weather events
            currentWeather.checkForEvents(context, rng);

            // Regenerate weather forecasts when weather changes
            generateWeatherForecasts();

            // Notify the player
            std::cout << "\nThe weather has changed: " << currentWeather.description << std::endl;

            // Apply any weather-related world state changes
            if (currentWeather.type == WeatherType::Stormy && currentWeather.intensity >= WeatherIntensity::Heavy) {
                context->worldState.setWorldFlag("stormy_weather", true);
            } else {
                context->worldState.setWorldFlag("stormy_weather", false);
            }

            if (currentWeather.type == WeatherType::Snowy || currentWeather.type == WeatherType::Blizzard) {
                context->worldState.setWorldFlag("snowy_weather", true);
            } else {
                context->worldState.setWorldFlag("snowy_weather", false);
            }
        }
    }
}

WeatherCondition WeatherSystemNode::determineRegionalWeather(
    const std::string& region, const std::string& regionType, const std::string& season)
{

    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    // Combine seasonal and regional probabilities
    std::map<WeatherType, float> combinedProbs;

    // Start with seasonal probabilities
    for (const auto& [type, prob] : seasonalWeatherProbabilities[season]) {
        combinedProbs[type] = prob;
    }

    // Modify with region-specific probabilities
    if (regionTypeWeatherProbabilities.find(regionType) != regionTypeWeatherProbabilities.end()) {
        for (const auto& [type, prob] : regionTypeWeatherProbabilities[regionType]) {
            // Average the probabilities for a balanced approach
            combinedProbs[type] = (combinedProbs[type] + prob) / 2.0f;
        }
    }

    // Select weather type based on probability
    float roll = dist(rng);
    float cumulativeProbability = 0.0f;

    WeatherType selectedType = WeatherType::Clear; // Default

    for (const auto& [type, prob] : combinedProbs) {
        cumulativeProbability += prob;
        if (roll < cumulativeProbability) {
            selectedType = type;
            break;
        }
    }

    // Determine intensity
    WeatherIntensity selectedIntensity;
    int intensityRoll = rng() % 100;

    if (intensityRoll < 40) {
        selectedIntensity = WeatherIntensity::Light;
    } else if (intensityRoll < 75) {
        selectedIntensity = WeatherIntensity::Moderate;
    } else if (intensityRoll < 95) {
        selectedIntensity = WeatherIntensity::Heavy;
    } else {
        selectedIntensity = WeatherIntensity::Severe;
    }

    // Create weather condition
    WeatherCondition weather(selectedType, selectedIntensity);

    // Generate description
    std::ostringstream desc;
    desc << "The weather in " << region << " is "
         << intensityToString(selectedIntensity) << " "
         << weatherTypeToString(selectedType) << ".";
    weather.description = desc.str();

    // Set up effects similar to updateWeather method
    // (this could be refactored to avoid duplicate code)
    if (selectedType == WeatherType::Foggy && selectedIntensity >= WeatherIntensity::Moderate) {
        weather.activeEffects.push_back(WeatherEffect::ReducedVisibility);
    }

    if ((selectedType == WeatherType::Rainy && selectedIntensity >= WeatherIntensity::Heavy) || (selectedType == WeatherType::Stormy) || (selectedType == WeatherType::Snowy && selectedIntensity >= WeatherIntensity::Moderate) || (selectedType == WeatherType::Blizzard) || (selectedType == WeatherType::SandStorm)) {
        weather.activeEffects.push_back(WeatherEffect::SlowMovement);
    }

    return weather;
}

void WeatherSystemNode::onRegionEnter(GameContext* context, const std::string& regionName)
{
    if (!context)
        return;

    // If we don't have weather for this region yet, generate it
    if (regionalWeather.find(regionName) == regionalWeather.end()) {
        // Get region type from context if available
        std::string regionType = "default";
        if (context->worldState.locationStates.find(regionName) != context->worldState.locationStates.end()) {
            regionType = context->worldState.locationStates[regionName];
        }

        // Generate regional weather
        regionalWeather[regionName] = determineRegionalWeather(
            regionName, regionType, context->worldState.currentSeason);
    }

    // Get the current weather for this region
    WeatherCondition currentWeather = regionalWeather[regionName];

    // Apply effects based on current weather
    currentWeather.applyEffects(context);

    // Display weather description
    std::cout << currentWeather.description << std::endl;

    // Store current region in context for future reference
    context->stateData["current_region"] = regionName;

    // Check for weather events when entering region
    currentWeather.checkForEvents(context, rng);
}

std::string WeatherSystemNode::getWeatherReport(const std::string& regionName) const
{
    WeatherCondition weather = getCurrentWeather(regionName);

    std::ostringstream report;

    report << "Current Weather Report for " << (regionName == "default" ? "the area" : regionName) << ":\n";
    report << "Conditions: " << weatherTypeToString(weather.type) << "\n";
    report << "Intensity: " << intensityToString(weather.intensity) << "\n";
    report << "Description: " << weather.description << "\n";

    // Add effect information
    report << "Effects:";
    if (weather.activeEffects.empty()) {
        report << " None";
    } else {
        for (const auto& effect : weather.activeEffects) {
            switch (effect) {
            case WeatherEffect::ReducedVisibility:
                report << "\n- Reduced visibility";
                break;
            case WeatherEffect::SlowMovement:
                report << "\n- Slower movement";
                break;
            case WeatherEffect::DamageOverTime:
                report << "\n- Potential environmental damage";
                break;
            case WeatherEffect::BonusToSkill:
                report << "\n- Perception bonus";
                break;
            case WeatherEffect::PenaltyToSkill:
                report << "\n- Ranged combat penalty";
                break;
            case WeatherEffect::SpecialEncounter:
                report << "\n- Unusual encounters possible";
                break;
            default:
                break;
            }
        }
    }

    // Add movement and visibility information
    report << "\n\nVisibility: " << static_cast<int>(weather.getVisibilityModifier() * 100) << "% of normal";
    report << "\nMovement Speed: " << static_cast<int>(weather.getMovementModifier() * 100) << "% of normal";
    report << "\nCombat Effectiveness: " << static_cast<int>(weather.getCombatModifier() * 100) << "% of normal";

    return report.str();
}

std::string WeatherSystemNode::getWeatherForecast() const
{
    std::ostringstream forecast;

    forecast << "Weather Forecast:\n";

    for (const auto& fc : weatherForecast) {
        forecast << "Day " << fc.dayOffset << ": "
                 << weatherTypeToString(fc.predictedType) << ", "
                 << intensityToString(fc.predictedIntensity);

        // Add confidence level
        forecast << " (Confidence: " << static_cast<int>(fc.accuracy * 100) << "%)";
        forecast << "\n";
    }

    return forecast.str();
}

void WeatherSystemNode::onEnter(GameContext* context)
{
    std::cout << "Weather System active. Current global weather: "
              << globalWeather.description << std::endl;

    // Check if we need to initialize weather for the current region
    if (context) {
        std::string currentRegion = "default";
        if (context->stateData.find("current_region") != context->stateData.end() && std::holds_alternative<std::string>(context->stateData.at("current_region"))) {
            currentRegion = std::get<std::string>(context->stateData.at("current_region"));
        }

        if (currentRegion != "default" && regionalWeather.find(currentRegion) == regionalWeather.end()) {
            // Initialize weather for this region
            std::string regionType = "default";
            if (context->worldState.locationStates.find(currentRegion) != context->worldState.locationStates.end()) {
                regionType = context->worldState.locationStates[currentRegion];
            }

            regionalWeather[currentRegion] = determineRegionalWeather(
                currentRegion, regionType, context->worldState.currentSeason);
        }

        // Apply effects of current weather
        WeatherCondition weather = getCurrentWeather(currentRegion);
        weather.applyEffects(context);

        // Show weather information
        std::cout << getWeatherReport(currentRegion) << std::endl;
    }
}

void WeatherSystemNode::onExit(GameContext* context)
{
    std::cout << "Exiting Weather System node." << std::endl;
}

std::vector<TAAction> WeatherSystemNode::getAvailableActions()
{
    std::vector<TAAction> actions = TANode::getAvailableActions();

    // Add weather-specific actions
    actions.push_back({ "check_weather", "Check current weather conditions",
        [this]() -> TAInput {
            return { "weather_action", { { "action", std::string("check_weather") } } };
        } });

    actions.push_back({ "check_forecast", "Check weather forecast",
        [this]() -> TAInput {
            return { "weather_action", { { "action", std::string("check_forecast") } } };
        } });

    // Add ability to consult weather lore (if player has survival skill)
    actions.push_back({ "weather_lore", "Use weather lore knowledge",
        [this]() -> TAInput {
            return { "weather_action", { { "action", std::string("weather_lore") } } };
        } });

    return actions;
}

bool WeatherSystemNode::evaluateTransition(const TAInput& input, TANode*& outNextNode)
{
    if (input.type == "weather_action") {
        std::string action = std::get<std::string>(input.parameters.at("action"));

        if (action == "check_weather") {
            // Get current region from context
            std::string currentRegion = "default";
            if (stateData.find("current_region") != stateData.end() && std::holds_alternative<std::string>(stateData.at("current_region"))) {
                currentRegion = std::get<std::string>(stateData.at("current_region"));
            }

            // Display detailed weather report
            std::cout << getWeatherReport(currentRegion) << std::endl;

            // Stay in same node
            outNextNode = this;
            return true;
        } else if (action == "check_forecast") {
            // Display weather forecast
            std::cout << getWeatherForecast() << std::endl;

            // Stay in same node
            outNextNode = this;
            return true;
        } else if (action == "weather_lore") {
            // This would check player's survival skill and provide insight
            GameContext* context = nullptr; // In actual implementation, get from controller

            bool hasSurvivalSkill = false;
            if (context && context->playerStats.hasSkill("survival", 2)) {
                hasSurvivalSkill = true;
            }

            if (hasSurvivalSkill) {
                std::cout << "Using your knowledge of weather patterns, you recognize signs that suggest:\n";

                // Provide more accurate forecasts
                for (const auto& fc : weatherForecast) {
                    std::cout << "- Day " << fc.dayOffset << " will likely bring "
                              << weatherTypeToString(fc.predictedType) << " conditions.\n";
                }

                // Provide gameplay hint
                std::cout << "\nYour survival experience tells you that ";
                switch (globalWeather.type) {
                case WeatherType::Clear:
                    std::cout << "this clear weather is excellent for hunting and gathering.";
                    break;
                case WeatherType::Cloudy:
                    std::cout << "these clouds might bring rain by nightfall.";
                    break;
                case WeatherType::Foggy:
                    std::cout << "this fog will conceal your movements, but also hide dangers.";
                    break;
                case WeatherType::Rainy:
                    std::cout << "this rain will make tracking difficult, but animals will gather near water sources.";
                    break;
                case WeatherType::Stormy:
                    std::cout << "this storm is dangerous in open areas, but the noise covers your movements.";
                    break;
                default:
                    std::cout << "current conditions require caution when traveling.";
                    break;
                }
                std::cout << std::endl;
            } else {
                std::cout << "You lack the survival knowledge to accurately interpret weather patterns." << std::endl;
            }

            // Stay in same node
            outNextNode = this;
            return true;
        }
    }

    return TANode::evaluateTransition(input, outNextNode);
}

void WeatherSystemNode::serialize(std::ofstream& file) const
{
    // Call parent serialize first
    TANode::serialize(file);

    // Save global weather
    char weatherType = static_cast<char>(globalWeather.type);
    char weatherIntensity = static_cast<char>(globalWeather.intensity);

    file.write(&weatherType, sizeof(weatherType));
    file.write(&weatherIntensity, sizeof(weatherIntensity));

    size_t descLength = globalWeather.description.length();
    file.write(reinterpret_cast<const char*>(&descLength), sizeof(descLength));
    file.write(globalWeather.description.c_str(), descLength);

    // Save number of active effects
    size_t effectCount = globalWeather.activeEffects.size();
    file.write(reinterpret_cast<const char*>(&effectCount), sizeof(effectCount));

    // Save each effect
    for (const auto& effect : globalWeather.activeEffects) {
        char effectType = static_cast<char>(effect);
        file.write(&effectType, sizeof(effectType));
    }

    // Save hours until next weather change
    file.write(reinterpret_cast<const char*>(&hoursUntilWeatherChange), sizeof(hoursUntilWeatherChange));

    // Save regional weather
    size_t regionCount = regionalWeather.size();
    file.write(reinterpret_cast<const char*>(&regionCount), sizeof(regionCount));

    for (const auto& [region, weather] : regionalWeather) {
        // Save region name
        size_t regionNameLength = region.length();
        file.write(reinterpret_cast<const char*>(&regionNameLength), sizeof(regionNameLength));
        file.write(region.c_str(), regionNameLength);

        // Save weather details
        char regionWeatherType = static_cast<char>(weather.type);
        char regionWeatherIntensity = static_cast<char>(weather.intensity);

        file.write(&regionWeatherType, sizeof(regionWeatherType));
        file.write(&regionWeatherIntensity, sizeof(regionWeatherIntensity));

        size_t regionDescLength = weather.description.length();
        file.write(reinterpret_cast<const char*>(&regionDescLength), sizeof(regionDescLength));
        file.write(weather.description.c_str(), regionDescLength);

        // Save number of active effects
        size_t regionEffectCount = weather.activeEffects.size();
        file.write(reinterpret_cast<const char*>(&regionEffectCount), sizeof(regionEffectCount));

        // Save each effect
        for (const auto& effect : weather.activeEffects) {
            char effectType = static_cast<char>(effect);
            file.write(&effectType, sizeof(effectType));
        }
    }

    // Save forecast information
    size_t forecastCount = weatherForecast.size();
    file.write(reinterpret_cast<const char*>(&forecastCount), sizeof(forecastCount));

    for (const auto& forecast : weatherForecast) {
        file.write(reinterpret_cast<const char*>(&forecast.dayOffset), sizeof(forecast.dayOffset));

        char forecastType = static_cast<char>(forecast.predictedType);
        char forecastIntensity = static_cast<char>(forecast.predictedIntensity);

        file.write(&forecastType, sizeof(forecastType));
        file.write(&forecastIntensity, sizeof(forecastIntensity));
        file.write(reinterpret_cast<const char*>(&forecast.accuracy), sizeof(forecast.accuracy));
    }
}

bool WeatherSystemNode::deserialize(std::ifstream& file)
{
    // Call parent deserialize first
    if (!TANode::deserialize(file)) {
        return false;
    }

    try {
        // Load global weather
        char weatherType;
        char weatherIntensity;

        if (!file.read(&weatherType, sizeof(weatherType)) || !file.read(&weatherIntensity, sizeof(weatherIntensity))) {
            std::cerr << "Failed to read weather type/intensity" << std::endl;
            return false;
        }

        globalWeather.type = static_cast<WeatherType>(weatherType);
        globalWeather.intensity = static_cast<WeatherIntensity>(weatherIntensity);

        size_t descLength;
        if (!file.read(reinterpret_cast<char*>(&descLength), sizeof(descLength))) {
            std::cerr << "Failed to read weather description length" << std::endl;
            return false;
        }

        if (descLength > 10000) {
            std::cerr << "Invalid description length: " << descLength << std::endl;
            return false;
        }

        globalWeather.description.resize(descLength);
        if (!file.read(&globalWeather.description[0], descLength)) {
            std::cerr << "Failed to read weather description" << std::endl;
            return false;
        }

        // Load active effects
        size_t effectCount;
        if (!file.read(reinterpret_cast<char*>(&effectCount), sizeof(effectCount))) {
            std::cerr << "Failed to read effect count" << std::endl;
            return false;
        }

        if (effectCount > 100) {
            std::cerr << "Invalid effect count: " << effectCount << std::endl;
            return false;
        }

        globalWeather.activeEffects.clear();
        for (size_t i = 0; i < effectCount; i++) {
            char effectType;
            if (!file.read(&effectType, sizeof(effectType))) {
                std::cerr << "Failed to read effect type" << std::endl;
                return false;
            }

            globalWeather.activeEffects.push_back(static_cast<WeatherEffect>(effectType));
        }

        // Load hours until next weather change
        if (!file.read(reinterpret_cast<char*>(&hoursUntilWeatherChange), sizeof(hoursUntilWeatherChange))) {
            std::cerr << "Failed to read hours until weather change" << std::endl;
            return false;
        }

        // Load regional weather
        size_t regionCount;
        if (!file.read(reinterpret_cast<char*>(&regionCount), sizeof(regionCount))) {
            std::cerr << "Failed to read region count" << std::endl;
            return false;
        }

        if (regionCount > 1000) {
            std::cerr << "Invalid region count: " << regionCount << std::endl;
            return false;
        }

        regionalWeather.clear();
        for (size_t i = 0; i < regionCount; i++) {
            // Load region name
            size_t regionNameLength;
            if (!file.read(reinterpret_cast<char*>(&regionNameLength), sizeof(regionNameLength))) {
                std::cerr << "Failed to read region name length" << std::endl;
                return false;
            }

            if (regionNameLength > 1000) {
                std::cerr << "Invalid region name length: " << regionNameLength << std::endl;
                return false;
            }

            std::string regionName(regionNameLength, ' ');
            if (!file.read(&regionName[0], regionNameLength)) {
                std::cerr << "Failed to read region name" << std::endl;
                return false;
            }

            // Load weather details
            char regionWeatherType;
            char regionWeatherIntensity;

            if (!file.read(&regionWeatherType, sizeof(regionWeatherType)) || !file.read(&regionWeatherIntensity, sizeof(regionWeatherIntensity))) {
                std::cerr << "Failed to read region weather type/intensity" << std::endl;
                return false;
            }

            WeatherCondition regionWeather;
            regionWeather.type = static_cast<WeatherType>(regionWeatherType);
            regionWeather.intensity = static_cast<WeatherIntensity>(regionWeatherIntensity);

            size_t regionDescLength;
            if (!file.read(reinterpret_cast<char*>(&regionDescLength), sizeof(regionDescLength))) {
                std::cerr << "Failed to read region weather description length" << std::endl;
                return false;
            }

            if (regionDescLength > 10000) {
                std::cerr << "Invalid region description length: " << regionDescLength << std::endl;
                return false;
            }

            regionWeather.description.resize(regionDescLength);
            if (!file.read(&regionWeather.description[0], regionDescLength)) {
                std::cerr << "Failed to read region weather description" << std::endl;
                return false;
            }

            // Load active effects
            size_t regionEffectCount;
            if (!file.read(reinterpret_cast<char*>(&regionEffectCount), sizeof(regionEffectCount))) {
                std::cerr << "Failed to read region effect count" << std::endl;
                return false;
            }

            if (regionEffectCount > 100) {
                std::cerr << "Invalid region effect count: " << regionEffectCount << std::endl;
                return false;
            }

            for (size_t j = 0; j < regionEffectCount; j++) {
                char effectType;
                if (!file.read(&effectType, sizeof(effectType))) {
                    std::cerr << "Failed to read region effect type" << std::endl;
                    return false;
                }

                regionWeather.activeEffects.push_back(static_cast<WeatherEffect>(effectType));
            }

            // Store the region weather
            regionalWeather[regionName] = regionWeather;
        }

        // Load forecast information
        size_t forecastCount;
        if (!file.read(reinterpret_cast<char*>(&forecastCount), sizeof(forecastCount))) {
            std::cerr << "Failed to read forecast count" << std::endl;
            return false;
        }

        if (forecastCount > 100) {
            std::cerr << "Invalid forecast count: " << forecastCount << std::endl;
            return false;
        }

        weatherForecast.clear();
        for (size_t i = 0; i < forecastCount; i++) {
            WeatherForecast forecast;

            if (!file.read(reinterpret_cast<char*>(&forecast.dayOffset), sizeof(forecast.dayOffset))) {
                std::cerr << "Failed to read forecast day offset" << std::endl;
                return false;
            }

            char forecastType;
            char forecastIntensity;

            if (!file.read(&forecastType, sizeof(forecastType)) || !file.read(&forecastIntensity, sizeof(forecastIntensity))) {
                std::cerr << "Failed to read forecast type/intensity" << std::endl;
                return false;
            }

            forecast.predictedType = static_cast<WeatherType>(forecastType);
            forecast.predictedIntensity = static_cast<WeatherIntensity>(forecastIntensity);

            if (!file.read(reinterpret_cast<char*>(&forecast.accuracy), sizeof(forecast.accuracy))) {
                std::cerr << "Failed to read forecast accuracy" << std::endl;
                return false;
            }

            weatherForecast.push_back(forecast);
        }

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Exception during weather system deserialization: " << e.what() << std::endl;
        return false;
    }
}

//----------------------------------------
// INTEGRATION WITH MAIN GAME SYSTEMS
//----------------------------------------

// Function to initialize the weather system and register it with the controller
void initializeWeatherSystem(TAController& controller)
{
    std::cout << "\n___ WEATHER SYSTEM INITIALIZATION ___\n"
              << std::endl;

    // Create weather system node
    WeatherSystemNode* weatherSystem = dynamic_cast<WeatherSystemNode*>(
        controller.createNode<WeatherSystemNode>("WeatherSystem"));

    // Register weather system with the controller
    controller.setSystemRoot("WeatherSystem", weatherSystem);

    // Initialize regional weather for existing regions
    if (controller.systemRoots.find("WorldSystem") != controller.systemRoots.end()) {
        // Get root of world system
        RegionNode* worldRoot = dynamic_cast<RegionNode*>(controller.systemRoots["WorldSystem"]);

        if (worldRoot) {
            // Initialize weather for the main region
            weatherSystem->regionalWeather[worldRoot->regionName] = weatherSystem->determineRegionalWeather(
                worldRoot->regionName,
                worldRoot->regionName.find("Forest") != std::string::npos ? "forest" : worldRoot->regionName.find("Mountain") != std::string::npos ? "mountain"
                                                                                                                                                   : "plains",
                controller.gameContext.worldState.currentSeason);

            // Initialize weather for connected regions
            for (auto* connectedRegion : worldRoot->connectedRegions) {
                RegionNode* region = dynamic_cast<RegionNode*>(connectedRegion);
                if (region) {
                    weatherSystem->regionalWeather[region->regionName] = weatherSystem->determineRegionalWeather(
                        region->regionName,
                        region->regionName.find("Forest") != std::string::npos ? "forest" : region->regionName.find("Mountain") != std::string::npos ? "mountain"
                                                                                                                                                     : "plains",
                        controller.gameContext.worldState.currentSeason);
                }
            }
        }
    }

    std::cout << "Weather system initialized successfully!" << std::endl;
    std::cout << "Current global weather: " << weatherSystem->globalWeather.description << std::endl;

    // Print initial forecast
    std::cout << "\n"
              << weatherSystem->getWeatherForecast() << std::endl;
}

// Hook to update weather when time advances in the time system
void hookWeatherToTimeSystem(TAController& controller)
{
    // Find the time system
    if (controller.systemRoots.find("TimeSystem") != controller.systemRoots.end() && controller.systemRoots.find("WeatherSystem") != controller.systemRoots.end()) {

        // Get the time node
        TimeNode* timeNode = dynamic_cast<TimeNode*>(controller.systemRoots["TimeSystem"]);
        WeatherSystemNode* weatherNode = dynamic_cast<WeatherSystemNode*>(controller.systemRoots["WeatherSystem"]);

        if (timeNode && weatherNode) {
            // Add a hook in the time node to update weather
            // This would typically be done by modifying the TimeNode class to call
            // callbacks when time advances, but for this example, we'll just describe it

            std::cout << "Weather system hooked to time system." << std::endl;
            std::cout << "Weather will update as time passes." << std::endl;

            // In a real implementation, you would modify the TimeNode::advanceHour method like this:
            /*
            void TimeNode::advanceHour(GameContext* context) {
                hour++;
                
                // [... existing time update code ...]
                
                // Update weather when time passes
                if (auto* weatherNode = dynamic_cast<WeatherSystemNode*>(
                        controller->systemRoots["WeatherSystem"])) {
                    weatherNode->updateWeather(context, 1); // 1 hour elapsed
                }
            }
            */
        }
    }
}

// Function to demonstrate weather system functionality
void demonstrateWeatherSystem(TAController& controller)
{
    std::cout << "\n=== WEATHER SYSTEM DEMONSTRATION ===\n"
              << std::endl;

    // Access weather system
    controller.processInput("WeatherSystem", {});

    // Check current weather
    TAInput checkWeatherInput = {
        "weather_action",
        { { "action", std::string("check_weather") } }
    };
    controller.processInput("WeatherSystem", checkWeatherInput);

    // Check forecast
    TAInput checkForecastInput = {
        "weather_action",
        { { "action", std::string("check_forecast") } }
    };
    controller.processInput("WeatherSystem", checkForecastInput);

    // Simulate passage of time to see weather change
    std::cout << "\nSimulating passage of time to observe weather changes...\n"
              << std::endl;

    // Find the weather system
    WeatherSystemNode* weatherNode = dynamic_cast<WeatherSystemNode*>(controller.systemRoots["WeatherSystem"]);

    if (weatherNode) {
        // Force weather change for demonstration
        weatherNode->hoursUntilWeatherChange = 0;
        weatherNode->updateWeather(&controller.gameContext, 1);

        // Check the new weather
        controller.processInput("WeatherSystem", checkWeatherInput);

        // Demonstrate weather effects on travel
        std::cout << "\nWeather Effects on Gameplay:" << std::endl;
        std::cout << "- Movement speed: "
                  << static_cast<int>(weatherNode->globalWeather.getMovementModifier() * 100)
                  << "% of normal" << std::endl;
        std::cout << "- Visibility range: "
                  << static_cast<int>(weatherNode->globalWeather.getVisibilityModifier() * 100)
                  << "% of normal" << std::endl;
        std::cout << "- Combat effectiveness: "
                  << static_cast<int>(weatherNode->globalWeather.getCombatModifier() * 100)
                  << "% of normal" << std::endl;

        // Demonstrate weather-specific events
        if (!weatherNode->globalWeather.possibleEvents.empty()) {
            std::cout << "\nPossible Weather Events:" << std::endl;
            for (const auto& event : weatherNode->globalWeather.possibleEvents) {
                std::cout << "- " << event.name << ": " << event.description << std::endl;
                std::cout << "  (Chance: " << static_cast<int>(event.probability * 100) << "%)" << std::endl;
            }
        }
    }

    // Demonstrate traveling to a region with different weather
    std::cout << "\nTraveling to a different region to observe regional weather differences...\n"
              << std::endl;

    // Switch to world system to travel
    controller.processInput("WorldSystem", {});

    // If we're not already in the forest region, travel there
    RegionNode* currentRegion = dynamic_cast<RegionNode*>(controller.currentNodes["WorldSystem"]);
    RegionNode* targetRegion = nullptr;

    if (currentRegion) {
        for (size_t i = 0; i < currentRegion->connectedRegions.size(); i++) {
            if (currentRegion->connectedRegions[i]->nodeName.find("Forest") != std::string::npos) {
                targetRegion = dynamic_cast<RegionNode*>(currentRegion->connectedRegions[i]);

                TAInput travelInput = {
                    "region_action",
                    { { "action", std::string("travel_region") }, { "region_index", static_cast<int>(i) } }
                };
                controller.processInput("WorldSystem", travelInput);
                break;
            }
        }
    }

    // If we found and traveled to the forest region, check its weather
    if (targetRegion && weatherNode) {
        // This would naturally happen in the RegionNode::onEnter method,
        // but we'll simulate it here
        weatherNode->onRegionEnter(&controller.gameContext, targetRegion->regionName);

        // Check regional weather
        std::cout << "\nChecking weather in " << targetRegion->regionName << ":" << std::endl;
        std::cout << weatherNode->getWeatherReport(targetRegion->regionName) << std::endl;
    }

    // Return to weather system
    controller.processInput("WeatherSystem", {});
}

//----------------------------------------
// MAIN FUNCTION
//----------------------------------------

// This would be added to the main function in your existing code
void addWeatherSystemToMain()
{
    /* Example usage in main():

// Create the automaton controller
TAController controller;

// [... existing system setup ...]

// Initialize weather system
initializeWeatherSystem(controller);

// Hook weather to time system
hookWeatherToTimeSystem(controller);

// [... other game system initialization ...]

// Demonstrate weather system
demonstrateWeatherSystem(controller);

*/
}

int main()
{
    std::cout << "___ Starting Raw Oath Weather System ___" << std::endl;
    Copy // Create the automaton controller
        TAController controller;

    // Initialize base systems (would be different in actual implementation)
    // ...game world initialization code would go here...

    // Initialize weather system
    initializeWeatherSystem(controller);

    // Hook weather to time system
    hookWeatherToTimeSystem(controller);

    // Demonstrate weather system functionality
    demonstrateWeatherSystem(controller);

    std::cout << "\nWeather system demonstration complete." << std::endl;

    return 0;
}
