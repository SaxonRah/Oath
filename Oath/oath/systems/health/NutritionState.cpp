// /oath/systems/health/NutritionState.cpp
#include "HealthState.hpp"
#include "NutritionState.hpp"
#include <algorithm>
#include <iostream>


NutritionState::NutritionState()
    : hunger(75.0f)
    , thirst(75.0f)
    , maxHunger(100.0f)
    , maxThirst(100.0f)
    , hungerRate(2.0f) // Default: lose 2% hunger per hour
    , thirstRate(3.0f) // Default: lose 3% thirst per hour (faster than hunger)
{
}

void NutritionState::initFromJson(const nlohmann::json& nutritionJson)
{
    maxHunger = nutritionJson.value("maxHunger", 100.0f);
    maxThirst = nutritionJson.value("maxThirst", 100.0f);
    hunger = nutritionJson.value("initialHunger", 75.0f);
    thirst = nutritionJson.value("initialThirst", 75.0f);
    hungerRate = nutritionJson.value("hungerRate", 2.0f);
    thirstRate = nutritionJson.value("thirstRate", 3.0f);
}

void NutritionState::update(float hoursPassed)
{
    // Reduce hunger and thirst based on time passed
    hunger = std::max(0.0f, hunger - (hungerRate * hoursPassed));
    thirst = std::max(0.0f, thirst - (thirstRate * hoursPassed));

    // Show warnings for critical levels
    if (hunger <= criticalThreshold) {
        std::cout << "You are critically hungry and need food immediately!" << std::endl;
    } else if (hunger <= lowThreshold) {
        std::cout << "Your stomach growls painfully. You need to eat soon." << std::endl;
    }

    if (thirst <= criticalThreshold) {
        std::cout << "You are severely dehydrated and need water immediately!" << std::endl;
    } else if (thirst <= lowThreshold) {
        std::cout << "Your throat is parched. You need to drink soon." << std::endl;
    }
}

void NutritionState::consumeFood(float nutritionValue)
{
    hunger = std::min(maxHunger, hunger + nutritionValue);

    // Feedback based on new hunger level
    if (hunger >= highThreshold) {
        std::cout << "You feel completely full." << std::endl;
    } else if (hunger >= normalThreshold) {
        std::cout << "You feel satisfied after eating." << std::endl;
    } else {
        std::cout << "You're still hungry, but that helped a bit." << std::endl;
    }
}

void NutritionState::consumeWater(float hydrationValue)
{
    thirst = std::min(maxThirst, thirst + hydrationValue);

    // Feedback based on new thirst level
    if (thirst >= highThreshold) {
        std::cout << "You feel completely hydrated." << std::endl;
    } else if (thirst >= normalThreshold) {
        std::cout << "Your thirst has been quenched." << std::endl;
    } else {
        std::cout << "You're still thirsty, but that helped." << std::endl;
    }
}

NutritionLevel NutritionState::getHungerLevel() const
{
    if (hunger >= highThreshold)
        return NutritionLevel::OVERFED;
    if (hunger >= normalThreshold)
        return NutritionLevel::SATISFIED;
    if (hunger >= lowThreshold)
        return NutritionLevel::NORMAL;
    if (hunger >= criticalThreshold)
        return NutritionLevel::HUNGRY;
    if (hunger > 0)
        return NutritionLevel::STARVING;
    return NutritionLevel::CRITICAL;
}

HydrationLevel NutritionState::getThirstLevel() const
{
    if (thirst >= highThreshold)
        return HydrationLevel::OVERHYDRATED;
    if (thirst >= normalThreshold)
        return HydrationLevel::HYDRATED;
    if (thirst >= lowThreshold)
        return HydrationLevel::NORMAL;
    if (thirst >= criticalThreshold)
        return HydrationLevel::THIRSTY;
    if (thirst > 0)
        return HydrationLevel::DEHYDRATED;
    return HydrationLevel::CRITICAL;
}

std::string NutritionState::getHungerLevelString() const
{
    switch (getHungerLevel()) {
    case NutritionLevel::OVERFED:
        return "Overfed";
    case NutritionLevel::SATISFIED:
        return "Satisfied";
    case NutritionLevel::NORMAL:
        return "Normal";
    case NutritionLevel::HUNGRY:
        return "Hungry";
    case NutritionLevel::STARVING:
        return "Starving";
    case NutritionLevel::CRITICAL:
        return "Critical";
    default:
        return "Unknown";
    }
}

std::string NutritionState::getThirstLevelString() const
{
    switch (getThirstLevel()) {
    case HydrationLevel::OVERHYDRATED:
        return "Overhydrated";
    case HydrationLevel::HYDRATED:
        return "Hydrated";
    case HydrationLevel::NORMAL:
        return "Normal";
    case HydrationLevel::THIRSTY:
        return "Thirsty";
    case HydrationLevel::DEHYDRATED:
        return "Dehydrated";
    case HydrationLevel::CRITICAL:
        return "Critical";
    default:
        return "Unknown";
    }
}

void NutritionState::applyEffects(GameContext* context) const
{
    if (!context)
        return;

    HealthState* health = &context->healthContext.playerHealth;
    if (!health)
        return;

    // Apply effects based on hunger
    switch (getHungerLevel()) {
    case NutritionLevel::CRITICAL:
        // Critical hunger causes damage
        health->takeDamage(1.0f);
        // Severe stat penalties
        context->playerStats.modifiers["strength"] -= 5.0f;
        context->playerStats.modifiers["dexterity"] -= 5.0f;
        context->playerStats.modifiers["constitution"] -= 5.0f;
        // Stamina reduction
        health->maxStamina = health->maxStamina * 0.5f;
        break;

    case NutritionLevel::STARVING:
        // Significant stat penalties
        context->playerStats.modifiers["strength"] -= 3.0f;
        context->playerStats.modifiers["dexterity"] -= 2.0f;
        context->playerStats.modifiers["constitution"] -= 2.0f;
        // Stamina reduction
        health->maxStamina = health->maxStamina * 0.7f;
        break;

    case NutritionLevel::HUNGRY:
        // Minor stat penalties
        context->playerStats.modifiers["strength"] -= 1.0f;
        context->playerStats.modifiers["dexterity"] -= 1.0f;
        // Stamina reduction
        health->maxStamina = health->maxStamina * 0.9f;
        break;

    case NutritionLevel::OVERFED:
        // Being overfed has some negative effects too
        context->playerStats.modifiers["dexterity"] -= 1.0f;
        break;

    default:
        // Normal hunger has no effects
        break;
    }

    // Apply effects based on thirst
    switch (getThirstLevel()) {
    case HydrationLevel::CRITICAL:
        // Critical thirst causes damage
        health->takeDamage(2.0f); // Dehydration is more immediately dangerous
        // Severe stat penalties
        context->playerStats.modifiers["strength"] -= 4.0f;
        context->playerStats.modifiers["dexterity"] -= 4.0f;
        context->playerStats.modifiers["constitution"] -= 6.0f;
        context->playerStats.modifiers["intelligence"] -= 3.0f;
        // Stamina reduction
        health->maxStamina = health->maxStamina * 0.4f;
        break;

    case HydrationLevel::DEHYDRATED:
        // Significant stat penalties
        context->playerStats.modifiers["strength"] -= 2.0f;
        context->playerStats.modifiers["dexterity"] -= 3.0f;
        context->playerStats.modifiers["constitution"] -= 3.0f;
        context->playerStats.modifiers["intelligence"] -= 2.0f;
        // Stamina reduction
        health->maxStamina = health->maxStamina * 0.6f;
        break;

    case HydrationLevel::THIRSTY:
        // Minor stat penalties
        context->playerStats.modifiers["dexterity"] -= 1.0f;
        context->playerStats.modifiers["constitution"] -= 1.0f;
        context->playerStats.modifiers["intelligence"] -= 1.0f;
        // Stamina reduction
        health->maxStamina = health->maxStamina * 0.8f;
        break;

    case HydrationLevel::OVERHYDRATED:
        // Minor negative effects
        context->playerStats.modifiers["dexterity"] -= 1.0f;
        break;

    default:
        // Normal thirst has no effects
        break;
    }
}