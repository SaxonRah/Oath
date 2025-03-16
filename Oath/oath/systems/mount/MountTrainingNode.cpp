#include "MountTrainingNode.hpp"
#include "../../core/TAAction.hpp"
#include "../../core/TAInput.hpp"
#include "../../data/GameContext.hpp"
#include "Mount.hpp"
#include "MountStats.hpp"
#include "MountSystemConfig.hpp"
#include "MountTrainingSession.hpp"
#include <iostream>

MountTrainingNode::MountTrainingNode(const std::string& name, Mount* mount, MountSystemConfig* cfg)
    : TANode(name)
    , trainingMount(mount)
    , config(cfg)
{
}

void MountTrainingNode::setTrainingMount(Mount* mount)
{
    trainingMount = mount;
}

void MountTrainingNode::setConfig(MountSystemConfig* cfg)
{
    config = cfg;

    // Update training types from config
    trainingTypes.clear();
    if (config) {
        for (const auto& [id, _] : config->trainingTypes) {
            trainingTypes.push_back(id);
        }
    }
}

void MountTrainingNode::onEnter(GameContext* context)
{
    if (!trainingMount) {
        std::cout << "No mount selected for training." << std::endl;
        return;
    }

    std::cout << "Training Session with " << trainingMount->name << std::endl;
    std::cout << trainingMount->getStateDescription() << std::endl;

    // Show current training levels
    std::cout << "\nCurrent Training Levels:" << std::endl;
    for (const auto& [skill, level] : trainingMount->stats.specialTraining) {
        std::cout << skill << ": " << level << "/100" << std::endl;
    }

    // Show mount's condition
    std::cout << "\nMount Condition:" << std::endl;
    std::cout << "Stamina: " << trainingMount->stats.stamina << "/" << trainingMount->stats.maxStamina << std::endl;
    std::cout << "Hunger: " << trainingMount->stats.hunger << "/100" << std::endl;
    std::cout << "Fatigue: " << trainingMount->stats.fatigue << "/100" << std::endl;

    if (trainingMount->stats.isExhausted()) {
        std::cout << "\nWARNING: Your mount is too exhausted for effective training!" << std::endl;
    }

    if (trainingMount->stats.isStarving()) {
        std::cout << "\nWARNING: Your mount is too hungry for effective training!" << std::endl;
    }

    std::cout << "\nAvailable Training Types:" << std::endl;
    if (config) {
        for (const auto& [typeId, typeDesc] : config->trainingTypes) {
            std::cout << "- " << typeId << ": " << typeDesc << std::endl;
        }
    } else {
        for (const auto& type : trainingTypes) {
            std::cout << "- " << type << std::endl;
        }
    }
}

std::vector<TAAction> MountTrainingNode::getAvailableActions()
{
    std::vector<TAAction> actions = TANode::getAvailableActions();

    if (!trainingMount) {
        actions.push_back({ "select_mount", "Select a mount to train",
            [this]() -> TAInput {
                return { "training_action", { { "action", std::string("select") } } };
            } });
        return actions;
    }

    // Training type actions
    for (const auto& type : trainingTypes) {
        actions.push_back({ "train_" + type, "Train " + type,
            [this, type]() -> TAInput {
                return { "training_action", { { "action", std::string("train") }, { "type", type } } };
            } });
    }

    // Feed mount before training
    actions.push_back({ "feed_before_training", "Feed your mount",
        [this]() -> TAInput {
            return { "training_action", { { "action", std::string("feed") } } };
        } });

    // Rest mount before training
    actions.push_back({ "rest_before_training", "Let your mount rest",
        [this]() -> TAInput {
            return { "training_action", { { "action", std::string("rest") } } };
        } });

    // Exit training
    actions.push_back({ "exit_training", "End training session",
        [this]() -> TAInput {
            return { "training_action", { { "action", std::string("exit") } } };
        } });

    return actions;
}

bool MountTrainingNode::evaluateTransition(const TAInput& input, TANode*& outNextNode)
{
    if (input.type == "training_action") {
        std::string action = std::get<std::string>(input.parameters.at("action"));

        if (action == "exit") {
            // Find the exit transition
            for (const auto& rule : transitionRules) {
                if (rule.description == "Exit") {
                    outNextNode = rule.targetNode;
                    return true;
                }
            }
        } else if (action == "train" && trainingMount && config) {
            std::string trainingType = std::get<std::string>(input.parameters.at("type"));

            // Create and run a training session
            MountTrainingSession session(trainingMount, trainingType, 60, 50, *config);
            bool success = session.conductTraining();

            if (success) {
                std::cout << "Training successful! " << trainingMount->name << " has improved "
                          << trainingType << " skills." << std::endl;
            } else {
                std::cout << "Training was difficult. " << trainingMount->name
                          << " struggled with the exercises." << std::endl;
            }

            // Stay in the same node
            outNextNode = this;
            return true;
        }

        // Other actions would stay in the same node
        outNextNode = this;
        return true;
    }

    return TANode::evaluateTransition(input, outNextNode);
}