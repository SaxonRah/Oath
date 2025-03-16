// /oath/systems/health/Disease.hpp
#pragma once

#include "Symptom.hpp"
#include <nlohmann/json.hpp>
#include <set>
#include <string>
#include <vector>

// Class to define a disease
class Disease {
public:
    std::string id;
    std::string name;
    std::string description;
    std::vector<Symptom> symptoms;
    int incubationPeriod; // Days before symptoms start
    int naturalDuration; // Days disease lasts if untreated
    float contagiousness; // 0.0 to 1.0, chance to spread to others
    float resistanceThreshold; // Minimum constitution/resistance to avoid infection
    bool isChronic; // If true, disease doesn't naturally cure
    std::set<std::string> regions; // Regions where this disease is common
    std::set<std::string> vectors; // How it spreads (air, water, contact, etc.)

    Disease(const std::string& diseaseId, const std::string& diseaseName);

    // Constructor from JSON
    Disease(const nlohmann::json& diseaseJson);

    void addSymptom(const Symptom& symptom);
    void addRegion(const std::string& region);
    void addVector(const std::string& vector);
    bool transmitsVia(const std::string& vector) const;
    bool isCommonIn(const std::string& region) const;
};