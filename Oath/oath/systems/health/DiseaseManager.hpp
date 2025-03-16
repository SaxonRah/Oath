// /oath/systems/health/DiseaseManager.hpp
#pragma once

#include "../../data/GameContext.hpp"
#include "Disease.hpp"
#include "HealingMethod.hpp"
#include <map>
#include <string>
#include <vector>

// Disease Manager class to handle disease-related logic
class DiseaseManager {
public:
    std::map<std::string, Disease> diseases;
    std::map<std::string, HealingMethod> healingMethods;
    std::map<std::string, float> regionDiseaseRisk; // Risk multiplier for each region

    // Load all data from JSON file
    bool loadFromJson(const std::string& filename);
    void registerDisease(const Disease& disease);
    void registerHealingMethod(const HealingMethod& method);
    void setRegionRisk(const std::string& region, float risk);
    float getRegionRisk(const std::string& region) const;
    const Disease* getDiseaseById(const std::string& diseaseId) const;
    std::vector<const Disease*> getDiseasesInRegion(const std::string& region) const;
    const HealingMethod* getHealingMethodById(const std::string& methodId) const;
    bool checkExposure(GameContext* context, const std::string& regionName, const std::string& vector);
    void updateDiseases(GameContext* context, int currentDay);
    void applySymptomEffects(GameContext* context);
};