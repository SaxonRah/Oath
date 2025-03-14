// System_DiseaseHealth.cpp
#include <algorithm>
#include <ctime>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <queue>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

// Forward declarations of existing system components
class TANode;
class TAController;
class Inventory;
class GameContext;
class TAInput;
class TAAction;
class TATransitionRule;
class NodeID;
class Item;

// Disease and Health System
// This implementation adds a comprehensive disease and health system to the RawOath framework

//----------------------------------------
// DISEASE AND HEALTH SYSTEM
//----------------------------------------

// Forward declarations for health system
class Disease;
class DiseaseManager;
class DiseaseNode;
class HealthState;
class HealingMethod;
class Symptom;
class Immunity;

// Enum for Symptom Severity
enum class SymptomSeverity {
    NONE,
    MILD,
    MODERATE,
    SEVERE,
    CRITICAL
};

// Class to represent a symptom of a disease
class Symptom {
public:
    std::string name;
    std::string description;
    SymptomSeverity severity;
    std::map<std::string, float> statEffects; // Affects player stats (strength, speed, etc.)
    std::function<void(GameContext*)> onUpdateEffect; // Effect applied during symptoms update

    Symptom(const std::string& symptomName, const std::string& desc,
        SymptomSeverity initialSeverity = SymptomSeverity::MILD)
        : name(symptomName)
        , description(desc)
        , severity(initialSeverity)
    {
    }

    void increaseSeverity()
    {
        if (severity == SymptomSeverity::CRITICAL)
            return;

        severity = static_cast<SymptomSeverity>(static_cast<int>(severity) + 1);
        std::cout << name << " symptom worsened to " << getSeverityString() << std::endl;
    }

    void decreaseSeverity()
    {
        if (severity == SymptomSeverity::NONE)
            return;

        severity = static_cast<SymptomSeverity>(static_cast<int>(severity) - 1);
        std::cout << name << " symptom improved to " << getSeverityString() << std::endl;
    }

    std::string getSeverityString() const
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

    void applyStatEffects(GameContext* context) const;

    void update(GameContext* context)
    {
        if (onUpdateEffect && severity != SymptomSeverity::NONE) {
            onUpdateEffect(context);
        }
    }
};

// Class to represent an immunity to a disease
class Immunity {
public:
    std::string diseaseId;
    float strength; // 0.0 to 1.0, where 1.0 is full immunity
    int durationDays; // How long the immunity lasts, -1 for permanent
    int dayAcquired; // Day the immunity was acquired

    Immunity(const std::string& disease, float immuneStrength, int duration, int currentDay)
        : diseaseId(disease)
        , strength(immuneStrength)
        , durationDays(duration)
        , dayAcquired(currentDay)
    {
    }

    bool isActive(int currentDay) const
    {
        return durationDays == -1 || (currentDay - dayAcquired) < durationDays;
    }

    float getEffectiveStrength(int currentDay) const
    {
        if (!isActive(currentDay))
            return 0.0f;

        // Immunity can wane over time
        if (durationDays > 0) {
            float progress = static_cast<float>(currentDay - dayAcquired) / durationDays;
            return strength * (1.0f - progress * 0.5f); // Gradually reduces to 50% of original strength
        }

        return strength;
    }
};

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

    Disease(const std::string& diseaseId, const std::string& diseaseName)
        : id(diseaseId)
        , name(diseaseName)
        , incubationPeriod(1)
        , naturalDuration(7)
        , contagiousness(0.3f)
        , resistanceThreshold(0.2f)
        , isChronic(false)
    {
    }

    void addSymptom(const Symptom& symptom)
    {
        symptoms.push_back(symptom);
    }

    void addRegion(const std::string& region)
    {
        regions.insert(region);
    }

    void addVector(const std::string& vector)
    {
        vectors.insert(vector);
    }

    // Check if the disease can be transmitted through a specific vector
    bool transmitsVia(const std::string& vector) const
    {
        return vectors.find(vector) != vectors.end();
    }

    // Check if the disease is common in a specific region
    bool isCommonIn(const std::string& region) const
    {
        return regions.find(region) != regions.end();
    }
};

// Class to represent a healing method
class HealingMethod {
public:
    std::string id;
    std::string name;
    std::string description;
    std::map<std::string, float> effectivenessAgainstDisease; // Disease ID to effectiveness (0.0-1.0)
    std::function<bool(GameContext*)> requirementCheck; // Check if the method can be used
    std::function<void(GameContext*, const std::string&)> applyEffect; // Apply healing effects

    HealingMethod(const std::string& methodId, const std::string& methodName)
        : id(methodId)
        , name(methodName)
    {
    }

    void setEffectiveness(const std::string& diseaseId, float effectiveness)
    {
        effectivenessAgainstDisease[diseaseId] = std::clamp(effectiveness, 0.0f, 1.0f);
    }

    float getEffectivenessAgainst(const std::string& diseaseId) const
    {
        auto it = effectivenessAgainstDisease.find(diseaseId);
        return (it != effectivenessAgainstDisease.end()) ? it->second : 0.0f;
    }

    bool canBeUsed(GameContext* context) const
    {
        return requirementCheck ? requirementCheck(context) : true;
    }

    void apply(GameContext* context, const std::string& diseaseId)
    {
        if (applyEffect && canBeUsed(context)) {
            applyEffect(context, diseaseId);
        }
    }
};

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

    HealthState()
        : currentHealth(100.0f)
        , maxHealth(100.0f)
        , stamina(100.0f)
        , maxStamina(100.0f)
        , naturalHealRate(1.0f)
        , diseaseResistance(0.0f)
    {
    }

    void takeDamage(float amount)
    {
        currentHealth = std::max(0.0f, currentHealth - amount);
        if (currentHealth <= 0) {
            std::cout << "You have fallen unconscious due to your injuries!" << std::endl;
        }
    }

    void heal(float amount)
    {
        currentHealth = std::min(maxHealth, currentHealth + amount);
    }

    void useStamina(float amount)
    {
        stamina = std::max(0.0f, stamina - amount);
    }

    void restoreStamina(float amount)
    {
        stamina = std::min(maxStamina, stamina + amount);
    }

    void contractDisease(const std::string& diseaseId, DiseaseManager* manager);

    void recoverFromDisease(const std::string& diseaseId, int currentDay, bool developImmunity = true);

    bool hasDisease(const std::string& diseaseId) const
    {
        return activeDiseaseDays.find(diseaseId) != activeDiseaseDays.end();
    }

    bool hadDisease(const std::string& diseaseId) const
    {
        return pastDiseases.find(diseaseId) != pastDiseases.end() && pastDiseases.at(diseaseId);
    }

    float getImmunityStrength(const std::string& diseaseId, int currentDay) const
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

    void addImmunity(const std::string& diseaseId, float strength, int duration, int currentDay)
    {
        immunities.push_back(Immunity(diseaseId, strength, duration, currentDay));
        std::cout << "Gained " << (duration == -1 ? "permanent" : "temporary")
                  << " immunity to " << diseaseId
                  << " (Strength: " << strength << ")" << std::endl;
    }

    void updateImmunities(int currentDay)
    {
        // Remove expired immunities
        immunities.erase(
            std::remove_if(immunities.begin(), immunities.end(),
                [currentDay](const Immunity& immunity) {
                    return !immunity.isActive(currentDay);
                }),
            immunities.end());
    }
};

// Add HealthState to the GameContext
struct HealthContext {
    HealthState playerHealth;
    std::map<std::string, Disease> knownDiseases;
    std::map<std::string, HealingMethod> healingMethods;
    std::map<std::string, float> regionDiseaseRisk; // Region name to disease risk multiplier
};

// Disease Manager class to handle disease-related logic
class DiseaseManager {
public:
    std::map<std::string, Disease> diseases;
    std::map<std::string, HealingMethod> healingMethods;
    std::map<std::string, float> regionDiseaseRisk; // Risk multiplier for each region

    void registerDisease(const Disease& disease)
    {
        diseases[disease.id] = disease;
    }

    void registerHealingMethod(const HealingMethod& method)
    {
        healingMethods[method.id] = method;
    }

    void setRegionRisk(const std::string& region, float risk)
    {
        regionDiseaseRisk[region] = risk;
    }

    float getRegionRisk(const std::string& region) const
    {
        auto it = regionDiseaseRisk.find(region);
        return (it != regionDiseaseRisk.end()) ? it->second : 1.0f;
    }

    const Disease* getDiseaseById(const std::string& diseaseId) const
    {
        auto it = diseases.find(diseaseId);
        return (it != diseases.end()) ? &it->second : nullptr;
    }

    std::vector<const Disease*> getDiseasesInRegion(const std::string& region) const
    {
        std::vector<const Disease*> result;
        for (const auto& [id, disease] : diseases) {
            if (disease.isCommonIn(region)) {
                result.push_back(&disease);
            }
        }
        return result;
    }

    const HealingMethod* getHealingMethodById(const std::string& methodId) const
    {
        auto it = healingMethods.find(methodId);
        return (it != healingMethods.end()) ? &it->second : nullptr;
    }

    bool checkExposure(GameContext* context, const std::string& regionName, const std::string& vector);

    void updateDiseases(GameContext* context, int currentDay);

    void applySymptomEffects(GameContext* context);
};

// Now implement the specific health system nodes

// Node representing the general health state
class HealthStateNode : public TANode {
public:
    HealthStateNode(const std::string& name)
        : TANode(name)
    {
    }

    void onEnter(GameContext* context) override
    {
        TANode::onEnter(context);

        if (!context)
            return;

        HealthState* health = getHealthState(context);
        if (!health)
            return;

        std::cout << "==== Health Status ====" << std::endl;
        std::cout << "Health: " << health->currentHealth << "/" << health->maxHealth << std::endl;
        std::cout << "Stamina: " << health->stamina << "/" << health->maxStamina << std::endl;

        if (!health->activeDiseaseDays.empty()) {
            std::cout << "\nActive Diseases:" << std::endl;
            DiseaseManager* manager = getDiseaseManager(context);

            for (const auto& [diseaseId, days] : health->activeDiseaseDays) {
                const Disease* disease = manager ? manager->getDiseaseById(diseaseId) : nullptr;
                std::string diseaseName = disease ? disease->name : diseaseId;

                std::cout << "- " << diseaseName << " (Day " << days << ")" << std::endl;

                // Show symptoms
                if (disease && days >= disease->incubationPeriod) {
                    std::cout << "  Symptoms:" << std::endl;
                    for (const auto& symptom : disease->symptoms) {
                        if (symptom.severity != SymptomSeverity::NONE) {
                            std::cout << "  - " << symptom.name << " ("
                                      << symptom.getSeverityString() << ")" << std::endl;
                        }
                    }
                }
            }
        } else {
            std::cout << "\nYou are currently in good health." << std::endl;
        }

        // Show immunities
        if (!health->immunities.empty()) {
            std::cout << "\nImmunities:" << std::endl;
            int currentDay = context->worldState.daysPassed;
            DiseaseManager* manager = getDiseaseManager(context);

            for (const auto& immunity : health->immunities) {
                if (immunity.isActive(currentDay)) {
                    const Disease* disease = manager ? manager->getDiseaseById(immunity.diseaseId) : nullptr;
                    std::string diseaseName = disease ? disease->name : immunity.diseaseId;

                    std::cout << "- " << diseaseName << " (Strength: "
                              << immunity.getEffectiveStrength(currentDay) << ")" << std::endl;

                    if (immunity.durationDays > 0) {
                        int daysLeft = immunity.dayAcquired + immunity.durationDays - currentDay;
                        std::cout << "  " << daysLeft << " days remaining" << std::endl;
                    } else {
                        std::cout << "  Permanent immunity" << std::endl;
                    }
                }
            }
        }
    }

    std::vector<TAAction> getAvailableActions() override
    {
        std::vector<TAAction> actions = TANode::getAvailableActions();

        actions.push_back({ "treat_diseases", "Treat Diseases",
            []() -> TAInput {
                return { "health_action", { { "action", std::string("treat") } } };
            } });

        actions.push_back({ "rest", "Rest to Recover",
            []() -> TAInput {
                return { "health_action", { { "action", std::string("rest") } } };
            } });

        return actions;
    }

    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override
    {
        if (input.type == "health_action") {
            std::string action = std::get<std::string>(input.parameters.at("action"));

            if (action == "treat") {
                for (const auto& rule : transitionRules) {
                    if (rule.description == "Treat Diseases") {
                        outNextNode = rule.targetNode;
                        return true;
                    }
                }
            } else if (action == "rest") {
                for (const auto& rule : transitionRules) {
                    if (rule.description == "Rest") {
                        outNextNode = rule.targetNode;
                        return true;
                    }
                }
            }
        }

        return TANode::evaluateTransition(input, outNextNode);
    }

private:
    HealthState* getHealthState(GameContext* context)
    {
        // This would access the health state from your extended GameContext
        // You'll need to adapt this based on how you integrate this system
        // For example, if you add a HealthContext to GameContext:
        // return &context->healthContext.playerHealth;
        return nullptr; // Placeholder
    }

    DiseaseManager* getDiseaseManager(GameContext* context)
    {
        // Similarly, this would access your disease manager
        // return &context->healthContext.diseaseManager;
        return nullptr; // Placeholder
    }
};

// Node representing a specific disease
class DiseaseNode : public TANode {
public:
    std::string diseaseId;

    DiseaseNode(const std::string& name, const std::string& id)
        : TANode(name)
        , diseaseId(id)
    {
    }

    void onEnter(GameContext* context) override
    {
        TANode::onEnter(context);

        if (!context)
            return;

        DiseaseManager* manager = getDiseaseManager(context);
        if (!manager)
            return;

        const Disease* disease = manager->getDiseaseById(diseaseId);
        if (!disease) {
            std::cout << "Unknown disease: " << diseaseId << std::endl;
            return;
        }

        std::cout << "=== " << disease->name << " ===" << std::endl;
        std::cout << disease->description << std::endl;

        std::cout << "\nContagiousness: " << (disease->contagiousness * 100) << "%" << std::endl;
        std::cout << "Incubation Period: " << disease->incubationPeriod << " days" << std::endl;
        std::cout << "Natural Duration: " << (disease->isChronic ? "Chronic (doesn't naturally heal)" : std::to_string(disease->naturalDuration) + " days") << std::endl;

        std::cout << "\nTransmission Vectors:" << std::endl;
        for (const auto& vector : disease->vectors) {
            std::cout << "- " << vector << std::endl;
        }

        std::cout << "\nCommon Regions:" << std::endl;
        for (const auto& region : disease->regions) {
            std::cout << "- " << region << std::endl;
        }

        std::cout << "\nSymptoms:" << std::endl;
        for (const auto& symptom : disease->symptoms) {
            std::cout << "- " << symptom.name << ": " << symptom.description << std::endl;
        }

        // Show effective treatments
        std::cout << "\nEffective Treatments:" << std::endl;
        bool foundTreatment = false;

        for (const auto& [methodId, method] : manager->healingMethods) {
            float effectiveness = method.getEffectivenessAgainst(diseaseId);
            if (effectiveness > 0.0f) {
                std::cout << "- " << method.name << " (Effectiveness: "
                          << (effectiveness * 100) << "%)" << std::endl;
                foundTreatment = true;
            }
        }

        if (!foundTreatment) {
            std::cout << "- No known effective treatments." << std::endl;
        }

        // Check if the player has this disease
        HealthState* health = getHealthState(context);
        if (health && health->hasDisease(diseaseId)) {
            int daysInfected = health->activeDiseaseDays[diseaseId];
            std::cout << "\nYou have been infected for " << daysInfected << " days." << std::endl;

            if (daysInfected < disease->incubationPeriod) {
                std::cout << "The disease is still in its incubation period. Symptoms will appear in "
                          << (disease->incubationPeriod - daysInfected) << " days." << std::endl;
            }
        }
    }

    std::vector<TAAction> getAvailableActions() override
    {
        std::vector<TAAction> actions = TANode::getAvailableActions();

        actions.push_back({ "treat_disease", "Treat This Disease",
            [this]() -> TAInput {
                return { "disease_action", { { "action", std::string("treat") }, { "disease_id", diseaseId } } };
            } });

        return actions;
    }

    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override
    {
        if (input.type == "disease_action") {
            std::string action = std::get<std::string>(input.parameters.at("action"));

            if (action == "treat" && std::get<std::string>(input.parameters.at("disease_id")) == diseaseId) {
                // Find the treatment node for this disease
                for (const auto& rule : transitionRules) {
                    if (rule.description == "Treat Disease") {
                        outNextNode = rule.targetNode;
                        return true;
                    }
                }
            }
        }

        return TANode::evaluateTransition(input, outNextNode);
    }

private:
    HealthState* getHealthState(GameContext* context)
    {
        // As before, implementation depends on how you integrate this
        return nullptr; // Placeholder
    }

    DiseaseManager* getDiseaseManager(GameContext* context)
    {
        // As before
        return nullptr; // Placeholder
    }
};

// Node for disease treatment options
class TreatmentNode : public TANode {
public:
    std::string targetDiseaseId; // Optional, if treating a specific disease

    TreatmentNode(const std::string& name, const std::string& diseaseId = "")
        : TANode(name)
        , targetDiseaseId(diseaseId)
    {
    }

    void onEnter(GameContext* context) override
    {
        TANode::onEnter(context);

        if (!context)
            return;

        DiseaseManager* manager = getDiseaseManager(context);
        HealthState* health = getHealthState(context);

        if (!manager || !health)
            return;

        std::cout << "=== Treatment Options ===" << std::endl;

        if (targetDiseaseId.empty()) {
            // Show all diseases the player has
            if (health->activeDiseaseDays.empty()) {
                std::cout << "You are not currently afflicted by any diseases." << std::endl;
                return;
            }

            std::cout << "Select a disease to treat:" << std::endl;
            int i = 1;
            for (const auto& [diseaseId, days] : health->activeDiseaseDays) {
                const Disease* disease = manager->getDiseaseById(diseaseId);
                std::string diseaseName = disease ? disease->name : diseaseId;
                std::cout << i++ << ". " << diseaseName << " (Day " << days << ")" << std::endl;
            }
        } else {
            // Show treatments for the specific disease
            const Disease* disease = manager->getDiseaseById(targetDiseaseId);
            if (!disease) {
                std::cout << "Unknown disease: " << targetDiseaseId << std::endl;
                return;
            }

            if (!health->hasDisease(targetDiseaseId)) {
                std::cout << "You are not currently afflicted by " << disease->name << "." << std::endl;
                return;
            }

            std::cout << "Available treatments for " << disease->name << ":" << std::endl;

            int i = 1;
            for (const auto& [methodId, method] : manager->healingMethods) {
                float effectiveness = method.getEffectivenessAgainst(targetDiseaseId);
                if (effectiveness > 0.0f && method.canBeUsed(context)) {
                    std::cout << i++ << ". " << method.name
                              << " (Effectiveness: " << (effectiveness * 100) << "%)" << std::endl;
                    std::cout << "   " << method.description << std::endl;
                }
            }

            if (i == 1) {
                std::cout << "No treatments available for this disease." << std::endl;
            }
        }
    }

    std::vector<TAAction> getAvailableActions() override
    {
        std::vector<TAAction> actions = TANode::getAvailableActions();

        if (targetDiseaseId.empty()) {
            // Add actions for each disease
            actions.push_back({ "select_disease", "Select Disease to Treat",
                []() -> TAInput {
                    return { "treatment_action", {
                                                     { "action", std::string("select_disease") }, { "index", 0 } // This would be set by the UI
                                                 } };
                } });
        } else {
            // Add actions for each method
            actions.push_back({ "apply_treatment", "Apply Treatment",
                [this]() -> TAInput {
                    return { "treatment_action", {
                                                     { "action", std::string("apply_treatment") }, { "disease_id", targetDiseaseId }, { "method_id", std::string("") } // This would be set by the UI
                                                 } };
                } });
        }

        // Add back option
        actions.push_back({ "back", "Return to Health Menu",
            []() -> TAInput {
                return { "treatment_action", { { "action", std::string("back") } } };
            } });

        return actions;
    }

    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override
    {
        if (input.type == "treatment_action") {
            std::string action = std::get<std::string>(input.parameters.at("action"));

            if (action == "select_disease") {
                int index = std::get<int>(input.parameters.at("index"));
                // Would navigate to the specific disease treatment node
                // Implementation depends on your node structure
            } else if (action == "apply_treatment") {
                std::string diseaseId = std::get<std::string>(input.parameters.at("disease_id"));
                std::string methodId = std::get<std::string>(input.parameters.at("method_id"));

                // Apply the treatment and show results
                // Implementation depends on your disease manager
            } else if (action == "back") {
                for (const auto& rule : transitionRules) {
                    if (rule.description == "Back to Health") {
                        outNextNode = rule.targetNode;
                        return true;
                    }
                }
            }
        }

        return TANode::evaluateTransition(input, outNextNode);
    }

private:
    HealthState* getHealthState(GameContext* context)
    {
        return nullptr; // Placeholder
    }

    DiseaseManager* getDiseaseManager(GameContext* context)
    {
        return nullptr; // Placeholder
    }
};

// Node for resting and natural healing
class RestNode : public TANode {
public:
    RestNode(const std::string& name)
        : TANode(name)
    {
    }

    void onEnter(GameContext* context) override
    {
        TANode::onEnter(context);

        if (!context)
            return;

        std::cout << "=== Rest and Recovery ===" << std::endl;
        std::cout << "How long would you like to rest?" << std::endl;
        std::cout << "1. Short Rest (1 hour)" << std::endl;
        std::cout << "2. Long Rest (8 hours)" << std::endl;
        std::cout << "3. Full Day's Rest (24 hours)" << std::endl;
    }

    std::vector<TAAction> getAvailableActions() override
    {
        std::vector<TAAction> actions = TANode::getAvailableActions();

        actions.push_back({ "short_rest", "Short Rest (1 hour)",
            []() -> TAInput {
                return { "rest_action", { { "action", std::string("rest") }, { "hours", 1 } } };
            } });

        actions.push_back({ "long_rest", "Long Rest (8 hours)",
            []() -> TAInput {
                return { "rest_action", { { "action", std::string("rest") }, { "hours", 8 } } };
            } });

        actions.push_back({ "full_rest", "Full Day's Rest (24 hours)",
            []() -> TAInput {
                return { "rest_action", { { "action", std::string("rest") }, { "hours", 24 } } };
            } });

        actions.push_back({ "cancel_rest", "Cancel",
            []() -> TAInput {
                return { "rest_action", { { "action", std::string("cancel") } } };
            } });

        return actions;
    }

    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override
    {
        if (input.type == "rest_action") {
            std::string action = std::get<std::string>(input.parameters.at("action"));

            if (action == "rest") {
                int hours = std::get<int>(input.parameters.at("hours"));
                applyRest(hours, getGameContext());

                // Stay in the same node to show the results
                outNextNode = this;
                return true;
            } else if (action == "cancel") {
                for (const auto& rule : transitionRules) {
                    if (rule.description == "Back to Health") {
                        outNextNode = rule.targetNode;
                        return true;
                    }
                }
            }
        }

        return TANode::evaluateTransition(input, outNextNode);
    }

    void applyRest(int hours, GameContext* context)
    {
        if (!context)
            return;

        HealthState* health = getHealthState(context);
        DiseaseManager* manager = getDiseaseManager(context);

        if (!health || !manager)
            return;

        // Calculate how many game hours have passed
        int gameHours = hours;

        // Update health based on the rest duration
        float healthRecovery = health->naturalHealRate * gameHours;
        float staminaRecovery = (health->maxStamina * 0.1f) * gameHours;

        // Reduce recovery if sick
        if (!health->activeDiseaseDays.empty()) {
            float sicknessFactor = 0.5f; // Recover at 50% rate when sick
            healthRecovery *= sicknessFactor;
            staminaRecovery *= sicknessFactor;
        }

        health->heal(healthRecovery);
        health->restoreStamina(staminaRecovery);

        // Update diseases based on how many days have passed
        int daysPassed = gameHours / 24;
        int hoursLeftover = gameHours % 24;

        // Process each full day
        for (int i = 0; i < daysPassed; i++) {
            // Advance game day
            context->worldState.advanceDay();

            // Update diseases
            manager->updateDiseases(context, context->worldState.daysPassed);
        }

        // Show the results
        std::cout << "You rested for " << hours << " hours." << std::endl;
        std::cout << "Recovered " << healthRecovery << " health and " << staminaRecovery << " stamina." << std::endl;

        if (daysPassed > 0) {
            std::cout << daysPassed << " days have passed." << std::endl;
        }

        // Check for changes in disease states
        if (!health->activeDiseaseDays.empty()) {
            std::cout << "\nDisease Updates:" << std::endl;
            for (const auto& [diseaseId, days] : health->activeDiseaseDays) {
                const Disease* disease = manager->getDiseaseById(diseaseId);
                std::string diseaseName = disease ? disease->name : diseaseId;

                std::cout << "- " << diseaseName << ": Day " << days << std::endl;

                // Show if symptoms appeared or changed
                if (disease && days >= disease->incubationPeriod) {
                    if (days == disease->incubationPeriod) {
                        std::cout << "  Symptoms have appeared!" << std::endl;
                    }

                    // Show current symptoms
                    for (const auto& symptom : disease->symptoms) {
                        if (symptom.severity != SymptomSeverity::NONE) {
                            std::cout << "  - " << symptom.name << " ("
                                      << symptom.getSeverityString() << ")" << std::endl;
                        }
                    }
                }
            }
        }
    }

private:
    GameContext* getGameContext()
    {
        // Return the current game context - implementation depends on your system
        return nullptr; // Placeholder
    }

    HealthState* getHealthState(GameContext* context)
    {
        return nullptr; // Placeholder
    }

    DiseaseManager* getDiseaseManager(GameContext* context)
    {
        return nullptr; // Placeholder
    }
};

// Node for epidemic events
class EpidemicNode : public TANode {
public:
    std::string diseaseId;
    std::string regionName;
    float severityMultiplier;

    EpidemicNode(const std::string& name, const std::string& disease,
        const std::string& region, float severity = 1.0f)
        : TANode(name)
        , diseaseId(disease)
        , regionName(region)
        , severityMultiplier(severity)
    {
    }

    void onEnter(GameContext* context) override
    {
        TANode::onEnter(context);

        if (!context)
            return;

        DiseaseManager* manager = getDiseaseManager(context);
        if (!manager)
            return;

        const Disease* disease = manager->getDiseaseById(diseaseId);
        if (!disease) {
            std::cout << "Unknown disease epidemic: " << diseaseId << std::endl;
            return;
        }

        // Start the epidemic event
        std::cout << "=== EPIDEMIC ALERT ===" << std::endl;
        std::cout << "A " << disease->name << " outbreak has been reported in " << regionName << "!" << std::endl;
        std::cout << disease->description << std::endl;

        std::cout << "\nSymptoms to watch for:" << std::endl;
        for (const auto& symptom : disease->symptoms) {
            std::cout << "- " << symptom.name << ": " << symptom.description << std::endl;
        }

        std::cout << "\nTransmission Vectors:" << std::endl;
        for (const auto& vector : disease->vectors) {
            std::cout << "- " << vector << std::endl;
        }

        std::cout << "\nTravel to affected areas is not recommended." << std::endl;
        if (severityMultiplier > 1.5f) {
            std::cout << "This is a severe outbreak! Extreme caution is advised." << std::endl;
        }

        // Set region disease risk
        float originalRisk = manager->getRegionRisk(regionName);
        manager->setRegionRisk(regionName, originalRisk * severityMultiplier);

        // Set a world flag indicating epidemic in this region
        context->worldState.setWorldFlag("epidemic_" + regionName, true);
    }

    std::vector<TAAction> getAvailableActions() override
    {
        std::vector<TAAction> actions = TANode::getAvailableActions();

        actions.push_back({ "learn_prevention", "Learn Prevention Methods",
            [this]() -> TAInput {
                return { "epidemic_action", { { "action", std::string("learn_prevention") }, { "disease_id", diseaseId } } };
            } });

        actions.push_back({ "offer_help", "Offer Help",
            [this]() -> TAInput {
                return { "epidemic_action", { { "action", std::string("offer_help") }, { "region", regionName } } };
            } });

        actions.push_back({ "ignore", "Ignore the Epidemic",
            []() -> TAInput {
                return { "epidemic_action", { { "action", std::string("ignore") } } };
            } });

        return actions;
    }

    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override
    {
        if (input.type == "epidemic_action") {
            std::string action = std::get<std::string>(input.parameters.at("action"));

            if (action == "learn_prevention") {
                // Show prevention information
                std::cout << "\n=== Prevention Methods for " << diseaseId << " ===" << std::endl;
                std::cout << "1. Avoid contact with infected individuals" << std::endl;
                std::cout << "2. Boil water before drinking if waterborne" << std::endl;
                std::cout << "3. Wear protective masks if airborne" << std::endl;
                std::cout << "4. Maintain cleanliness and good hygiene" << std::endl;
                std::cout << "5. Seek healing potions or visit healers for preventive treatment" << std::endl;

                // Potentially learn a fact about disease prevention
                GameContext* context = getGameContext();
                if (context) {
                    context->playerStats.learnFact("prevention_" + diseaseId);
                    std::cout << "\nYou've learned valuable information about preventing " << diseaseId << "." << std::endl;
                }

                // Stay in this node
                outNextNode = this;
                return true;
            } else if (action == "offer_help") {
                // Create a quest to help with the epidemic
                for (const auto& rule : transitionRules) {
                    if (rule.description == "Help with Epidemic") {
                        outNextNode = rule.targetNode;
                        // Could set up a quest here
                        return true;
                    }
                }
            } else if (action == "ignore") {
                for (const auto& rule : transitionRules) {
                    if (rule.description == "Ignore Epidemic") {
                        outNextNode = rule.targetNode;
                        return true;
                    }
                }
            }
        }

        return TANode::evaluateTransition(input, outNextNode);
    }

private:
    GameContext* getGameContext()
    {
        return nullptr; // Placeholder
    }

    DiseaseManager* getDiseaseManager(GameContext* context)
    {
        return nullptr; // Placeholder
    }
};

// Implementation of HealthState member functions
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

// Implementation of Symptom::applyStatEffects
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

    // Apply each stat effect - this would need to be adapted to your stat system
    for (const auto& [statName, baseEffect] : statEffects) {
        float actualEffect = baseEffect * severityMultiplier;

        // For example, apply to character stats
        if (statName == "strength") {
            // Temporary stat reduction
            // context->playerStats.strengthModifier -= actualEffect;
        } else if (statName == "stamina") {
            // Reduce max stamina
            // context->healthContext.playerHealth.maxStamina -= actualEffect;
        }
        // Add other stats as needed
    }
}

// DiseaseManager functions
bool DiseaseManager::checkExposure(GameContext* context, const std::string& regionName, const std::string& vector)
{
    if (!context)
        return false;

    HealthState* health = &context->healthContext.playerHealth; // Adapt to your structure
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

    HealthState* health = &context->healthContext.playerHealth; // Adapt to your structure
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

    HealthState* health = &context->healthContext.playerHealth; // Adapt to your structure
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

// Main function to set up the disease and health system
void setupDiseaseHealthSystem(TAController& controller)
{
    std::cout << "Setting up Disease and Health System..." << std::endl;

    // Create root node for the health system
    HealthStateNode* healthRoot = dynamic_cast<HealthStateNode*>(
        controller.createNode<HealthStateNode>("HealthSystem"));

    // Create treatment node
    TreatmentNode* treatmentNode = dynamic_cast<TreatmentNode*>(
        controller.createNode<TreatmentNode>("TreatmentOptions"));

    // Create rest node
    RestNode* restNode = dynamic_cast<RestNode*>(
        controller.createNode<RestNode>("RestNode"));

    // Set up transitions
    healthRoot->addTransition(
        [](const TAInput& input) {
            return input.type == "health_action" && std::get<std::string>(input.parameters.at("action")) == "treat";
        },
        treatmentNode, "Treat Diseases");

    healthRoot->addTransition(
        [](const TAInput& input) {
            return input.type == "health_action" && std::get<std::string>(input.parameters.at("action")) == "rest";
        },
        restNode, "Rest");

    treatmentNode->addTransition(
        [](const TAInput& input) {
            return input.type == "treatment_action" && std::get<std::string>(input.parameters.at("action")) == "back";
        },
        healthRoot, "Back to Health");

    restNode->addTransition(
        [](const TAInput& input) {
            return input.type == "rest_action" && std::get<std::string>(input.parameters.at("action")) == "cancel";
        },
        healthRoot, "Back to Health");

    // Create disease nodes for common diseases
    DiseaseNode* commonColdNode = dynamic_cast<DiseaseNode*>(
        controller.createNode<DiseaseNode>("CommonColdDisease", "common_cold"));

    DiseaseNode* plagueNode = dynamic_cast<DiseaseNode*>(
        controller.createNode<DiseaseNode>("PlagueDisease", "black_plague"));

    DiseaseNode* feverNode = dynamic_cast<DiseaseNode*>(
        controller.createNode<DiseaseNode>("MountainFeverDisease", "mountain_fever"));

    // Create epidemic event nodes
    EpidemicNode* plagueEpidemicNode = dynamic_cast<EpidemicNode*>(
        controller.createNode<EpidemicNode>("PlagueEpidemic", "black_plague", "Village", 2.0f));

    EpidemicNode* feverEpidemicNode = dynamic_cast<EpidemicNode*>(
        controller.createNode<EpidemicNode>("FeverEpidemic", "mountain_fever", "Mountain", 1.5f));

    // Register the health system
    controller.setSystemRoot("HealthSystem", healthRoot);

    // Initialize the disease manager and health context
    // This would be integrated with your existing GameContext
    DiseaseManager diseaseManager;

    // Create diseases
    Disease commonCold("common_cold", "Common Cold");
    commonCold.description = "A mild respiratory illness characterized by sneezing, congestion, and coughing.";
    commonCold.incubationPeriod = 1;
    commonCold.naturalDuration = 5;
    commonCold.contagiousness = 0.4f;
    commonCold.resistanceThreshold = 0.1f;
    commonCold.isChronic = false;
    commonCold.addVector("air");
    commonCold.addVector("contact");
    commonCold.addRegion("Village");
    commonCold.addRegion("Town");

    // Add symptoms to common cold
    Symptom coldCough("Cough", "A persistent cough that may disrupt sleep.");
    Symptom coldCongestion("Congestion", "Nasal congestion making breathing difficult.");
    Symptom coldFatigue("Fatigue", "Mild tiredness and reduced energy.", SymptomSeverity::MILD);

    coldCough.statEffects["stamina"] = -5.0f;
    coldCongestion.statEffects["stamina"] = -3.0f;
    coldFatigue.statEffects["strength"] = -1.0f;
    coldFatigue.statEffects["stamina"] = -10.0f;

    commonCold.addSymptom(coldCough);
    commonCold.addSymptom(coldCongestion);
    commonCold.addSymptom(coldFatigue);

    // Register the disease
    diseaseManager.registerDisease(commonCold);

    // Create Black Plague
    Disease plague("black_plague", "Black Plague");
    plague.description = "A deadly disease characterized by swollen lymph nodes, fever, and often fatal if untreated.";
    plague.incubationPeriod = 3;
    plague.naturalDuration = 14;
    plague.contagiousness = 0.7f;
    plague.resistanceThreshold = 0.4f;
    plague.isChronic = false;
    plague.addVector("air");
    plague.addVector("contact");
    plague.addVector("vermin");
    plague.addRegion("Village");
    plague.addRegion("Town");
    plague.addRegion("Slums");

    // Add symptoms to plague
    Symptom plagueFever("High Fever", "Dangerously elevated body temperature.");
    Symptom plagueBuboes("Buboes", "Painful, swollen lymph nodes.");
    Symptom plagueWeakness("Extreme Weakness", "Severe fatigue and inability to perform physical tasks.");

    plagueFever.statEffects["constitution"] = -10.0f;
    plagueFever.statEffects["stamina"] = -15.0f;
    plagueBuboes.statEffects["dexterity"] = -8.0f;
    plagueBuboes.statEffects["strength"] = -5.0f;
    plagueWeakness.statEffects["strength"] = -15.0f;
    plagueWeakness.statEffects["stamina"] = -25.0f;

    // Add custom effects
    plagueFever.onUpdateEffect = [](GameContext* context) {
        // Apply damage over time from fever
        if (context) {
            HealthState* health = &context->healthContext.playerHealth;
            if (health) {
                health->takeDamage(0.5f); // Damage per update
            }
        }
    };

    plague.addSymptom(plagueFever);
    plague.addSymptom(plagueBuboes);
    plague.addSymptom(plagueWeakness);

    // Register the plague
    diseaseManager.registerDisease(plague);

    // Create Mountain Fever
    Disease mountainFever("mountain_fever", "Mountain Fever");
    mountainFever.description = "A severe illness contracted in mountainous regions, causing high fever and delirium.";
    mountainFever.incubationPeriod = 2;
    mountainFever.naturalDuration = 10;
    mountainFever.contagiousness = 0.3f;
    mountainFever.resistanceThreshold = 0.3f;
    mountainFever.isChronic = false;
    mountainFever.addVector("water");
    mountainFever.addVector("insect");
    mountainFever.addRegion("Mountain");
    mountainFever.addRegion("Forest");

    // Add symptoms to mountain fever
    Symptom feverChills("Chills", "Uncontrollable shivering despite high body temperature.");
    Symptom feverDelirium("Delirium", "Confusion and hallucinations due to high fever.");
    Symptom feverJointPain("Joint Pain", "Severe pain in joints making movement difficult.");

    feverChills.statEffects["constitution"] = -5.0f;
    feverChills.statEffects["dexterity"] = -3.0f;
    feverDelirium.statEffects["intelligence"] = -10.0f;
    feverDelirium.statEffects["wisdom"] = -8.0f;
    feverJointPain.statEffects["dexterity"] = -12.0f;
    feverJointPain.statEffects["strength"] = -7.0f;

    mountainFever.addSymptom(feverChills);
    mountainFever.addSymptom(feverDelirium);
    mountainFever.addSymptom(feverJointPain);

    // Register the mountain fever
    diseaseManager.registerDisease(mountainFever);

    // Create healing methods
    HealingMethod restMethod("rest_healing", "Bed Rest");
    restMethod.description = "Simply resting in a comfortable bed helps the body recover naturally.";
    restMethod.setEffectiveness("common_cold", 0.4f);
    restMethod.setEffectiveness("mountain_fever", 0.2f);
    restMethod.setEffectiveness("black_plague", 0.05f);

    // Requirement check and effect application
    restMethod.requirementCheck = [](GameContext* context) {
        // Check if in a suitable location for rest
        // For example, check if in an inn or player home
        return true; // Simplified for example
    };

    restMethod.applyEffect = [](GameContext* context, const std::string& diseaseId) {
        if (!context)
            return;

        HealthState* health = &context->healthContext.playerHealth;
        if (!health)
            return;

        // Improve health
        health->heal(10.0f);

        // Chance to recover based on disease
        float recoveryChance = 0.0f;

        if (diseaseId == "common_cold") {
            recoveryChance = 0.4f;
        } else if (diseaseId == "mountain_fever") {
            recoveryChance = 0.2f;
        } else if (diseaseId == "black_plague") {
            recoveryChance = 0.05f;
        }

        float roll = static_cast<float>(rand()) / RAND_MAX;
        if (roll < recoveryChance) {
            health->recoverFromDisease(diseaseId, context->worldState.daysPassed, true);
        } else {
            std::cout << "The bed rest helped, but you're still suffering from the illness." << std::endl;
        }
    };

    diseaseManager.registerHealingMethod(restMethod);

    // Create herb potion method
    HealingMethod herbPotion("herb_potion", "Herbal Remedy");
    herbPotion.description = "A medicinal tea made from various healing herbs.";
    herbPotion.setEffectiveness("common_cold", 0.6f);
    herbPotion.setEffectiveness("mountain_fever", 0.4f);
    herbPotion.setEffectiveness("black_plague", 0.1f);

    herbPotion.requirementCheck = [](GameContext* context) {
        // Check if player has the necessary herbs
        if (!context)
            return false;

        return context->playerInventory.hasItem("medicinal_herbs", 2);
    };

    herbPotion.applyEffect = [](GameContext* context, const std::string& diseaseId) {
        if (!context)
            return;

        // Consume herbs
        context->playerInventory.removeItem("medicinal_herbs", 2);

        HealthState* health = &context->healthContext.playerHealth;
        if (!health)
            return;

        // Improve health
        health->heal(15.0f);

        // Chance to recover based on disease
        float recoveryChance = 0.0f;

        if (diseaseId == "common_cold") {
            recoveryChance = 0.6f;
        } else if (diseaseId == "mountain_fever") {
            recoveryChance = 0.4f;
        } else if (diseaseId == "black_plague") {
            recoveryChance = 0.1f;
        }

        float roll = static_cast<float>(rand()) / RAND_MAX;
        if (roll < recoveryChance) {
            health->recoverFromDisease(diseaseId, context->worldState.daysPassed, true);
        } else {
            std::cout << "The herbal remedy provided some relief, but hasn't cured you completely." << std::endl;
        }
    };

    diseaseManager.registerHealingMethod(herbPotion);

    // Create temple healing method
    HealingMethod templeHealing("temple_healing", "Temple Healing");
    templeHealing.description = "Healing rituals performed by priests or clerics at temples.";
    templeHealing.setEffectiveness("common_cold", 0.8f);
    templeHealing.setEffectiveness("mountain_fever", 0.7f);
    templeHealing.setEffectiveness("black_plague", 0.5f);

    templeHealing.requirementCheck = [](GameContext* context) {
        // Check if at a temple location
        if (!context)
            return false;

        // This would check location in your actual implementation
        return true;
    };

    templeHealing.applyEffect = [](GameContext* context, const std::string& diseaseId) {
        if (!context)
            return;

        HealthState* health = &context->healthContext.playerHealth;
        if (!health)
            return;

        // Healing cost (gold)
        int cost = 0;
        if (diseaseId == "common_cold")
            cost = 20;
        else if (diseaseId == "mountain_fever")
            cost = 50;
        else if (diseaseId == "black_plague")
            cost = 100;

        // Would need to check/remove gold in implementation

        // Improve health
        health->heal(30.0f);

        // Chance to recover based on disease
        float recoveryChance = 0.0f;

        if (diseaseId == "common_cold") {
            recoveryChance = 0.8f;
        } else if (diseaseId == "mountain_fever") {
            recoveryChance = 0.7f;
        } else if (diseaseId == "black_plague") {
            recoveryChance = 0.5f;
        }

        float roll = static_cast<float>(rand()) / RAND_MAX;
        if (roll < recoveryChance) {
            health->recoverFromDisease(diseaseId, context->worldState.daysPassed, true);
            std::cout << "The temple ritual was successful! You have been cured." << std::endl;
        } else {
            std::cout << "The temple healing has improved your condition, but the disease persists." << std::endl;
        }
    };

    diseaseManager.registerHealingMethod(templeHealing);

    // Create alchemical cure method
    HealingMethod alchemyCure("alchemy_cure", "Alchemical Remedy");
    alchemyCure.description = "A powerful alchemical potion that can cure most diseases.";
    alchemyCure.setEffectiveness("common_cold", 0.9f);
    alchemyCure.setEffectiveness("mountain_fever", 0.8f);
    alchemyCure.setEffectiveness("black_plague", 0.7f);

    alchemyCure.requirementCheck = [](GameContext* context) {
        // Check if player has rare healing potion
        if (!context)
            return false;

        return context->playerInventory.hasItem("strong_healing_potion", 1);
    };

    alchemyCure.applyEffect = [](GameContext* context, const std::string& diseaseId) {
        if (!context)
            return;

        // Consume potion
        context->playerInventory.removeItem("strong_healing_potion", 1);

        HealthState* health = &context->healthContext.playerHealth;
        if (!health)
            return;

        // Improve health
        health->heal(50.0f);

        // Chance to recover based on disease
        float recoveryChance = 0.0f;

        if (diseaseId == "common_cold") {
            recoveryChance = 0.9f;
        } else if (diseaseId == "mountain_fever") {
            recoveryChance = 0.8f;
        } else if (diseaseId == "black_plague") {
            recoveryChance = 0.7f;
        }

        float roll = static_cast<float>(rand()) / RAND_MAX;
        if (roll < recoveryChance) {
            health->recoverFromDisease(diseaseId, context->worldState.daysPassed, true);
            std::cout << "The potent alchemical remedy has purged the disease from your body!" << std::endl;
        } else {
            std::cout << "Despite the powerful remedy, traces of the disease remain in your system." << std::endl;
        }
    };

    diseaseManager.registerHealingMethod(alchemyCure);

    // Set region disease risks
    diseaseManager.setRegionRisk("Village", 1.0f);
    diseaseManager.setRegionRisk("Town", 1.2f);
    diseaseManager.setRegionRisk("City", 1.5f);
    diseaseManager.setRegionRisk("Slums", 2.0f);
    diseaseManager.setRegionRisk("Forest", 0.8f);
    diseaseManager.setRegionRisk("Mountain", 1.0f);
    diseaseManager.setRegionRisk("Desert", 0.5f);
    diseaseManager.setRegionRisk("Swamp", 1.8f);

    std::cout << "Disease and Health System initialized with "
              << diseaseManager.diseases.size() << " diseases and "
              << diseaseManager.healingMethods.size() << " healing methods." << std::endl;

    // In a full implementation, you would attach the diseaseManager to your game context
    // context.healthContext.diseaseManager = diseaseManager;
}

// Example usage of the disease system in a game loop
void exampleDiseaseSystemUsage(TAController& controller)
{
    GameContext* context = &controller.gameContext;

    // Initialize your health context and disease manager in your game context
    // This would need to be adapted to your game structure
    setupDiseaseHealthSystem(controller);

    // Example of checking for disease exposure during travel
    std::cout << "\n=== TRAVELING TO A NEW REGION ===\n"
              << std::endl;
    std::cout << "You are traveling to the Mountain region..." << std::endl;

    // Simulate travel to mountain region
    // This would call your disease manager's checkExposure method
    // diseaseManager.checkExposure(context, "Mountain", "air");

    // Example of disease progression during rest
    std::cout << "\n=== RESTING WHILE DISEASED ===\n"
              << std::endl;
    std::cout << "You decide to rest at an inn..." << std::endl;

    // Simulate resting
    // This would call your RestNode's applyRest method
    // restNode->applyRest(8, context);

    // Example of treating a disease
    std::cout << "\n=== TREATING A DISEASE ===\n"
              << std::endl;
    std::cout << "You visit a healer to treat your Mountain Fever..." << std::endl;

    // Simulate using a healing method
    // const HealingMethod* method = diseaseManager.getHealingMethodById("temple_healing");
    // if (method) method->apply(context, "mountain_fever");

    // Example of an epidemic event
    std::cout << "\n=== EPIDEMIC EVENT ===\n"
              << std::endl;
    std::cout << "News spreads of a plague outbreak in the nearby town..." << std::endl;

    // This would be triggered by entering the epidemic node
    // controller.processInput("HealthSystem", {});

    std::cout << "\nDisease and Health System demo complete." << std::endl;
}

// Main function to demonstrate the disease system
int main()
{
    // This would be integrated with your existing main function
    std::cout << "Disease and Health System for Daggerfall-like RPG\n"
              << std::endl;

    TAController controller;
    // Set up your existing systems

    // Set up disease system
    setupDiseaseHealthSystem(controller);

    // Show example usage
    exampleDiseaseSystemUsage(controller);

    return 0;
}