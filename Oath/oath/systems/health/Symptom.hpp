// /oath/systems/health/Symptom.hpp
#pragma once

#include <functional>
#include <map>
#include <nlohmann/json.hpp>
#include <string>

#include "../../data/GameContext.hpp"

struct GameContext;

// Enum for Symptom Severity
enum class SymptomSeverity {
    NONE,
    MILD,
    MODERATE,
    SEVERE,
    CRITICAL
};

// Helper functions for SymptomSeverity
SymptomSeverity stringToSeverity(const std::string& severityStr);
std::string severityToString(SymptomSeverity severity);

// Class to represent a symptom of a disease
class Symptom {
public:
    std::string name;
    std::string description;
    SymptomSeverity severity;
    std::map<std::string, float> statEffects; // Affects player stats
    std::function<void(GameContext*)> onUpdateEffect; // Effect applied during symptoms update
    bool hasDamageOverTime;
    float damagePerUpdate;

    Symptom(const std::string& symptomName, const std::string& desc,
        SymptomSeverity initialSeverity = SymptomSeverity::MILD);

    // Constructor from JSON
    Symptom(const nlohmann::json& symptomJson);

    void increaseSeverity();
    void decreaseSeverity();
    std::string getSeverityString() const;
    void applyStatEffects(GameContext* context) const;
    void update(GameContext* context);
};