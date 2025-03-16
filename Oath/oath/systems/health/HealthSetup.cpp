// /oath/systems/health/HealthSetup.cpp
#include "../../core/TAController.hpp"
#include "DiseaseManager.hpp"
#include "HealthNodes.hpp"
#include <iostream>


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
    if (!diseaseManager.loadFromJson("resources/json/diseases.json")) {
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
    if (&controller.gameContext.healthContext.playerHealth) {
        std::ifstream file("resources/json/diseases.json");
        if (file.is_open()) {
            nlohmann::json j;
            file >> j;
            file.close();

            controller.gameContext.healthContext.playerHealth.initFromJson(j["defaultHealthState"]);
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