// /oath/systems/health/HealthState.cpp
#include "HealthState.hpp"
#include "DiseaseManager.hpp"
#include <algorithm>
#include <iostream>


HealthState::HealthState()
    : currentHealth(100.0f)
    , maxHealth(100.0f)
    , stamina(100.0f)
    , maxStamina(100.0f)
    , naturalHealRate(1.0f)
    , diseaseResistance(0.0f)
{
}

void HealthState::initFromJson(const nlohmann::json& healthJson)
{
    maxHealth = healthJson["maxHealth"];
    currentHealth = maxHealth;
    maxStamina = healthJson["maxStamina"];
    stamina = maxStamina;
    naturalHealRate = healthJson["naturalHealRate"];
    diseaseResistance = healthJson["diseaseResistance"];
}

void HealthState::takeDamage(float amount)
{
    currentHealth = std::max(0.0f, currentHealth - amount);
    if (currentHealth <= 0) {
        std::cout << "You have fallen unconscious due to your injuries!" << std::endl;
    }
}

void HealthState::heal(float amount)
{
    currentHealth = std::min(maxHealth, currentHealth + amount);
}

void HealthState::useStamina(float amount)
{
    stamina = std::max(0.0f, stamina - amount);
}

void HealthState::restoreStamina(float amount)
{
    stamina = std::min(maxStamina, stamina + amount);
}

void HealthState::contractDisease(const std::string& diseaseId, DiseaseManager* manager)
{
    if (!manager)
        return;

    const Disease* disease = manager->getDiseaseById(diseaseId);
    if (!disease)
        return;

    // Check if already infected
    if (hasDisease(diseaseId)) {
        return;
    }

    // Check for immunity
    float immunity = getImmunityStrength(diseaseId, 0); // currentDay would be passed in real implementation
    float resistanceRoll = static_cast<float>(rand()) / RAND_MAX;

    if (resistanceRoll < immunity) {
        std::cout << "Your immunity to " << disease->name << " has protected you from infection!" << std::endl;
        return;
    }

    // Check for disease resistance
    float resistanceCheck = diseaseResistance + (static_cast<float>(rand()) / RAND_MAX * 0.2f); // Add some randomness
    if (resistanceCheck > disease->resistanceThreshold) {
        std::cout << "Your natural resistance has protected you from " << disease->name << "!" << std::endl;
        return;
    }

    // Contract the disease
    activeDiseaseDays[diseaseId] = 0;
    std::cout << "You have contracted " << disease->name << "!" << std::endl;

    if (disease->incubationPeriod > 0) {
        std::cout << "Symptoms will begin to appear in " << disease->incubationPeriod << " days." << std::endl;
    } else {
        std::cout << "Symptoms are already appearing!" << std::endl;
    }
}

void HealthState::recoverFromDisease(const std::string& diseaseId, int currentDay, bool developImmunity)
{
    if (!hasDisease(diseaseId))
        return;

    // Remove the disease
    activeDiseaseDays.erase(diseaseId);

    // Mark as having had this disease
    pastDiseases[diseaseId] = true;

    // Develop immunity if applicable
    if (developImmunity) {
        // Immunity strength is between 0.7 and 0.95
        float immunityStrength = 0.7f + (static_cast<float>(rand()) / RAND_MAX * 0.25f);

        // Duration between 60-180 days, or permanent (=-1) with 10% chance
        int immunityDuration = -1;
        if (rand() % 100 < 90) { // 90% chance of temporary immunity
            immunityDuration = 60 + (rand() % 121); // 60-180 days
        }

        addImmunity(diseaseId, immunityStrength, immunityDuration, currentDay);
    }

    std::cout << "You have recovered from " << diseaseId << "!" << std::endl;
}

bool HealthState::hasDisease(const std::string& diseaseId) const
{
    return activeDiseaseDays.find(diseaseId) != activeDiseaseDays.end();
}

bool HealthState::hadDisease(const std::string& diseaseId) const
{
    return pastDiseases.find(diseaseId) != pastDiseases.end() && pastDiseases.at(diseaseId);
}

float HealthState::getImmunityStrength(const std::string& diseaseId, int currentDay) const
{
    float strongest = 0.0f;

    for (const auto& immunity : immunities) {
        if (immunity.diseaseId == diseaseId) {
            float strength = immunity.getEffectiveStrength(currentDay);
            if (strength > strongest) {
                strongest = strength;
            }
        }
    }

    return strongest;
}

void HealthState::addImmunity(const std::string& diseaseId, float strength, int duration, int currentDay)
{
    immunities.push_back(Immunity(diseaseId, strength, duration, currentDay));
    std::cout << "Gained " << (duration == -1 ? "permanent" : "temporary")
              << " immunity to " << diseaseId
              << " (Strength: " << strength << ")" << std::endl;
}

void HealthState::updateImmunities(int currentDay)
{
    // Remove expired immunities
    immunities.erase(
        std::remove_if(immunities.begin(), immunities.end(),
            [currentDay](const Immunity& immunity) {
                return !immunity.isActive(currentDay);
            }),
        immunities.end());
}