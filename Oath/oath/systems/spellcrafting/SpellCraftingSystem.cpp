#include "SpellCraftingSystem.hpp"
#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>

SpellCraftingSystem::SpellCraftingSystem()
{
    // Initialize with empty config
}

SpellCraftingSystem::~SpellCraftingSystem()
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

std::string SpellCraftingSystem::getJsonString(const nlohmann::json& j, const std::string& key, const std::string& defaultValue)
{
    if (j.contains(key) && j[key].is_string()) {
        return j[key].get<std::string>();
    }
    return defaultValue;
}

std::vector<std::string> SpellCraftingSystem::getJsonStringArray(const nlohmann::json& j, const std::string& key)
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

bool SpellCraftingSystem::loadFromFile(const std::string& filename)
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

std::vector<SpellComponent*> SpellCraftingSystem::getAllComponents() const
{
    std::vector<SpellComponent*> components;
    for (const auto& [_, component] : componentMap) {
        components.push_back(component);
    }
    return components;
}

std::vector<SpellModifier*> SpellCraftingSystem::getAllModifiers() const
{
    std::vector<SpellModifier*> modifiers;
    for (const auto& [_, modifier] : modifierMap) {
        modifiers.push_back(modifier);
    }
    return modifiers;
}

std::vector<SpellDelivery*> SpellCraftingSystem::getAllDeliveryMethods() const
{
    std::vector<SpellDelivery*> deliveries;
    for (const auto& [_, delivery] : deliveryMap) {
        deliveries.push_back(delivery);
    }
    return deliveries;
}

std::vector<SpellDesign*> SpellCraftingSystem::getAllPredefinedSpells() const
{
    std::vector<SpellDesign*> spells;
    for (const auto& [_, spell] : predefinedSpellMap) {
        spells.push_back(spell);
    }
    return spells;
}

SpellComponent* SpellCraftingSystem::getComponent(const std::string& id) const
{
    auto it = componentMap.find(id);
    if (it != componentMap.end()) {
        return it->second;
    }
    return nullptr;
}

SpellModifier* SpellCraftingSystem::getModifier(const std::string& id) const
{
    auto it = modifierMap.find(id);
    if (it != modifierMap.end()) {
        return it->second;
    }
    return nullptr;
}

SpellDelivery* SpellCraftingSystem::getDeliveryMethod(const std::string& id) const
{
    auto it = deliveryMap.find(id);
    if (it != deliveryMap.end()) {
        return it->second;
    }
    return nullptr;
}

SpellDesign* SpellCraftingSystem::getPredefinedSpell(const std::string& id) const
{
    auto it = predefinedSpellMap.find(id);
    if (it != predefinedSpellMap.end()) {
        return it->second;
    }
    return nullptr;
}

bool SpellCraftingSystem::isMagicalSkill(const std::string& skill) const
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

std::string SpellCraftingSystem::generateComponentName(const std::string& school, int skillLevel) const
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

std::string SpellCraftingSystem::generateModifierName(const std::string& school, int skillLevel) const
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

std::string SpellCraftingSystem::generateUniqueID() const
{
    static int counter = 0;
    std::stringstream ss;

    // Get current time
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);

    ss << "spell_" << time << "_" << counter++;
    return ss.str();
}

SpellComponent* SpellCraftingSystem::createResearchComponent(const std::string& researchArea, int skillLevel)
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
    nlohmann::json componentJson;

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
    nlohmann::json schoolReqs;
    schoolReqs[researchArea] = std::max(1, skillLevel / 3);
    componentJson["school_requirements"] = schoolReqs;

    // Create the component
    SpellComponent* newComponent = new SpellComponent(componentJson);

    // Add to our component map
    componentMap[componentId] = newComponent;

    return newComponent;
}

SpellModifier* SpellCraftingSystem::createResearchModifier(const std::string& researchArea, int skillLevel)
{
    // Create a JSON object for the new modifier
    nlohmann::json modifierJson;

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

bool SpellCraftingSystem::saveToFile(const std::string& filename) const
{
    try {
        nlohmann::json saveData;

        // Save components
        nlohmann::json componentsArray = nlohmann::json::array();
        for (const auto& [id, component] : componentMap) {
            componentsArray.push_back(component->toJson());
        }
        saveData["spell_components"] = componentsArray;

        // Save modifiers
        nlohmann::json modifiersArray = nlohmann::json::array();
        for (const auto& [id, modifier] : modifierMap) {
            modifiersArray.push_back(modifier->toJson());
        }
        saveData["spell_modifiers"] = modifiersArray;

        // Save delivery methods
        nlohmann::json deliveryArray = nlohmann::json::array();
        for (const auto& [id, delivery] : deliveryMap) {
            deliveryArray.push_back(delivery->toJson());
        }
        saveData["spell_delivery"] = deliveryArray;

        // Save predefined spells
        nlohmann::json spellsArray = nlohmann::json::array();
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