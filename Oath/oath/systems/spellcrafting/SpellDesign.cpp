#include "SpellDesign.hpp"
#include "SpellUtils.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <random>
#include <set>
#include <sstream>

SpellDesign::SpellDesign(const std::string& spellId, const std::string& spellName)
    : id(spellId)
    , name(spellName)
    , delivery(nullptr)
    , targetType(SpellTargetType::SingleTarget)
    , totalManaCost(0)
    , castingTime(1.0f)
    , power(0)
    , duration(0.0f)
    , range(0.0f)
    , area(0.0f)
    , complexityRating(0)
    , isLearned(false)
    , isFavorite(false)
{
}

SpellDesign::SpellDesign(const nlohmann::json& spellJson,
    const std::map<std::string, SpellComponent*>& componentMap,
    const std::map<std::string, SpellModifier*>& modifierMap,
    const std::map<std::string, SpellDelivery*>& deliveryMap)
    : id(spellJson["id"])
    , name(spellJson["name"])
    , description(spellJson.contains("description") ? spellJson["description"].get<std::string>() : "")
    , delivery(nullptr)
    , targetType(stringToTargetType(spellJson["target_type"]))
    , totalManaCost(0)
    , castingTime(1.0f)
    , power(0)
    , duration(0.0f)
    , range(0.0f)
    , area(0.0f)
    , complexityRating(spellJson.contains("complexity_rating") ? spellJson["complexity_rating"].get<int>() : 0)
    , isLearned(spellJson.contains("is_learned") ? spellJson["is_learned"].get<bool>() : false)
    , isFavorite(spellJson.contains("is_favorite") ? spellJson["is_favorite"].get<bool>() : false)
{
    // Load components
    if (spellJson.contains("components")) {
        for (const auto& componentId : spellJson["components"]) {
            std::string compId = componentId.get<std::string>();
            if (componentMap.count(compId)) {
                components.push_back(componentMap.at(compId));
            }
        }
    }

    // Load modifiers
    if (spellJson.contains("modifiers")) {
        for (const auto& modifierId : spellJson["modifiers"]) {
            std::string modId = modifierId.get<std::string>();
            if (modifierMap.count(modId)) {
                modifiers.push_back(modifierMap.at(modId));
            }
        }
    }

    // Load delivery method
    if (spellJson.contains("delivery")) {
        std::string deliveryId = spellJson["delivery"].get<std::string>();
        if (deliveryMap.count(deliveryId)) {
            delivery = deliveryMap.at(deliveryId);
        }
    }

    // Load visual effects
    if (spellJson.contains("casting_visual")) {
        castingVisual = spellJson["casting_visual"];
    }

    if (spellJson.contains("impact_visual")) {
        impactVisual = spellJson["impact_visual"];
    }

    if (spellJson.contains("spell_icon")) {
        spellIcon = spellJson["spell_icon"];
    }
}

void SpellDesign::calculateAttributes(const GameContext& context)
{
    totalManaCost = 0;
    power = 0;
    castingTime = 1.0f;
    duration = 0.0f;
    complexityRating = 0;

    // Base values from components
    for (const SpellComponent* component : components) {
        totalManaCost += component->getAdjustedManaCost(context);
        power += component->getAdjustedPower(context);
        complexityRating += component->complexity;
    }

    // Apply modifier effects
    float powerMultiplier = 1.0f;
    float durationMultiplier = 1.0f;
    float costMultiplier = 1.0f;
    float timeMultiplier = 1.0f;
    float areaMultiplier = 1.0f;
    float rangeMultiplier = 1.0f;

    for (const SpellModifier* modifier : modifiers) {
        powerMultiplier *= modifier->powerMultiplier;
        durationMultiplier *= modifier->durationMultiplier;
        costMultiplier *= modifier->manaCostMultiplier;
        timeMultiplier *= modifier->castingTimeMultiplier;
        areaMultiplier *= modifier->areaMultiplier;
        rangeMultiplier *= modifier->rangeMultiplier;
    }

    power = static_cast<int>(power * powerMultiplier);
    duration *= durationMultiplier;
    totalManaCost = static_cast<int>(totalManaCost * costMultiplier);
    castingTime *= timeMultiplier;
    area *= areaMultiplier;

    // Apply delivery method
    if (delivery) {
        totalManaCost += delivery->manaCostModifier;
        castingTime *= delivery->castingTimeModifier;
        range = delivery->getAdjustedRange(context) * rangeMultiplier;
    }

    // Ensure minimums
    totalManaCost = std::max(1, totalManaCost);
    castingTime = std::max(0.5f, castingTime);

    // Apply target type adjustments
    if (targetType == SpellTargetType::MultiTarget) {
        totalManaCost = static_cast<int>(totalManaCost * 1.5f);
    } else if (targetType == SpellTargetType::AreaEffect) {
        totalManaCost = static_cast<int>(totalManaCost * 2.0f);
        power = static_cast<int>(power * 0.8f); // Less powerful per target
    }
}

bool SpellDesign::canCast(const GameContext& context) const
{
    // Check if we know the spell
    if (!isLearned) {
        return false;
    }

    // Check mana
    int playerMana = context.playerStats.mana;
    if (totalManaCost > playerMana) {
        return false;
    }

    // Check component requirements
    for (const SpellComponent* component : components) {
        for (const auto& [school, requirement] : component->schoolRequirements) {
            if (!context.playerStats.hasSkill(school, requirement)) {
                return false;
            }
        }
    }

    // Check delivery method
    if (delivery && !delivery->canUse(context)) {
        return false;
    }

    return true;
}

bool SpellDesign::canLearn(const GameContext& context) const
{
    if (isLearned) {
        return false;
    }

    // Check intelligence requirement (higher complexity requires higher INT)
    int requiredInt = 8 + (complexityRating / 5);
    if (context.playerStats.intelligence < requiredInt) {
        return false;
    }

    // Check school requirements - must have at least basic skill in all schools used
    std::set<std::string> schoolsUsed;

    for (const SpellComponent* component : components) {
        for (const auto& [school, _] : component->schoolRequirements) {
            schoolsUsed.insert(school);
        }
    }

    for (const std::string& school : schoolsUsed) {
        if (!context.playerStats.hasSkill(school, 1)) {
            return false;
        }
    }

    return true;
}

bool SpellDesign::cast(GameContext* context)
{
    if (!context || !canCast(*context)) {
        return false;
    }

    // Deduct mana cost
    context->playerStats.mana -= totalManaCost;

    // Calculate success chance based on complexity and skills
    int successChance = 100 - (complexityRating * 2);

    // Improve chance based on related skills
    std::set<std::string> schoolsUsed;
    for (const SpellComponent* component : components) {
        for (const auto& [school, requirement] : component->schoolRequirements) {
            schoolsUsed.insert(school);
        }
    }

    int totalSkillBonus = 0;
    for (const std::string& school : schoolsUsed) {
        int skillLevel = context->playerStats.skills.count(school) ? context->playerStats.skills.at(school) : 0;
        totalSkillBonus += skillLevel;
    }

    if (!schoolsUsed.empty()) {
        successChance += int((totalSkillBonus / schoolsUsed.size()) * 3);
    }

    // Cap success chance
    successChance = std::min(95, std::max(5, successChance));

    // Roll for success
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 100);
    int roll = dis(gen);

    if (roll <= successChance) {
        // Spell succeeds
        std::cout << "Successfully cast " << name << "!" << std::endl;

        // Apply spell experience for each school used
        for (const std::string& school : schoolsUsed) {
            context->playerStats.improveSkill(school, 1);
        }

        return true;
    } else {
        // Spell fails
        std::cout << "Failed to cast " << name << "!" << std::endl;

        // Critical failure on very bad roll
        if (roll >= 95) {
            int backfireEffect = std::min(100, complexityRating * 5);
            std::cout << "The spell backfires with " << backfireEffect << " points of damage!" << std::endl;
            // Apply backfire damage
            context->playerStats.health -= backfireEffect;
        }

        return false;
    }
}

std::string SpellDesign::getDescription() const
{
    std::stringstream ss;
    ss << name << " - ";

    if (!description.empty()) {
        ss << description << "\n";
    }

    ss << "Mana Cost: " << totalManaCost << "\n";
    ss << "Casting Time: " << castingTime << " seconds\n";
    ss << "Power: " << power << "\n";

    if (duration > 0) {
        ss << "Duration: " << duration << " seconds\n";
    }

    if (delivery) {
        ss << "Delivery: " << delivery->name << "\n";
        if (range > 0) {
            ss << "Range: " << range << " meters\n";
        }
    }

    if (area > 0) {
        ss << "Area: " << area << " meter radius\n";
    }

    ss << "Complexity: " << complexityRating << "\n";

    ss << "Components: ";
    for (size_t i = 0; i < components.size(); i++) {
        ss << components[i]->name;
        if (i < components.size() - 1) {
            ss << ", ";
        }
    }

    if (!modifiers.empty()) {
        ss << "\nModifiers: ";
        for (size_t i = 0; i < modifiers.size(); i++) {
            ss << modifiers[i]->name;
            if (i < modifiers.size() - 1) {
                ss << ", ";
            }
        }
    }

    return ss.str();
}

nlohmann::json SpellDesign::toJson() const
{
    nlohmann::json j;
    j["id"] = id;
    j["name"] = name;
    j["description"] = description;

    // Components by ID
    nlohmann::json componentsJson = nlohmann::json::array();
    for (const auto* comp : components) {
        componentsJson.push_back(comp->id);
    }
    j["components"] = componentsJson;

    // Modifiers by ID
    nlohmann::json modifiersJson = nlohmann::json::array();
    for (const auto* mod : modifiers) {
        modifiersJson.push_back(mod->id);
    }
    j["modifiers"] = modifiersJson;

    // Delivery by ID
    if (delivery) {
        j["delivery"] = delivery->id;
    }

    j["target_type"] = targetTypeToString(targetType);
    j["casting_visual"] = castingVisual;
    j["impact_visual"] = impactVisual;
    j["spell_icon"] = spellIcon;
    j["complexity_rating"] = complexityRating;
    j["is_learned"] = isLearned;
    j["is_favorite"] = isFavorite;

    return j;
}