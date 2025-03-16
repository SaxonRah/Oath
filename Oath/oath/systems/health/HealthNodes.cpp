// /oath/systems/health/HealthNodes.cpp
#include "HealthNodes.hpp"
#include "DiseaseManager.hpp"
#include "HealthState.hpp"
#include <iostream>

// HealthStateNode implementation
HealthStateNode::HealthStateNode(const std::string& name)
    : TANode(name)
{
}

void HealthStateNode::onEnter(GameContext* context) override
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

std::vector<TAAction> HealthStateNode::getAvailableActions()
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

bool HealthStateNode::evaluateTransition(const TAInput& input, TANode*& outNextNode)
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

HealthState* HealthStateNode::getHealthState(GameContext* context)
{
    if (!context)
        return nullptr;
    return &context->healthContext.playerHealth;
}

DiseaseManager* HealthStateNode::getDiseaseManager(GameContext* context)
{
    if (!context)
        return nullptr;
    return &context->diseaseManager;
}

// DiseaseNode implementation
DiseaseNode::DiseaseNode(const std::string& name, const std::string& id)
    : TANode(name)
    , diseaseId(id)
{
}

void DiseaseNode::onEnter(GameContext* context)
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

std::vector<TAAction> DiseaseNode::getAvailableActions()
{
    std::vector<TAAction> actions = TANode::getAvailableActions();

    actions.push_back({ "treat_disease", "Treat This Disease",
        [this]() -> TAInput {
            return { "disease_action", { { "action", std::string("treat") }, { "disease_id", diseaseId } } };
        } });

    return actions;
}

bool DiseaseNode::evaluateTransition(const TAInput& input, TANode*& outNextNode)
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

HealthState* DiseaseNode::getHealthState(GameContext* context)
{
    if (!context)
        return nullptr;
    return &context->healthContext.playerHealth;
}

DiseaseManager* DiseaseNode::getDiseaseManager(GameContext* context)
{
    if (!context)
        return nullptr;
    return &context->diseaseManager;
}

// TreatmentNode implementation
TreatmentNode::TreatmentNode(const std::string& name, const std::string& diseaseId)
    : TANode(name)
    , targetDiseaseId(diseaseId)
{
}

void TreatmentNode::onEnter(GameContext* context)
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

std::vector<TAAction> TreatmentNode::getAvailableActions()
{
    std::vector<TAAction> actions = TANode::getAvailableActions();

    if (targetDiseaseId.empty()) {
        // Add actions for each disease
        actions.push_back({ "select_disease", "Select Disease to Treat",
            []() -> TAInput {
                return { "treatment_action", {
                                                 { "action", std::string("select_disease") }, { "index", 0 } // Set by UI
                                             } };
            } });
    } else {
        // Add actions for each method
        actions.push_back({ "apply_treatment", "Apply Treatment",
            [this]() -> TAInput {
                return { "treatment_action", {
                                                 { "action", std::string("apply_treatment") }, { "disease_id", targetDiseaseId }, { "method_id", std::string("") } // Set by UI
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

bool TreatmentNode::evaluateTransition(const TAInput& input, TANode*& outNextNode)
{
    if (input.type == "treatment_action") {
        std::string action = std::get<std::string>(input.parameters.at("action"));

        if (action == "select_disease") {
            int index = std::get<int>(input.parameters.at("index"));
            // Navigate to specific disease treatment node
            // Implementation depends on node structure
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

GameContext* TreatmentNode::getGameContext()
{
    // In a real implementation, this would access the context from somewhere
    return nullptr; // Placeholder
}

HealthState* TreatmentNode::getHealthState(GameContext* context)
{
    if (!context)
        return nullptr;
    return &context->healthContext.playerHealth;
}

DiseaseManager* TreatmentNode::getDiseaseManager(GameContext* context)
{
    if (!context)
        return nullptr;
    return &context->diseaseManager;
}

// RestNode implementation
RestNode::RestNode(const std::string& name)
    : TANode(name)
{
}

void RestNode::onEnter(GameContext* context)
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

std::vector<TAAction> RestNode::getAvailableActions()
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

bool RestNode::evaluateTransition(const TAInput& input, TANode*& outNextNode)
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

void RestNode::applyRest(int hours, GameContext* context)
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

GameContext* RestNode::getGameContext()
{
    // Return the current game context - implementation depends on your system
    return nullptr; // Placeholder
}

HealthState* RestNode::getHealthState(GameContext* context)
{
    if (!context)
        return nullptr;
    return &context->healthContext.playerHealth;
}

DiseaseManager* RestNode::getDiseaseManager(GameContext* context)
{
    if (!context)
        return nullptr;
    return &context->diseaseManager;
}

// EpidemicNode implementation
EpidemicNode::EpidemicNode(const std::string& name, const std::string& disease,
    const std::string& region, float severity)
    : TANode(name)
    , diseaseId(disease)
    , regionName(region)
    , severityMultiplier(severity)
{
}

void EpidemicNode::onEnter(GameContext* context)
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

std::vector<TAAction> EpidemicNode::getAvailableActions()
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

bool EpidemicNode::evaluateTransition(const TAInput& input, TANode*& outNextNode)
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

GameContext* EpidemicNode::getGameContext()
{
    return nullptr; // Placeholder
}

DiseaseManager* EpidemicNode::getDiseaseManager(GameContext* context)
{
    if (!context)
        return nullptr;
    return &context->diseaseManager;
}