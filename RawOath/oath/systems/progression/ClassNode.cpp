#include <ClassNode.hpp>

ClassNode::ClassNode(const std::string& name, const std::string& classType)
    : TANode(name)
    , className(classType)
{
}

void ClassNode::onEnter(GameContext* context)
{
    std::cout << "Selected class: " << className << std::endl;
    std::cout << description << std::endl;

    if (context) {
        // Apply stat bonuses
        for (const auto& [stat, bonus] : statBonuses) {
            if (stat == "strength")
                context->playerStats.strength += bonus;
            else if (stat == "dexterity")
                context->playerStats.dexterity += bonus;
            else if (stat == "constitution")
                context->playerStats.constitution += bonus;
            else if (stat == "intelligence")
                context->playerStats.intelligence += bonus;
            else if (stat == "wisdom")
                context->playerStats.wisdom += bonus;
            else if (stat == "charisma")
                context->playerStats.charisma += bonus;
        }

        // Grant starting abilities
        for (const auto& ability : startingAbilities) {
            context->playerStats.unlockAbility(ability);
        }
    }
}