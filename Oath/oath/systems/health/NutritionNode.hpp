// /oath/systems/health/NutritionNode.hpp
#pragma once

#include "../../core/TAAction.hpp"
#include "../../core/TAInput.hpp"
#include "../../core/TANode.hpp"
#include "../../data/GameContext.hpp"

class NutritionNode : public TANode {
public:
    NutritionNode(const std::string& name);
    void onEnter(GameContext* context) override;
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;

private:
    NutritionState* getNutritionState(GameContext* context);
    void consumeFood(GameContext* context, const std::string& itemId, float nutritionValue);
    void consumeWater(GameContext* context, const std::string& itemId, float hydrationValue);
};