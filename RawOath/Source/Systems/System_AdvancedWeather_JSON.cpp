// System_AdvancedWeather.cpp
#include <algorithm>
#include <ctime>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <queue>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <variant>
#include <vector>


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
    WeatherSystemNode(const std::string& name)
        : TANode(name)
    {
        // Load weather configuration
        loadWeatherConfig();

        // Initialize RNG with random seed
        std::random_device rd;
        rng.seed(rd());

        // Initialize hours until weather change (from config)
        int minHours = weatherConfig["weatherChangeInterval"]["min"];
        int maxHours = weatherConfig["weatherChangeInterval"]["max"];
        hoursUntilWeatherChange = minHours + (rng() % (maxHours - minHours + 1));

        // Initialize with default weather
        globalWeather = WeatherCondition(WeatherType::Clear, WeatherIntensity::None, "Clear skies with a gentle breeze.");

        // Generate initial forecasts
        generateWeatherForecasts();
    }

    // Load weather configuration from JSON file
    void loadWeatherConfig()
    {
        try {
            std::ifstream file("Weather.JSON");
            if (!file.is_open()) {
                std::cerr << "Failed to open Weather.JSON" << std::endl;
                return;
            }

            weatherConfig = json::parse(file);
            file.close();
        } catch (const std::exception& e) {
            std::cerr << "Error loading weather config: " << e.what() << std::endl;
            // Set a basic default configuration if loading fails
            weatherConfig = json::object();
        }
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

float WeatherCondition::getMovementModifier(const json& weatherData) const
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

float WeatherCondition::getVisibilityModifier(const json& weatherData) const
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

float WeatherCondition::getCombatModifier(const json& weatherData) const
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

        // Reset timer for next change
        int minHours = weatherConfig["weatherChangeInterval"]["min"];
        int maxHours = weatherConfig["weatherChangeInterval"]["max"];
        hoursUntilWeatherChange = minHours + (rng() % (maxHours - minHours + 1));

        // Get current season from context
        std::string currentSeason = context->worldState.currentSeason;

        // Apply season modifiers to transition probabilities
        float roll = dist(rng);
        float cumulativeProbability = 0.0f;

        // Get current weather type as string
        std::string currentTypeStr = weatherTypeToString(globalWeather.type);

        // Get transition probabilities from JSON
        auto transitionsJson = weatherConfig["weatherTransitionProbabilities"][currentTypeStr];

        // Determine next weather type based on transition probabilities
        WeatherType nextType = globalWeather.type; // Default to current
        std::string nextTypeStr = currentTypeStr;

        // Convert JSON transitions to map for processing
        std::map<std::string, float> transitions;
        float totalProb = 0.0f;

        for (auto& [type, probability] : transitionsJson.items()) {
            float prob = probability.get<float>();

            // Skip inappropriate weather for season
            if ((type == "Snowy" || type == "Blizzard") && currentSeason != "winter") {
                continue;
            }

            // Apply seasonal weighting from config
            if (weatherConfig.contains("seasonalModifiers") && weatherConfig["seasonalModifiers"].contains(currentSeason) && weatherConfig["seasonalModifiers"][currentSeason].contains(type)) {

                prob *= weatherConfig["seasonalModifiers"][currentSeason][type].get<float>();
            }

            transitions[type] = prob;
            totalProb += prob;
        }

        // Normalize probabilities
        if (totalProb > 0) {
            for (auto& [type, prob] : transitions) {
                prob /= totalProb;
            }
        }

        // Select next weather type
        roll = dist(rng);
        cumulativeProbability = 0.0f;

        for (const auto& [type, probability] : transitions) {
            cumulativeProbability += probability;
            if (roll < cumulativeProbability) {
                nextTypeStr = type;
                nextType = stringToWeatherType(type);
                break;
            }
        }

        // Select intensity based on weather type from JSON configuration
        WeatherIntensity nextIntensity = WeatherIntensity::Light; // Default

        if (weatherConfig.contains("intensityProbabilities") && weatherConfig["intensityProbabilities"].contains(nextTypeStr)) {

            // Get intensity probabilities
            auto& intensityData = weatherConfig["intensityProbabilities"][nextTypeStr];
            std::map<std::string, float> intensityProbs;
            float totalIntensityProb = 0.0f;

            for (auto& [intensity, prob] : intensityData.items()) {
                intensityProbs[intensity] = prob.get<float>();
                totalIntensityProb += prob.get<float>();
            }

            // Normalize if needed
            if (totalIntensityProb > 0) {
                for (auto& [intensity, prob] : intensityProbs) {
                    prob /= totalIntensityProb;
                }
            }

            // Select intensity
            float intensityRoll = dist(rng);
            float cumIntensityProb = 0.0f;

            for (const auto& [intensity, probability] : intensityProbs) {
                cumIntensityProb += probability;
                if (intensityRoll < cumIntensityProb) {
                    nextIntensity = stringToWeatherIntensity(intensity);
                    break;
                }
            }
        }

        // Create the new weather condition
        WeatherCondition newWeather(nextType, nextIntensity);

        // Generate appropriate description from JSON
        std::string nextIntensityStr = weatherIntensityToString(nextIntensity);
        std::string description;

        if (weatherConfig.contains("weatherDescriptions") && weatherConfig["weatherDescriptions"].contains(nextTypeStr)) {

            auto& descData = weatherConfig["weatherDescriptions"][nextTypeStr];

            // Handle Clear weather with time of day
            if (nextType == WeatherType::Clear && descData.contains(context->worldState.timeOfDay)) {
                description = descData[context->worldState.timeOfDay].get<std::string>();
            }
            // Try intensity-specific description
            else if (descData.contains(nextIntensityStr)) {
                description = descData[nextIntensityStr].get<std::string>();
            }
            // Fall back to default
            else if (descData.contains("default")) {
                description = descData["default"].get<std::string>();
            } else {
                // Generic fallback
                description = "The weather is " + nextTypeStr + ".";
            }
        } else {
            description = "The weather is " + nextTypeStr + ".";
        }

        newWeather.description = description;

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

        // Add special weather events from JSON
        if (weatherConfig.contains("weatherEvents") && weatherConfig["weatherEvents"].contains(nextTypeStr) && weatherConfig["weatherEvents"][nextTypeStr].contains(nextIntensityStr)) {

            auto& eventsData = weatherConfig["weatherEvents"][nextTypeStr][nextIntensityStr];

            for (auto& eventData : eventsData) {
                std::string eventName = eventData["name"];
                std::string eventDesc = eventData["description"];
                double eventProb = eventData["probability"];

                newWeather.possibleEvents.push_back({ eventName,
                    eventDesc,
                    eventProb,
                    [eventName](GameContext* ctx) {
                        if (ctx) {
                            if (eventName == "Lightning Strike") {
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
                            } else if (eventName == "Strange Sounds") {
                                // Fog can hide special encounters
                                if (rand() % 100 < 50) {
                                    std::cout << "The mist parts briefly, revealing something you might have otherwise missed." << std::endl;
                                    ctx->worldState.worldFlags["fog_revealed_secret"] = true;
                                } else {
                                    std::cout << "You feel as if something is watching you from within the fog." << std::endl;
                                    ctx->worldState.worldFlags["fog_hides_danger"] = true;
                                }
                            }
                        }
                    } });
            }
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
                regionDesc << "In " << regionName << ": " << description;
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

WeatherCondition WeatherSystemNode::determineRegionalWeather(
    const std::string& region, const std::string& regionType, const std::string& season)
{
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    // Combine seasonal and regional probabilities from JSON
    std::map<std::string, float> combinedProbs;

    // Start with seasonal probabilities
    if (weatherConfig.contains("seasonalWeatherProbabilities") && weatherConfig["seasonalWeatherProbabilities"].contains(season)) {

        for (auto& [type, prob] : weatherConfig["seasonalWeatherProbabilities"][season].items()) {
            combinedProbs[type] = prob.get<float>();
        }
    }

    // Modify with region-specific probabilities
    if (weatherConfig.contains("regionTypeWeatherProbabilities") && weatherConfig["regionTypeWeatherProbabilities"].contains(regionType)) {

        for (auto& [type, prob] : weatherConfig["regionTypeWeatherProbabilities"][regionType].items()) {
            // Average the probabilities for a balanced approach
            if (combinedProbs.find(type) != combinedProbs.end()) {
                combinedProbs[type] = (combinedProbs[type] + prob.get<float>()) / 2.0f;
            } else {
                combinedProbs[type] = prob.get<float>();
            }
        }
    }

    // Select weather type based on probability
    float roll = dist(rng);
    float cumulativeProbability = 0.0f;
    float totalProb = 0.0f;

    // Calculate total probability
    for (const auto& [type, prob] : combinedProbs) {
        totalProb += prob;
    }

    // Normalize if needed
    if (totalProb > 0) {
        for (auto& [type, prob] : combinedProbs) {
            prob /= totalProb;
        }
    }

    std::string selectedTypeStr = "Clear"; // Default

    for (const auto& [type, prob] : combinedProbs) {
        cumulativeProbability += prob;
        if (roll < cumulativeProbability) {
            selectedTypeStr = type;
            break;
        }
    }

    WeatherType selectedType = stringToWeatherType(selectedTypeStr);

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
    std::string intensityStr = weatherIntensityToString(selectedIntensity);
    std::string typeStr = weatherTypeToString(selectedType);

    std::ostringstream desc;
    desc << "The weather in " << region << " is " << intensityStr << " " << typeStr << ".";
    weather.description = desc.str();

    // Set up effects
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
    report << "Intensity: " << weatherIntensityToString(weather.intensity) << "\n";
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
    report << "\n\nVisibility: " << static_cast<int>(weather.getVisibilityModifier(weatherConfig) * 100) << "% of normal";
    report << "\nMovement Speed: " << static_cast<int>(weather.getMovementModifier(weatherConfig) * 100) << "% of normal";
    report << "\nCombat Effectiveness: " << static_cast<int>(weather.getCombatModifier(weatherConfig) * 100) << "% of normal";

    return report.str();
}

std::string WeatherSystemNode::getWeatherForecast() const
{
    std::ostringstream forecast;

    forecast << "Weather Forecast:\n";

    for (const auto& fc : weatherForecast) {
        forecast << "Day " << fc.dayOffset << ": "
                 << weatherTypeToString(fc.predictedType) << ", "
                 << weatherIntensityToString(fc.predictedIntensity);

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

        // Reload weather configuration
        loadWeatherConfig();

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
                  << static_cast<int>(weatherNode->globalWeather.getMovementModifier(weatherNode->weatherConfig) * 100)
                  << "% of normal" << std::endl;
        std::cout << "- Visibility range: "
                  << static_cast<int>(weatherNode->globalWeather.getVisibilityModifier(weatherNode->weatherConfig) * 100)
                  << "% of normal" << std::endl;
        std::cout << "- Combat effectiveness: "
                  << static_cast<int>(weatherNode->globalWeather.getCombatModifier(weatherNode->weatherConfig) * 100)
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

    // Create the automaton controller
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