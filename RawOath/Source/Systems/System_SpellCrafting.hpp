#ifndef SYSTEM_SPELLCRAFTING_HPP
#define SYSTEM_SPELLCRAFTING_HPP

#include <algorithm>
#include <array>
#include <chrono>
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

    SpellComponent(
        const std::string& componentId,
        const std::string& componentName,
        SpellEffectType type,
        int cost,
        int power,
        int compLevel);

    // Get the adjusted power based on caster stats and skills
    int getAdjustedPower(const GameContext& context) const;

    // Get mana cost adjusted for skills and abilities
    int getAdjustedManaCost(const GameContext& context) const;
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

    SpellModifier(
        const std::string& modId,
        const std::string& modName,
        float power = 1.0f,
        float range = 1.0f,
        float duration = 1.0f,
        float area = 1.0f,
        float castTime = 1.0f,
        float mana = 1.0f);

    bool canApply(const GameContext& context) const;
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

    SpellDelivery(
        const std::string& deliveryId,
        const std::string& deliveryName,
        SpellDeliveryMethod deliveryMethod,
        int costMod,
        float timeMod,
        float range);

    bool canUse(const GameContext& context) const;
    float getAdjustedRange(const GameContext& context) const;
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

    SpellDesign(const std::string& spellId, const std::string& spellName);

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

// Node for handling spell crafting interactions
class SpellCraftingNode : public TANode {
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

    SpellCraftingNode(const std::string& name);

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

    // Generate names for discovered components
    std::string generateComponentName(const std::string& school, int skillLevel);

    // Generate names for discovered modifiers
    std::string generateModifierName(const std::string& school, int skillLevel);

    // Generate a unique ID for components, modifiers, and spells
    std::string generateUniqueID();

    // Get available actions specific to spell crafting
    std::vector<TAAction> getAvailableActions() override;

    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;
};

// Spell examination node for studying spells and learning
class SpellExaminationNode : public TANode {
public:
    // Spells that can be examined and potentially learned
    std::vector<SpellDesign*> availableSpells;

    // Reference to player's known spells
    std::vector<SpellDesign*>& playerSpells;

    // Current spell being examined
    SpellDesign* currentExamination;

    // Learning progress for the current spell
    float learningProgress;

    SpellExaminationNode(const std::string& name, std::vector<SpellDesign*>& knownSpells);

    void onEnter(GameContext* context) override;
    void displayAvailableSpells();

    // Select a spell to examine
    void examineSpell(int spellIndex, GameContext* context);

    // Study the current spell to make learning progress
    void studySpell(int hoursSpent, GameContext* context);

    // Get available actions for spell examination
    std::vector<TAAction> getAvailableActions() override;

    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;
};

// SpellbookNode for managing learned spells and quick access
class SpellbookNode : public TANode {
public:
    // Reference to player's known spells
    std::vector<SpellDesign*>& playerSpells;

    // Spell organization
    std::map<std::string, std::vector<SpellDesign*>> spellsBySchool;
    std::vector<SpellDesign*> favoriteSpells;

    // Quick-access spell slots
    std::array<SpellDesign*, 8> quickSlots;

    SpellbookNode(const std::string& name, std::vector<SpellDesign*>& knownSpells);

    void onEnter(GameContext* context) override;
    void organizeSpells();
    std::string determinePrimarySchool(const SpellDesign* spell);
    void displaySpellbook();
    std::string capitalizeFirstLetter(const std::string& str);

    // View details of a specific spell
    void viewSpellDetails(int spellIndex);

    // Toggle favorite status for a spell
    void toggleFavorite(int spellIndex);

    // Assign a spell to a quick slot
    void assignToQuickSlot(int spellIndex, int slotIndex);

    // Cast a spell from the spellbook
    bool castSpell(int spellIndex, GameContext* context);

    // Cast a spell from a quick slot
    bool castQuickSlot(int slotIndex, GameContext* context);

    // Get available actions for spellbook
    std::vector<TAAction> getAvailableActions() override;

    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;
};

// Magical training node for practicing and improving spell skills
class MagicTrainingNode : public TANode {
public:
    enum class TrainingFocus {
        Control, // Reduce mana cost and failure chance
        Power, // Increase effect magnitude
        Speed, // Reduce casting time
        Range, // Extend spell range
        Efficiency // Balance of all aspects
    };

    std::map<std::string, int> skillProgress;
    std::map<std::string, int> trainingSessionsCompleted;

    // Current training session
    struct TrainingSession {
        std::string school;
        TrainingFocus focus;
        int duration; // in hours
        int difficulty;
        bool inProgress;

        TrainingSession();
    } currentSession;

    MagicTrainingNode(const std::string& name);

    void onEnter(GameContext* context) override;
    void displaySkills(GameContext* context);
    bool isMagicalSkill(const std::string& skill);
    std::string getFocusName(TrainingFocus focus);

    // Start a new training session
    void startTraining(const std::string& school, TrainingFocus focus, int hours, int difficulty, GameContext* context);

    // Complete the current training session
    void completeTraining(GameContext* context);

    // Abandon the current training session
    void abandonTraining();

    // Get available actions for magic training
    std::vector<TAAction> getAvailableActions() override;

    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;
};

// Integration function to set up the full spell crafting system in a game
void setupSpellCraftingSystem(TAController& controller, TANode* worldRoot);

// Helper function to find a node by name in the node hierarchy
TANode* findNodeByName(TANode* root, const std::string& name);

// Implementation of battle magic for combat situations
class BattleMagicSystem {
public:
    // Reference to player's spell collection
    std::vector<SpellDesign*>& playerSpells;

    // Quick-access battle spells
    std::array<SpellDesign*, 8> battleSlots;

    // Current combat state
    struct CombatState {
        bool inCombat;
        float castingCooldown;
        SpellDesign* currentlyCasting;
        float castProgress;

        CombatState();
    } combatState;

    BattleMagicSystem(std::vector<SpellDesign*>& knownSpells);

    // Assign a spell to a battle slot
    bool assignToBattleSlot(SpellDesign* spell, int slotIndex);

    // Begin casting a spell in combat
    bool beginCasting(SpellDesign* spell, GameContext* context);

    // Begin casting from a battle slot
    bool castFromBattleSlot(int slotIndex, GameContext* context);

    // Update casting progress (call this each combat frame)
    void updateCasting(float deltaTime, GameContext* context);

    // Cancel the current spell cast
    void cancelCasting();

    // Get casting progress as percentage
    float getCastingProgressPercent() const;

    // Check if player can cast any spell (not on cooldown)
    bool canCastSpell() const;

    // Get currently casting spell name
    std::string getCurrentCastingName() const;
};

#endif // SYSTEM_SPELLCRAFTING_HPP