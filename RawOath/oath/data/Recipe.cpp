#include "Recipe.hpp"

Recipe::Recipe(const std::string& id, const std::string& recipeName)
    : recipeId(id)
    , name(recipeName)
    , discovered(false)
{
}

bool Recipe::canCraft(const GameContext& context) const
{
    // Check skill requirements
    for (const auto& [skill, level] : skillRequirements) {
        if (!context.playerStats.hasSkill(skill, level)) {
            return false;
        }
    }

    // Check ingredients
    for (const auto& ingredient : ingredients) {
        if (!context.playerInventory.hasItem(ingredient.itemId,
                ingredient.quantity)) {
            return false;
        }
    }

    return true;
}

bool Recipe::craft(GameContext* context)
{
    if (!context || !canCraft(*context)) {
        return false;
    }

    // Consume ingredients
    for (const auto& ingredient : ingredients) {
        context->playerInventory.removeItem(ingredient.itemId,
            ingredient.quantity);
    }

    // Create result item
    Item craftedItem(result.itemId, result.name, result.type, 1,
        result.quantity);
    craftedItem.properties = result.properties;

    // Add to inventory
    context->playerInventory.addItem(craftedItem);

    // Mark as discovered
    discovered = true;

    std::cout << "Crafted " << result.quantity << "x " << result.name
              << std::endl;
    return true;
}
