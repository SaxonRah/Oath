#ifndef SYSTEM_SPELLCRAFTING_JSON_HPP
#define SYSTEM_SPELLCRAFTING_JSON_HPP

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

// Forward declarations
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
SpellEffectType stringToEffectType(const std::string& typeStr);
SpellDeliveryMethod stringToDeliveryMethod(const std::string& methodStr);
SpellTargetType stringToTargetType(const std::string& targetStr);
std::string effectTypeToString(SpellEffectType type);
std::string deliveryMethodToString(SpellDeliveryMethod method);
std::string targetTypeToString(SpellTargetType type);

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
    SpellComponent(const json& componentJson);

    // Get the adjusted power based on caster stats and skills
    int getAdjustedPower(const GameContext& context) const;

    // Get mana cost adjusted for skills and abilities
    int getAdjustedManaCost(const GameContext& context) const;

    // Convert to JSON
    json toJson() const;
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
    SpellModifier(const json& modifierJson);

    bool canApply(const GameContext& context) const;

    // Convert to JSON
    json toJson() const;
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
    SpellDelivery(const json& deliveryJson);

    bool canUse(const GameContext& context) const;

    float getAdjustedRange(const GameContext& context) const;

    // Convert to JSON
    json toJson() const;
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
    SpellDesign(const std::string& spellId, const std::string& spellName);

    // Constructor from JSON - special version that takes component/modifier/delivery maps
    SpellDesign(const json& spellJson,
        const std::map<std::string, SpellComponent*>& componentMap,
        const std::map<std::string, SpellModifier*>& modifierMap,
        const std::map<std::string, SpellDelivery*>& deliveryMap);

    // Calculate the total mana cost and other attributes
    void calculateAttributes(const GameContext& context);

    // Check if the player can cast this spell
    bool canCast(const GameContext& context) const;

    // Check if the player can learn this spell
    bool canLearn(const GameContext& context) const;

    // Try to cast the spell and return success/failure
    bool cast(GameContext* context);

    // Create a string representation of the spell for display
    std::string getDescription() const;

    // Convert to JSON
    json toJson() const;
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
    std::string getJsonString(const json& j, const std::string& key, const std::string& defaultValue = "");

    // Helper function to get a vector of strings from a JSON array
    std::vector<std::string> getJsonStringArray(const json& j, const std::string& key);

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

    SpellCraftingNode(const std::string& name, SpellCraftingSystem* system);

    void onEnter(GameContext* context) override;

    void listKnownSpells();
    void listAvailableComponents();
    void listAvailableModifiers();
    void listAvailableDeliveryMethods();

    std::string getEffectTypeName(SpellEffectType type);
    std::string getDeliveryMethodName(SpellDeliveryMethod method);
    std::string getTargetTypeName(SpellTargetType type);

    // Start creating a new spell
    void startNewSpell(const std::string& name, GameContext* context);

    // Add a component to the current spell
    void addComponent(int componentIndex, GameContext* context);

    // Add a modifier to the current spell
    void addModifier(int modifierIndex, GameContext* context);

    // Set delivery method for the current spell
    void setDeliveryMethod(int deliveryIndex, GameContext* context);

    // Set target type for the current spell
    void setTargetType(SpellTargetType targetType, GameContext* context);

    // Finalize and learn the current spell design
    bool finalizeSpell(GameContext* context);

    // Abandon the current spell design
    void abandonSpell();

    // Cast a known spell from your spellbook
    bool castSpell(int spellIndex, GameContext* context);

    // Research to discover new components or improve existing ones
    SpellResearchResult conductResearch(const std::string& researchArea, int hoursSpent, GameContext* context);

    // Get available actions specific to spell crafting
    std::vector<TAAction> getAvailableActions() override;

    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;
};

// Forward declarations for classes referenced but not defined in the provided code
class SpellExaminationNode;
class SpellbookNode;
class MagicTrainingNode;

// Integration function to set up the full spell crafting system in a game
void setupSpellCraftingSystem(TAController& controller, TANode* worldRoot);

// Helper function to find a node by name in the node hierarchy
TANode* findNodeByName(TANode* root, const std::string& name);

#endif // SYSTEM_SPELLCRAFTING_JSON_HPP