// /oath/systems/health/HealthContext.hpp
#pragma once

#include "Disease.hpp"
#include "HealingMethod.hpp"
#include "HealthState.hpp"
#include "NutritionState.hpp"
#include <map>
#include <string>

// Add HealthState to the GameContext
struct HealthContext {
    HealthState playerHealth;
    NutritionState playerNutrition;
    std::map<std::string, Disease> knownDiseases;
    std::map<std::string, HealingMethod> healingMethods;
    std::map<std::string, float> regionDiseaseRisk; // Region name to disease risk multiplier
};