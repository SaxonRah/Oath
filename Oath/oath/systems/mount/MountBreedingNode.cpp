#include "MountBreedingNode.hpp"
#include "../../core/TAAction.hpp"
#include "../../core/TAInput.hpp"
#include "../../data/GameContext.hpp"
#include "Mount.hpp"
#include "MountBreed.hpp"
#include "MountSystemConfig.hpp"
#include <iostream>
#include <nlohmann/json.hpp>
#include <random>


MountBreedingNode::MountBreedingNode(const std::string& name, const std::string& center, int fee, MountSystemConfig* cfg)
    : TANode(name)
    , centerName(center)
    , breedingFee(fee)
    , config(cfg)
{
}

void MountBreedingNode::setConfig(MountSystemConfig* cfg)
{
    config = cfg;
}

MountBreedingNode* MountBreedingNode::createFromJson(const std::string& name, const nlohmann::json& j,
    std::map<std::string, MountBreed*>& breedTypes, MountSystemConfig& config)
{
    std::string centerName = j["name"];
    int fee = j["fee"];

    MountBreedingNode* node = new MountBreedingNode(name, centerName, fee, &config);

    // Create breeding stock from templates
    if (j.contains("breedingStock") && j["breedingStock"].is_array()) {
        for (const auto& mountTemplate : j["breedingStock"]) {
            std::string breedId = mountTemplate["templateId"];

            // Find the breed
            if (breedTypes.find(breedId) != breedTypes.end()) {
                MountBreed* breed = breedTypes[breedId];
                Mount* mount = Mount::createFromTemplate(mountTemplate, breed, config);

                if (mount) {
                    node->availableForBreeding.push_back(mount);
                }
            }
        }
    }

    return node;
}

void MountBreedingNode::onEnter(GameContext* context)
{
    std::cout << "Welcome to the " << centerName << " Breeding Center!" << std::endl;
    std::cout << "Breeding Fee: " << breedingFee << " gold" << std::endl;

    if (availableForBreeding.empty()) {
        std::cout << "We currently have no mounts available for breeding." << std::endl;
        return;
    }

    std::cout << "\nMounts Available for Breeding:" << std::endl;
    for (size_t i = 0; i < availableForBreeding.size(); i++) {
        Mount* mount = availableForBreeding[i];
        std::cout << i + 1 << ". " << mount->name << " (" << mount->breed->name << ")" << std::endl;
        std::cout << "   Age: " << (mount->age / 12) << " years, "
                  << (mount->age % 12) << " months" << std::endl;

        // Show special traits
        std::cout << "   Notable Traits: ";
        bool hasTraits = false;

        if (mount->stats.canJump) {
            std::cout << "Jumping";
            hasTraits = true;
        }

        if (mount->stats.canSwim) {
            if (hasTraits)
                std::cout << ", ";
            std::cout << "Swimming";
            hasTraits = true;
        }

        if (mount->stats.canClimb) {
            if (hasTraits)
                std::cout << ", ";
            std::cout << "Climbing";
            hasTraits = true;
        }

        for (const auto& [skill, level] : mount->stats.specialTraining) {
            if (level >= 70) {
                if (hasTraits)
                    std::cout << ", ";
                std::cout << "High " << skill;
                hasTraits = true;
            }
        }

        if (!hasTraits) {
            std::cout << "None";
        }

        std::cout << std::endl;
    }
}

Mount* MountBreedingNode::breedMounts(Mount* playerMount, Mount* centerMount, const std::string& foalName)
{
    if (!playerMount || !centerMount || !config)
        return nullptr;

    // Create new mount breed object if needed (hybrid of parents)
    std::string hybridBreedId = playerMount->breed->id + "_" + centerMount->breed->id;
    std::string hybridBreedName = playerMount->breed->name + "-" + centerMount->breed->name + " Cross";

    MountBreed* hybridBreed = new MountBreed(hybridBreedId, hybridBreedName);

    // Inherit base stats from parents (average with slight randomization)
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> variationDist(-10, 10);

    hybridBreed->baseSpeed = (playerMount->breed->baseSpeed + centerMount->breed->baseSpeed) / 2 + variationDist(gen);
    hybridBreed->baseStamina = (playerMount->breed->baseStamina + centerMount->breed->baseStamina) / 2 + variationDist(gen);
    hybridBreed->baseCarryCapacity = (playerMount->breed->baseCarryCapacity + centerMount->breed->baseCarryCapacity) / 2 + variationDist(gen);
    hybridBreed->baseTrainability = (playerMount->breed->baseTrainability + centerMount->breed->baseTrainability) / 2 + variationDist(gen);

    // Natural abilities have a chance to be inherited
    std::uniform_int_distribution<> inheritDist(1, 100);
    hybridBreed->naturalSwimmer = (playerMount->breed->naturalSwimmer || centerMount->breed->naturalSwimmer) && inheritDist(gen) <= 70;
    hybridBreed->naturalJumper = (playerMount->breed->naturalJumper || centerMount->breed->naturalJumper) && inheritDist(gen) <= 70;
    hybridBreed->naturalClimber = (playerMount->breed->naturalClimber || centerMount->breed->naturalClimber) && inheritDist(gen) <= 60;

    // Create the new foal
    std::string foalId = hybridBreedId + "_foal_" + std::to_string(gen());
    Mount* foal = new Mount(foalId, foalName, hybridBreed);

    // Young age (6 months)
    foal->age = 6;

    // Initialize foal with breed characteristics
    hybridBreed->initializeMountStats(foal->stats);

    // Set as owned by player
    foal->isOwned = true;

    // Chance to inherit special abilities directly from parents
    // even if they're not natural to the breed
    if ((playerMount->stats.canJump || centerMount->stats.canJump) && inheritDist(gen) <= 40) {
        foal->stats.canJump = true;
    }

    if ((playerMount->stats.canSwim || centerMount->stats.canSwim) && inheritDist(gen) <= 40) {
        foal->stats.canSwim = true;
    }

    if ((playerMount->stats.canClimb || centerMount->stats.canClimb) && inheritDist(gen) <= 30) {
        foal->stats.canClimb = true;
    }

    // Assign a color to the foal - use from config if available
    foal->color = config->getRandomColor();

    return foal;
}

std::vector<TAAction> MountBreedingNode::getAvailableActions()
{
    std::vector<TAAction> actions = TANode::getAvailableActions();

    if (!availableForBreeding.empty()) {
        actions.push_back({ "breed_mount", "Breed your mount",
            [this]() -> TAInput {
                return { "breeding_action", { { "action", std::string("breed") } } };
            } });
    }

    // View available mounts in detail
    actions.push_back({ "view_breeding_stock", "Examine breeding stock",
        [this]() -> TAInput {
            return { "breeding_action", { { "action", std::string("view") } } };
        } });

    // Ask about breeding process
    actions.push_back({ "breeding_info", "Ask about the breeding process",
        [this]() -> TAInput {
            return { "breeding_action", { { "action", std::string("info") } } };
        } });

    // Exit breeding center
    actions.push_back({ "exit_breeding", "Leave breeding center",
        [this]() -> TAInput {
            return { "breeding_action", { { "action", std::string("exit") } } };
        } });

    return actions;
}

bool MountBreedingNode::evaluateTransition(const TAInput& input, TANode*& outNextNode)
{
    if (input.type == "breeding_action") {
        std::string action = std::get<std::string>(input.parameters.at("action"));

        if (action == "exit") {
            // Find the exit transition
            for (const auto& rule : transitionRules) {
                if (rule.description == "Exit") {
                    outNextNode = rule.targetNode;
                    return true;
                }
            }
        }

        // Other actions stay in same node
        outNextNode = this;
        return true;
    }

    return TANode::evaluateTransition(input, outNextNode);
}