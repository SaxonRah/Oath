#include "Source/GameContext.hpp"
#include "Source/GameSystemManager.hpp"
#include "Source/GameSystemPlugin.hpp"
#include "Source/TAController.hpp"
#include <ctime>
#include <functional>
#include <map>
#include <memory>
#include <random>
#include <string>
#include <vector>

// Utility functions for random number generation
namespace utils {
std::mt19937 rng(std::time(nullptr));

int randomInt(int min, int max)
{
    std::uniform_int_distribution<int> dist(min, max);
    return dist(rng);
}

bool randomChance(double probability)
{
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(rng) < probability;
}
}

// Weather System Plugin
class WeatherSystemPlugin : public GameSystemPlugin {
private:
    TAController* controller = nullptr;
    GameContext* context = nullptr;
    TANode* rootNode = nullptr;

    // System-specific data
    std::vector<std::string> weatherTypes = { "clear", "cloudy", "rainy", "stormy", "snowy" };
    json weatherConfig;

public:
    static constexpr const char* SystemName = "WeatherSystem";

    std::string getSystemName() const override { return SystemName; }
    std::string getDescription() const override { return "System for weather simulation and effects."; }
    TANode* getRootNode() const override { return rootNode; }

    void initialize(TAController* ctrl, GameContext* ctx) override
    {
        controller = ctrl;
        context = ctx;

        // Load configuration
        loadConfig();

        // Create root node
        rootNode = controller->createNode<TANode>("weather_system");

        // Register with TAController
        controller->setSystemRoot("WeatherSystem", rootNode);

        // Initialize context data
        context->worldState.setWorldFlag("weather_initialized", true);
    }

    void shutdown() override
    {
        // Cleanup if needed
    }

    void update(float deltaTime) override
    {
        // Determine weather based on region and time
        std::string currentRegion = context->worldState.getCurrentRegion();
        int time = context->worldState.daysPassed;
        double seasonFactor = (time % 4) / 4.0; // Simple season calculation

        int weatherIndex = utils::randomInt(0, 4);

        // Region-specific adjustments
        if (currentRegion == "mountains") {
            weatherIndex = std::min(4, weatherIndex + 1); // More snow in mountains
        } else if (currentRegion == "desert") {
            weatherIndex = std::max(0, weatherIndex - 2); // More clear in desert
        }

        std::string newWeather = weatherTypes[weatherIndex];

        // Only update if weather changed
        if (context->worldState.getLocationState("current_weather") != newWeather) {
            context->worldState.setLocationState("current_weather", newWeather);

            // Weather effects on player stats
            if (newWeather == "stormy") {
                context->playerStats.modifiers["visibility"] = -2;
            } else if (newWeather == "snowy") {
                context->playerStats.modifiers["mobility"] = -2;
            } else {
                // Clear modifiers if weather improves
                context->playerStats.modifiers.erase("visibility");
                context->playerStats.modifiers.erase("mobility");
            }

            // Send weather changed event
            json eventData = {
                { "oldWeather", context->worldState.getLocationState("current_weather") },
                { "newWeather", newWeather },
                { "region", currentRegion }
            };

            context->systemManager->dispatchEvent("weather.changed", eventData);
        }
    }

    json saveState() const override
    {
        json state;
        state["currentWeather"] = context->worldState.getLocationState("current_weather");
        return state;
    }

    bool loadState(const json& data) override
    {
        try {
            if (data.contains("currentWeather")) {
                context->worldState.setLocationState("current_weather", data["currentWeather"]);
            }
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error loading weather state: " << e.what() << std::endl;
            return false;
        }
    }

    bool handleEvent(const std::string& eventType, const json& eventData) override
    {
        if (eventType == "region.changed") {
            // Force weather recalculation when region changes
            update(0.0f);
            return true;
        }
        return false;
    }

private:
    void loadConfig()
    {
        // Load from JSON file or use defaults
        try {
            std::ifstream file("data/weather_config.json");
            if (file.is_open()) {
                file >> weatherConfig;
                file.close();

                // Load weather types from config if available
                if (weatherConfig.contains("weatherTypes") && weatherConfig["weatherTypes"].is_array()) {
                    weatherTypes.clear();
                    for (const auto& type : weatherConfig["weatherTypes"]) {
                        weatherTypes.push_back(type);
                    }
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Weather config loading error: " << e.what() << std::endl;
            // Continue with default values
        }
    }
};

// Crime System Plugin
class CrimeSystemPlugin : public GameSystemPlugin {
private:
    TAController* controller = nullptr;
    GameContext* context = nullptr;
    TANode* rootNode = nullptr;

    // System-specific data
    json crimeConfig;

public:
    static constexpr const char* SystemName = "CrimeSystem";

    std::string getSystemName() const override { return SystemName; }
    std::string getDescription() const override { return "System for crimes, bounties, and law enforcement."; }
    TANode* getRootNode() const override { return rootNode; }

    void initialize(TAController* ctrl, GameContext* ctx) override
    {
        controller = ctrl;
        context = ctx;

        // Load configuration
        loadConfig();

        // Create root node
        rootNode = controller->createNode<TANode>("crime_system");

        // Register with TAController
        controller->setSystemRoot("CrimeSystem", rootNode);

        // Initialize context data
        context->crimeLawContext.bounty = 0;
    }

    void shutdown() override
    {
        // Cleanup if needed
    }

    void update(float deltaTime) override
    {
        // Process crime detection and bounty accumulation
        if (context->playerStats.skills.find("lastCrime") != context->playerStats.skills.end()) {
            int witnessChance = 30;
            std::string currentRegion = context->worldState.getCurrentRegion();

            // Adjust witness chance based on region
            if (currentRegion == "city")
                witnessChance += 30;
            else if (currentRegion == "wilderness")
                witnessChance -= 20;

            // Adjust based on time (night = lower chance)
            int hourOfDay = context->worldState.getTimeOfDay();
            if (hourOfDay >= 22 || hourOfDay <= 5)
                witnessChance -= 15;

            if (utils::randomChance(witnessChance / 100.0)) {
                int lastCrimeValue = context->playerStats.skills["lastCrime"];
                context->crimeLawContext.bounty += lastCrimeValue;

                // Dispatch event for crime detected
                json eventData = {
                    { "crimeType", "lastCrime" },
                    { "severity", lastCrimeValue },
                    { "detected", true },
                    { "region", currentRegion }
                };
                context->systemManager->dispatchEvent("crime.detected", eventData);
            }

            // Clear last crime
            context->playerStats.skills.erase("lastCrime");
        }

        // Handle guard encounters based on bounty
        std::string currentRegion = context->worldState.getCurrentRegion();
        if (context->crimeLawContext.bounty > 0 && currentRegion != "wilderness") {
            double encounterChance = 0.05 * context->crimeLawContext.bounty;

            if (utils::randomChance(std::min(0.8, encounterChance))) {
                // Trigger guard encounter event
                json eventData = {
                    { "bounty", context->crimeLawContext.bounty },
                    { "region", currentRegion }
                };
                context->systemManager->dispatchEvent("crime.guardEncounter", eventData);
            }
        }
    }

    json saveState() const override
    {
        json state;
        state["bounty"] = context->crimeLawContext.bounty;
        state["guardAlerted"] = context->crimeLawContext.guardAlerted;
        state["guardSuspicion"] = context->crimeLawContext.guardSuspicion;
        return state;
    }

    bool loadState(const json& data) override
    {
        try {
            if (data.contains("bounty")) {
                context->crimeLawContext.bounty = data["bounty"];
            }
            if (data.contains("guardAlerted")) {
                context->crimeLawContext.guardAlerted = data["guardAlerted"].get<std::map<std::string, bool>>();
            }
            if (data.contains("guardSuspicion")) {
                context->crimeLawContext.guardSuspicion = data["guardSuspicion"].get<std::map<std::string, int>>();
            }
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error loading crime state: " << e.what() << std::endl;
            return false;
        }
    }

    bool handleEvent(const std::string& eventType, const json& eventData) override
    {
        if (eventType == "player.action.crime") {
            // Handle crime committed
            int crimeType = eventData["type"];
            int severity = 0;
            std::string region = context->worldState.getCurrentRegion();

            if (crimeType == 0) { // Theft
                severity = 10;
                context->playerStats.skills["lastCrime"] = severity;

                // Add stolen goods to inventory if provided
                if (eventData.contains("value")) {
                    int value = eventData["value"];
                    changeGold(value);
                }

                // Report crime for immediate processing
                commitCrime("theft", severity, region);
            } else if (crimeType == 1) { // Assault
                severity = 25;
                context->playerStats.skills["lastCrime"] = severity;

                // Report crime for immediate processing
                commitCrime("assault", severity, region);
            }

            return true;
        } else if (eventType == "player.action.payBounty") {
            // Pay bounty
            int amount = eventData["amount"];
            return payBounty(amount);
        }

        return false;
    }

    // Crime-specific methods
    void commitCrime(const std::string& crimeType, int severity, const std::string& region)
    {
        // Set up crime record
        context->playerStats.skills["lastCrime"] = severity;

        // Trigger immediate detection check
        int witnessChance = 30;

        // Adjust witness chance based on region
        if (region == "city")
            witnessChance += 30;
        else if (region == "wilderness")
            witnessChance -= 20;

        // Adjust based on time (night = lower chance)
        int hourOfDay = context->worldState.getTimeOfDay();
        if (hourOfDay >= 22 || hourOfDay <= 5)
            witnessChance -= 15;

        bool detected = utils::randomChance(witnessChance / 100.0);

        if (detected) {
            context->crimeLawContext.bounty += severity;

            // Dispatch event for crime detected
            json eventData = {
                { "crimeType", crimeType },
                { "severity", severity },
                { "detected", true },
                { "region", region }
            };
            context->systemManager->dispatchEvent("crime.detected", eventData);
        }
    }

    bool payBounty(int amount)
    {
        if (context->economyContext.playerGold < amount) {
            return false; // Not enough gold
        }

        // Take gold
        changeGold(-amount);

        // Clear bounty
        int oldBounty = context->crimeLawContext.bounty;
        context->crimeLawContext.bounty = 0;

        // Reset guard alerts
        for (auto& [region, _] : context->crimeLawContext.guardAlerted) {
            context->crimeLawContext.guardAlerted[region] = false;
        }

        // Dispatch event
        json eventData = {
            { "amountPaid", amount },
            { "oldBounty", oldBounty }
        };
        context->systemManager->dispatchEvent("crime.bountyPaid", eventData);

        return true;
    }

private:
    void loadConfig()
    {
        // Load from JSON file or use defaults
        try {
            std::ifstream file("data/crime_config.json");
            if (file.is_open()) {
                file >> crimeConfig;
                file.close();
            }
        } catch (const std::exception& e) {
            std::cerr << "Crime config loading error: " << e.what() << std::endl;
            // Continue with default values
        }
    }

    // Helper to change gold (used by multiple methods)
    void changeGold(int amount)
    {
        context->economyContext.playerGold += amount;

        // Dispatch gold changed event
        json eventData = {
            { "amount", amount },
            { "newTotal", context->economyContext.playerGold }
        };
        context->systemManager->dispatchEvent("economy.goldChanged", eventData);
    }
};

// Health System Plugin
class HealthSystemPlugin : public GameSystemPlugin {
private:
    TAController* controller = nullptr;
    GameContext* context = nullptr;
    TANode* rootNode = nullptr;

    // System-specific data
    json healthConfig;

public:
    static constexpr const char* SystemName = "HealthSystem";

    std::string getSystemName() const override { return SystemName; }
    std::string getDescription() const override { return "System for health, diseases, and healing."; }
    TANode* getRootNode() const override { return rootNode; }

    void initialize(TAController* ctrl, GameContext* ctx) override
    {
        controller = ctrl;
        context = ctx;

        // Load configuration
        loadConfig();

        // Create root node
        rootNode = controller->createNode<TANode>("health_system");

        // Register with TAController
        controller->setSystemRoot("HealthSystem", rootNode);

        // Initialize context data
        context->healthContext.playerHealth.currentHealth = 100;
        context->healthContext.playerHealth.maxHealth = 100;
    }

    void shutdown() override
    {
        // Cleanup if needed
    }

    void update(float deltaTime) override
    {
        // Process active diseases
        auto& health = context->healthContext.playerHealth;

        for (auto it = health.activeDiseaseDays.begin(); it != health.activeDiseaseDays.end();) {
            // Disease progression
            std::string disease = it->first;
            int days = it->second;

            // Disease progresses if untreated
            bool treated = (context->playerStats.skills.find("treated_" + disease) != context->playerStats.skills.end());

            if (!treated) {
                // Increase days
                health.activeDiseaseDays[disease] = days + 1;

                // Apply disease effects
                float severity = days * 0.1f;
                health.takeDamage(severity);

                // Disease can be cured if immunity develops
                if (utils::randomChance(0.1) || days > 10) {
                    // Develop immunity
                    health.addImmunity(disease, 0.8f, 180, context->worldState.daysPassed);
                    it = health.activeDiseaseDays.erase(it);

                    // Dispatch disease recovered event
                    json eventData = {
                        { "disease", disease },
                        { "recoveryType", "natural" },
                        { "immunityDeveloped", true }
                    };
                    context->systemManager->dispatchEvent("health.diseaseRecovered", eventData);
                } else {
                    ++it;
                }
            } else {
                // Treatment helps recovery
                if (days > 1) {
                    health.activeDiseaseDays[disease] = days - 1;
                } else {
                    it = health.activeDiseaseDays.erase(it);

                    // Dispatch disease cured event
                    json eventData = {
                        { "disease", disease },
                        { "recoveryType", "treatment" }
                    };
                    context->systemManager->dispatchEvent("health.diseaseRecovered", eventData);
                    continue;
                }
                ++it;
            }
        }

        // Chance of contracting new diseases based on region
        std::string currentRegion = context->worldState.getCurrentRegion();
        std::string currentWeather = context->worldState.getLocationState("current_weather");

        double diseaseChance = 0.01;
        if (currentRegion == "swamp")
            diseaseChance *= 3;
        if (currentWeather == "rainy")
            diseaseChance *= 1.5;

        if (utils::randomChance(diseaseChance)) {
            std::vector<std::string> possibleDiseases = { "plague", "fever", "cough" };
            std::string newDisease = possibleDiseases[utils::randomInt(0, possibleDiseases.size() - 1)];

            // Check for immunity
            float immunity = health.getImmunityStrength(newDisease, context->worldState.daysPassed);
            if (immunity < 0.5f) {
                contractDisease(newDisease);
            }
        }

        // Natural healing over time
        if (health.currentHealth < health.maxHealth) {
            float healRate = health.naturalHealRate * deltaTime;
            if (health.activeDiseaseDays.empty()) {
                healRate *= 2.0f; // Faster healing when healthy
            }
            health.heal(healRate);
        }
    }

    json saveState() const override
    {
        json state;
        auto& health = context->healthContext.playerHealth;

        state["currentHealth"] = health.currentHealth;
        state["maxHealth"] = health.maxHealth;
        state["naturalHealRate"] = health.naturalHealRate;

        // Save diseases
        json diseases = json::object();
        for (const auto& [disease, days] : health.activeDiseaseDays) {
            diseases[disease] = days;
        }
        state["activeDiseaseDays"] = diseases;

        // Save immunities
        json immunities = json::array();
        for (const auto& immunity : health.immunities) {
            json immunityJson = {
                { "diseaseId", immunity.diseaseId },
                { "strength", immunity.strength },
                { "durationDays", immunity.durationDays },
                { "dayAcquired", immunity.dayAcquired }
            };
            immunities.push_back(immunityJson);
        }
        state["immunities"] = immunities;

        return state;
    }

    bool loadState(const json& data) override
    {
        try {
            auto& health = context->healthContext.playerHealth;

            if (data.contains("currentHealth")) {
                health.currentHealth = data["currentHealth"];
            }
            if (data.contains("maxHealth")) {
                health.maxHealth = data["maxHealth"];
            }
            if (data.contains("naturalHealRate")) {
                health.naturalHealRate = data["naturalHealRate"];
            }

            // Load diseases
            if (data.contains("activeDiseaseDays") && data["activeDiseaseDays"].is_object()) {
                health.activeDiseaseDays.clear();
                for (auto& [disease, days] : data["activeDiseaseDays"].items()) {
                    health.activeDiseaseDays[disease] = days;
                }
            }

            // Load immunities
            if (data.contains("immunities") && data["immunities"].is_array()) {
                health.immunities.clear();
                for (const auto& immunityJson : data["immunities"]) {
                    Immunity immunity(
                        immunityJson["diseaseId"],
                        immunityJson["strength"],
                        immunityJson["durationDays"],
                        immunityJson["dayAcquired"]);
                    health.immunities.push_back(immunity);
                }
            }

            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error loading health state: " << e.what() << std::endl;
            return false;
        }
    }

    bool handleEvent(const std::string& eventType, const json& eventData) override
    {
        if (eventType == "player.action.treatment") {
            std::string diseaseId = eventData["disease"];

            // Mark disease as treated
            context->playerStats.skills["treated_" + diseaseId] = 1;

            return true;
        } else if (eventType == "weather.changed") {
            // Weather can affect health
            std::string newWeather = eventData["newWeather"];
            if (newWeather == "stormy" || newWeather == "snowy") {
                // Increase chance of illness in bad weather
                if (utils::randomChance(0.1)) {
                    contractDisease("cough");
                }
            }
            return true;
        }

        return false;
    }

    // Health-specific methods
    void contractDisease(const std::string& diseaseId)
    {
        auto& health = context->healthContext.playerHealth;

        // Check for immunity
        float immunity = health.getImmunityStrength(diseaseId, context->worldState.daysPassed);
        if (immunity > 0.5f)
            return; // Immune

        // Add disease if not already present
        if (health.activeDiseaseDays.find(diseaseId) == health.activeDiseaseDays.end()) {
            health.activeDiseaseDays[diseaseId] = 1; // Day 1 of disease

            // Dispatch event
            json eventData = {
                { "disease", diseaseId },
                { "region", context->worldState.getCurrentRegion() }
            };
            context->systemManager->dispatchEvent("health.diseaseContracted", eventData);
        }
    }

    void heal(float amount)
    {
        auto& health = context->healthContext.playerHealth;
        float oldHealth = health.currentHealth;
        health.heal(amount);

        // Dispatch healing event
        json eventData = {
            { "amount", amount },
            { "oldHealth", oldHealth },
            { "newHealth", health.currentHealth }
        };
        context->systemManager->dispatchEvent("health.healed", eventData);
    }

private:
    void loadConfig()
    {
        // Load from JSON file or use defaults
        try {
            std::ifstream file("data/health_config.json");
            if (file.is_open()) {
                file >> healthConfig;
                file.close();
            }
        } catch (const std::exception& e) {
            std::cerr << "Health config loading error: " << e.what() << std::endl;
            // Continue with default values
        }
    }
};

// Economy System Plugin
class EconomySystemPlugin : public GameSystemPlugin {
private:
    TAController* controller = nullptr;
    GameContext* context = nullptr;
    TANode* rootNode = nullptr;

    // System-specific data
    json economyConfig;

public:
    static constexpr const char* SystemName = "EconomySystem";

    std::string getSystemName() const override { return SystemName; }
    std::string getDescription() const override { return "System for economy, markets, and trade."; }
    TANode* getRootNode() const override { return rootNode; }

    void initialize(TAController* ctrl, GameContext* ctx) override
    {
        controller = ctrl;
        context = ctx;

        // Load configuration
        loadConfig();

        // Create root node
        rootNode = controller->createNode<TANode>("economy_system");

        // Register with TAController
        controller->setSystemRoot("EconomySystem", rootNode);

        // Initialize context data
        context->economyContext.playerGold = 1000;
        context->economyContext.economyFactors["tax_rate"] = 1.0;
    }

    void shutdown() override
    {
        // Cleanup if needed
    }

    void update(float deltaTime) override
    {
        // Update market prices based on region and events
        std::vector<std::string> goods = { "food", "weapons", "cloth", "medicine" };
        std::string currentRegion = context->worldState.getCurrentRegion();
        std::string currentWeather = context->worldState.getLocationState("current_weather");

        for (const auto& good : goods) {
            // Base price adjustment
            double basePrice = 10.0;
            double priceMultiplier = 1.0;

            // Region-specific price adjustments
            if (currentRegion == "city") {
                if (good == "food")
                    priceMultiplier *= 1.2;
                if (good == "cloth")
                    priceMultiplier *= 0.8;
            } else if (currentRegion == "village") {
                if (good == "food")
                    priceMultiplier *= 0.8;
                if (good == "weapons")
                    priceMultiplier *= 1.3;
            }

            // Weather effects on economy
            if (currentWeather == "stormy" && good == "food") {
                priceMultiplier *= 1.4; // Food more expensive during storms
            }

            // Random fluctuations
            priceMultiplier *= (0.9 + utils::randomInt(0, 20) / 100.0);

            // Store the price in the economy factors
            context->economyContext.economyFactors[good + "_price"] = basePrice * priceMultiplier;
        }

        // Process property income if player owns property
        if (context->playerStats.skills.find("owns_property") != context->playerStats.skills.end()) {
            int propertyLevel = context->playerStats.skills["property_level"];
            int propertyIncome = 5 * propertyLevel;
            changeGold(propertyIncome);
        }

        // Process investments
        for (auto it = context->playerStats.skills.begin(); it != context->playerStats.skills.end(); ++it) {
            if (it->first.find("invest_") == 0) {
                std::string business = it->first.substr(7);
                double returnRate = 0.03; // 3% return

                // Better returns for certain businesses in certain regions
                if (business == "mining" && currentRegion == "mountains") {
                    returnRate *= 1.5;
                }

                int investmentReturn = static_cast<int>(it->second * returnRate);
                changeGold(investmentReturn);

                // Dispatch investment return event
                json eventData = {
                    { "business", business },
                    { "investment", it->second },
                    { "return", investmentReturn }
                };
                context->systemManager->dispatchEvent("economy.investmentReturn", eventData);
            }
        }
    }

    json saveState() const override
    {
        json state;
        state["playerGold"] = context->economyContext.playerGold;
        state["economyFactors"] = context->economyContext.economyFactors;

        // Save property and investment data
        json properties = json::array();
        json investments = json::array();

        for (auto& [key, value] : context->playerStats.skills) {
            if (key.find("property_") == 0) {
                properties.push_back({ { "key", key },
                    { "value", value } });
            } else if (key.find("invest_") == 0) {
                investments.push_back({ { "business", key.substr(7) },
                    { "amount", value } });
            }
        }

        state["properties"] = properties;
        state["investments"] = investments;

        return state;
    }

    bool loadState(const json& data) override
    {
        try {
            if (data.contains("playerGold")) {
                context->economyContext.playerGold = data["playerGold"];
            }
            if (data.contains("economyFactors")) {
                context->economyContext.economyFactors = data["economyFactors"].get<std::map<std::string, double>>();
            }

            // Load property data
            if (data.contains("properties") && data["properties"].is_array()) {
                for (const auto& prop : data["properties"]) {
                    context->playerStats.skills[prop["key"]] = prop["value"];
                }
            }

            // Load investment data
            if (data.contains("investments") && data["investments"].is_array()) {
                for (const auto& inv : data["investments"]) {
                    std::string business = inv["business"];
                    int amount = inv["amount"];
                    context->playerStats.skills["invest_" + business] = amount;
                }
            }

            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error loading economy state: " << e.what() << std::endl;
            return false;
        }
    }

    bool handleEvent(const std::string& eventType, const json& eventData) override
    {
        if (eventType == "player.action.buyProperty") {
            int cost = eventData["cost"];
            int level = eventData["level"];

            if (buyProperty(cost, level)) {
                return true;
            }
        } else if (eventType == "player.action.invest") {
            std::string business = eventData["business"];
            int amount = eventData["amount"];

            if (invest(business, amount)) {
                return true;
            }
        } else if (eventType == "faction.reputation_changed") {
            // Faction reputation can affect prices
            std::string faction = eventData["faction"];
            int newValue = eventData["newValue"];

            if (faction == "traders" || faction == "merchants") {
                // Better trade prices with good merchant relations
                double discount = 0.0;
                if (newValue > 50)
                    discount = 0.1;
                else if (newValue > 20)
                    discount = 0.05;

                context->economyContext.economyFactors["merchant_discount"] = discount;
            }

            return true;
        }

        return false;
    }

    // Economy-specific methods
    bool buyProperty(int cost, int level)
    {
        if (context->economyContext.playerGold < cost) {
            return false;
        }

        // Spend gold
        changeGold(-cost);

        // Add property
        context->playerStats.skills["owns_property"] = 1;
        context->playerStats.skills["property_level"] = level;

        // Dispatch property purchased event
        json eventData = {
            { "cost", cost },
            { "level", level }
        };

        context->systemManager->dispatchEvent("economy.propertyPurchased", eventData);

        return true;
    }

    bool invest(const std::string& business, int amount)
    {
        if (context->economyContext.playerGold < amount) {
            return false;
        }

        // Spend gold
        changeGold(-amount);

        // Make investment
        context->playerStats.skills["invest_" + business] += amount;

        // Dispatch investment event
        json eventData = {
            { "business", business },
            { "amount", amount }
        };
        context->systemManager->dispatchEvent("economy.investmentMade", eventData);

        return true;
    }

    void changeGold(int amount)
    {
        context->economyContext.playerGold += amount;

        // Dispatch gold changed event
        json eventData = {
            { "amount", amount },
            { "newTotal", context->economyContext.playerGold }
        };
        context->systemManager->dispatchEvent("economy.goldChanged", eventData);
    }

private:
    void loadConfig()
    {
        // Load from JSON file or use defaults
        try {
            std::ifstream file("data/economy_config.json");
            if (file.is_open()) {
                file >> economyConfig;
                file.close();
            }
        } catch (const std::exception& e) {
            std::cerr << "Economy config loading error: " << e.what() << std::endl;
            // Continue with default values
        }
    }
};

// Faction System Plugin
class FactionSystemPlugin : public GameSystemPlugin {
private:
    TAController* controller = nullptr;
    GameContext* context = nullptr;
    TANode* rootNode = nullptr;

    // System-specific data
    json factionConfig;
    std::vector<std::string> factions = { "traders", "nobles", "thieves", "mages" };

public:
    static constexpr const char* SystemName = "FactionSystem";

    std::string getSystemName() const override { return SystemName; }
    std::string getDescription() const override { return "System for factions, reputation, and politics."; }
    TANode* getRootNode() const override { return rootNode; }

    void initialize(TAController* ctrl, GameContext* ctx) override
    {
        controller = ctrl;
        context = ctx;

        // Load configuration
        loadConfig();

        // Create root node
        rootNode = controller->createNode<TANode>("faction_system");

        // Register with TAController
        controller->setSystemRoot("FactionSystem", rootNode);

        // Initialize faction relations
        for (const auto& faction : factions) {
            context->factionContext.factionStanding[faction] = 0;
        }
    }

    void shutdown() override
    {
        // Cleanup if needed
    }

    void update(float deltaTime) override
    {
        // Process faction relations over time
        for (const auto& faction : factions) {
            int relation = context->factionContext.factionStanding[faction];

            // Factions have relationships with each other
            for (const auto& otherFaction : factions) {
                if (faction != otherFaction) {
                    // Rival factions
                    if ((faction == "nobles" && otherFaction == "thieves") || (faction == "thieves" && otherFaction == "nobles") || (faction == "traders" && otherFaction == "mages")) {

                        // If player helps one faction, the rival faction relation decreases
                        if (context->playerStats.skills.find("helped_" + faction) != context->playerStats.skills.end()) {
                            int oldValue = context->factionContext.factionStanding[otherFaction];
                            context->factionContext.factionStanding[otherFaction] = oldValue - 1;
                            context->playerStats.skills.erase("helped_" + faction);

                            // Dispatch faction relation changed event
                            json eventData = {
                                { "faction", otherFaction },
                                { "oldValue", oldValue },
                                { "newValue", oldValue - 1 },
                                { "reason", "helped_rival_faction" }
                            };
                            context->systemManager->dispatchEvent("faction.reputation_changed", eventData);
                        }
                    }
                }
            }

            // Factions control different areas - adjust prices and access
            if (context->worldState.getCurrentRegion() == "city" && faction == "nobles") {
                // Nobles control cities
                if (relation < 0) {
                    // Higher prices if disliked by nobles in the city
                    context->economyContext.economyFactors["tax_rate"] = 1.2;
                } else if (relation > 20) {
                    // Discounts if well-liked
                    context->economyContext.economyFactors["tax_rate"] = 0.9;
                }
            }

            // Political shifts happen over time
            static int lastPoliticalShift = 0;
            int currentDay = context->worldState.daysPassed;

            if (currentDay - lastPoliticalShift >= 30) { // Monthly shift
                // Random political changes
                if (utils::randomChance(0.3)) {
                    int shift = utils::randomInt(-5, 5);
                    for (const auto& f : factions) {
                        int oldValue = context->factionContext.factionStanding[f];
                        context->factionContext.factionStanding[f] = oldValue + shift;

                        // Dispatch faction relation changed event
                        json eventData = {
                            { "faction", f },
                            { "oldValue", oldValue },
                            { "newValue", oldValue + shift },
                            { "reason", "political_shift" }
                        };
                        context->systemManager->dispatchEvent("faction.reputation_changed", eventData);
                    }
                }

                lastPoliticalShift = currentDay;
            }
        }
    }

    json saveState() const override
    {
        json state;
        state["factionStanding"] = context->factionContext.factionStanding;
        return state;
    }

    bool loadState(const json& data) override
    {
        try {
            if (data.contains("factionStanding")) {
                context->factionContext.factionStanding = data["factionStanding"].get<std::map<std::string, int>>();
            }
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error loading faction state: " << e.what() << std::endl;
            return false;
        }
    }

    bool handleEvent(const std::string& eventType, const json& eventData) override
    {
        if (eventType == "player.action.helpFaction") {
            std::string faction = eventData["faction"];

            // Mark this faction as helped
            context->playerStats.skills["helped_" + faction] = 1;

            // Improve standing with this faction
            int oldValue = context->factionContext.factionStanding[faction];
            int increase = eventData.contains("amount") ? eventData["amount"].get<int>() : 5;
            context->factionContext.factionStanding[faction] = oldValue + increase;

            // Dispatch faction relation changed event
            json eventData = {
                { "faction", faction },
                { "oldValue", oldValue },
                { "newValue", oldValue + increase },
                { "reason", "player_helped" }
            };
            context->systemManager->dispatchEvent("faction.reputation_changed", eventData);

            return true;
        } else if (eventType == "crime.detected") {
            // Crime affects faction standings
            std::string region = eventData["region"];
            int severity = eventData["severity"];

            // Different regions are controlled by different factions
            std::string controllingFaction = "nobles"; // Default

            if (region == "city")
                controllingFaction = "nobles";
            else if (region == "village")
                controllingFaction = "traders";

            // Decrease faction standing
            int decrease = severity / 2;
            int oldValue = context->factionContext.factionStanding[controllingFaction];
            context->factionContext.factionStanding[controllingFaction] = oldValue - decrease;

            // Thieves guild might actually like criminal activity
            if (controllingFaction != "thieves") {
                int thievesOldValue = context->factionContext.factionStanding["thieves"];
                context->factionContext.factionStanding["thieves"] = thievesOldValue + decrease / 2;

                // Notify of thieves reputation change
                json thievesEventData = {
                    { "faction", "thieves" },
                    { "oldValue", thievesOldValue },
                    { "newValue", thievesOldValue + decrease / 2 },
                    { "reason", "criminal_activity" }
                };
                context->systemManager->dispatchEvent("faction.reputation_changed", thievesEventData);
            }

            // Dispatch faction relation changed event
            json factionEventData = {
                { "faction", controllingFaction },
                { "oldValue", oldValue },
                { "newValue", oldValue - decrease },
                { "reason", "criminal_activity" }
            };
            context->systemManager->dispatchEvent("faction.reputation_changed", factionEventData);

            return true;
        }

        return false;
    }

    // Faction-specific methods
    int getFactionReputation(const std::string& faction)
    {
        auto it = context->factionContext.factionStanding.find(faction);
        if (it != context->factionContext.factionStanding.end()) {
            return it->second;
        }
        return 0;
    }

    void changeFactionReputation(const std::string& faction, int amount)
    {
        auto it = context->factionContext.factionStanding.find(faction);
        if (it != context->factionContext.factionStanding.end()) {
            int oldValue = it->second;
            it->second += amount;

            // Limit to reasonable values
            it->second = std::max(-100, std::min(100, it->second));

            // Dispatch faction relation changed event
            json eventData = {
                { "faction", faction },
                { "oldValue", oldValue },
                { "newValue", it->second },
                { "reason", "direct_change" }
            };
            context->systemManager->dispatchEvent("faction.reputation_changed", eventData);
        }
    }

private:
    void loadConfig()
    {
        // Load from JSON file or use defaults
        try {
            std::ifstream file("data/faction_config.json");
            if (file.is_open()) {
                file >> factionConfig;
                file.close();

                // Load factions list from config if available
                if (factionConfig.contains("factions") && factionConfig["factions"].is_array()) {
                    factions.clear();
                    for (const auto& faction : factionConfig["factions"]) {
                        factions.push_back(faction);
                    }
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Faction config loading error: " << e.what() << std::endl;
            // Continue with default values
        }
    }
};

// Mount System Plugin
class MountSystemPlugin : public GameSystemPlugin {
private:
    TAController* controller = nullptr;
    GameContext* context = nullptr;
    TANode* rootNode = nullptr;

    // System-specific data
    json mountConfig;

public:
    static constexpr const char* SystemName = "MountSystem";

    std::string getSystemName() const override { return SystemName; }
    std::string getDescription() const override { return "System for mounts, riding, and animal companions."; }
    TANode* getRootNode() const override { return rootNode; }

    void initialize(TAController* ctrl, GameContext* ctx) override
    {
        controller = ctrl;
        context = ctx;

        // Load configuration
        loadConfig();

        // Create root node
        rootNode = controller->createNode<TANode>("mount_system");

        // Register with TAController
        controller->setSystemRoot("MountSystem", rootNode);
    }

    void shutdown() override
    {
        // Cleanup if needed
    }

    void update(float deltaTime) override
    {
        // Check if player has a mount
        if (context->playerStats.skills.find("has_mount") != context->playerStats.skills.end()) {
            // Mount stats
            int mountSpeed = context->playerStats.skills["mount_speed"];
            int mountStamina = context->playerStats.skills["mount_stamina"];
            int mountHealth = context->playerStats.skills["mount_health"];

            // Mount gets tired during travel
            if (context->playerStats.skills.find("traveling") != context->playerStats.skills.end()) {
                mountStamina = std::max(0, mountStamina - 1);
                context->playerStats.skills["mount_stamina"] = mountStamina;

                // Speed bonus from mount, reduced when tired
                double staminaFactor = std::max(0.5, mountStamina / 100.0);
                context->playerStats.skills["travel_speed"] += static_cast<int>(mountSpeed * staminaFactor);

                // Special movement abilities
                if (context->worldState.getCurrentRegion() == "mountains" && mountStamina > 50) {
                    context->playerStats.skills["can_climb"] = 1;
                }

                if (mountStamina > 75) {
                    context->playerStats.skills["can_jump_ravines"] = 1;
                }

                // Dispatch mount stamina event if it's getting low
                if (mountStamina < 30) {
                    json eventData = {
                        { "mountStamina", mountStamina },
                        { "staminaState", "low" }
                    };
                    context->systemManager->dispatchEvent("mount.lowStamina", eventData);
                }
            }

            // Mount care - feeding
            if (context->playerStats.skills.find("fed_mount") != context->playerStats.skills.end()) {
                int oldStamina = mountStamina;
                mountStamina = std::min(100, mountStamina + 20);
                context->playerStats.skills["mount_stamina"] = mountStamina;
                context->playerStats.skills.erase("fed_mount");

                // Dispatch mount fed event
                json eventData = {
                    { "staminaGained", mountStamina - oldStamina },
                    { "newStamina", mountStamina }
                };
                context->systemManager->dispatchEvent("mount.fed", eventData);
            }

            // Mount training
            if (context->playerStats.skills.find("training_mount") != context->playerStats.skills.end()) {
                int trainingType = context->playerStats.skills["training_type"];
                if (trainingType == 0) {
                    // Speed training
                    context->playerStats.skills["mount_speed"] += 1;

                    // Dispatch training event
                    json eventData = {
                        { "trainingType", "speed" },
                        { "newSpeed", context->playerStats.skills["mount_speed"] }
                    };
                    context->systemManager->dispatchEvent("mount.trained", eventData);
                } else if (trainingType == 1) {
                    // Stamina training
                    context->playerStats.skills["mount_stamina_max"] += 2;

                    // Dispatch training event
                    json eventData = {
                        { "trainingType", "stamina" },
                        { "newStaminaMax", context->playerStats.skills["mount_stamina_max"] }
                    };
                    context->systemManager->dispatchEvent("mount.trained", eventData);
                } else if (trainingType == 2) {
                    // Combat training
                    context->playerStats.skills["mount_combat"] += 1;

                    // Dispatch training event
                    json eventData = {
                        { "trainingType", "combat" },
                        { "newCombatSkill", context->playerStats.skills["mount_combat"] }
                    };
                    context->systemManager->dispatchEvent("mount.trained", eventData);
                }
                context->playerStats.skills.erase("training_mount");
            }

            // Weather effects on mount
            std::string currentWeather = context->worldState.getLocationState("current_weather");
            if (currentWeather == "stormy" || currentWeather == "snowy") {
                // Mount moves slower in bad weather
                context->playerStats.skills["travel_speed"] = std::max(1, context->playerStats.skills["travel_speed"] - 2);
            }
        }
    }

    json saveState() const override
    {
        json state;

        // Save mount data
        if (context->playerStats.skills.find("has_mount") != context->playerStats.skills.end()) {
            state["has_mount"] = true;
            state["mount_speed"] = context->playerStats.skills["mount_speed"];
            state["mount_stamina"] = context->playerStats.skills["mount_stamina"];
            state["mount_health"] = context->playerStats.skills["mount_health"];
            state["mount_stamina_max"] = context->playerStats.skills["mount_stamina_max"];
            state["mount_combat"] = context->playerStats.skills["mount_combat"];

            if (context->playerStats.skills.find("mount_type") != context->playerStats.skills.end()) {
                state["mount_type"] = context->playerStats.skills["mount_type"];
            }
        } else {
            state["has_mount"] = false;
        }

        return state;
    }

    bool loadState(const json& data) override
    {
        try {
            if (data.contains("has_mount") && data["has_mount"].get<bool>()) {
                context->playerStats.skills["has_mount"] = 1;
                context->playerStats.skills["mount_speed"] = data["mount_speed"];
                context->playerStats.skills["mount_stamina"] = data["mount_stamina"];
                context->playerStats.skills["mount_health"] = data["mount_health"];
                context->playerStats.skills["mount_stamina_max"] = data["mount_stamina_max"];
                context->playerStats.skills["mount_combat"] = data["mount_combat"];

                if (data.contains("mount_type")) {
                    context->playerStats.skills["mount_type"] = data["mount_type"];
                }
            }
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error loading mount state: " << e.what() << std::endl;
            return false;
        }
    }

    bool handleEvent(const std::string& eventType, const json& eventData) override
    {
        if (eventType == "player.action.feedMount") {
            context->playerStats.skills["fed_mount"] = 1;
            return true;
        } else if (eventType == "player.action.trainMount") {
            int trainingType = eventData["training_type"];
            context->playerStats.skills["training_mount"] = 1;
            context->playerStats.skills["training_type"] = trainingType;
            return true;
        } else if (eventType == "player.action.travel") {
            // Mark player as traveling for mount stamina calculations
            context->playerStats.skills["traveling"] = 1;
            return true;
        } else if (eventType == "travel.complete") {
            // Clear traveling flag
            context->playerStats.skills.erase("traveling");
            return true;
        } else if (eventType == "weather.changed") {
            // Weather can affect mount travel
            return true;
        }

        return false;
    }

    // Mount-specific methods
    bool acquireMount(int type, int quality)
    {
        // Initialize mount with given type and quality
        context->playerStats.skills["has_mount"] = 1;
        context->playerStats.skills["mount_type"] = type;

        // Basic stats based on quality (1-10)
        context->playerStats.skills["mount_speed"] = 5 + quality;
        context->playerStats.skills["mount_stamina"] = 50 + (quality * 5);
        context->playerStats.skills["mount_stamina_max"] = 50 + (quality * 5);
        context->playerStats.skills["mount_health"] = 50 + (quality * 5);
        context->playerStats.skills["mount_combat"] = quality / 2;

        // Dispatch mount acquired event
        json eventData = {
            { "mountType", type },
            { "quality", quality }
        };
        context->systemManager->dispatchEvent("mount.acquired", eventData);

        return true;
    }

    void feedMount()
    {
        context->playerStats.skills["fed_mount"] = 1;
    }

private:
    void loadConfig()
    {
        // Load from JSON file or use defaults
        try {
            std::ifstream file("data/mount_config.json");
            if (file.is_open()) {
                file >> mountConfig;
                file.close();
            }
        } catch (const std::exception& e) {
            std::cerr << "Mount config loading error: " << e.what() << std::endl;
            // Continue with default values
        }
    }
};

// Relationship System Plugin
class RelationshipSystemPlugin : public GameSystemPlugin {
private:
    TAController* controller = nullptr;
    GameContext* context = nullptr;
    TANode* rootNode = nullptr;

    // System-specific data
    json relationshipConfig;
    std::vector<std::string> npcs = { "merchant", "guard", "innkeeper", "mage" };

public:
    static constexpr const char* SystemName = "RelationshipSystem";

    std::string getSystemName() const override { return SystemName; }
    std::string getDescription() const override { return "System for NPC relationships and social interactions."; }
    TANode* getRootNode() const override { return rootNode; }

    void initialize(TAController* ctrl, GameContext* ctx) override
    {
        controller = ctrl;
        context = ctx;

        // Load configuration
        loadConfig();

        // Create root node
        rootNode = controller->createNode<TANode>("relationship_system");

        // Register with TAController
        controller->setSystemRoot("RelationshipSystem", rootNode);

        // Initialize NPC relations
        for (const auto& npc : npcs) {
            context->relationshipContext.npcRelationships[npc] = 0;
        }
    }

    void shutdown() override
    {
        // Cleanup if needed
    }

    void update(float deltaTime) override
    {
        // Process NPC relationships
        for (const auto& npc : npcs) {
            int relation = context->relationshipContext.npcRelationships[npc];

            // Gift effects
            if (context->playerStats.skills.find("gift_to_" + npc) != context->playerStats.skills.end()) {
                int giftValue = context->playerStats.skills["gift_to_" + npc];

                // Different NPCs prefer different gifts
                double giftMultiplier = 1.0;
                if (npc == "merchant" && context->playerStats.skills.find("gift_type") != context->playerStats.skills.end()
                    && context->playerStats.skills["gift_type"] == 0) {
                    // Merchant prefers valuable items
                    giftMultiplier = 1.5;
                } else if (npc == "mage" && context->playerStats.skills.find("gift_type") != context->playerStats.skills.end()
                    && context->playerStats.skills["gift_type"] == 1) {
                    // Mage prefers magical items
                    giftMultiplier = 1.5;
                }

                int relationshipIncrease = static_cast<int>(giftValue * giftMultiplier / 10);
                int oldValue = relation;
                context->relationshipContext.npcRelationships[npc] = relation + relationshipIncrease;

                // Dispatch relationship change event
                json eventData = {
                    { "npc", npc },
                    { "oldValue", oldValue },
                    { "newValue", relation + relationshipIncrease },
                    { "reason", "gift" }
                };
                context->systemManager->dispatchEvent("relationship.changed", eventData);

                context->playerStats.skills.erase("gift_to_" + npc);
                if (context->playerStats.skills.find("gift_type") != context->playerStats.skills.end()) {
                    context->playerStats.skills.erase("gift_type");
                }
            }

            // Conversation effects
            if (context->playerStats.skills.find("talked_to_" + npc) != context->playerStats.skills.end()) {
                // Success depends on charisma and previous relationship
                double successChance = 0.5 + (context->playerStats.charisma / 100.0)
                    + (relation / 200.0);

                if (utils::randomChance(successChance)) {
                    int oldValue = relation;
                    context->relationshipContext.npcRelationships[npc] = relation + 1;

                    // Dispatch relationship change event
                    json eventData = {
                        { "npc", npc },
                        { "oldValue", oldValue },
                        { "newValue", relation + 1 },
                        { "reason", "conversation" }
                    };
                    context->systemManager->dispatchEvent("relationship.changed", eventData);
                }
                context->playerStats.skills.erase("talked_to_" + npc);
            }

            // Companion system
            if (context->playerStats.skills.find("companion_" + npc) != context->playerStats.skills.end()) {
                // Loyalty based on relationship
                int loyalty = relation / 10;

                // Companions may leave if loyalty is too low
                if (loyalty < 3 && utils::randomChance(0.1)) {
                    context->playerStats.skills.erase("companion_" + npc);

                    // Dispatch companion left event
                    json eventData = {
                        { "npc", npc },
                        { "reason", "low_loyalty" }
                    };
                    context->systemManager->dispatchEvent("relationship.companionLeft", eventData);
                }

                // Companions provide bonuses
                if (npc == "merchant")
                    context->economyContext.economyFactors["shop_discount"] = 0.1;
                else if (npc == "guard")
                    context->playerStats.improveSkill("combat", 2);
                else if (npc == "mage")
                    context->playerStats.improveSkill("magic", 2);
            }

            // NPCs have schedules
            int hourOfDay = context->worldState.getTimeOfDay();
            if (npc == "merchant" && (hourOfDay < 8 || hourOfDay > 18)) {
                context->playerStats.skills["merchant_available"] = 0;
            } else if (npc == "merchant") {
                context->playerStats.skills["merchant_available"] = 1;
            }
        }
    }

    json saveState() const override
    {
        json state;
        state["npcRelationships"] = context->relationshipContext.npcRelationships;

        // Save companion data
        json companions = json::array();
        for (auto& [key, value] : context->playerStats.skills) {
            if (key.find("companion_") == 0) {
                companions.push_back({ { "npc", key.substr(10) },
                    { "value", value } });
            }
        }
        state["companions"] = companions;

        return state;
    }

    bool loadState(const json& data) override
    {
        try {
            if (data.contains("npcRelationships")) {
                context->relationshipContext.npcRelationships = data["npcRelationships"].get<std::map<std::string, int>>();
            }

            // Load companion data
            if (data.contains("companions") && data["companions"].is_array()) {
                for (const auto& comp : data["companions"]) {
                    std::string npc = comp["npc"];
                    int value = comp["value"];
                    context->playerStats.skills["companion_" + npc] = value;
                }
            }

            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error loading relationship state: " << e.what() << std::endl;
            return false;
        }
    }

    bool handleEvent(const std::string& eventType, const json& eventData) override
    {
        if (eventType == "player.action.giveGift") {
            std::string npc = eventData["npc"];
            int value = eventData["value"];
            int giftType = eventData.contains("gift_type") ? eventData["gift_type"].get<int>() : -1;

            context->playerStats.skills["gift_to_" + npc] = value;
            if (giftType >= 0) {
                context->playerStats.skills["gift_type"] = giftType;
            }

            return true;
        } else if (eventType == "player.action.talk") {
            std::string npc = eventData["npc"];
            context->playerStats.skills["talked_to_" + npc] = 1;
            return true;
        } else if (eventType == "player.action.recruitCompanion") {
            std::string npc = eventData["npc"];

            // Check relationship threshold
            int relation = context->relationshipContext.npcRelationships[npc];
            if (relation >= 20) { // Minimum relationship to recruit
                context->playerStats.skills["companion_" + npc] = 1;

                // Dispatch companion joined event
                json eventData = {
                    { "npc", npc }
                };
                context->systemManager->dispatchEvent("relationship.companionJoined", eventData);

                return true;
            }
        }

        return false;
    }

    // Relationship-specific methods
    int getNPCRelationship(const std::string& npc)
    {
        auto it = context->relationshipContext.npcRelationships.find(npc);
        if (it != context->relationshipContext.npcRelationships.end()) {
            return it->second;
        }
        return 0;
    }

    void changeNPCRelationship(const std::string& npc, int amount)
    {
        auto it = context->relationshipContext.npcRelationships.find(npc);
        if (it != context->relationshipContext.npcRelationships.end()) {
            int oldValue = it->second;
            it->second += amount;

            // Limit to reasonable values
            it->second = std::max(-100, std::min(100, it->second));

            // Dispatch relationship change event
            json eventData = {
                { "npc", npc },
                { "oldValue", oldValue },
                { "newValue", it->second },
                { "reason", "direct_change" }
            };
            context->systemManager->dispatchEvent("relationship.changed", eventData);
        }
    }

private:
    void loadConfig()
    {
        // Load from JSON file or use defaults
        try {
            std::ifstream file("data/relationship_config.json");
            if (file.is_open()) {
                file >> relationshipConfig;
                file.close();

                // Load NPCs list from config if available
                if (relationshipConfig.contains("npcs") && relationshipConfig["npcs"].is_array()) {
                    npcs.clear();
                    for (const auto& npc : relationshipConfig["npcs"]) {
                        npcs.push_back(npc);
                    }
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Relationship config loading error: " << e.what() << std::endl;
            // Continue with default values
        }
    }
};

// Religion System Plugin
class ReligionSystemPlugin : public GameSystemPlugin {
private:
    TAController* controller = nullptr;
    GameContext* context = nullptr;
    TANode* rootNode = nullptr;

    // System-specific data
    json religionConfig;
    std::vector<std::string> deities = { "sun_god", "moon_goddess", "war_god", "harvest_goddess" };

public:
    static constexpr const char* SystemName = "ReligionSystem";

    std::string getSystemName() const override { return SystemName; }
    std::string getDescription() const override { return "System for deities, worship, and divine favor."; }
    TANode* getRootNode() const override { return rootNode; }

    void initialize(TAController* ctrl, GameContext* ctx) override
    {
        controller = ctrl;
        context = ctx;

        // Load configuration
        loadConfig();

        // Create root node
        rootNode = controller->createNode<TANode>("religion_system");

        // Register with TAController
        controller->setSystemRoot("ReligionSystem", rootNode);

        // Initialize deity favor
        for (const auto& deity : deities) {
            context->playerStats.skills["deity_" + deity] = 0;
        }
    }

    void shutdown() override
    {
        // Cleanup if needed
    }

    void update(float deltaTime) override
    {
        // Process deity favor
        for (const auto& deity : deities) {
            int favor = context->playerStats.skills["deity_" + deity];

            // Prayer effects
            if (context->playerStats.skills.find("prayed_to_" + deity) != context->playerStats.skills.end()) {
                // Prayer increases favor
                int oldFavor = favor;
                context->playerStats.skills["deity_" + deity] = favor + 1;
                context->playerStats.skills.erase("prayed_to_" + deity);

                // Dispatch deity favor changed event
                json eventData = {
                    { "deity", deity },
                    { "oldFavor", oldFavor },
                    { "newFavor", favor + 1 },
                    { "reason", "prayer" }
                };
                context->systemManager->dispatchEvent("religion.favorChanged", eventData);

                // Prayer benefits
                if (deity == "sun_god") {
                    // Sun god provides combat bonuses
                    context->playerStats.skills["combat"] += favor / 10;
                } else if (deity == "moon_goddess") {
                    // Moon goddess provides magic bonuses
                    context->playerStats.skills["magic"] += favor / 10;
                } else if (deity == "war_god") {
                    // War god provides health bonuses
                    context->healthContext.playerHealth.maxHealth += favor / 20;
                } else if (deity == "harvest_goddess") {
                    // Harvest goddess provides food bonuses
                    context->playerStats.skills["food"] += favor / 5;
                }
            }

            // Temple quests
            if (context->playerStats.skills.find("temple_quest_" + deity) != context->playerStats.skills.end()) {
                int questProgress = context->playerStats.skills["temple_quest_" + deity];

                if (questProgress >= 100) {
                    // Quest completed
                    int oldFavor = favor;
                    context->playerStats.skills["deity_" + deity] = favor + 10;
                    context->playerStats.skills.erase("temple_quest_" + deity);

                    // Dispatch deity favor changed event
                    json eventData = {
                        { "deity", deity },
                        { "oldFavor", oldFavor },
                        { "newFavor", favor + 10 },
                        { "reason", "quest_complete" }
                    };
                    context->systemManager->dispatchEvent("religion.favorChanged", eventData);

                    // Special blessings
                    if (deity == "sun_god") {
                        context->playerStats.skills["blessed_weapon"] = 1;
                    } else if (deity == "moon_goddess") {
                        context->playerStats.skills["blessed_magic"] = 1;
                    }
                }
            }

            // Divine intervention
            auto& health = context->healthContext.playerHealth;
            if (favor > 50 && health.currentHealth < 10 && utils::randomChance(favor / 200.0)) {
                // Deity intervenes to save player
                float oldHealth = health.currentHealth;
                health.currentHealth = health.maxHealth / 2;

                int oldFavor = favor;
                context->playerStats.skills["deity_" + deity] = favor - 10; // Costs favor

                // Dispatch divine intervention event
                json eventData = {
                    { "deity", deity },
                    { "oldHealth", oldHealth },
                    { "newHealth", health.currentHealth },
                    { "favorCost", 10 }
                };
                context->systemManager->dispatchEvent("religion.divineIntervention", eventData);
            }

            // Religious conflicts
            for (const auto& otherDeity : deities) {
                if (deity != otherDeity) {
                    // Rival deities
                    if ((deity == "sun_god" && otherDeity == "moon_goddess") || (deity == "moon_goddess" && otherDeity == "sun_god")) {

                        // Helping one deity displeases the other
                        if (context->playerStats.skills.find("helped_" + deity) != context->playerStats.skills.end()) {
                            int otherFavor = context->playerStats.skills["deity_" + otherDeity];
                            int oldFavor = otherFavor;
                            context->playerStats.skills["deity_" + otherDeity] = otherFavor - 2;
                            context->playerStats.skills.erase("helped_" + deity);

                            // Dispatch deity favor changed event
                            json eventData = {
                                { "deity", otherDeity },
                                { "oldFavor", oldFavor },
                                { "newFavor", otherFavor - 2 },
                                { "reason", "rival_deity_conflict" }
                            };
                            context->systemManager->dispatchEvent("religion.favorChanged", eventData);
                        }
                    }
                }
            }

            // Sacred days
            int dayOfYear = context->worldState.daysPassed % 365;
            if (deity == "sun_god" && dayOfYear == 172) { // Summer solstice
                // Sun god festival
                int oldFavor = favor;
                context->playerStats.skills["deity_" + deity] = favor + 5;
                context->playerStats.skills["sun_blessing"] = 1; // Temporary blessing

                // Dispatch deity favor changed event
                json eventData = {
                    { "deity", deity },
                    { "oldFavor", oldFavor },
                    { "newFavor", favor + 5 },
                    { "reason", "sacred_day" }
                };
                context->systemManager->dispatchEvent("religion.favorChanged", eventData);
            }
        }
    }

    json saveState() const override
    {
        json state;

        // Save deity favor
        json deityFavor = json::object();
        for (const auto& deity : deities) {
            deityFavor[deity] = context->playerStats.skills["deity_" + deity];
        }
        state["deityFavor"] = deityFavor;

        // Save temple quests
        json templeQuests = json::object();
        for (auto& [key, value] : context->playerStats.skills) {
            if (key.find("temple_quest_") == 0) {
                templeQuests[key.substr(13)] = value;
            }
        }
        state["templeQuests"] = templeQuests;

        // Save blessings
        json blessings = json::array();
        for (auto& [key, value] : context->playerStats.skills) {
            if (key.find("blessed_") == 0) {
                blessings.push_back(key.substr(8));
            }
        }
        state["blessings"] = blessings;

        return state;
    }

    bool loadState(const json& data) override
    {
        try {
            // Load deity favor
            if (data.contains("deityFavor") && data["deityFavor"].is_object()) {
                for (auto& [deity, favor] : data["deityFavor"].items()) {
                    context->playerStats.skills["deity_" + deity] = favor;
                }
            }

            // Load temple quests
            if (data.contains("templeQuests") && data["templeQuests"].is_object()) {
                for (auto& [deity, progress] : data["templeQuests"].items()) {
                    context->playerStats.skills["temple_quest_" + deity] = progress;
                }
            }

            // Load blessings
            if (data.contains("blessings") && data["blessings"].is_array()) {
                for (const auto& blessing : data["blessings"]) {
                    context->playerStats.skills["blessed_" + blessing.get<std::string>()] = 1;
                }
            }

            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error loading religion state: " << e.what() << std::endl;
            return false;
        }
    }

    bool handleEvent(const std::string& eventType, const json& eventData) override
    {
        if (eventType == "player.action.pray") {
            std::string deity = eventData["deity"];

            // Mark this deity as prayed to
            context->playerStats.skills["prayed_to_" + deity] = 1;

            return true;
        } else if (eventType == "player.action.templeQuest") {
            std::string deity = eventData["deity"];
            int progress = eventData["progress"];

            // Update temple quest progress
            context->playerStats.skills["temple_quest_" + deity] = progress;

            return true;
        } else if (eventType == "player.action.helpDeity") {
            std::string deity = eventData["deity"];

            // Mark this deity as helped
            context->playerStats.skills["helped_" + deity] = 1;

            return true;
        }

        return false;
    }

    // Religion-specific methods
    int getDeityFavor(const std::string& deity)
    {
        auto it = context->playerStats.skills.find("deity_" + deity);
        if (it != context->playerStats.skills.end()) {
            return it->second;
        }
        return 0;
    }

    void changeDeityFavor(const std::string& deity, int amount)
    {
        auto it = context->playerStats.skills.find("deity_" + deity);
        if (it != context->playerStats.skills.end()) {
            int oldFavor = it->second;
            it->second += amount;

            // Dispatch deity favor changed event
            json eventData = {
                { "deity", deity },
                { "oldFavor", oldFavor },
                { "newFavor", it->second },
                { "reason", "direct_change" }
            };
            context->systemManager->dispatchEvent("religion.favorChanged", eventData);
        }
    }

private:
    void loadConfig()
    {
        // Load from JSON file or use defaults
        try {
            std::ifstream file("data/religion_config.json");
            if (file.is_open()) {
                file >> religionConfig;
                file.close();

                // Load deities list from config if available
                if (religionConfig.contains("deities") && religionConfig["deities"].is_array()) {
                    deities.clear();
                    for (const auto& deity : religionConfig["deities"]) {
                        deities.push_back(deity);
                    }
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Religion config loading error: " << e.what() << std::endl;
            // Continue with default values
        }
    }
};

// Spell Crafting System Plugin
class SpellCraftingSystemPlugin : public GameSystemPlugin {
private:
    TAController* controller = nullptr;
    GameContext* context = nullptr;
    TANode* rootNode = nullptr;

    // System-specific data
    json spellConfig;

public:
    static constexpr const char* SystemName = "SpellCraftingSystem";

    std::string getSystemName() const override { return SystemName; }
    std::string getDescription() const override { return "System for spell creation, magical research, and casting."; }
    TANode* getRootNode() const override { return rootNode; }

    void initialize(TAController* ctrl, GameContext* ctx) override
    {
        controller = ctrl;
        context = ctx;

        // Load configuration
        loadConfig();

        // Create root node
        rootNode = controller->createNode<TANode>("spellcrafting_system");

        // Register with TAController
        controller->setSystemRoot("SpellCraftingSystem", rootNode);
    }

    void shutdown() override
    {
        // Cleanup if needed
    }

    void update(float deltaTime) override
    {
        // Process spell crafting if player is working on a spell
        if (context->playerStats.skills.find("crafting_spell") != context->playerStats.skills.end()) {
            int spellType = context->playerStats.skills["spell_type"];
            int spellPower = context->playerStats.skills["spell_power"];
            int spellComplexity = context->playerStats.skills["spell_complexity"];
            int spellProgress = context->playerStats.skills["spell_progress"];

            // Components increase spell progress
            if (context->playerStats.skills.find("used_component") != context->playerStats.skills.end()) {
                int componentQuality = context->playerStats.skills["component_quality"];
                spellProgress += componentQuality;
                context->playerStats.skills["spell_progress"] = spellProgress;
                context->playerStats.skills.erase("used_component");

                // Dispatch component used event
                json eventData = {
                    { "componentQuality", componentQuality },
                    { "newProgress", spellProgress }
                };
                context->systemManager->dispatchEvent("spellcrafting.componentUsed", eventData);
            }

            // Magic skill affects success rate
            double successChance = 0.4 + (context->playerStats.skills["magic"] / 100.0) - (spellComplexity / 50.0);

            // Spell completion
            if (spellProgress >= 100) {
                if (utils::randomChance(successChance)) {
                    // Spell successfully created
                    std::string spellName;
                    if (spellType == 0)
                        spellName = "fireball";
                    else if (spellType == 1)
                        spellName = "healing";
                    else if (spellType == 2)
                        spellName = "shield";

                    context->playerStats.skills["spell_" + spellName] = spellPower;

                    // Dispatch spell completed event
                    json eventData = {
                        { "spellName", spellName },
                        { "spellPower", spellPower },
                        { "success", true }
                    };
                    context->systemManager->dispatchEvent("spellcrafting.spellCompleted", eventData);
                } else {
                    // Spell failure
                    int backfireDamage = spellComplexity / 5;
                    context->healthContext.playerHealth.takeDamage(backfireDamage);

                    // Dispatch spell failure event
                    json eventData = {
                        { "spellType", spellType },
                        { "backfireDamage", backfireDamage },
                        { "success", false }
                    };
                    context->systemManager->dispatchEvent("spellcrafting.spellFailed", eventData);
                }

                // Reset spell crafting
                context->playerStats.skills.erase("crafting_spell");
                context->playerStats.skills.erase("spell_type");
                context->playerStats.skills.erase("spell_power");
                context->playerStats.skills.erase("spell_complexity");
                context->playerStats.skills.erase("spell_progress");
            }

            // Magical research
            if (context->playerStats.skills.find("researching_magic") != context->playerStats.skills.end()) {
                int researchType = context->playerStats.skills["research_type"];
                int researchProgress = context->playerStats.skills["research_progress"];

                // Research progresses over time
                researchProgress += 1 + (context->playerStats.intelligence / 20);
                context->playerStats.skills["research_progress"] = researchProgress;

                if (researchProgress >= 100) {
                    // Research completed
                    if (researchType == 0) {
                        // Fire magic research
                        context->playerStats.skills["fire_magic"] += 5;

                        // Dispatch research completed event
                        json eventData = {
                            { "researchType", "fire" },
                            { "skillGain", 5 }
                        };
                        context->systemManager->dispatchEvent("spellcrafting.researchCompleted", eventData);
                    } else if (researchType == 1) {
                        // Healing magic research
                        context->playerStats.skills["healing_magic"] += 5;

                        // Dispatch research completed event
                        json eventData = {
                            { "researchType", "healing" },
                            { "skillGain", 5 }
                        };
                        context->systemManager->dispatchEvent("spellcrafting.researchCompleted", eventData);
                    } else if (researchType == 2) {
                        // Protection magic research
                        context->playerStats.skills["protection_magic"] += 5;

                        // Dispatch research completed event
                        json eventData = {
                            { "researchType", "protection" },
                            { "skillGain", 5 }
                        };
                        context->systemManager->dispatchEvent("spellcrafting.researchCompleted", eventData);
                    }

                    // Reset research
                    context->playerStats.skills.erase("researching_magic");
                    context->playerStats.skills.erase("research_type");
                    context->playerStats.skills.erase("research_progress");
                }
            }
        }
    }

    json saveState() const override
    {
        json state;

        // Save spell crafting progress
        if (context->playerStats.skills.find("crafting_spell") != context->playerStats.skills.end()) {
            state["craftingSpell"] = true;
            state["spellType"] = context->playerStats.skills["spell_type"];
            state["spellPower"] = context->playerStats.skills["spell_power"];
            state["spellComplexity"] = context->playerStats.skills["spell_complexity"];
            state["spellProgress"] = context->playerStats.skills["spell_progress"];
        } else {
            state["craftingSpell"] = false;
        }

        // Save research progress
        if (context->playerStats.skills.find("researching_magic") != context->playerStats.skills.end()) {
            state["researchingMagic"] = true;
            state["researchType"] = context->playerStats.skills["research_type"];
            state["researchProgress"] = context->playerStats.skills["research_progress"];
        } else {
            state["researchingMagic"] = false;
        }

        // Save known spells
        json knownSpells = json::object();
        for (auto& [key, value] : context->playerStats.skills) {
            if (key.find("spell_") == 0) {
                knownSpells[key.substr(6)] = value;
            }
        }
        state["knownSpells"] = knownSpells;

        // Save magic skills
        json magicSkills = json::object();
        for (auto& [key, value] : context->playerStats.skills) {
            if (key.find("_magic") != std::string::npos) {
                magicSkills[key] = value;
            }
        }
        state["magicSkills"] = magicSkills;

        return state;
    }

    bool loadState(const json& data) override
    {
        try {
            // Load spell crafting progress
            if (data.contains("craftingSpell") && data["craftingSpell"].get<bool>()) {
                context->playerStats.skills["crafting_spell"] = 1;
                context->playerStats.skills["spell_type"] = data["spellType"];
                context->playerStats.skills["spell_power"] = data["spellPower"];
                context->playerStats.skills["spell_complexity"] = data["spellComplexity"];
                context->playerStats.skills["spell_progress"] = data["spellProgress"];
            }

            // Load research progress
            if (data.contains("researchingMagic") && data["researchingMagic"].get<bool>()) {
                context->playerStats.skills["researching_magic"] = 1;
                context->playerStats.skills["research_type"] = data["researchType"];
                context->playerStats.skills["research_progress"] = data["researchProgress"];
            }

            // Load known spells
            if (data.contains("knownSpells") && data["knownSpells"].is_object()) {
                for (auto& [spell, power] : data["knownSpells"].items()) {
                    context->playerStats.skills["spell_" + spell] = power;
                }
            }

            // Load magic skills
            if (data.contains("magicSkills") && data["magicSkills"].is_object()) {
                for (auto& [skill, level] : data["magicSkills"].items()) {
                    context->playerStats.skills[skill] = level;
                }
            }

            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error loading spell crafting state: " << e.what() << std::endl;
            return false;
        }
    }

    bool handleEvent(const std::string& eventType, const json& eventData) override
    {
        if (eventType == "player.action.craftSpell") {
            int spellType = eventData["type"];
            int spellPower = eventData["power"];
            int spellComplexity = eventData["complexity"];

            // Start spell crafting
            context->playerStats.skills["crafting_spell"] = 1;
            context->playerStats.skills["spell_type"] = spellType;
            context->playerStats.skills["spell_power"] = spellPower;
            context->playerStats.skills["spell_complexity"] = spellComplexity;
            context->playerStats.skills["spell_progress"] = 0;

            return true;
        } else if (eventType == "player.action.useComponent") {
            int componentQuality = eventData["quality"];

            // Use component in spell crafting
            context->playerStats.skills["used_component"] = 1;
            context->playerStats.skills["component_quality"] = componentQuality;

            return true;
        } else if (eventType == "player.action.researchMagic") {
            int researchType = eventData["type"];

            // Start magical research
            context->playerStats.skills["researching_magic"] = 1;
            context->playerStats.skills["research_type"] = researchType;
            context->playerStats.skills["research_progress"] = 0;

            return true;
        } else if (eventType == "religion.favorChanged") {
            // Deity favor can affect spell power
            std::string deity = eventData["deity"];

            if (deity == "moon_goddess") {
                // Moon goddess improves magic
                context->playerStats.skills["magic"] += 1;
            }

            return true;
        }

        return false;
    }

    // Spell-specific methods
    bool castSpell(const std::string& spellName, int targetType, const std::string& targetId)
    {
        auto it = context->playerStats.skills.find("spell_" + spellName);
        if (it == context->playerStats.skills.end()) {
            return false; // Don't have this spell
        }

        int spellPower = it->second;

        // Different effects based on spell type
        if (spellName == "fireball") {
            // Attack spell
            int damage = spellPower + (context->playerStats.skills["fire_magic"] / 2);

            // Dispatch spell cast event
            json eventData = {
                { "spellName", spellName },
                { "spellPower", spellPower },
                { "damage", damage },
                { "targetType", targetType },
                { "targetId", targetId }
            };
            context->systemManager->dispatchEvent("spellcrafting.spellCast", eventData);

            return true;
        } else if (spellName == "healing") {
            // Healing spell
            int healAmount = spellPower + (context->playerStats.skills["healing_magic"] / 2);
            context->healthContext.playerHealth.heal(healAmount);

            // Dispatch spell cast event
            json eventData = {
                { "spellName", spellName },
                { "spellPower", spellPower },
                { "healAmount", healAmount }
            };
            context->systemManager->dispatchEvent("spellcrafting.spellCast", eventData);

            return true;
        } else if (spellName == "shield") {
            // Protection spell
            int protection = spellPower + (context->playerStats.skills["protection_magic"] / 2);
            context->playerStats.skills["magical_protection"] = protection;

            // Dispatch spell cast event
            json eventData = {
                { "spellName", spellName },
                { "spellPower", spellPower },
                { "protection", protection }
            };
            context->systemManager->dispatchEvent("spellcrafting.spellCast", eventData);

            return true;
        }

        return false;
    }

private:
    void loadConfig()
    {
        // Load from JSON file or use defaults
        try {
            std::ifstream file("data/spellcrafting_config.json");
            if (file.is_open()) {
                file >> spellConfig;
                file.close();
            }
        } catch (const std::exception& e) {
            std::cerr << "Spell crafting config loading error: " << e.what() << std::endl;
            // Continue with default values
        }
    }
};

// Main game system using plugin architecture
class GameSystem {
public:
    GameSystem()
    {
        // Create controller and game context
        controller_ = std::make_shared<TAController>();
        gameContext_ = std::make_shared<GameContext>();

        // Initialize system manager
        systemManager_ = std::make_shared<GameSystemManager>(controller_.get(), gameContext_.get());
        gameContext_->systemManager = systemManager_.get();

        // Register all plugins
        systemManager_->registerPlugin(createPlugin<WeatherSystemPlugin>());
        systemManager_->registerPlugin(createPlugin<CrimeSystemPlugin>());
        systemManager_->registerPlugin(createPlugin<HealthSystemPlugin>());
        systemManager_->registerPlugin(createPlugin<EconomySystemPlugin>());
        systemManager_->registerPlugin(createPlugin<FactionSystemPlugin>());
        systemManager_->registerPlugin(createPlugin<MountSystemPlugin>());
        systemManager_->registerPlugin(createPlugin<RelationshipSystemPlugin>());
        systemManager_->registerPlugin(createPlugin<ReligionSystemPlugin>());
        systemManager_->registerPlugin(createPlugin<SpellCraftingSystemPlugin>());

        // Initialize all systems
        systemManager_->initializeAll();

        // Initialize world state
        gameContext_->worldState.setLocationState("current_region", "city");
        gameContext_->healthContext.playerHealth.currentHealth = 100;
        gameContext_->healthContext.playerHealth.maxHealth = 100;
        gameContext_->economyContext.playerGold = 50;
        gameContext_->playerStats.charisma = 10;
        gameContext_->playerStats.intelligence = 10;
        gameContext_->playerStats.skills["magic"] = 5;
        gameContext_->playerStats.skills["combat"] = 5;
    }

    // Advance the game state by one time unit
    void update()
    {
        systemManager_->updateAll(1.0f / 60.0f);
        gameContext_->worldState.daysPassed++;
    }

    // Process a player action
    void processAction(const std::string& action, const std::map<std::string, int>& params)
    {
        // Convert to event format
        json eventData = json::object();
        for (const auto& [key, value] : params) {
            eventData[key] = value;
        }

        // Dispatch to appropriate system
        if (action == "commit_crime") {
            systemManager_->dispatchEvent("player.action.crime", eventData);
        } else if (action == "pray") {
            std::string deity = "sun_god";
            if (params.find("deity") != params.end()) {
                int deityId = params.at("deity");
                if (deityId == 0)
                    deity = "sun_god";
                else if (deityId == 1)
                    deity = "moon_goddess";
                else if (deityId == 2)
                    deity = "war_god";
                else if (deityId == 3)
                    deity = "harvest_goddess";
            }
            eventData["deity"] = deity;
            systemManager_->dispatchEvent("player.action.pray", eventData);
        } else if (action == "craft_spell") {
            systemManager_->dispatchEvent("player.action.craftSpell", eventData);
        } else if (action == "feed_mount") {
            systemManager_->dispatchEvent("player.action.feedMount", eventData);
        } else if (action == "give_gift") {
            std::string npc = "merchant";
            if (params.find("npc") != params.end()) {
                int npcId = params.at("npc");
                if (npcId == 0)
                    npc = "merchant";
                else if (npcId == 1)
                    npc = "guard";
                else if (npcId == 2)
                    npc = "innkeeper";
                else if (npcId == 3)
                    npc = "mage";
            }
            eventData["npc"] = npc;
            systemManager_->dispatchEvent("player.action.giveGift", eventData);
        } else if (action == "buy_property") {
            systemManager_->dispatchEvent("player.action.buyProperty", eventData);
        } else if (action == "invest") {
            std::string business = "shop";
            if (params.find("business") != params.end()) {
                int businessId = params.at("business");
                if (businessId == 0)
                    business = "shop";
                else if (businessId == 1)
                    business = "mine";
                else if (businessId == 2)
                    business = "farm";
            }
            eventData["business"] = business;
            systemManager_->dispatchEvent("player.action.invest", eventData);
        } else if (action == "travel") {
            std::string region = "city";
            if (params.find("region") != params.end()) {
                int regionId = params.at("region");
                if (regionId == 0)
                    region = "city";
                else if (regionId == 1)
                    region = "village";
                else if (regionId == 2)
                    region = "mountains";
                else if (regionId == 3)
                    region = "forest";
                else if (regionId == 4)
                    region = "desert";
                else if (regionId == 5)
                    region = "swamp";
                else if (regionId == 6)
                    region = "wilderness";
            }

            // Update world state directly
            gameContext_->worldState.setLocationState("current_region", region);

            // Dispatch travel events
            eventData["region"] = region;
            systemManager_->dispatchEvent("player.action.travel", eventData);
            systemManager_->dispatchEvent("region.changed", eventData);

            // Mark as traveling for mount system
            gameContext_->playerStats.skills["traveling"] = 1;
        }
    }

    // Get current game state information
    const GameContext& getState() const
    {
        return *gameContext_;
    }

    // Save game state
    bool saveGame(const std::string& filename)
    {
        return systemManager_->saveAllSystems(filename);
    }

    // Load game state
    bool loadGame(const std::string& filename)
    {
        return systemManager_->loadAllSystems(filename);
    }

private:
    std::shared_ptr<TAController> controller_;
    std::shared_ptr<GameContext> gameContext_;
    std::shared_ptr<GameSystemManager> systemManager_;
};

// Example usage
int main()
{
    GameSystem game;

    // Process some player actions
    game.processAction("pray", { { "deity", 0 } });
    game.update();

    game.processAction("commit_crime", { { "type", 0 }, { "value", 20 } });
    game.update();

    game.processAction("buy_property", { { "cost", 100 }, { "level", 1 } });
    game.update();

    // Travel to different region
    game.processAction("travel", { { "region", 2 } });
    game.update();

    // Craft a spell
    game.processAction("craft_spell", { { "type", 0 }, { "power", 5 }, { "complexity", 3 } });
    game.update();

    // Save the game
    game.saveGame("saves/save_game.json");

    // Load the game
    game.loadGame("saves/save_game.json");

    return 0;
}