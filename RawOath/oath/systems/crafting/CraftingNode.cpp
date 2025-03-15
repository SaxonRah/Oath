#include <CraftingNode.hpp>

CraftingNode::CraftingNode(const std::string& name, const std::string& type)
    : TANode(name)
    , stationType(type)
{
}

void CraftingNode::onEnter(GameContext* context)
{
    std::cout << "At " << stationType << " station." << std::endl;
    std::cout << description << std::endl;

    // Show available recipes
    std::cout << "Available recipes:" << std::endl;
    for (size_t i = 0; i < availableRecipes.size(); i++) {
        const auto& recipe = availableRecipes[i];
        if (recipe.discovered) {
            std::cout << i + 1 << ". " << recipe.name;
            if (context && recipe.canCraft(*context)) {
                std::cout << " (Can craft)";
            }
            std::cout << std::endl;
        } else {
            std::cout << i + 1 << ". ???" << std::endl;
        }
    }
}

std::vector<TAAction> CraftingNode::getAvailableActions()
{
    std::vector<TAAction> actions = TANode::getAvailableActions();

    // Add crafting actions for discovered recipes
    for (size_t i = 0; i < availableRecipes.size(); i++) {
        if (availableRecipes[i].discovered) {
            actions.push_back({ "craft_" + std::to_string(i),
                "Craft " + availableRecipes[i].name,
                [this, i]() -> TAInput {
                    return { "crafting_action",
                        { { "action", std::string("craft") },
                            { "recipe_index", static_cast<int>(i) } } };
                } });
        }
    }

    // Add exit action
    actions.push_back(
        { "exit_crafting", "Exit crafting station", [this]() -> TAInput {
             return { "crafting_action", { { "action", std::string("exit") } } };
         } });

    return actions;
}

bool CraftingNode::evaluateTransition(const TAInput& input, TANode*& outNextNode)
{
    if (input.type == "crafting_action") {
        std::string action = std::get<std::string>(input.parameters.at("action"));

        if (action == "craft") {
            int recipeIndex = std::get<int>(input.parameters.at("recipe_index"));
            if (recipeIndex >= 0 && recipeIndex < static_cast<int>(availableRecipes.size())) {
                // Stay in same node after crafting
                outNextNode = this;
                return true;
            }
        } else if (action == "exit") {
            // Return to default node (would be set in game logic)
            for (const auto& rule : transitionRules) {
                if (rule.description == "Exit") {
                    outNextNode = rule.targetNode;
                    return true;
                }
            }
        }
    }

    return TANode::evaluateTransition(input, outNextNode);
}

void CraftingNode::addRecipe(const Recipe& recipe)
{
    availableRecipes.push_back(recipe);
}
