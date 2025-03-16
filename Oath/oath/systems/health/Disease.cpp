// /oath/systems/health/Disease.cpp
#include "Disease.hpp"

Disease::Disease(const std::string& diseaseId, const std::string& diseaseName)
    : id(diseaseId)
    , name(diseaseName)
    , incubationPeriod(1)
    , naturalDuration(7)
    , contagiousness(0.3f)
    , resistanceThreshold(0.2f)
    , isChronic(false)
{
}

Disease::Disease(const nlohmann::json& diseaseJson)
    : id(diseaseJson["id"])
    , name(diseaseJson["name"])
    , description(diseaseJson["description"])
    , incubationPeriod(diseaseJson["incubationPeriod"])
    , naturalDuration(diseaseJson["naturalDuration"])
    , contagiousness(diseaseJson["contagiousness"])
    , resistanceThreshold(diseaseJson["resistanceThreshold"])
    , isChronic(diseaseJson["isChronic"])
{
    // Load regions
    for (const auto& region : diseaseJson["regions"]) {
        regions.insert(region);
    }

    // Load vectors
    for (const auto& vector : diseaseJson["vectors"]) {
        vectors.insert(vector);
    }

    // Load symptoms
    for (const auto& symptomJson : diseaseJson["symptoms"]) {
        symptoms.push_back(Symptom(symptomJson));
    }
}

void Disease::addSymptom(const Symptom& symptom)
{
    symptoms.push_back(symptom);
}

void Disease::addRegion(const std::string& region)
{
    regions.insert(region);
}

void Disease::addVector(const std::string& vector)
{
    vectors.insert(vector);
}

bool Disease::transmitsVia(const std::string& vector) const
{
    return vectors.find(vector) != vectors.end();
}

bool Disease::isCommonIn(const std::string& region) const
{
    return regions.find(region) != regions.end();
}