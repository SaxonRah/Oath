// /oath/systems/health/HealthState.hpp
#pragma once

#include "Immunity.hpp"
#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

// Forward declaration
class DiseaseManager;

// Class to represent the player's health state
class HealthState {
public:
    float currentHealth;
    float maxHealth;
    float stamina;
    float maxStamina;
    float naturalHealRate;
    float diseaseResistance;
    std::map<std::string, int> activeDiseaseDays; // Disease ID to days infected
    std::map<std::string, bool> pastDiseases; // Diseases the player has recovered from
    std::vector<Immunity> immunities; // Immunities player has developed

    HealthState();

    // Initialize from JSON
    void initFromJson(const nlohmann::json& healthJson);

    void takeDamage(float amount);
    void heal(float amount);
    void useStamina(float amount);
    void restoreStamina(float amount);
    void contractDisease(const std::string& diseaseId, DiseaseManager* manager);
    void recoverFromDisease(const std::string& diseaseId, int currentDay, bool developImmunity = true);
    bool hasDisease(const std::string& diseaseId) const;
    bool hadDisease(const std::string& diseaseId) const;
    float getImmunityStrength(const std::string& diseaseId, int currentDay) const;
    void addImmunity(const std::string& diseaseId, float strength, int duration, int currentDay);
    void updateImmunities(int currentDay);
};