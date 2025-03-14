// System_SpellCrafting_JSON.cpp

#include "System_SpellCrafting_JSON.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

// Forward declarations for RawOathFull.cpp components
class TANode;
class TAController;
class GameContext;
class TAInput;
class TAAction;
struct NodeID;

// Define spell effect types
enum class SpellEffectType {
    Damage,
    Healing,
    Protection,
    Control,
    Alteration,
    Conjuration,
    Illusion,
    Divination
};

// Define delivery methods
enum class SpellDeliveryMethod {
    Touch,
    Projectile,
    AreaOfEffect,
    Self,
    Ray,
    Rune
};

// Define targeting types
enum class SpellTargetType {
    SingleTarget,
    MultiTarget,
    Self,
    AlliesOnly,
    EnemiesOnly,
    AreaEffect
};

// Helper functions to convert between string IDs and enum values
SpellEffectType stringToEffectType(const std::string& typeStr)
{
    if (typeStr == "damage")
        return SpellEffectType::Damage;
    if (typeStr == "healing")
        return SpellEffectType::Healing;
    if (typeStr == "protection")
        return SpellEffectType::Protection;
    if (typeStr == "control")
        return SpellEffectType::Control;
    if (typeStr == "alteration")
        return SpellEffectType::Alteration;
    if (typeStr == "conjuration")
        return SpellEffectType::Conjuration;
    if (typeStr == "illusion")
        return SpellEffectType::Illusion;
    if (typeStr == "divination")
        return SpellEffectType::Divination;

    // Default
    return SpellEffectType::Damage;
}

SpellDeliveryMethod stringToDeliveryMethod(const std::string& methodStr)
{
    if (methodStr == "touch")
        return SpellDeliveryMethod::Touch;
    if (methodStr == "projectile")
        return SpellDeliveryMethod::Projectile;
    if (methodStr == "area_of_effect")
        return SpellDeliveryMethod::AreaOfEffect;
    if (methodStr == "self")
        return SpellDeliveryMethod::Self;
    if (methodStr == "ray")
        return SpellDeliveryMethod::Ray;
    if (methodStr == "rune")
        return SpellDeliveryMethod::Rune;

    // Default
    return SpellDeliveryMethod::Touch;
}

SpellTargetType stringToTargetType(const std::string& targetStr)
{
    if (targetStr == "single_target")
        return SpellTargetType::SingleTarget;
    if (targetStr == "multi_target")
        return SpellTargetType::MultiTarget;
    if (targetStr == "self")
        return SpellTargetType::Self;
    if (targetStr == "allies_only")
        return SpellTargetType::AlliesOnly;
    if (targetStr == "enemies_only")
        return SpellTargetType::EnemiesOnly;
    if (targetStr == "area_effect")
        return SpellTargetType::AreaEffect;

    // Default
    return SpellTargetType::SingleTarget;
}

std::string effectTypeToString(SpellEffectType type)
{
    if (type == SpellEffectType::Damage)
        return "damage";
    if (type == SpellEffectType::Healing)
        return "healing";
    if (type == SpellEffectType::Protection)
        return "protection";
    if (type == SpellEffectType::Control)
        return "control";
    if (type == SpellEffectType::Alteration)
        return "alteration";
    if (type == SpellEffectType::Conjuration)
        return "conjuration";
    if (type == SpellEffectType::Illusion)
        return "illusion";
    if (type == SpellEffectType::Divination)
        return "divination";

    // Default to Unknown
    return "unknown";
}

std::string deliveryMethodToString(SpellDeliveryMethod method)
{
    if (method == SpellDeliveryMethod::Touch)
        return "touch";
    if (method == SpellDeliveryMethod::Projectile)
        return "projectile";
    if (method == SpellDeliveryMethod::AreaOfEffect)
        return "area_of_effect";
    if (method == SpellDeliveryMethod::Self)
        return "self";
    if (method == SpellDeliveryMethod::Ray)
        return "ray";
    if (method == SpellDeliveryMethod::Rune)
        return "rune";

    // Default to Unknown
    return "unknown";
}

std::string targetTypeToString(SpellTargetType type)
{
    if (type == SpellTargetType::SingleTarget)
        return "single_target";
    if (type == SpellTargetType::MultiTarget)
        return "multi_target";
    if (type == SpellTargetType::Self)
        return "self";
    if (type == SpellTargetType::AlliesOnly)
        return "allies_only";
    if (type == SpellTargetType::EnemiesOnly)
        return "enemies_only";
    if (type == SpellTargetType::AreaEffect)
        return "area_effect";

    // Default to Unknown
    return "unknown";
}

// Spell component representing a fundamental magical effect
class SpellComponent {
public:
    std::string id;
    std::string name;
    std::string description;
    SpellEffectType effectType;

    // Base attributes
    int manaCost;
    int basePower;
    int complexity; // Difficulty to learn/use

    // Skill requirements
    std::map<std::string, int> schoolRequirements;

    // Special modifiers
    std::map<std::string, float> modifiers;

    // Visual effects
    std::string castingEffect;
    std::string impactEffect;

    // Constructor from JSON
    SpellComponent(const json& componentJson)
        : id(componentJson["id"])
        , name(componentJson["name"])
        , description(componentJson["description"])
        , effectType(stringToEffectType(componentJson["effect_type"]))
        , manaCost(componentJson["mana_cost"])
        , basePower(componentJson["base_power"])
        , complexity(componentJson["complexity"])
    {
        // Load school requirements
        if (componentJson.contains("school_requirements")) {
            for (auto& [school, level] : componentJson["school_requirements"].items()) {
                schoolRequirements[school] = level;
            }
        }

        // Load modifiers if present
        if (componentJson.contains("modifiers")) {
            for (auto& [modKey, modValue] : componentJson["modifiers"].items()) {
                modifiers[modKey] = modValue;
            }
        }

        // Load visual effects
        if (componentJson.contains("casting_effect")) {
            castingEffect = componentJson["casting_effect"];
        }

        if (componentJson.contains("impact_effect")) {
            impactEffect = componentJson["impact_effect"];
        }
    }

    // Get the adjusted power based on caster stats and skills
    int getAdjustedPower(const GameContext& context) const
    {
        int adjustedPower = basePower;

        // Apply skill bonuses
        for (const auto& [school, requirement] : schoolRequirements) {
            int actualSkill = context.playerStats.skills.count(school) ? context.playerStats.skills.at(school) : 0;
            if (actualSkill > requirement) {
                // Bonus for exceeding requirement
                adjustedPower += (actualSkill - requirement) * 2;
            }
        }

        // Apply intelligence bonus
        adjustedPower += (context.playerStats.intelligence - 10) / 2;

        return adjustedPower;
    }

    // Get mana cost adjusted for skills and abilities
    int getAdjustedManaCost(const GameContext& context) const
    {
        float costMultiplier = 1.0f;

        // Apply skill discounts
        for (const auto& [school, requirement] : schoolRequirements) {
            int actualSkill = context.playerStats.skills.count(school) ? context.playerStats.skills.at(school) : 0;
            if (actualSkill > requirement) {
                // Discount for higher skill
                costMultiplier -= std::min(0.5f, (actualSkill - requirement) * 0.02f);
            }
        }

        // Apply special abilities
        if (context.playerStats.hasAbility("mana_efficiency")) {
            costMultiplier -= 0.1f;
        }

        // Ensure minimum cost
        costMultiplier = std::max(0.5f, costMultiplier);

        return static_cast<int>(manaCost * costMultiplier);
    }

    // Convert to JSON
    json toJson() const
    {
        json j;
        j["id"] = id;
        j["name"] = name;
        j["description"] = description;
        j["effect_type"] = effectTypeToString(effectType);
        j["mana_cost"] = manaCost;
        j["base_power"] = basePower;
        j["complexity"] = complexity;
        j["school_requirements"] = schoolRequirements;
        j["modifiers"] = modifiers;
        j["casting_effect"] = castingEffect;
        j["impact_effect"] = impactEffect;
        return j;
    }
};

// Spell modifier that alters the behavior of spell components
class SpellModifier {
public:
    std::string id;
    std::string name;
    std::string description;

    // Effect on spell attributes
    float powerMultiplier;
    float rangeMultiplier;
    float durationMultiplier;
    float areaMultiplier;
    float castingTimeMultiplier;
    float manaCostMultiplier;

    // Required skill to use
    std::string requiredSchool;
    int requiredLevel;

    // Constructor from JSON
    SpellModifier(const json& modifierJson)
        : id(modifierJson["id"])
        , name(modifierJson["name"])
        , description(modifierJson.contains("description") ? modifierJson["description"].get<std::string>() : "")
        , powerMultiplier(modifierJson["power_multiplier"])
        , rangeMultiplier(modifierJson["range_multiplier"])
        , durationMultiplier(modifierJson["duration_multiplier"])
        , areaMultiplier(modifierJson["area_multiplier"])
        , castingTimeMultiplier(modifierJson["casting_time_multiplier"])
        , manaCostMultiplier(modifierJson["mana_cost_multiplier"])
        , requiredSchool(modifierJson["required_school"])
        , requiredLevel(modifierJson["required_level"])
    {
    }

    bool canApply(const GameContext& context) const
    {
        if (requiredSchool.empty() || requiredLevel <= 0) {
            return true;
        }

        return context.playerStats.hasSkill(requiredSchool, requiredLevel);
    }

    // Convert to JSON
    json toJson() const
    {
        json j;
        j["id"] = id;
        j["name"] = name;
        j["description"] = description;
        j["power_multiplier"] = powerMultiplier;
        j["range_multiplier"] = rangeMultiplier;
        j["duration_multiplier"] = durationMultiplier;
        j["area_multiplier"] = areaMultiplier;
        j["casting_time_multiplier"] = castingTimeMultiplier;
        j["mana_cost_multiplier"] = manaCostMultiplier;
        j["required_school"] = requiredSchool;
        j["required_level"] = requiredLevel;
        return j;
    }
};

// Delivery method determines how the spell reaches its target
class SpellDelivery {
public:
    std::string id;
    std::string name;
    std::string description;
    SpellDeliveryMethod method;

    // Base attributes
    int manaCostModifier;
    float castingTimeModifier;
    float rangeBase;

    // Required skill to use
    std::string requiredSchool;
    int requiredLevel;

    // Constructor from JSON
    SpellDelivery(const json& deliveryJson)
        : id(deliveryJson["id"])
        , name(deliveryJson["name"])
        , description(deliveryJson.contains("description") ? deliveryJson["description"].get<std::string>() : "")
        , method(stringToDeliveryMethod(deliveryJson["method"]))
        , manaCostModifier(deliveryJson["mana_cost_modifier"])
        , castingTimeModifier(deliveryJson["casting_time_modifier"])
        , rangeBase(deliveryJson["range_base"])
        , requiredSchool(deliveryJson["required_school"])
        , requiredLevel(deliveryJson["required_level"])
    {
    }

    bool canUse(const GameContext& context) const
    {
        if (requiredSchool.empty() || requiredLevel <= 0) {
            return true;
        }

        return context.playerStats.hasSkill(requiredSchool, requiredLevel);
    }

    float getAdjustedRange(const GameContext& context) const
    {
        float skillBonus = 1.0f;

        if (!requiredSchool.empty()) {
            int actualSkill = context.playerStats.skills.count(requiredSchool) ? context.playerStats.skills.at(requiredSchool) : 0;
            skillBonus += (actualSkill - requiredLevel) * 0.05f;
        }

        return rangeBase * skillBonus;
    }

    // Convert to JSON
    json toJson() const
    {
        json j;
        j["id"] = id;
        j["name"] = name;
        j["description"] = description;
        j["method"] = deliveryMethodToString(method);
        j["mana_cost_modifier"] = manaCostModifier;
        j["casting_time_modifier"] = castingTimeModifier;
        j["range_base"] = rangeBase;
        j["required_school"] = requiredSchool;
        j["required_level"] = requiredLevel;
        return j;
    }
};

// A complete spell design with components, modifiers, and delivery method
class SpellDesign {
public:
    std::string id;
    std::string name;
    std::string description;
    std::vector<SpellComponent*> components;
    std::vector<SpellModifier*> modifiers;
    SpellDelivery* delivery;
    SpellTargetType targetType;

    // Visual effects
    std::string castingVisual;
    std::string impactVisual;
    std::string spellIcon;

    // Calculated attributes
    int totalManaCost;
    float castingTime;
    int power;
    float duration;
    float range;
    float area;

    // Difficulty and learning
    int complexityRating;
    bool isLearned;
    bool isFavorite;

    // Basic constructor
    SpellDesign(const std::string& spellId, const std::string& spellName)
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

    // Constructor from JSON - special version that takes component/modifier/delivery maps
    SpellDesign(const json& spellJson,
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
        , isLearned(false)
        , isFavorite(false)
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

    // Calculate the total mana cost and other attributes
    void calculateAttributes(const GameContext& context)
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

    // Check if the player can cast this spell
    bool canCast(const GameContext& context) const
    {
        // Check if we know the spell
        if (!isLearned) {
            return false;
        }

        // Check mana
        int playerMana = 100; // Placeholder - get from context
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

    // Check if the player can learn this spell
    bool canLearn(const GameContext& context) const
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

    // Try to cast the spell and return success/failure
    bool cast(GameContext* context)
    {
        if (!context || !canCast(*context)) {
            return false;
        }

        // Deduct mana cost
        // context->playerMana -= totalManaCost;

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
            successChance += (totalSkillBonus / schoolsUsed.size()) * 3;
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
            }

            return false;
        }
    }

    // Create a string representation of the spell for display
    std::string getDescription() const
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

    // Convert to JSON
    json toJson() const
    {
        json j;
        j["id"] = id;
        j["name"] = name;
        j["description"] = description;

        // Components by ID
        json componentsJson = json::array();
        for (const auto* comp : components) {
            componentsJson.push_back(comp->id);
        }
        j["components"] = componentsJson;

        // Modifiers by ID
        json modifiersJson = json::array();
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
};

// Spell research result - what happens when experimenting
struct SpellResearchResult {
    enum ResultType {
        Success,
        PartialSuccess,
        Failure,
        Disaster
    } type;

    std::string message;
    SpellComponent* discoveredComponent;
    SpellModifier* discoveredModifier;
    float skillProgress;
};

// Main class to load and manage the spell crafting system
class SpellCraftingSystem {
private:
    json configData;
    std::map<std::string, SpellComponent*> componentMap;
    std::map<std::string, SpellModifier*> modifierMap;
    std::map<std::string, SpellDelivery*> deliveryMap;
    std::map<std::string, SpellDesign*> predefinedSpellMap;

    // Helper function to safely get a string from JSON or return a default
    std::string getJsonString(const json& j, const std::string& key, const std::string& defaultValue = "")
    {
        if (j.contains(key) && j[key].is_string()) {
            return j[key].get<std::string>();
        }
        return defaultValue;
    }

    // Helper function to get a vector of strings from a JSON array
    std::vector<std::string> getJsonStringArray(const json& j, const std::string& key)
    {
        std::vector<std::string> result;
        if (j.contains(key) && j[key].is_array()) {
            for (const auto& item : j[key]) {
                if (item.is_string()) {
                    result.push_back(item.get<std::string>());
                }
            }
        }
        return result;
    }

public:
    SpellCraftingSystem()
    {
        // Initialize with empty config
    }

    ~SpellCraftingSystem()
    {
        // Clean up allocated memory
        for (auto& pair : componentMap) {
            delete pair.second;
        }

        for (auto& pair : modifierMap) {
            delete pair.second;
        }

        for (auto& pair : deliveryMap) {
            delete pair.second;
        }

        for (auto& pair : predefinedSpellMap) {
            delete pair.second;
        }
    }

    // Load spell crafting configuration from JSON file
    bool loadFromFile(const std::string& filename)
    {
        try {
            std::ifstream file(filename);
            if (!file.is_open()) {
                std::cerr << "Failed to open spell configuration file: " << filename << std::endl;
                return false;
            }

            // Parse JSON
            file >> configData;
            file.close();

            // Load components
            if (configData.contains("spell_components") && configData["spell_components"].is_array()) {
                for (const auto& componentJson : configData["spell_components"]) {
                    SpellComponent* component = new SpellComponent(componentJson);
                    componentMap[component->id] = component;
                }
                std::cout << "Loaded " << componentMap.size() << " spell components" << std::endl;
            }

            // Load modifiers
            if (configData.contains("spell_modifiers") && configData["spell_modifiers"].is_array()) {
                for (const auto& modifierJson : configData["spell_modifiers"]) {
                    SpellModifier* modifier = new SpellModifier(modifierJson);
                    modifierMap[modifier->id] = modifier;
                }
                std::cout << "Loaded " << modifierMap.size() << " spell modifiers" << std::endl;
            }

            // Load delivery methods
            if (configData.contains("spell_delivery") && configData["spell_delivery"].is_array()) {
                for (const auto& deliveryJson : configData["spell_delivery"]) {
                    SpellDelivery* delivery = new SpellDelivery(deliveryJson);
                    deliveryMap[delivery->id] = delivery;
                }
                std::cout << "Loaded " << deliveryMap.size() << " spell delivery methods" << std::endl;
            }

            // Load predefined spells
            if (configData.contains("predefined_spells") && configData["predefined_spells"].is_array()) {
                for (const auto& spellJson : configData["predefined_spells"]) {
                    SpellDesign* spell = new SpellDesign(spellJson, componentMap, modifierMap, deliveryMap);
                    predefinedSpellMap[spell->id] = spell;
                }
                std::cout << "Loaded " << predefinedSpellMap.size() << " predefined spells" << std::endl;
            }

            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error loading spell configuration: " << e.what() << std::endl;
            return false;
        }
    }

    // Get all loaded components
    std::vector<SpellComponent*> getAllComponents() const
    {
        std::vector<SpellComponent*> components;
        for (const auto& [_, component] : componentMap) {
            components.push_back(component);
        }
        return components;
    }

    // Get all loaded modifiers
    std::vector<SpellModifier*> getAllModifiers() const
    {
        std::vector<SpellModifier*> modifiers;
        for (const auto& [_, modifier] : modifierMap) {
            modifiers.push_back(modifier);
        }
        return modifiers;
    }

    // Get all loaded delivery methods
    std::vector<SpellDelivery*> getAllDeliveryMethods() const
    {
        std::vector<SpellDelivery*> deliveries;
        for (const auto& [_, delivery] : deliveryMap) {
            deliveries.push_back(delivery);
        }
        return deliveries;
    }

    // Get all predefined spells
    std::vector<SpellDesign*> getAllPredefinedSpells() const
    {
        std::vector<SpellDesign*> spells;
        for (const auto& [_, spell] : predefinedSpellMap) {
            spells.push_back(spell);
        }
        return spells;
    }

    // Get a component by ID
    SpellComponent* getComponent(const std::string& id) const
    {
        auto it = componentMap.find(id);
        if (it != componentMap.end()) {
            return it->second;
        }
        return nullptr;
    }

    // Get a modifier by ID
    SpellModifier* getModifier(const std::string& id) const
    {
        auto it = modifierMap.find(id);
        if (it != modifierMap.end()) {
            return it->second;
        }
        return nullptr;
    }

    // Get a delivery method by ID
    SpellDelivery* getDeliveryMethod(const std::string& id) const
    {
        auto it = deliveryMap.find(id);
        if (it != deliveryMap.end()) {
            return it->second;
        }
        return nullptr;
    }

    // Get a predefined spell by ID
    SpellDesign* getPredefinedSpell(const std::string& id) const
    {
        auto it = predefinedSpellMap.find(id);
        if (it != predefinedSpellMap.end()) {
            return it->second;
        }
        return nullptr;
    }

    // Check if a school is a valid magical skill
    bool isMagicalSkill(const std::string& skill) const
    {
        if (configData.contains("magical_skills") && configData["magical_skills"].is_array()) {
            for (const auto& magicSkill : configData["magical_skills"]) {
                if (magicSkill.is_string() && magicSkill.get<std::string>() == skill) {
                    return true;
                }
            }
        }
        return false;
    }

    // Generate a component name based on research results
    std::string generateComponentName(const std::string& school, int skillLevel) const
    {
        // Get the appropriate prefixes and suffixes from the config
        std::vector<std::string> prefixes;
        std::vector<std::string> suffixes;

        // Check if this school has specific prefixes
        if (configData.contains("research_prefixes") && configData["research_prefixes"].contains(school) && configData["research_prefixes"][school].is_array()) {

            for (const auto& prefix : configData["research_prefixes"][school]) {
                prefixes.push_back(prefix.get<std::string>());
            }
        } else if (configData.contains("research_prefixes") && configData["research_prefixes"].contains("default") && configData["research_prefixes"]["default"].is_array()) {

            for (const auto& prefix : configData["research_prefixes"]["default"]) {
                prefixes.push_back(prefix.get<std::string>());
            }
        } else {
            // Fallback prefixes if config doesn't have them
            prefixes = { "Arcane", "Mystical", "Eldritch", "Ancient", "Ethereal" };
        }

        // Check if this school has specific suffixes
        if (configData.contains("research_suffixes") && configData["research_suffixes"].contains(school) && configData["research_suffixes"][school].is_array()) {

            for (const auto& suffix : configData["research_suffixes"][school]) {
                suffixes.push_back(suffix.get<std::string>());
            }
        } else if (configData.contains("research_suffixes") && configData["research_suffixes"].contains("default") && configData["research_suffixes"]["default"].is_array()) {

            for (const auto& suffix : configData["research_suffixes"]["default"]) {
                suffixes.push_back(suffix.get<std::string>());
            }
        } else {
            // Fallback suffixes
            suffixes = { "Formula", "Technique", "Method", "Process", "Principle" };
        }

        // Use more advanced prefixes/suffixes based on skill level
        int index = std::min(static_cast<int>(prefixes.size() - 1), skillLevel / 5);

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> disPre(0, index);
        std::uniform_int_distribution<> disSuf(0, index);

        return prefixes[disPre(gen)] + " " + suffixes[disSuf(gen)];
    }

    // Generate a modifier name based on research results
    std::string generateModifierName(const std::string& school, int skillLevel) const
    {
        std::vector<std::string> prefixes;
        std::vector<std::string> effects;

        // Get prefixes from config
        if (configData.contains("modifier_prefixes") && configData["modifier_prefixes"].is_array()) {
            for (const auto& prefix : configData["modifier_prefixes"]) {
                prefixes.push_back(prefix.get<std::string>());
            }
        } else {
            // Fallback
            prefixes = { "Minor", "Standard", "Improved", "Greater", "Master" };
        }

        // Get effects from config
        if (configData.contains("modifier_effects") && configData["modifier_effects"].contains(school) && configData["modifier_effects"][school].is_array()) {

            for (const auto& effect : configData["modifier_effects"][school]) {
                effects.push_back(effect.get<std::string>());
            }
        } else if (configData.contains("modifier_effects") && configData["modifier_effects"].contains("default") && configData["modifier_effects"]["default"].is_array()) {

            for (const auto& effect : configData["modifier_effects"]["default"]) {
                effects.push_back(effect.get<std::string>());
            }
        } else {
            // Fallback
            effects = { "Extension", "Projection", "Expansion", "Propagation", "Diffusion" };
        }

        // Use more advanced prefixes/effects based on skill level
        int index = std::min(static_cast<int>(prefixes.size() - 1), skillLevel / 5);

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> disPre(0, index);
        std::uniform_int_distribution<> disEff(0, index);

        return prefixes[disPre(gen)] + " " + effects[disEff(gen)];
    }

    // Generate a unique ID for components, modifiers, and spells
    std::string generateUniqueID() const
    {
        static int counter = 0;
        std::stringstream ss;

        // Get current time
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);

        ss << "spell_" << time << "_" << counter++;
        return ss.str();
    }

    // Create a new spell component during research
    SpellComponent* createResearchComponent(const std::string& researchArea, int skillLevel)
    {
        SpellEffectType effectType;

        // Determine effect type based on research area
        if (researchArea == "destruction") {
            effectType = SpellEffectType::Damage;
        } else if (researchArea == "restoration") {
            effectType = SpellEffectType::Healing;
        } else if (researchArea == "alteration") {
            effectType = SpellEffectType::Alteration;
        } else if (researchArea == "conjuration") {
            effectType = SpellEffectType::Conjuration;
        } else if (researchArea == "illusion") {
            effectType = SpellEffectType::Illusion;
        } else if (researchArea == "mysticism") {
            effectType = SpellEffectType::Divination;
        } else {
            effectType = SpellEffectType::Control;
        }

        // Create a JSON object for the new component
        json componentJson;

        std::string componentId = generateUniqueID();
        std::string componentName = generateComponentName(researchArea, skillLevel);

        componentJson["id"] = componentId;
        componentJson["name"] = componentName;
        componentJson["description"] = "A " + researchArea + " spell component discovered through research.";
        componentJson["effect_type"] = effectTypeToString(effectType);
        componentJson["mana_cost"] = 10 + skillLevel;
        componentJson["base_power"] = 5 + skillLevel * 2;
        componentJson["complexity"] = std::max(1, skillLevel / 2);

        // Set school requirements
        json schoolReqs;
        schoolReqs[researchArea] = std::max(1, skillLevel / 3);
        componentJson["school_requirements"] = schoolReqs;

        // Create the component
        SpellComponent* newComponent = new SpellComponent(componentJson);

        // Add to our component map
        componentMap[componentId] = newComponent;

        return newComponent;
    }

    // Create a new spell modifier during research
    SpellModifier* createResearchModifier(const std::string& researchArea, int skillLevel)
    {
        // Create a JSON object for the new modifier
        json modifierJson;

        std::string modifierId = generateUniqueID();
        std::string modifierName = generateModifierName(researchArea, skillLevel);

        modifierJson["id"] = modifierId;
        modifierJson["name"] = modifierName;
        modifierJson["description"] = "A " + researchArea + " spell modifier discovered through research.";

        // Set effect based on research area
        if (researchArea == "mysticism") {
            modifierJson["power_multiplier"] = 1.2f;
            modifierJson["range_multiplier"] = 1.0f;
            modifierJson["duration_multiplier"] = 1.0f;
            modifierJson["area_multiplier"] = 1.0f;
            modifierJson["casting_time_multiplier"] = 1.0f;
            modifierJson["mana_cost_multiplier"] = 1.1f;
        } else if (researchArea == "illusion") {
            modifierJson["power_multiplier"] = 1.0f;
            modifierJson["range_multiplier"] = 1.0f;
            modifierJson["duration_multiplier"] = 1.5f;
            modifierJson["area_multiplier"] = 1.0f;
            modifierJson["casting_time_multiplier"] = 1.0f;
            modifierJson["mana_cost_multiplier"] = 1.2f;
        } else {
            modifierJson["power_multiplier"] = 1.0f;
            modifierJson["range_multiplier"] = 1.3f;
            modifierJson["duration_multiplier"] = 1.0f;
            modifierJson["area_multiplier"] = 1.0f;
            modifierJson["casting_time_multiplier"] = 1.0f;
            modifierJson["mana_cost_multiplier"] = 1.15f;
        }

        modifierJson["required_school"] = researchArea;
        modifierJson["required_level"] = std::max(1, skillLevel / 3);

        // Create the modifier
        SpellModifier* newModifier = new SpellModifier(modifierJson);

        // Add to our modifier map
        modifierMap[modifierId] = newModifier;

        return newModifier;
    }

    // Save the current spell system state to a JSON file
    bool saveToFile(const std::string& filename) const
    {
        try {
            json saveData;

            // Save components
            json componentsArray = json::array();
            for (const auto& [id, component] : componentMap) {
                componentsArray.push_back(component->toJson());
            }
            saveData["spell_components"] = componentsArray;

            // Save modifiers
            json modifiersArray = json::array();
            for (const auto& [id, modifier] : modifierMap) {
                modifiersArray.push_back(modifier->toJson());
            }
            saveData["spell_modifiers"] = modifiersArray;

            // Save delivery methods
            json deliveryArray = json::array();
            for (const auto& [id, delivery] : deliveryMap) {
                deliveryArray.push_back(delivery->toJson());
            }
            saveData["spell_delivery"] = deliveryArray;

            // Save predefined spells
            json spellsArray = json::array();
            for (const auto& [id, spell] : predefinedSpellMap) {
                spellsArray.push_back(spell->toJson());
            }
            saveData["predefined_spells"] = spellsArray;

            // Write to file with nice formatting
            std::ofstream file(filename);
            if (!file.is_open()) {
                std::cerr << "Failed to open file for writing: " << filename << std::endl;
                return false;
            }

            file << std::setw(4) << saveData << std::endl;
            file.close();

            std::cout << "Spell system saved to " << filename << std::endl;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error saving spell system: " << e.what() << std::endl;
            return false;
        }
    }
};

// Node for handling spell crafting interactions - modified to use SpellCraftingSystem
class SpellCraftingNode : public TANode {
private:
    SpellCraftingSystem* spellSystem;

public:
    std::vector<SpellComponent*> availableComponents;
    std::vector<SpellModifier*> availableModifiers;
    std::vector<SpellDelivery*> availableDeliveryMethods;
    std::vector<SpellDesign*> knownSpells;

    // Current work in progress spell
    SpellDesign* currentDesign;

    // Research progress tracking
    std::map<std::string, float> researchProgress;
    std::map<std::string, bool> discoveredSecrets;

    SpellCraftingNode(const std::string& name, SpellCraftingSystem* system)
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

    void onEnter(GameContext* context) override
    {
        std::cout << "Welcome to the Arcane Laboratory." << std::endl;
        std::cout << "Here you can craft new spells, research magic, and manage your spellbook." << std::endl;

        if (currentDesign) {
            std::cout << "\nCurrent work in progress: " << currentDesign->name << std::endl;
        }

        listKnownSpells();
    }

    void listKnownSpells()
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

    void listAvailableComponents()
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

    void listAvailableModifiers()
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

    void listAvailableDeliveryMethods()
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

    std::string getEffectTypeName(SpellEffectType type)
    {
        if (type == SpellEffectType::Damage)
            return "Damage";
        if (type == SpellEffectType::Healing)
            return "Healing";
        if (type == SpellEffectType::Protection)
            return "Protection";
        if (type == SpellEffectType::Control)
            return "Control";
        if (type == SpellEffectType::Alteration)
            return "Alteration";
        if (type == SpellEffectType::Conjuration)
            return "Conjuration";
        if (type == SpellEffectType::Illusion)
            return "Illusion";
        if (type == SpellEffectType::Divination)
            return "Divination";

        // Default to Unknown
        return "Unknown";
    }

    std::string getDeliveryMethodName(SpellDeliveryMethod method)
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
}

std::string
getTargetTypeName(SpellTargetType type)
{
    switch (type) {
    if (type == SpellTargetType::SingleTarget:
        return "Single Target";
    if (type == SpellTargetType::MultiTarget:
        return "Multi Target";
    if (type == SpellTargetType::Self:
        return "Self";
    if (type == SpellTargetType::AlliesOnly:
        return "Allies Only";
    if (type == SpellTargetType::EnemiesOnly:
        return "Enemies Only";
    if (type == SpellTargetType::AreaEffect:
        return "Area Effect";
    default:
        return "Unknown";
    }
}

// Start creating a new spell
void startNewSpell(const std::string& name, GameContext* context)
{
    if (currentDesign) {
        std::cout << "You're already working on a spell. Finish or abandon it first." << std::endl;
        return;
    }

    currentDesign = new SpellDesign(spellSystem->generateUniqueID(), name);
    std::cout << "Starting design of new spell: " << name << std::endl;
}

// Add a component to the current spell
void addComponent(int componentIndex, GameContext* context)
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

// Add a modifier to the current spell
void addModifier(int modifierIndex, GameContext* context)
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

// Set delivery method for the current spell
void setDeliveryMethod(int deliveryIndex, GameContext* context)
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

// Set target type for the current spell
void setTargetType(SpellTargetType targetType, GameContext* context)
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

// Finalize and learn the current spell design
bool finalizeSpell(GameContext* context)
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

// Abandon the current spell design
void abandonSpell()
{
    if (!currentDesign) {
        std::cout << "You're not working on any spell design." << std::endl;
        return;
    }

    std::cout << "Abandoned spell design: " << currentDesign->name << std::endl;
    delete currentDesign;
    currentDesign = nullptr;
}

// Cast a known spell from your spellbook
bool castSpell(int spellIndex, GameContext* context)
{
    if (spellIndex < 0 || spellIndex >= static_cast<int>(knownSpells.size())) {
        std::cout << "Invalid spell selection." << std::endl;
        return false;
    }

    SpellDesign* spell = knownSpells[spellIndex];
    return spell->cast(context);
}

// Research to discover new components or improve existing ones
SpellResearchResult conductResearch(const std::string& researchArea, int hoursSpent, GameContext* context)
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

// Get available actions specific to spell crafting
std::vector<TAAction> getAvailableActions() override
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

bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override
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
}
;

// Integration function to set up the full spell crafting system in a game
void setupSpellCraftingSystem(TAController& controller, TANode* worldRoot)
{
    std::cout << "Setting up Spell Crafting System..." << std::endl;

    // Create and load the spell crafting system
    SpellCraftingSystem* spellSystem = new SpellCraftingSystem();
    if (!spellSystem->loadFromFile("SpellCrafting.json")) {
        std::cerr << "Failed to load spell crafting configuration! Using defaults." << std::endl;
    }

    // Create the main spell crafting node with the spell system
    SpellCraftingNode* spellCraftingNode = dynamic_cast<SpellCraftingNode*>(
        controller.createNode<SpellCraftingNode>("SpellCrafting", spellSystem));

    // Create the spell examination node
    SpellExaminationNode* spellExaminationNode = dynamic_cast<SpellExaminationNode*>(
        controller.createNode<SpellExaminationNode>("SpellExamination", spellCraftingNode->knownSpells));

    // Add predefined spells to the examination node
    spellExaminationNode->availableSpells = spellSystem->getAllPredefinedSpells();

    // Create the spellbook node
    SpellbookNode* spellbookNode = dynamic_cast<SpellbookNode*>(
        controller.createNode<SpellbookNode>("Spellbook", spellCraftingNode->knownSpells));

    // Create the magic training node
    MagicTrainingNode* magicTrainingNode = dynamic_cast<MagicTrainingNode*>(
        controller.createNode<MagicTrainingNode>("MagicTraining"));

    // Set up transitions between nodes
    spellCraftingNode->addTransition(
        [](const TAInput& input) {
            return input.type == "spellcraft_action" && std::get<std::string>(input.parameters.at("action")) == "exit";
        },
        worldRoot, "Exit to world");

    spellCraftingNode->addTransition(
        [](const TAInput& input) {
            return input.type == "spellcraft_action" && std::get<std::string>(input.parameters.at("action")) == "go_to_examination";
        },
        spellExaminationNode, "Go to Arcane Library");

    spellCraftingNode->addTransition(
        [](const TAInput& input) {
            return input.type == "spellcraft_action" && std::get<std::string>(input.parameters.at("action")) == "go_to_spellbook";
        },
        spellbookNode, "Go to Spellbook");

    spellCraftingNode->addTransition(
        [](const TAInput& input) {
            return input.type == "spellcraft_action" && std::get<std::string>(input.parameters.at("action")) == "go_to_training";
        },
        magicTrainingNode, "Go to Magic Training");

    spellExaminationNode->addTransition(
        [](const TAInput& input) {
            return input.type == "examination_action" && std::get<std::string>(input.parameters.at("action")) == "exit";
        },
        spellCraftingNode, "Return to Spell Crafting");

    spellbookNode->addTransition(
        [](const TAInput& input) {
            return input.type == "spellbook_action" && std::get<std::string>(input.parameters.at("action")) == "exit";
        },
        spellCraftingNode, "Return to Spell Crafting");

    magicTrainingNode->addTransition(
        [](const TAInput& input) {
            return input.type == "training_action" && std::get<std::string>(input.parameters.at("action")) == "exit";
        },
        spellCraftingNode, "Return to Spell Crafting");

    // Create a root node for the entire spell system
    TANode* spellSystemRoot = controller.createNode("SpellSystemRoot");
    spellSystemRoot->addChild(spellCraftingNode);
    spellSystemRoot->addChild(spellExaminationNode);
    spellSystemRoot->addChild(spellbookNode);
    spellSystemRoot->addChild(magicTrainingNode);

    // Add transitions from the main spellcraft node to child nodes
    spellSystemRoot->addTransition(
        [](const TAInput& input) {
            return input.type == "spell_system" && std::get<std::string>(input.parameters.at("destination")) == "crafting";
        },
        spellCraftingNode, "Go to Spell Crafting");

    spellSystemRoot->addTransition(
        [](const TAInput& input) {
            return input.type == "spell_system" && std::get<std::string>(input.parameters.at("destination")) == "library";
        },
        spellExaminationNode, "Go to Arcane Library");

    spellSystemRoot->addTransition(
        [](const TAInput& input) {
            return input.type == "spell_system" && std::get<std::string>(input.parameters.at("destination")) == "spellbook";
        },
        spellbookNode, "Open Spellbook");

    spellSystemRoot->addTransition(
        [](const TAInput& input) {
            return input.type == "spell_system" && std::get<std::string>(input.parameters.at("destination")) == "training";
        },
        magicTrainingNode, "Go to Magic Training");

    // Register the spell system with the controller
    controller.setSystemRoot("SpellSystem", spellSystemRoot);

    // Connect the spell system to the world
    // This assumes there's a node in the world called "MagesGuild" or similar
    TANode* magesGuildNode = findNodeByName(worldRoot, "MagesGuild");
    if (magesGuildNode) {
        magesGuildNode->addTransition(
            [](const TAInput& input) {
                return input.type == "location_action" && std::get<std::string>(input.parameters.at("action")) == "enter_spell_crafting";
            },
            spellCraftingNode, "Enter Spell Crafting Chamber");
    } else {
        // If we can't find a specific location, add to world root as fallback
        worldRoot->addTransition(
            [](const TAInput& input) {
                return input.type == "world_action" && std::get<std::string>(input.parameters.at("action")) == "enter_spell_system";
            },
            spellSystemRoot, "Enter Spell System");
    }

    std::cout << "Spell Crafting System successfully integrated!" << std::endl;
}

// Helper function to find a node by name in the node hierarchy
TANode* findNodeByName(TANode* root, const std::string& name)
{
    if (root->nodeName == name) {
        return root;
    }

    for (TANode* child : root->childNodes) {
        TANode* result = findNodeByName(child, name);
        if (result) {
            return result;
        }
    }

    return nullptr;
}

int main()
{
    std::cout << "=== Comprehensive Spell Crafting System Demo ===" << std::endl;

    // Create controller
    TAController controller;

    // Set up some basic world
    TANode* worldRoot = controller.createNode("WorldRoot");

    // Create a mages guild location
    TANode* magesGuildNode = controller.createNode("MagesGuild");
    worldRoot->addChild(magesGuildNode);

    // Set up player stats for testing
    GameContext& gameContext = controller.gameContext;
    gameContext.playerStats.intelligence = 15;
    gameContext.playerStats.improveSkill("destruction", 3);
    gameContext.playerStats.improveSkill("restoration", 2);
    gameContext.playerStats.improveSkill("alteration", 1);
    gameContext.playerStats.improveSkill("mysticism", 2);

    // Create and set up the spell crafting system using JSON configuration
    SpellCraftingSystem spellSystem;
    if (!spellSystem.loadFromFile("SpellCrafting.json")) {
        std::cerr << "Failed to load spell crafting configuration. Exiting." << std::endl;
        return 1;
    }

    // Set up the spell crafting system with the game controller
    setupSpellCraftingSystem(controller, worldRoot);

    // Demo: Enter the world and mages guild
    controller.processInput("WorldSystem", {});
    controller.processInput("WorldSystem",
        { "world_action", { { "action", std::string("enter_spell_system") } } });
    controller.processInput("SpellSystem",
        { "spell_system", { { "destination", std::string("crafting") } } });

    // Get the spell crafting node for further operations
    SpellCraftingNode* spellCraftingNode = dynamic_cast<SpellCraftingNode*>(controller.currentNodes["SpellSystem"]);

    if (!spellCraftingNode) {
        std::cerr << "Failed to get spell crafting node. Exiting." << std::endl;
        return 1;
    }

    // Scenario 1: Create a Healing Aura spell
    std::cout << "\n=== DEMO: CREATING A HEALING AURA SPELL ===\n"
              << std::endl;
    spellCraftingNode->startNewSpell("Healing Aura", &gameContext);

    // Assuming component index 1 is healing
    spellCraftingNode->addComponent(1, &gameContext);

    // Assuming modifier index 1 is extend
    spellCraftingNode->addModifier(1, &gameContext);

    // Assuming delivery index 0 is touch
    spellCraftingNode->setDeliveryMethod(0, &gameContext);

    spellCraftingNode->setTargetType(SpellTargetType::Self, &gameContext);
    spellCraftingNode->finalizeSpell(&gameContext);

    // Scenario 2: Create a Fire Shield spell
    std::cout << "\n=== DEMO: CREATING A FIRE SHIELD SPELL ===\n"
              << std::endl;
    spellCraftingNode->startNewSpell("Fire Shield", &gameContext);

    // Assuming component index 0 is fire damage
    spellCraftingNode->addComponent(0, &gameContext);

    // Assuming modifier index 0 is amplify
    spellCraftingNode->addModifier(0, &gameContext);

    // Assuming delivery index 2 is self
    spellCraftingNode->setDeliveryMethod(2, &gameContext);

    spellCraftingNode->setTargetType(SpellTargetType::Self, &gameContext);
    spellCraftingNode->finalizeSpell(&gameContext);

    // Cast spells
    std::cout << "\n=== DEMO: CASTING SPELLS ===\n"
              << std::endl;
    if (!spellCraftingNode->knownSpells.empty()) {
        spellCraftingNode->castSpell(0, &gameContext);
        spellCraftingNode->castSpell(1, &gameContext);
    }

    // Magical Research
    std::cout << "\n=== DEMO: MAGICAL RESEARCH ===\n"
              << std::endl;
    SpellResearchResult result = spellCraftingNode->conductResearch("destruction", 3, &gameContext);
    std::cout << result.message << std::endl;
    if (result.type == SpellResearchResult::Success || result.type == SpellResearchResult::PartialSuccess) {
        std::cout << "Skill improvement: " << static_cast<int>(result.skillProgress / 10.0f) << " points" << std::endl;
    }

    // Save the current state of the spell system
    spellSystem.saveToFile("SpellCraftingSave.json");

    return 0;
}