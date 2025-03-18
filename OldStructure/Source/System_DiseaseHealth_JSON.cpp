// System_DiseaseHealth_JSON.cpp

#include "System_DiseaseHealth_JSON.hpp"

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

#include <nlohmann/json.hpp>
using json = nlohmann::json;

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

// Class to represent a symptom of a disease
class Symptom {
public:
    std::string name;
    std::string description;
    SymptomSeverity severity;
    std::map<std::string, float> statEffects; // Affects player stats (strength, speed, etc.)
    std::function<void(GameContext*)> onUpdateEffect; // Effect applied during symptoms update
    bool hasDamageOverTime;
    float damagePerUpdate;

    Symptom(const std::string& symptomName, const std::string& desc,
        SymptomSeverity initialSeverity = SymptomSeverity::MILD)
        : name(symptomName)
        , description(desc)
        , severity(initialSeverity)
        , hasDamageOverTime(false)
        , damagePerUpdate(0.0f)
    {
    }

    // Constructor from JSON
    Symptom(const json& symptomJson)
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

    // Constructor from JSON
    Disease(const json& diseaseJson)
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
    float healAmount;
    bool requiresItem;
    std::string requiredItem;
    int requiredAmount;
    bool requiresLocation;
    std::string requiredLocationType;
    std::map<std::string, int> costs; // Disease ID to cost

    HealingMethod(const std::string& methodId, const std::string& methodName)
        : id(methodId)
        , name(methodName)
        , healAmount(0.0f)
        , requiresItem(false)
        , requiredAmount(0)
        , requiresLocation(false)
    {
    }

    // Constructor from JSON
    HealingMethod(const json& methodJson)
        : id(methodJson["id"])
        , name(methodJson["name"])
        , description(methodJson["description"])
        , healAmount(methodJson["healAmount"])
        , requiresItem(methodJson["requiresItem"])
        , requiredAmount(0)
        , requiresLocation(methodJson.value("requiresLocation", false))
    {
        // Load effectiveness
        for (auto& [disease, effect] : methodJson["effectiveness"].items()) {
            effectivenessAgainstDisease[disease] = effect;
        }

        // Load item requirements if needed
        if (requiresItem) {
            requiredItem = methodJson["requiredItem"];
            requiredAmount = methodJson["requiredAmount"];
        }

        // Load location requirements if needed
        if (requiresLocation) {
            requiredLocationType = methodJson["requiredLocationType"];
        }

        // Load costs if present
        if (methodJson.contains("costs")) {
            for (auto& [disease, cost] : methodJson["costs"].items()) {
                costs[disease] = cost;
            }
        }

        // Set up requirement check based on JSON
        setupRequirementCheck();

        // Set up apply effect based on JSON
        setupApplyEffect();
    }

    void setupRequirementCheck()
    {
        requirementCheck = [this](GameContext* context) {
            if (!context)
                return false;

            // Check item requirements
            if (requiresItem) {
                if (!context->playerInventory.hasItem(requiredItem, requiredAmount)) {
                    std::cout << "You need " << requiredAmount << " " << requiredItem << " to use this treatment." << std::endl;
                    return false;
                }
            }

            // Check location requirements
            if (requiresLocation) {
                // This would check if player is in the right location type
                // For demo purposes, always return true
                return true;
            }

            return true;
        };
    }

    void setupApplyEffect()
    {
        applyEffect = [this](GameContext* context, const std::string& diseaseId) {
            if (!context)
                return;

            HealthState* health = &context->healthContext.playerHealth;
            if (!health)
                return;

            // Consume required items
            if (requiresItem) {
                context->playerInventory.removeItem(requiredItem, requiredAmount);
                std::cout << "Used " << requiredAmount << " " << requiredItem << " for treatment." << std::endl;
            }

            // Handle cost if applicable
            if (costs.find(diseaseId) != costs.end()) {
                int cost = costs[diseaseId];
                // Would deduct gold here
                std::cout << "Paid " << cost << " gold for treatment." << std::endl;
            }

            // Apply healing
            health->heal(healAmount);
            std::cout << "Recovered " << healAmount << " health points." << std::endl;

            // Chance to recover based on effectiveness
            float recoveryChance = getEffectivenessAgainst(diseaseId);
            float roll = static_cast<float>(rand()) / RAND_MAX;

            if (roll < recoveryChance) {
                health->recoverFromDisease(diseaseId, context->worldState.daysPassed, true);
                std::cout << "The treatment was successful! You have recovered from " << diseaseId << "." << std::endl;
            } else {
                std::cout << "The treatment provided some relief, but hasn't cured you completely." << std::endl;
            }
        };
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

    // Initialize from JSON
    void initFromJson(const json& healthJson)
    {
        maxHealth = healthJson["maxHealth"];
        currentHealth = maxHealth;
        maxStamina = healthJson["maxStamina"];
        stamina = maxStamina;
        naturalHealRate = healthJson["naturalHealRate"];
        diseaseResistance = healthJson["diseaseResistance"];
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

    // Load all data from JSON file
    bool loadFromJson(const std::string& filename)
    {
        try {
            // Read JSON file
            std::ifstream file(filename);
            if (!file.is_open()) {
                std::cerr << "Failed to open file: " << filename << std::endl;
                return false;
            }

            json j;
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
        if (!context)
            return nullptr;
        return &context->healthContext.playerHealth;
    }

    DiseaseManager* getDiseaseManager(GameContext* context)
    {
        if (!context)
            return nullptr;
        return &context->diseaseManager;
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
        if (!context)
            return nullptr;
        return &context->healthContext.playerHealth;
    }

    DiseaseManager* getDiseaseManager(GameContext* context)
    {
        if (!context)
            return nullptr;
        return &context->diseaseManager;
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
                GameContext* context = getGameContext();
                if (context) {
                    DiseaseManager* manager = getDiseaseManager(context);
                    if (manager) {
                        const HealingMethod* method = manager->getHealingMethodById(methodId);
                        if (method) {
                            method->apply(context, diseaseId);
                        }
                    }
                }
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
    GameContext* getGameContext()
    {
        // In a real implementation, you'd access the context from somewhere
        return nullptr; // Placeholder
    }

    HealthState* getHealthState(GameContext* context)
    {
        if (!context)
            return nullptr;
        return &context->healthContext.playerHealth;
    }

    DiseaseManager* getDiseaseManager(GameContext* context)
    {
        if (!context)
            return nullptr;
        return &context->diseaseManager;
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
        if (!context)
            return nullptr;
        return &context->healthContext.playerHealth;
    }

    DiseaseManager* getDiseaseManager(GameContext* context)
    {
        if (!context)
            return nullptr;
        return &context->diseaseManager;
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
        if (!context)
            return nullptr;
        return &context->diseaseManager;
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
            context->playerStats.modifiers["strength"] -= actualEffect;
        } else if (statName == "stamina") {
            // Reduce max stamina
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

    // Initialize the disease manager
    DiseaseManager diseaseManager;

    // Load disease data from JSON
    if (!diseaseManager.loadFromJson("DiseaseHealth.json")) {
        std::cerr << "Failed to load disease system data. Using default values." << std::endl;

        // You could add fallback hardcoded values here if needed
    }

    // Create disease nodes for each disease dynamically
    for (const auto& [diseaseId, disease] : diseaseManager.diseases) {
        DiseaseNode* diseaseNode = dynamic_cast<DiseaseNode*>(
            controller.createNode<DiseaseNode>(diseaseId + "Disease", diseaseId));

        // Add transitions for this disease node
        healthRoot->addTransition(
            [diseaseId](const TAInput& input) {
                return input.type == "view_disease" && std::get<std::string>(input.parameters.at("disease_id")) == diseaseId;
            },
            diseaseNode, "View " + disease.name);

        diseaseNode->addTransition(
            [](const TAInput& input) {
                return input.type == "back_to_health";
            },
            healthRoot, "Back to Health Menu");
    }

    // Create epidemic event nodes based on JSON data
    // For demonstration, we'll create epidemic nodes for a couple of diseases
    if (diseaseManager.diseases.find("black_plague") != diseaseManager.diseases.end()) {
        EpidemicNode* plagueEpidemicNode = dynamic_cast<EpidemicNode*>(
            controller.createNode<EpidemicNode>("PlagueEpidemic", "black_plague", "Village", 2.0f));

        // Add transitions for epidemic node
        plagueEpidemicNode->addTransition(
            [](const TAInput& input) {
                return input.type == "epidemic_action" && std::get<std::string>(input.parameters.at("action")) == "ignore";
            },
            healthRoot, "Ignore Epidemic");
    }

    if (diseaseManager.diseases.find("mountain_fever") != diseaseManager.diseases.end()) {
        EpidemicNode* feverEpidemicNode = dynamic_cast<EpidemicNode*>(
            controller.createNode<EpidemicNode>("FeverEpidemic", "mountain_fever", "Mountain", 1.5f));

        // Add transitions for epidemic node
        feverEpidemicNode->addTransition(
            [](const TAInput& input) {
                return input.type == "epidemic_action" && std::get<std::string>(input.parameters.at("action")) == "ignore";
            },
            healthRoot, "Ignore Epidemic");
    }

    // Register the health system
    controller.setSystemRoot("HealthSystem", healthRoot);

    // Initialize player health state from JSON
    if (controller.gameContext.healthContext.playerHealth) {
        std::ifstream file("DiseaseHealth.json");
        if (file.is_open()) {
            json j;
            file >> j;
            file.close();

            controller.gameContext.healthContext.playerHealth->initFromJson(j["defaultHealthState"]);
        }
    }

    // Add disease manager to game context
    controller.gameContext.diseaseManager = diseaseManager;

    std::cout << "Disease and Health System initialized with "
              << diseaseManager.diseases.size() << " diseases and "
              << diseaseManager.healingMethods.size() << " healing methods." << std::endl;
}

// Example usage of the disease system in a game loop
void exampleDiseaseSystemUsage(TAController& controller)
{
    GameContext* context = &controller.gameContext;

    // Initialize your health context and disease manager in your game context
    setupDiseaseHealthSystem(controller);

    // Example of checking for disease exposure during travel
    std::cout << "\n=== TRAVELING TO A NEW REGION ===\n"
              << std::endl;
    std::cout << "You are traveling to the Mountain region..." << std::endl;

    // Simulate travel to mountain region
    context->diseaseManager.checkExposure(context, "Mountain", "air");

    // Example of disease progression during rest
    std::cout << "\n=== RESTING WHILE DISEASED ===\n"
              << std::endl;
    std::cout << "You decide to rest at an inn..." << std::endl;

    // Get the rest node and simulate resting
    RestNode* restNode = dynamic_cast<RestNode*>(controller.getNode("RestNode"));
    if (restNode) {
        restNode->applyRest(8, context);
    }

    // Example of treating a disease
    std::cout << "\n=== TREATING A DISEASE ===\n"
              << std::endl;
    std::cout << "You visit a healer to treat your Mountain Fever..." << std::endl;

    // Simulate using a healing method
    const HealingMethod* method = context->diseaseManager.getHealingMethodById("temple_healing");
    if (method)
        method->apply(context, "mountain_fever");

    // Example of an epidemic event
    std::cout << "\n=== EPIDEMIC EVENT ===\n"
              << std::endl;
    std::cout << "News spreads of a plague outbreak in the nearby town..." << std::endl;

    // Simulate epidemic event
    EpidemicNode* epidemicNode = dynamic_cast<EpidemicNode*>(controller.getNode("PlagueEpidemic"));
    if (epidemicNode) {
        epidemicNode->onEnter(context);
    }

    std::cout << "\nDisease and Health System demo complete." << std::endl;
}

// Main function to demonstrate the disease system
int main()
{
    // This would be integrated with your existing main function
    std::cout << "Disease and Health System for Oath RPG\n"
              << std::endl;

    TAController controller;
    // Set up your existing systems

    // Set up disease system
    setupDiseaseHealthSystem(controller);

    // Show example usage
    exampleDiseaseSystemUsage(controller);

    return 0;
}