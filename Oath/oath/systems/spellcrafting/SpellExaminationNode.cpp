#include "SpellExaminationNode.hpp"
#include <algorithm>
#include <iostream>


SpellExaminationNode::SpellExaminationNode(const std::string& name, std::vector<SpellDesign*>& playerSpells)
    : TANode(name)
    , knownSpells(playerSpells)
{
}

void SpellExaminationNode::onEnter(GameContext* context)
{
    std::cout << "Welcome to the Arcane Library." << std::endl;
    std::cout << "Here you can study various spell designs and learn new spells." << std::endl;

    listAvailableSpells();
}

void SpellExaminationNode::listAvailableSpells()
{
    if (availableSpells.empty()) {
        std::cout << "There are no spell designs available to study at the moment." << std::endl;
        return;
    }

    std::cout << "\nAvailable spell designs to study:" << std::endl;
    for (size_t i = 0; i < availableSpells.size(); i++) {
        SpellDesign* spell = availableSpells[i];
        // Check if already learned
        bool isKnown = std::find(knownSpells.begin(), knownSpells.end(), spell) != knownSpells.end();

        std::cout << (i + 1) << ". " << spell->name;
        if (isKnown) {
            std::cout << " (Already learned)";
        }
        std::cout << std::endl;
    }
}

void SpellExaminationNode::examineSpell(int spellIndex)
{
    if (spellIndex < 0 || spellIndex >= static_cast<int>(availableSpells.size())) {
        std::cout << "Invalid spell selection." << std::endl;
        return;
    }

    SpellDesign* spell = availableSpells[spellIndex];
    std::cout << "\n=== Examining " << spell->name << " ===" << std::endl;
    std::cout << spell->getDescription() << std::endl;

    // Additional lore or information could be added here
    std::cout << "\nThis spell was created by " << (rand() % 2 == 0 ? "Master Arcanist Thelindra" : "Sage Marius of the High Tower") << "." << std::endl;
}

bool SpellExaminationNode::learnSpell(int spellIndex, GameContext* context)
{
    if (spellIndex < 0 || spellIndex >= static_cast<int>(availableSpells.size())) {
        std::cout << "Invalid spell selection." << std::endl;
        return false;
    }

    SpellDesign* spell = availableSpells[spellIndex];

    // Check if already learned
    if (std::find(knownSpells.begin(), knownSpells.end(), spell) != knownSpells.end()) {
        std::cout << "You already know this spell." << std::endl;
        return false;
    }

    // Check if player can learn this spell
    if (!spell->canLearn(*context)) {
        std::cout << "You lack the necessary skills or intelligence to learn this spell." << std::endl;

        // Show requirements
        std::cout << "Required Intelligence: " << (8 + (spell->complexityRating / 5)) << std::endl;

        std::set<std::string> schoolsUsed;
        for (const SpellComponent* component : spell->components) {
            for (const auto& [school, req] : component->schoolRequirements) {
                std::cout << "Required " << school << ": Level " << req << std::endl;
                schoolsUsed.insert(school);
            }
        }

        return false;
    }

    // Add to known spells
    spell->isLearned = true;
    knownSpells.push_back(spell);

    std::cout << "You have successfully learned " << spell->name << "!" << std::endl;
    return true;
}

std::vector<TAAction> SpellExaminationNode::getAvailableActions()
{
    std::vector<TAAction> actions = TANode::getAvailableActions();

    if (!availableSpells.empty()) {
        actions.push_back(
            { "examine_spell", "Examine a spell", []() -> TAInput {
                 return { "examination_action", { { "action", std::string("examine") } } };
             } });

        actions.push_back(
            { "learn_spell", "Learn a spell", []() -> TAInput {
                 return { "examination_action", { { "action", std::string("learn") } } };
             } });
    }

    actions.push_back(
        { "exit_library", "Exit the Arcane Library", []() -> TAInput {
             return { "examination_action", { { "action", std::string("exit") } } };
         } });

    return actions;
}

bool SpellExaminationNode::evaluateTransition(const TAInput& input, TANode*& outNextNode)
{
    if (input.type == "examination_action") {
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