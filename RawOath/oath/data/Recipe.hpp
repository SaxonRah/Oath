#pragma once

#include "GameContext.hpp"
#include "Item.hpp"

#include <map>
#include <string>
#include <vector>

// Forward declaration
struct GameContext;
struct Item;

// Recipe for crafting items
class Recipe {
public:
    std::string recipeId;
    std::string name;
    std::string description;
    bool discovered;

    // Ingredients needed
    struct Ingredient {
        std::string itemId;
        int quantity;
    };
    std::vector<Ingredient> ingredients;

    // Result of crafting
    struct Result {
        std::string itemId;
        std::string name;
        std::string type;
        int quantity;
        std::map<std::string, std::variant<int, float, std::string, bool>> properties;
    };
    Result result;

    // Skill requirements
    std::map<std::string, int> skillRequirements;

    Recipe(const std::string& id, const std::string& recipeName);
    bool canCraft(const GameContext& context) const;
    bool craft(GameContext* context);
};