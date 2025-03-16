#pragma once

#include "../../core/TANode.hpp"
#include "../../data/Recipe.hpp"

#include <string>
#include <vector>

// Forward declaration
class TANode;
class Recipe;

// Crafting station node
class CraftingNode : public TANode {
public:
    std::string stationType;
    std::string description;
    std::vector<Recipe> availableRecipes;

    CraftingNode(const std::string& name, const std::string& type);
    void onEnter(GameContext* context) override;
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;
    void addRecipe(const Recipe& recipe);
};