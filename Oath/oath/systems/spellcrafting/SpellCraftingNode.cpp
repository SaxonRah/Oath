#include "SpellCraftingNode.hpp"
#include <iostream>
#include <random>
#include <set>

SpellCraftingNode::SpellCraftingNode(const std::string& name, SpellCraftingSystem* system)
    : TANode(name)
    , spellSystem(system)
    , currentDesign(nullptr)
{
    // Initialize available components, modifiers, etc. from the spell system
    if (system) {
        availableComponents = system->getAllComponents();
        availableModifiers = system->getAllModifiers();
        availableDeliveryMethods = system->getAllDeliveryMethods();

        // Note: We don't automatically add predefined spells to known spells
        // Those would typically be learned through gameplay
    }
}

void SpellCraftingNode::onEnter(GameContext* context)
{
    std::cout << "Welcome to the Arcane Laboratory." << std::endl;
    std::cout << "Here you can craft new spells, research magic, and manage your spellbook." << std::endl;

    if (currentDesign) {
        std::cout << "\nCurrent work in progress: " << currentDesign->name << std::endl;
    }

    listKnownSpells();
}

void SpellCraftingNode::listKnownSpells()
{
    if (knownSpells.empty()) {
        std::cout << "You don't know any spells yet." << std::endl;
        return;
    }

    std::cout << "\nYour spellbook contains:" << std::endl;
    for (size_t i = 0; i < knownSpells.size(); i++) {
        std::cout << (i + 1) << ". " << knownSpells[i]->name;
        if (knownSpells[i]->isFavorite) {
            std::cout << " ★";
        }
        std::cout << std::endl;
    }
}

void SpellCraftingNode::listAvailableComponents()
{
    if (availableComponents.empty()) {
        std::cout << "You don't know any spell components yet." << std::endl;
        return;
    }

    std::cout << "\nAvailable spell components:" << std::endl;
    for (size_t i = 0; i < availableComponents.size(); i++) {
        std::cout << (i + 1) << ". " << availableComponents[i]->name
                  << " (" << getEffectTypeName(availableComponents[i]->effectType) << ")"
                  << std::endl;
    }
}

void SpellCraftingNode::listAvailableModifiers()
{
    if (availableModifiers.empty()) {
        std::cout << "You don't know any spell modifiers yet." << std::endl;
        return;
    }

    std::cout << "\nAvailable spell modifiers:" << std::endl;
    for (size_t i = 0; i < availableModifiers.size(); i++) {
        std::cout << (i + 1) << ". " << availableModifiers[i]->name << std::endl;
    }
}

void SpellCraftingNode::listAvailableDeliveryMethods()
{
    if (availableDeliveryMethods.empty()) {
        std::cout << "You don't know any spell delivery methods yet." << std::endl;
        return;
    }

    std::cout << "\nAvailable delivery methods:" << std::endl;
    for (size_t i = 0; i < availableDeliveryMethods.size(); i++) {
        std::cout << (i + 1) << ". " << availableDeliveryMethods[i]->name
                  << " (" << getDeliveryMethodName(availableDeliveryMethods[i]->method) << ")"
                  << std::endl;
    }
}

std::string SpellCraftingNode::getEffectTypeName(SpellEffectType type)
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

std::string SpellCraftingNode::getDeliveryMethodName(SpellDeliveryMethod method)
{
    if (method == SpellDeliveryMethod::Touch)
        return "Touch";
    if (method == SpellDeliveryMethod::Projectile:
        return "Projectile";
    if (method == SpellDeliveryMethod::AreaOfEffect:
        return "Area of Effect";
    if (method == SpellDeliveryMethod::Self:
        return "Self";
    if (method == SpellDeliveryMethod::Ray:
        return "Ray";
    if (method == SpellDeliveryMethod::Rune:
        return "Rune";
    default:
        return "Unknown";
}

std::string SpellCraftingNode::getTargetTypeName(SpellTargetType type)
{
    switch (type) {
    case SpellTargetType::SingleTarget:
        return "Single Target";
    case SpellTargetType::MultiTarget:
        return "Multi Target";
    case SpellTargetType::Self:
        return "Self";
    case SpellTargetType::AlliesOnly:
        return "Allies Only";
    case SpellTargetType::EnemiesOnly:
        return "Enemies Only";
    case SpellTargetType::AreaEffect:
        return "Area Effect";
    default:
        return "Unknown";
    }
}

void SpellCraftingNode::startNewSpell(const std::string& name, GameContext* context)
{
    if (currentDesign) {
        std::cout << "You're already working on a spell. Finish or abandon it first." << std::endl;
        return;
    }

    currentDesign = new SpellDesign(spellSystem->generateUniqueID(), name);
    std::cout << "Starting design of new spell: " << name << std::endl;
}

void SpellCraftingNode::addComponent(int componentIndex, GameContext* context)
{
    if (!currentDesign) {
        std::cout << "You need to start a new spell design first." << std::endl;
        return;
    }

    if (componentIndex < 0 || componentIndex >= static_cast<int>(availableComponents.size())) {
        std::cout << "Invalid component selection." << std::endl;
        return;
    }

    SpellComponent* component = availableComponents[componentIndex];
    currentDesign->components.push_back(component);
    std::cout << "Added " << component->name << " to spell design." << std::endl;

    // Recalculate spell attributes
    currentDesign->calculateAttributes(*context);

    // Show updated spell info
    std::cout << currentDesign->getDescription() << std::endl;
}

void SpellCraftingNode::addModifier(int modifierIndex, GameContext* context)
{
    if (!currentDesign) {
        std::cout << "You need to start a new spell design first." << std::endl;
        return;
    }

    if (modifierIndex < 0 || modifierIndex >= static_cast<int>(availableModifiers.size())) {
        std::cout << "Invalid modifier selection." << std::endl;
        return;
    }

    SpellModifier* modifier = availableModifiers[modifierIndex];

    // Check if player can use this modifier
    if (!modifier->canApply(*context)) {
        std::cout << "You lack the required skill to add this modifier." << std::endl;
        return;
    }

    currentDesign->modifiers.push_back(modifier);
    std::cout << "Added " << modifier->name << " to spell design." << std::endl;

    // Recalculate spell attributes
    currentDesign->calculateAttributes(*context);

    // Show updated spell info
    std::cout << currentDesign->getDescription() << std::endl;
}

void SpellCraftingNode::setDeliveryMethod(int deliveryIndex, GameContext* context)
{
    if (!currentDesign) {
        std::cout << "You need to start a new spell design first." << std::endl;
        return;
    }

    if (deliveryIndex < 0 || deliveryIndex >= static_cast<int>(availableDeliveryMethods.size())) {
        std::cout << "Invalid delivery method selection." << std::endl;
        return;
    }

    SpellDelivery* delivery = availableDeliveryMethods[deliveryIndex];

    // Check if player can use this delivery method
    if (!delivery->canUse(*context)) {
        std::cout << "You lack the required skill to use this delivery method." << std::endl;
        return;
    }

    currentDesign->delivery = delivery;
    std::cout << "Set delivery method to " << delivery->name << "." << std::endl;

    // Recalculate spell attributes
    currentDesign->calculateAttributes(*context);

    // Show updated spell info
    std::cout << currentDesign->getDescription() << std::endl;
}

void SpellCraftingNode::setTargetType(SpellTargetType targetType, GameContext* context)
{
    if (!currentDesign) {
        std::cout << "You need to start a new spell design first." << std::endl;
        return;
    }

    currentDesign->targetType = targetType;
    std::cout << "Set target type to " << getTargetTypeName(targetType) << "." << std::endl;

    // Recalculate spell attributes
    currentDesign->calculateAttributes(*context);

    // Show updated spell info
    std::cout << currentDesign->getDescription() << std::endl;
}

bool SpellCraftingNode::finalizeSpell(GameContext* context)
{
    if (!currentDesign) {
        std::cout << "You need to start a new spell design first." << std::endl;
        return false;
    }

    // Check if design is valid
    if (currentDesign->components.empty()) {
        std::cout << "The spell needs at least one component." << std::endl;
        return false;
    }

    if (!currentDesign->delivery) {
        std::cout << "The spell needs a delivery method." << std::endl;
        return false;
    }

    // Calculate final attributes
    currentDesign->calculateAttributes(*context);

    // Check if player can learn this spell
    if (!currentDesign->canLearn(*context)) {
        std::cout << "This spell is too complex for you to learn with your current skills." << std::endl;
        std::cout << "Required Intelligence: " << (8 + (currentDesign->complexityRating / 5)) << std::endl;
        return false;
    }

    // Mark as learned and add to spellbook
    currentDesign->isLearned = true;
    knownSpells.push_back(currentDesign);

    std::cout << "Successfully finalized and learned " << currentDesign->name << "!" << std::endl;
    std::cout << currentDesign->getDescription() << std::endl;

    // Reset current design
    currentDesign = nullptr;

    return true;
}

void SpellCraftingNode::abandonSpell()
{
    if (!currentDesign) {
        std::cout << "You're not working on any spell design." << std::endl;
        return;
    }

    std::cout << "Abandoned spell design: " << currentDesign->name << std::endl;
    delete currentDesign;
    currentDesign = nullptr;
}

bool SpellCraftingNode::castSpell(int spellIndex, GameContext* context)
{
    if (spellIndex < 0 || spellIndex >= static_cast<int>(knownSpells.size())) {
        std::cout << "Invalid spell selection." << std::endl;
        return false;
    }

    SpellDesign* spell = knownSpells[spellIndex];
    return spell->cast(context);
}

SpellResearchResult SpellCraftingNode::conductResearch(const std::string& researchArea, int hoursSpent, GameContext* context)
{
    SpellResearchResult result;
    result.discoveredComponent = nullptr;
    result.discoveredModifier = nullptr;

    // Check if the research area is a valid magical skill
    if (!spellSystem->isMagicalSkill(researchArea)) {
        std::cout << "Invalid research area. Please choose a valid magical school." << std::endl;
        result.type = SpellResearchResult::Failure;
        result.message = "Invalid research area: " + researchArea;
        result.skillProgress = 0;
        return result;
    }

    // Base success chance based on intelligence and relevant skill
    int intelligence = context->playerStats.intelligence;
    int relevantSkill = 0;

    if (context->playerStats.skills.count(researchArea)) {
        relevantSkill = context->playerStats.skills.at(researchArea);
    }

    int baseSuccessChance = 10 + (intelligence - 10) * 2 + relevantSkill * 3;

    // Adjust for time spent
    float timeMultiplier = std::min(3.0f, hoursSpent / 2.0f);
    int successChance = static_cast<int>(baseSuccessChance * timeMultiplier);

    // Cap at reasonable values
    successChance = std::min(95, std::max(5, successChance));

    // Roll for success
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 100);
    int roll = dis(gen);

    // Determine research progress
    float progressGained = (hoursSpent * 0.5f) * (1.0f + (relevantSkill * 0.1f));

    // Add to research progress
    if (!researchProgress.count(researchArea)) {
        researchProgress[researchArea] = 0.0f;
    }
    researchProgress[researchArea] += progressGained;

    // Set result based on roll
    if (roll <= successChance / 3) {
        // Great success
        result.type = SpellResearchResult::Success;
        result.message = "Your research yields exceptional results!";
        result.skillProgress = progressGained * 1.5f;
    } else if (roll <= successChance) {
        // Normal success
        result.type = SpellResearchResult::PartialSuccess;
        result.message = "Your research progresses well.";
        result.skillProgress = progressGained;
    } else if (roll <= 90) {
        // Failure
        result.type = SpellResearchResult::Failure;
        result.message = "Your research yields no significant results.";
        result.skillProgress = progressGained * 0.5f;
    } else {
        // Disaster
        result.type = SpellResearchResult::Disaster;
        result.message = "Your experiment backfires spectacularly!";
        result.skillProgress = progressGained * 0.25f;

        // Apply some negative effect
        // Damage or temporary skill reduction
        context->playerStats.health -= 10;
    }

    // Discover a new component or modifier if we've reached enough research points
    if (researchProgress[researchArea] >= 100.0f) {
        researchProgress[researchArea] -= 100.0f;

        // Determine what we discovered
        if (researchArea == "destruction" || researchArea == "restoration" || researchArea == "alteration" || researchArea == "conjuration") {
            // Create a new component
            SpellComponent* newComponent = spellSystem->createResearchComponent(researchArea, relevantSkill);

            // Add to available components
            availableComponents.push_back(newComponent);
            result.discoveredComponent = newComponent;

            result.message += " You've discovered a new spell component: " + newComponent->name + "!";
        } else {
            // Create a new modifier
            SpellModifier* newModifier = spellSystem->createResearchModifier(researchArea, relevantSkill);

            // Add to available modifiers
            availableModifiers.push_back(newModifier);
            result.discoveredModifier = newModifier;

            result.message += " You've discovered a new spell modifier: " + newModifier->name + "!";
        }
    } else {
        // Display progress
        result.message += " Research progress: " + std::to_string(static_cast<int>(researchProgress[researchArea])) + "/100";
    }

    // Improve skill from research
    context->playerStats.improveSkill(researchArea, static_cast<int>(result.skillProgress / 10.0f));

    return result;
}

std::vector<TAAction> SpellCraftingNode::getAvailableActions()
{
    std::vector<TAAction> actions = TANode::getAvailableActions();

    // Add spell crafting specific actions
    if (!currentDesign) {
        actions.push_back(
            { "start_new_spell", "Start a new spell design", []() -> TAInput {
                 return { "spellcraft_action", { { "action", std::string("start_new") } } };
             } });
    } else {
        actions.push_back(
            { "add_component", "Add component", []() -> TAInput {
                 return { "spellcraft_action", { { "action", std::string("add_component") } } };
             } });

        actions.push_back(
            { "add_modifier", "Add modifier", []() -> TAInput {
                 return { "spellcraft_action", { { "action", std::string("add_modifier") } } };
             } });

        actions.push_back(
            { "set_delivery", "Set delivery method", []() -> TAInput {
                 return { "spellcraft_action", { { "action", std::string("set_delivery") } } };
             } });

        actions.push_back(
            { "set_target", "Set target type", []() -> TAInput {
                 return { "spellcraft_action", { { "action", std::string("set_target") } } };
             } });

        actions.push_back(
            { "finalize_spell", "Finalize spell", []() -> TAInput {
                 return { "spellcraft_action", { { "action", std::string("finalize") } } };
             } });

        actions.push_back(
            { "abandon_spell", "Abandon spell design", []() -> TAInput {
                 return { "spellcraft_action", { { "action", std::string("abandon") } } };
             } });
    }

    // Always available actions
    if (!knownSpells.empty()) {
        actions.push_back(
            { "cast_spell", "Cast a spell", []() -> TAInput {
                 return { "spellcraft_action", { { "action", std::string("cast_spell") } } };
             } });
    }

    actions.push_back(
        { "conduct_research", "Conduct magical research", []() -> TAInput {
             return { "spellcraft_action", { { "action", std::string("research") } } };
         } });

    actions.push_back(
        { "exit_spellcrafting", "Exit spell crafting", []() -> TAInput {
             return { "spellcraft_action", { { "action", std::string("exit") } } };
         } });

    return actions;
}

bool SpellCraftingNode::evaluateTransition(const TAInput& input, TANode*& outNextNode)
{
    if (input.type == "spellcraft_action") {
        std::string action = std::get<std::string>(input.parameters.at("action"));

        if (action == "exit") {
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