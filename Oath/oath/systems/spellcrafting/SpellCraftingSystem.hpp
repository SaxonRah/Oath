#pragma once

#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>


#include "../../data/GameContext.hpp"
#include "SpellComponent.hpp"
#include "SpellDelivery.hpp"
#include "SpellDesign.hpp"
#include "SpellModifier.hpp"
#include "SpellResearch.hpp"


class SpellCraftingSystem {
private:
    nlohmann::json configData;
    std::map<std::string, SpellComponent*> componentMap;
    std::map<std::string, SpellModifier*> modifierMap;
    std::map<std::string, SpellDelivery*> deliveryMap;
    std::map<std::string, SpellDesign*> predefinedSpellMap;

    // Helper function to safely get a string from JSON or return a default
    std::string getJsonString(const nlohmann::json& j, const std::string& key, const std::string& defaultValue = "");

    // Helper function to get a vector of strings from a JSON array
    std::vector<std::string> getJsonStringArray(const nlohmann::json& j, const std::string& key);

public:
    SpellCraftingSystem();
    ~SpellCraftingSystem();

    // Load spell crafting configuration from JSON file
    bool loadFromFile(const std::string& filename);

    // Get all loaded components
    std::vector<SpellComponent*> getAllComponents() const;

    // Get all loaded modifiers
    std::vector<SpellModifier*> getAllModifiers() const;

    // Get all loaded delivery methods
    std::vector<SpellDelivery*> getAllDeliveryMethods() const;

    // Get all predefined spells
    std::vector<SpellDesign*> getAllPredefinedSpells() const;

    // Get a component by ID
    SpellComponent* getComponent(const std::string& id) const;

    // Get a modifier by ID
    SpellModifier* getModifier(const std::string& id) const;

    // Get a delivery method by ID
    SpellDelivery* getDeliveryMethod(const std::string& id) const;

    // Get a predefined spell by ID
    SpellDesign* getPredefinedSpell(const std::string& id) const;

    // Check if a school is a valid magical skill
    bool isMagicalSkill(const std::string& skill) const;

    // Generate a component name based on research results
    std::string generateComponentName(const std::string& school, int skillLevel) const;

    // Generate a modifier name based on research results
    std::string generateModifierName(const std::string& school, int skillLevel) const;

    // Generate a unique ID for components, modifiers, and spells
    std::string generateUniqueID() const;

    // Create a new spell component during research
    SpellComponent* createResearchComponent(const std::string& researchArea, int skillLevel);

    // Create a new spell modifier during research
    SpellModifier* createResearchModifier(const std::string& researchArea, int skillLevel);

    // Save the current spell system state to a JSON file
    bool saveToFile(const std::string& filename) const;
};