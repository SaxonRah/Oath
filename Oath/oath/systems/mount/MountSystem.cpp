#include "MountSystem.hpp"
#include "../../core/TAController.hpp"
#include "MountBreedingNode.hpp"
#include "MountEquipment.hpp"
#include "MountEquipmentShopNode.hpp"
#include "MountInteractionNode.hpp"
#include "MountRacingNode.hpp"
#include "MountStable.hpp"
#include "MountStableNode.hpp"
#include "MountSystemController.hpp"
#include "MountTrainingNode.hpp"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>


void setupMountSystem(TAController& controller, const std::string& configPath)
{
    std::cout << "Setting up Mount System..." << std::endl;

    // Create the mount system controller with JSON config path
    MountSystemController* mountSystem = dynamic_cast<MountSystemController*>(
        controller.createNode<MountSystemController>("MountSystem", configPath));

    // Create mount interaction node
    MountInteractionNode* mountInteraction = dynamic_cast<MountInteractionNode*>(
        controller.createNode<MountInteractionNode>("MountInteraction", nullptr, &mountSystem->config));

    // Create mount training node
    MountTrainingNode* mountTraining = dynamic_cast<MountTrainingNode*>(
        controller.createNode<MountTrainingNode>("MountTraining", nullptr, &mountSystem->config));

    // Get stable nodes from loaded configuration
    std::vector<MountStableNode*> stableNodes;
    for (MountStable* stable : mountSystem->stables) {
        std::string nodeName = stable->name;
        // Remove spaces for node name
        nodeName.erase(std::remove(nodeName.begin(), nodeName.end(), ' '), nodeName.end());

        MountStableNode* stableNode = dynamic_cast<MountStableNode*>(
            controller.createNode<MountStableNode>(nodeName, stable));
        stableNodes.push_back(stableNode);
    }

    // Create equipment shop
    MountEquipmentShopNode* equipmentShop = dynamic_cast<MountEquipmentShopNode*>(
        controller.createNode<MountEquipmentShopNode>("MountEquipmentShop", "Horseman's Gear"));

    // Add known equipment to shop
    for (MountEquipment* equipment : mountSystem->knownEquipment) {
        equipmentShop->availableEquipment.push_back(equipment);
    }

    // Create a mount racing node from JSON config
    nlohmann::json config;
    try {
        std::ifstream file(configPath);
        file >> config;

        if (config.contains("racetrack")) {
            MountRacingNode* racingNode = MountRacingNode::createFromJson(
                "MountRacing", config["racetrack"]);

            // Connect racing node
            mountSystem->addTransition(
                [](const TAInput& input) {
                    return input.type == "mount_system" && std::get<std::string>(input.parameters.at("action")) == "race";
                },
                racingNode, "Go to racetrack");

            racingNode->addTransition(
                [](const TAInput& input) {
                    return input.type == "race_action" && std::get<std::string>(input.parameters.at("action")) == "exit";
                },
                mountSystem, "Exit");
        }

        // Create a mount breeding node
        if (config.contains("breedingCenter")) {
            MountBreedingNode* breedingNode = MountBreedingNode::createFromJson(
                "MountBreeding", config["breedingCenter"], mountSystem->breedTypes, mountSystem->config);

            // Connect breeding node
            mountSystem->addTransition(
                [](const TAInput& input) {
                    return input.type == "mount_system" && std::get<std::string>(input.parameters.at("action")) == "breed";
                },
                breedingNode, "Visit breeding center");

            breedingNode->addTransition(
                [](const TAInput& input) {
                    return input.type == "breeding_action" && std::get<std::string>(input.parameters.at("action")) == "exit";
                },
                mountSystem, "Exit");
        }

    } catch (const std::exception& e) {
        std::cerr << "Error loading racing/breeding configuration: " << e.what() << std::endl;
    }

    // Set up connections between nodes
    mountSystem->addTransition(
        [](const TAInput& input) {
            return input.type == "mount_system" && std::get<std::string>(input.parameters.at("action")) == "interact";
        },
        mountInteraction, "Interact with mount");

    mountInteraction->addTransition(
        [](const TAInput& input) {
            return input.type == "mount_action" && std::get<std::string>(input.parameters.at("action")) == "exit";
        },
        mountSystem, "Return to mount system");

    // Connect mount system to stable nodes
    for (MountStableNode* stableNode : stableNodes) {
        MountStable* stable = stableNode->stable;
        mountSystem->addTransition(
            [stable](const TAInput& input) {
                return input.type == "mount_system" && std::get<std::string>(input.parameters.at("action")) == "stable" &&
                    // In a real system, would check player location to find nearest stable
                    true; // Simplified for this example
            },
            stableNode, "Visit " + stable->name);

        // Connect back to mount system
        stableNode->addTransition(
            [](const TAInput& input) {
                return input.type == "stable_action" && std::get<std::string>(input.parameters.at("action")) == "exit";
            },
            mountSystem, "Exit");
    }

    // Connect mount interaction to training
    mountInteraction->addTransition(
        [](const TAInput& input) {
            return input.type == "mount_action" && std::get<std::string>(input.parameters.at("action")) == "train";
        },
        mountTraining, "Train mount");

    mountTraining->addTransition(
        [](const TAInput& input) {
            return input.type == "training_action" && std::get<std::string>(input.parameters.at("action")) == "exit";
        },
        mountInteraction, "Exit");

    // Connect stables to equipment shop
    if (!stableNodes.empty() && equipmentShop) {
        stableNodes[0]->addTransition(
            [](const TAInput& input) {
                return input.type == "stable_action" && std::get<std::string>(input.parameters.at("action")) == "shop";
            },
            equipmentShop, "Visit equipment shop");

        equipmentShop->addTransition(
            [](const TAInput& input) {
                return input.type == "shop_action" && std::get<std::string>(input.parameters.at("action")) == "exit";
            },
            stableNodes[0], "Exit");
    }

    // Register the mount system
    controller.setSystemRoot("MountSystem", mountSystem);

    std::cout << "Mount System setup complete!" << std::endl;
}