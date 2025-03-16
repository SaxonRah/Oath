// /oath/systems/health/DiseaseManager.cpp
#include "DiseaseManager.hpp"
#include "HealthState.hpp"
#include <fstream>
#include <iostream>


bool DiseaseManager::loadFromJson(const std::string& filename)
{
    try {
        // Read JSON file
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << filename << std::endl;
            return false;
        }

        nlohmann::json j;
        file >> j;
        file.close();

        // Load diseases
        for (const auto& diseaseJson : j["diseases"]) {
            Disease disease(diseaseJson);
            diseases[disease.id] = disease;
        }

        // Load healing methods
        for (const auto& methodJson : j["healingMethods"]) {
            HealingMethod method(methodJson);
            healingMethods[method.id] = method;
        }

        // Load region risks
        for (auto& [region, risk] : j["regionDiseaseRisks"].items()) {
            regionDiseaseRisk[region] = risk;
        }

        std::cout << "Successfully loaded " << diseases.size() << " diseases, "
                  << healingMethods.size() << " healing methods, and "
                  << regionDiseaseRisk.size() << " region risk factors from " << filename << std::endl;

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading disease system from JSON: " << e.what() << std::endl;
        return false;
    }
}

void DiseaseManager::registerDisease(const Disease& disease)
{
    diseases[disease.id] = disease;
}

void DiseaseManager::registerHealingMethod(const HealingMethod& method)
{
    healingMethods[method.id] = method;
}

void DiseaseManager::setRegionRisk(const std::string& region, float risk)
{
    regionDiseaseRisk[region] = risk;
}

float DiseaseManager::getRegionRisk(const std::string& region) const
{
    auto it = regionDiseaseRisk.find(region);
    return (it != regionDiseaseRisk.end()) ? it->second : 1.0f;
}

const Disease* DiseaseManager::getDiseaseById(const std::string& diseaseId) const
{
    auto it = diseases.find(diseaseId);
    return (it != diseases.end()) ? &it->second : nullptr;
}

std::vector<const Disease*> DiseaseManager::getDiseasesInRegion(const std::string& region) const
{
    std::vector<const Disease*> result;
    for (const auto& [id, disease] : diseases) {
        if (disease.isCommonIn(region)) {
            result.push_back(&disease);
        }
    }
    return result;
}

const HealingMethod* DiseaseManager::getHealingMethodById(const std::string& methodId) const
{
    auto it = healingMethods.find(methodId);
    return (it != healingMethods.end()) ? &it->second : nullptr;
}

bool DiseaseManager::checkExposure(GameContext* context, const std::string& regionName, const std::string& vector)
{
    if (!context)
        return false;

    HealthState* health = &context->healthContext.playerHealth;
    if (!health)
        return false;

    // Get region risk factor
    float regionRisk = getRegionRisk(regionName);

    // Check each disease that could be present in this region
    std::vector<const Disease*> regionalDiseases = getDiseasesInRegion(regionName);

    for (const Disease* disease : regionalDiseases) {
        // Skip if not transmitted by this vector
        if (!disease->transmitsVia(vector))
            continue;

        // Skip if already infected
        if (health->hasDisease(disease->id))
            continue;

        // Calculate infection chance
        float baseChance = disease->contagiousness * regionRisk;

        // Adjust for immunity
        float immunity = health->getImmunityStrength(disease->id, context->worldState.daysPassed);
        baseChance *= (1.0f - immunity);

        // Random roll
        float roll = static_cast<float>(rand()) / RAND_MAX;

        if (roll < baseChance) {
            // Infection occurs!
            health->contractDisease(disease->id, this);
            return true;
        }
    }

    return false;
}

void DiseaseManager::updateDiseases(GameContext* context, int currentDay)
{
    if (!context)
        return;

    HealthState* health = &context->healthContext.playerHealth;
    if (!health)
        return;

    // Update immunities
    health->updateImmunities(currentDay);

    // Create a copy to avoid modification during iteration
    std::map<std::string, int> diseasesCopy = health->activeDiseaseDays;

    for (auto& [diseaseId, days] : diseasesCopy) {
        const Disease* disease = getDiseaseById(diseaseId);
        if (!disease)
            continue;

        // Increment days infected
        health->activeDiseaseDays[diseaseId]++;
        days++; // Update local copy

        // Check if symptoms should appear or change
        if (days == disease->incubationPeriod) {
            std::cout << "Symptoms of " << disease->name << " have appeared!" << std::endl;
        }

        // Check for natural recovery
        if (!disease->isChronic && days >= disease->naturalDuration) {
            // Roll for recovery chance
            float recoveryChance = 0.2f; // 20% chance per day after natural duration
            float roll = static_cast<float>(rand()) / RAND_MAX;

            if (roll < recoveryChance) {
                health->recoverFromDisease(diseaseId, currentDay, true);
            }
        }

        // Disease progression - symptoms may worsen
        if (days >= disease->incubationPeriod) {
            // For each symptom, there's a chance it might worsen
            for (auto& symptom : disease->symptoms) {
                float worsenChance = 0.05f; // 5% chance per day
                worsenChance *= (1.0f + days / 10.0f); // Increases over time

                float roll = static_cast<float>(rand()) / RAND_MAX;
                if (roll < worsenChance) {
                    symptom.increaseSeverity();
                }
            }
        }
    }

    // Apply symptom effects
    applySymptomEffects(context);
}

void DiseaseManager::applySymptomEffects(GameContext* context)
{
    if (!context)
        return;

    HealthState* health = &context->healthContext.playerHealth;
    if (!health)
        return;

    for (const auto& [diseaseId, days] : health->activeDiseaseDays) {
        const Disease* disease = getDiseaseById(diseaseId);
        if (!disease)
            continue;

        // Skip if still in incubation
        if (days < disease->incubationPeriod)
            continue;

        // Apply each symptom's effects
        for (const auto& symptom : disease->symptoms) {
            // Apply stat effects
            symptom.applyStatEffects(context);

            // Apply any custom effects
            symptom.update(context);
        }
    }
}