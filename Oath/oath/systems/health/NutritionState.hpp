// /oath/systems/health/NutritionState.hpp
#pragma once

#include <nlohmann/json.hpp>
#include <string>

enum class NutritionLevel {
    OVERFED,
    SATISFIED,
    NORMAL,
    HUNGRY,
    STARVING,
    CRITICAL
};

enum class HydrationLevel {
    OVERHYDRATED,
    HYDRATED,
    NORMAL,
    THIRSTY,
    DEHYDRATED,
    CRITICAL
};

// Class to represent player's nutrition and hydration state
class NutritionState {
public:
    float hunger; // 0-100 scale, 0 = critical, 100 = overfed
    float thirst; // 0-100 scale, 0 = critical, 100 = overhydrated
    float maxHunger; // Default 100
    float maxThirst; // Default 100

    float hungerRate; // How fast hunger decreases (per hour)
    float thirstRate; // How fast thirst decreases (per hour)

    // Thresholds for different levels
    const float criticalThreshold = 10.0f;
    const float lowThreshold = 25.0f;
    const float normalThreshold = 50.0f;
    const float highThreshold = 75.0f;

    NutritionState();

    // Initialize from JSON
    void initFromJson(const nlohmann::json& nutritionJson);

    // Core functions
    void update(float hoursPassed);
    void consumeFood(float nutritionValue);
    void consumeWater(float hydrationValue);

    // Status getters
    NutritionLevel getHungerLevel() const;
    HydrationLevel getThirstLevel() const;
    std::string getHungerLevelString() const;
    std::string getThirstLevelString() const;

    // Apply effects based on current levels
    void applyEffects(GameContext* context) const;
};