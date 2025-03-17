// /oath/systems/health/NutritionNode.cpp
#include "NutritionNode.hpp"
#include <iostream>

NutritionNode::NutritionNode(const std::string& name)
    : TANode(name)
{
}

void NutritionNode::onEnter(GameContext* context)
{
    TANode::onEnter(context);

    if (!context)
        return;

    NutritionState* nutrition = getNutritionState(context);
    if (!nutrition)
        return;

    std::cout << "==== Nutrition Status ====" << std::endl;
    std::cout << "Hunger: " << nutrition->hunger << "/100 (" << nutrition->getHungerLevelString() << ")" << std::endl;
    std::cout << "Thirst: " << nutrition->thirst << "/100 (" << nutrition->getThirstLevelString() << ")" << std::endl;

    // Check for items in inventory that can be consumed
    std::cout << "\nFood Items Available:" << std::endl;
    bool hasFood = false;

    // This would need to access your inventory system
    // For now, we'll simulate some food items
    std::vector<std::pair<std::string, float>> foodItems = {
        { "bread", 15.0f },
        { "apple", 8.0f },
        { "meat", 25.0f }
    };

    for (size_t i = 0; i < foodItems.size(); i++) {
        std::cout << (i + 1) << ". " << foodItems[i].first << " (+" << foodItems[i].second << " hunger)" << std::endl;
        hasFood = true;
    }

    if (!hasFood) {
        std::cout << "No food items available in your inventory." << std::endl;
    }

    std::cout << "\nDrink Items Available:" << std::endl;
    bool hasDrinks = false;

    // Simulate some drink items
    std::vector<std::pair<std::string, float>> drinkItems = {
        { "water", 20.0f },
        { "juice", 15.0f },
        { "wine", 10.0f }
    };

    for (size_t i = 0; i < drinkItems.size(); i++) {
        std::cout << (i + 1) << ". " << drinkItems[i].first << " (+" << drinkItems[i].second << " hydration)" << std::endl;
        hasDrinks = true;
    }

    if (!hasDrinks) {
        std::cout << "No drink items available in your inventory." << std::endl;
    }
}

std::vector<TAAction> NutritionNode::getAvailableActions()
{
    std::vector<TAAction> actions = TANode::getAvailableActions();

    actions.push_back({ "eat_food", "Eat Food",
        []() -> TAInput {
            return { "nutrition_action", { { "action", std::string("eat") }, { "item_id", std::string("") } } };
        } });

    actions.push_back({ "drink_water", "Drink Something",
        []() -> TAInput {
            return { "nutrition_action", { { "action", std::string("drink") }, { "item_id", std::string("") } } };
        } });

    actions.push_back({ "back", "Return to Health Menu",
        []() -> TAInput {
            return { "nutrition_action", { { "action", std::string("back") } } };
        } });

    return actions;
}

bool NutritionNode::evaluateTransition(const TAInput& input, TANode*& outNextNode)
{
    if (input.type == "nutrition_action") {
        std::string action = std::get<std::string>(input.parameters.at("action"));

        if (action == "eat") {
            std::string itemId = std::get<std::string>(input.parameters.at("item_id"));
            // For this example, we'll use a fixed nutrition value
            consumeFood(getGameContext(), itemId, 20.0f);

            // Stay in this node
            outNextNode = this;
            return true;
        } else if (action == "drink") {
            std::string itemId = std::get<std::string>(input.parameters.at("item_id"));
            // For this example, we'll use a fixed hydration value
            consumeWater(getGameContext(), itemId, 25.0f);

            // Stay in this node
            outNextNode = this;
            return true;
        } else if (action == "back") {
            for (const auto& rule : transitionRules) {
                if (rule.description == "Back to Health") {
                    outNextNode = rule.targetNode;
                    return true;
                }
            }
        }
    }

    return TANode::evaluateTransition(input, outNextNode);
}

NutritionState* NutritionNode::getNutritionState(GameContext* context)
{
    if (!context)
        return nullptr;
    return &context->healthContext.playerNutrition;
}

void NutritionNode::consumeFood(GameContext* context, const std::string& itemId, float nutritionValue)
{
    if (!context)
        return;

    NutritionState* nutrition = getNutritionState(context);
    if (!nutrition)
        return;

    // Remove item from inventory (in your actual implementation)
    // context->playerInventory.removeItem(itemId, 1);

    // For demo purposes, we'll just apply the nutrition
    nutrition->consumeFood(nutritionValue);
    std::cout << "You eat the " << itemId << "." << std::endl;
}

void NutritionNode::consumeWater(GameContext* context, const std::string& itemId, float hydrationValue)
{
    if (!context)
        return;

    NutritionState* nutrition = getNutritionState(context);
    if (!nutrition)
        return;

    // Remove item from inventory (in your actual implementation)
    // context->playerInventory.removeItem(itemId, 1);

    // Apply hydration
    nutrition->consumeWater(hydrationValue);
    std::cout << "You drink the " << itemId << "." << std::endl;
}

// Note: This would need to be implemented based on your game context system
GameContext* NutritionNode::getGameContext()
{
    // In a real implementation, this would access the context from somewhere
    return nullptr; // Placeholder
}