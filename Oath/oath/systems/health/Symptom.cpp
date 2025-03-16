// /oath/systems/health/Symptom.cpp
#include "Symptom.hpp"
#include <algorithm>
#include <iostream>


// Helper function to convert string to SymptomSeverity
SymptomSeverity stringToSeverity(const std::string& severityStr)
{
    if (severityStr == "NONE")
        return SymptomSeverity::NONE;
    if (severityStr == "MILD")
        return SymptomSeverity::MILD;
    if (severityStr == "MODERATE")
        return SymptomSeverity::MODERATE;
    if (severityStr == "SEVERE")
        return SymptomSeverity::SEVERE;
    if (severityStr == "CRITICAL")
        return SymptomSeverity::CRITICAL;
    return SymptomSeverity::MILD; // Default
}

// Helper function to convert SymptomSeverity to string
std::string severityToString(SymptomSeverity severity)
{
    switch (severity) {
    case SymptomSeverity::NONE:
        return "NONE";
    case SymptomSeverity::MILD:
        return "MILD";
    case SymptomSeverity::MODERATE:
        return "MODERATE";
    case SymptomSeverity::SEVERE:
        return "SEVERE";
    case SymptomSeverity::CRITICAL:
        return "CRITICAL";
    default:
        return "UNKNOWN";
    }
}

Symptom::Symptom(const std::string& symptomName, const std::string& desc,
    SymptomSeverity initialSeverity)
    : name(symptomName)
    , description(desc)
    , severity(initialSeverity)
    , hasDamageOverTime(false)
    , damagePerUpdate(0.0f)
{
}

Symptom::Symptom(const nlohmann::json& symptomJson)
    : name(symptomJson["name"])
    , description(symptomJson["description"])
    , severity(stringToSeverity(symptomJson["initialSeverity"]))
    , hasDamageOverTime(symptomJson.value("hasDamageOverTime", false))
    , damagePerUpdate(symptomJson.value("damagePerUpdate", 0.0f))
{
    // Load stat effects
    if (symptomJson.contains("statEffects")) {
        for (auto& [stat, effect] : symptomJson["statEffects"].items()) {
            statEffects[stat] = effect;
        }
    }

    // Set up damage over time effect if needed
    if (hasDamageOverTime) {
        onUpdateEffect = [this](GameContext* context) {
            if (context) {
                HealthState* health = &context->healthContext.playerHealth;
                if (health) {
                    health->takeDamage(damagePerUpdate);
                }
            }
        };
    }
}

void Symptom::increaseSeverity()
{
    if (severity == SymptomSeverity::CRITICAL)
        return;

    severity = static_cast<SymptomSeverity>(static_cast<int>(severity) + 1);
    std::cout << name << " symptom worsened to " << getSeverityString() << std::endl;
}

void Symptom::decreaseSeverity()
{
    if (severity == SymptomSeverity::NONE)
        return;

    severity = static_cast<SymptomSeverity>(static_cast<int>(severity) - 1);
    std::cout << name << " symptom improved to " << getSeverityString() << std::endl;
}

std::string Symptom::getSeverityString() const
{
    switch (severity) {
    case SymptomSeverity::NONE:
        return "None";
    case SymptomSeverity::MILD:
        return "Mild";
    case SymptomSeverity::MODERATE:
        return "Moderate";
    case SymptomSeverity::SEVERE:
        return "Severe";
    case SymptomSeverity::CRITICAL:
        return "Critical";
    default:
        return "Unknown";
    }
}

void Symptom::applyStatEffects(GameContext* context) const
{
    if (!context)
        return;

    // Apply stat modifications based on severity
    float severityMultiplier = 0.0f;
    switch (severity) {
    case SymptomSeverity::MILD:
        severityMultiplier = 0.05f;
        break;
    case SymptomSeverity::MODERATE:
        severityMultiplier = 0.15f;
        break;
    case SymptomSeverity::SEVERE:
        severityMultiplier = 0.30f;
        break;
    case SymptomSeverity::CRITICAL:
        severityMultiplier = 0.50f;
        break;
    default:
        return; // No effect for NONE severity
    }

    // Apply each stat effect
    for (const auto& [statName, baseEffect] : statEffects) {
        float actualEffect = baseEffect * severityMultiplier;

        // Apply to character stats
        if (statName == "strength") {
            context->playerStats.modifiers["strength"] -= actualEffect;
        } else if (statName == "stamina") {
            context->healthContext.playerHealth.maxStamina -= actualEffect;
        } else if (statName == "constitution") {
            context->playerStats.modifiers["constitution"] -= actualEffect;
        } else if (statName == "dexterity") {
            context->playerStats.modifiers["dexterity"] -= actualEffect;
        } else if (statName == "intelligence") {
            context->playerStats.modifiers["intelligence"] -= actualEffect;
        } else if (statName == "wisdom") {
            context->playerStats.modifiers["wisdom"] -= actualEffect;
        }
    }
}

void Symptom::update(GameContext* context)
{
    if (onUpdateEffect && severity != SymptomSeverity::NONE) {
        onUpdateEffect(context);
    }
}