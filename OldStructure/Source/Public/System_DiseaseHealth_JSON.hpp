// System_DiseaseHealth_JSON.hpp
#ifndef SYSTEM_DISEASE_HEALTH_JSON_HPP
#define SYSTEM_DISEASE_HEALTH_JSON_HPP

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

// Enum for Symptom Severity
enum class SymptomSeverity {
    NONE,
    MILD,
    MODERATE,
    SEVERE,
    CRITICAL
};

// Forward declarations for health system
class Disease;
class DiseaseManager;
class DiseaseNode;
class HealthState;
class HealingMethod;
class Symptom;
class Immunity;

// Helper functions for SymptomSeverity
SymptomSeverity stringToSeverity(const std::string& severityStr);
std::string severityToString(SymptomSeverity severity);

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
        SymptomSeverity initialSeverity = SymptomSeverity::MILD);

    // Constructor from JSON
    Symptom(const json& symptomJson);

    void increaseSeverity();
    void decreaseSeverity();
    std::string getSeverityString() const;
    void applyStatEffects(GameContext* context) const;
    void update(GameContext* context);
};

// Class to represent an immunity to a disease
class Immunity {
public:
    std::string diseaseId;
    float strength; // 0.0 to 1.0, where 1.0 is full immunity
    int durationDays; // How long the immunity lasts, -1 for permanent
    int dayAcquired; // Day the immunity was acquired

    Immunity(const std::string& disease, float immuneStrength, int duration, int currentDay);

    bool isActive(int currentDay) const;
    float getEffectiveStrength(int currentDay) const;
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

    Disease(const std::string& diseaseId, const std::string& diseaseName);

    // Constructor from JSON
    Disease(const json& diseaseJson);

    void addSymptom(const Symptom& symptom);
    void addRegion(const std::string& region);
    void addVector(const std::string& vector);
    bool transmitsVia(const std::string& vector) const;
    bool isCommonIn(const std::string& region) const;
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

    HealingMethod(const std::string& methodId, const std::string& methodName);

    // Constructor from JSON
    HealingMethod(const json& methodJson);

    void setupRequirementCheck();
    void setupApplyEffect();
    void setEffectiveness(const std::string& diseaseId, float effectiveness);
    float getEffectivenessAgainst(const std::string& diseaseId) const;
    bool canBeUsed(GameContext* context) const;
    void apply(GameContext* context, const std::string& diseaseId);
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

    HealthState();

    // Initialize from JSON
    void initFromJson(const json& healthJson);

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

// Health system nodes
class HealthStateNode : public TANode {
public:
    HealthStateNode(const std::string& name);
    void onEnter(GameContext* context) override;
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;

private:
    HealthState* getHealthState(GameContext* context);
    DiseaseManager* getDiseaseManager(GameContext* context);
};

class DiseaseNode : public TANode {
public:
    std::string diseaseId;

    DiseaseNode(const std::string& name, const std::string& id);
    void onEnter(GameContext* context) override;
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;

private:
    HealthState* getHealthState(GameContext* context);
    DiseaseManager* getDiseaseManager(GameContext* context);
};

class TreatmentNode : public TANode {
public:
    std::string targetDiseaseId; // Optional, if treating a specific disease

    TreatmentNode(const std::string& name, const std::string& diseaseId = "");
    void onEnter(GameContext* context) override;
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;

private:
    GameContext* getGameContext();
    HealthState* getHealthState(GameContext* context);
    DiseaseManager* getDiseaseManager(GameContext* context);
};

class RestNode : public TANode {
public:
    RestNode(const std::string& name);
    void onEnter(GameContext* context) override;
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;
    void applyRest(int hours, GameContext* context);

private:
    GameContext* getGameContext();
    HealthState* getHealthState(GameContext* context);
    DiseaseManager* getDiseaseManager(GameContext* context);
};

class EpidemicNode : public TANode {
public:
    std::string diseaseId;
    std::string regionName;
    float severityMultiplier;

    EpidemicNode(const std::string& name, const std::string& disease,
        const std::string& region, float severity = 1.0f);
    void onEnter(GameContext* context) override;
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;

private:
    GameContext* getGameContext();
    DiseaseManager* getDiseaseManager(GameContext* context);
};

// Setup functions
void setupDiseaseHealthSystem(TAController& controller);
void exampleDiseaseSystemUsage(TAController& controller);

#endif // SYSTEM_DISEASE_HEALTH_JSON_HPP