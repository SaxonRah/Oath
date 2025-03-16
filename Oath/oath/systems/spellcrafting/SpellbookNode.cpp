#include "SpellbookNode.hpp"
#include <algorithm>
#include <iomanip>
#include <iostream>


SpellbookNode::SpellbookNode(const std::string& name, std::vector<SpellDesign*>& playerSpells)
    : TANode(name)
    , knownSpells(playerSpells)
{
}

void SpellbookNode::onEnter(GameContext* context)
{
    std::cout << "You open your spellbook." << std::endl;

    if (knownSpells.empty()) {
        std::cout << "Your spellbook is empty. Learn some spells first." << std::endl;
    } else {
        listSpells();
    }
}

void SpellbookNode::listSpells()
{
    std::cout << "\n=== Your Spellbook ===" << std::endl;

    if (knownSpells.empty()) {
        std::cout << "You haven't learned any spells yet." << std::endl;
        return;
    }

    // Sort spells: favorites first, then alphabetically
    std::vector<SpellDesign*> sortedSpells = knownSpells;
    std::sort(sortedSpells.begin(), sortedSpells.end(), [](SpellDesign* a, SpellDesign* b) {
        if (a->isFavorite && !b->isFavorite)
            return true;
        if (!a->isFavorite && b->isFavorite)
            return false;
        return a->name < b->name;
    });

    std::cout << std::left << std::setw(4) << "#"
              << std::setw(25) << "Name"
              << std::setw(15) << "Mana Cost"
              << std::setw(10) << "Power"
              << "Type" << std::endl;
    std::cout << std::string(70, '-') << std::endl;

    for (size_t i = 0; i < sortedSpells.size(); i++) {
        SpellDesign* spell = sortedSpells[i];
        std::string type;
        if (!spell->components.empty()) {
            type = getEffectTypeName(spell->components[0]->effectType);
        }

        std::cout << std::left << std::setw(4) << (i + 1)
                  << std::setw(25) << (spell->name + (spell->isFavorite ? " ★" : ""))
                  << std::setw(15) << spell->totalManaCost
                  << std::setw(10) << spell->power
                  << type << std::endl;
    }
}

void SpellbookNode::examineSpell(int spellIndex)
{
    if (spellIndex < 0 || spellIndex >= static_cast<int>(knownSpells.size())) {
        std::cout << "Invalid spell selection." << std::endl;
        return;
    }

    SpellDesign* spell = knownSpells[spellIndex];
    std::cout << "\n=== " << spell->name << " ===" << std::endl;
    std::cout << spell->getDescription() << std::endl;
}

void SpellbookNode::toggleFavorite(int spellIndex)
{
    if (spellIndex < 0 || spellIndex >= static_cast<int>(knownSpells.size())) {
        std::cout << "Invalid spell selection." << std::endl;
        return;
    }

    SpellDesign* spell = knownSpells[spellIndex];
    spell->isFavorite = !spell->isFavorite;

    if (spell->isFavorite) {
        std::cout << spell->name << " marked as favorite." << std::endl;
    } else {
        std::cout << spell->name << " removed from favorites." << std::endl;
    }
}

bool SpellbookNode::castSpell(int spellIndex, GameContext* context)
{
    if (spellIndex < 0 || spellIndex >= static_cast<int>(knownSpells.size())) {
        std::cout << "Invalid spell selection." << std::endl;
        return false;
    }

    SpellDesign* spell = knownSpells[spellIndex];
    return spell->cast(context);
}

std::string getEffectTypeName(SpellEffectType type)
{
    switch (type) {
    case SpellEffectType::Damage:
        return "Damage";
    case SpellEffectType::Healing:
        return "Healing";
    case SpellEffectType::Protection:
        return "Protection";
    case SpellEffectType::Control:
        return "Control";
    case SpellEffectType::Alteration:
        return "Alteration";
    case SpellEffectType::Conjuration:
        return "Conjuration";
    case SpellEffectType::Illusion:
        return "Illusion";
    case SpellEffectType::Divination:
        return "Divination";
    default:
        return "Unknown";
    }
}

std::vector<TAAction> SpellbookNode::getAvailableActions()
{
    std::vector<TAAction> actions = TANode::getAvailableActions();

    if (!knownSpells.empty()) {
        actions.push_back(
            { "examine_spell", "Examine a spell", []() -> TAInput {
                 return { "spellbook_action", { { "action", std::string("examine") } } };
             } });

        actions.push_back(
            { "cast_spell", "Cast a spell", []() -> TAInput {
                 return { "spellbook_action", { { "action", std::string("cast") } } };
             } });

        actions.push_back(
            { "toggle_favorite", "Mark/unmark as favorite", []() -> TAInput {
                 return { "spellbook_action", { { "action", std::string("favorite") } } };
             } });
    }

    actions.push_back(
        { "close_spellbook", "Close spellbook", []() -> TAInput {
             return { "spellbook_action", { { "action", std::string("exit") } } };
         } });

    return actions;
}

bool SpellbookNode::evaluateTransition(const TAInput& input, TANode*& outNextNode)
{
    if (input.type == "spellbook_action") {
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