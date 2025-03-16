#include "MagicTrainingNode.hpp"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <random>


MagicTrainingNode::MagicTrainingNode(const std::string& name)
    : TANode(name)
{
    // Initialize the available magic schools
    availableSchools = {
        "destruction",
        "restoration",
        "alteration",
        "conjuration",
        "illusion",
        "mysticism"
    };
}

void MagicTrainingNode::onEnter(GameContext* context)
{
    std::cout << "Welcome to the Magic Training Chamber." << std::endl;
    std::cout << "Here you can practice and improve your magical skills." << std::endl;

    // Display current skills
    std::cout << "\nYour current magical skills:" << std::endl;
    std::cout << std::left << std::setw(15) << "School"
              << "Level" << std::endl;
    std::cout << std::string(25, '-') << std::endl;

    for (const auto& school : availableSchools) {
        int level = context->playerStats.skills.count(school) ? context->playerStats.skills.at(school) : 0;
        std::cout << std::left << std::setw(15) << school << level << std::endl;
    }
}

void MagicTrainingNode::listAvailableSchools()
{
    std::cout << "\nAvailable magical schools for training:" << std::endl;
    for (size_t i = 0; i < availableSchools.size(); i++) {
        std::cout << (i + 1) << ". " << availableSchools[i] << std::endl;
    }
}

void MagicTrainingNode::trainSchool(const std::string& school, int hours, GameContext* context)
{
    // Check if the school is valid
    if (std::find(availableSchools.begin(), availableSchools.end(), school) == availableSchools.end()) {
        std::cout << "Invalid magical school. Please choose a valid school to train." << std::endl;
        return;
    }

    // Check hours (prevent negative or excessive values)
    hours = std::max(1, std::min(24, hours));

    std::cout << "You spend " << hours << " hours training in " << school << " magic." << std::endl;

    // Calculate training effectiveness based on intelligence
    float intelligenceBonus = (context->playerStats.intelligence - 10) * 0.1f;
    if (intelligenceBonus < -0.5f)
        intelligenceBonus = -0.5f; // Minimum effectiveness is 50%

    // Base progress per hour
    float baseProgress = 1.0f;

    // Current skill level affects progress (higher levels are harder to improve)
    int currentLevel = context->playerStats.skills.count(school) ? context->playerStats.skills.at(school) : 0;
    float levelMultiplier = 1.0f / (1.0f + (currentLevel * 0.1f));

    // Calculate total skill progress
    float totalProgress = hours * baseProgress * (1.0f + intelligenceBonus) * levelMultiplier;

    // Random variation (80-120% of calculated progress)
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.8, 1.2);
    totalProgress *= dis(gen);

    // Round to nearest integer for skill improvement
    int skillGain = static_cast<int>(std::round(totalProgress));

    // Ensure minimum gain for effort
    skillGain = std::max(1, skillGain);

    // Apply the skill improvement
    context->playerStats.improveSkill(school, skillGain);

    // Get new skill level
    int newLevel = context->playerStats.skills.at(school);

    std::cout << "Your " << school << " skill has improved";
    if (newLevel > currentLevel) {
        std::cout << " to level " << newLevel << "!";
    } else {
        std::cout << " by " << skillGain << " points.";
    }
    std::cout << std::endl;

    // Special event on significant improvement or reaching level thresholds
    if (skillGain >= 5 || (newLevel % 5 == 0 && newLevel > currentLevel)) {
        std::cout << "\nYour understanding of " << school << " magic deepens significantly!" << std::endl;
        // Could unlock a special spell component or modifier here
    }

    // Training fatigue (reduce player stamina/energy)
    int fatigueAmount = hours * 5;
    context->playerStats.stamina = std::max(0, context->playerStats.stamina - fatigueAmount);

    if (context->playerStats.stamina <= 10) {
        std::cout << "\nYou feel exhausted from the intense magical training." << std::endl;
    }
}

std::vector<TAAction> MagicTrainingNode::getAvailableActions()
{
    std::vector<TAAction> actions = TANode::getAvailableActions();

    actions.push_back(
        { "list_schools", "View available schools", []() -> TAInput {
             return { "training_action", { { "action", std::string("list_schools") } } };
         } });

    actions.push_back(
        { "train_destruction", "Train Destruction magic", []() -> TAInput {
             return { "training_action", { { "action", std::string("train") }, { "school", std::string("destruction") } } };
         } });

    actions.push_back(
        { "train_restoration", "Train Restoration magic", []() -> TAInput {
             return { "training_action", { { "action", std::string("train") }, { "school", std::string("restoration") } } };
         } });

    actions.push_back(
        { "train_alteration", "Train Alteration magic", []() -> TAInput {
             return { "training_action", { { "action", std::string("train") }, { "school", std::string("alteration") } } };
         } });

    actions.push_back(
        { "exit_training", "Exit training chamber", []() -> TAInput {
             return { "training_action", { { "action", std::string("exit") } } };
         } });

    return actions;
}

bool MagicTrainingNode::evaluateTransition(const TAInput& input, TANode*& outNextNode)
{
    if (input.type == "training_action") {
        std::string action = std::get<std::string>(input.parameters.at("action"));

        if (action == "exit") {
            for (const auto& rule : transitionRules) {
                if (rule.description == "Return to Spell Crafting") {
                    outNextNode = rule.targetNode;
                    return true;
                }
            }
        }
    }

    return TANode::evaluateTransition(input, outNextNode);
}