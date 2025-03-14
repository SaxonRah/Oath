// System_System_SpellCrafting.cpp
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
        int compLevel)
        : id(componentId)
        , name(componentName)
        , effectType(type)
        , manaCost(cost)
        , basePower(power)
        , complexity(compLevel)
    {
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
        float mana = 1.0f)
        : id(modId)
        , name(modName)
        , powerMultiplier(power)
        , rangeMultiplier(range)
        , durationMultiplier(duration)
        , areaMultiplier(area)
        , castingTimeMultiplier(castTime)
        , manaCostMultiplier(mana)
        , requiredSchool("")
        , requiredLevel(0)
    {
    }

    bool canApply(const GameContext& context) const
    {
        if (requiredSchool.empty() || requiredLevel <= 0) {
            return true;
        }

        return context.playerStats.hasSkill(requiredSchool, requiredLevel);
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

    SpellDelivery(
        const std::string& deliveryId,
        const std::string& deliveryName,
        SpellDeliveryMethod deliveryMethod,
        int costMod,
        float timeMod,
        float range)
        : id(deliveryId)
        , name(deliveryName)
        , method(deliveryMethod)
        , manaCostModifier(costMod)
        , castingTimeModifier(timeMod)
        , rangeBase(range)
        , requiredSchool("")
        , requiredLevel(0)
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

    SpellCraftingNode(const std::string& name)
        : TANode(name)
        , currentDesign(nullptr)
    {
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

    std::string getDeliveryMethodName(SpellDeliveryMethod method)
    {
        switch (method) {
        case SpellDeliveryMethod::Touch:
            return "Touch";
        case SpellDeliveryMethod::Projectile:
            return "Projectile";
        case SpellDeliveryMethod::AreaOfEffect:
            return "Area of Effect";
        case SpellDeliveryMethod::Self:
            return "Self";
        case SpellDeliveryMethod::Ray:
            return "Ray";
        case SpellDeliveryMethod::Rune:
            return "Rune";
        default:
            return "Unknown";
        }
    }

    std::string getTargetTypeName(SpellTargetType type)
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

    // Start creating a new spell
    void startNewSpell(const std::string& name, GameContext* context)
    {
        if (currentDesign) {
            std::cout << "You're already working on a spell. Finish or abandon it first." << std::endl;
            return;
        }

        currentDesign = new SpellDesign(generateUniqueID(), name);
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
        }

        // Discover a new component or modifier if we've reached enough research points
        if (researchProgress[researchArea] >= 100.0f) {
            researchProgress[researchArea] -= 100.0f;

            // Determine what we discovered
            if (researchArea == "destruction" || researchArea == "restoration" || researchArea == "alteration" || researchArea == "conjuration") {
                // Create a new component
                SpellEffectType effectType;

                if (researchArea == "destruction") {
                    effectType = SpellEffectType::Damage;
                } else if (researchArea == "restoration") {
                    effectType = SpellEffectType::Healing;
                } else if (researchArea == "alteration") {
                    effectType = SpellEffectType::Alteration;
                } else { // conjuration
                    effectType = SpellEffectType::Conjuration;
                }

                // Create new component based on skill level
                std::string componentName = generateComponentName(researchArea, relevantSkill);
                SpellComponent* newComponent = new SpellComponent(
                    generateUniqueID(),
                    componentName,
                    effectType,
                    10 + relevantSkill, // mana cost
                    5 + relevantSkill * 2, // power
                    std::max(1, relevantSkill / 2) // complexity
                );

                // Add school requirement
                newComponent->schoolRequirements[researchArea] = std::max(1, relevantSkill / 3);

                // Add to available components
                availableComponents.push_back(newComponent);
                result.discoveredComponent = newComponent;

                result.message += " You've discovered a new spell component: " + componentName + "!";
            } else {
                // Create a new modifier
                std::string modifierName = generateModifierName(researchArea, relevantSkill);
                SpellModifier* newModifier = new SpellModifier(
                    generateUniqueID(),
                    modifierName);

                // Set modifier attributes based on research area
                if (researchArea == "mysticism") {
                    newModifier->powerMultiplier = 1.2f;
                    newModifier->manaCostMultiplier = 1.1f;
                } else if (researchArea == "illusion") {
                    newModifier->durationMultiplier = 1.5f;
                    newModifier->manaCostMultiplier = 1.2f;
                } else { // other magic schools
                    newModifier->rangeMultiplier = 1.3f;
                    newModifier->manaCostMultiplier = 1.15f;
                }

                // Add requirement to use
                newModifier->requiredSchool = researchArea;
                newModifier->requiredLevel = std::max(1, relevantSkill / 3);

                // Add to available modifiers
                availableModifiers.push_back(newModifier);
                result.discoveredModifier = newModifier;

                result.message += " You've discovered a new spell modifier: " + modifierName + "!";
            }
        }
    }
    else if (roll <= successChance)
    {
        // Normal success
        result.type = SpellResearchResult::PartialSuccess;
        result.message = "Your research progresses well.";
        result.skillProgress = progressGained;

        // Display progress
        result.message += " Research progress: " + std::to_string(static_cast<int>(researchProgress[researchArea])) + "/100";
    }
    else if (roll <= 90)
    {
        // Failure
        result.type = SpellResearchResult::Failure;
        result.message = "Your research yields no significant results.";
        result.skillProgress = progressGained * 0.5f;
    }
    else
    {
        // Disaster
        result.type = SpellResearchResult::Disaster;
        result.message = "Your experiment backfires spectacularly!";
        result.skillProgress = progressGained * 0.25f;

        // Apply some negative effect
        // Damage or temporary skill reduction
    }

    // Improve skill from research
    context->playerStats.improveSkill(researchArea, static_cast<int>(result.skillProgress / 10.0f));

    return result;
}

// Generate names for discovered components
std::string
generateComponentName(const std::string& school, int skillLevel)
{
    std::vector<std::string> prefixes;
    std::vector<std::string> suffixes;

    if (school == "destruction") {
        prefixes = { "Burning", "Shocking", "Freezing", "Searing", "Blasting" };
        suffixes = { "Bolt", "Blast", "Nova", "Strike", "Barrage" };
    } else if (school == "restoration") {
        prefixes = { "Healing", "Mending", "Rejuvenating", "Soothing", "Divine" };
        suffixes = { "Touch", "Aura", "Pulse", "Wave", "Blessing" };
    } else if (school == "alteration") {
        prefixes = { "Shielding", "Fortifying", "Enhancing", "Transmuting", "Warping" };
        suffixes = { "Barrier", "Shell", "Field", "Membrane", "Form" };
    } else if (school == "conjuration") {
        prefixes = { "Summoning", "Binding", "Calling", "Manifesting", "Conjuring" };
        suffixes = { "Portal", "Gate", "Rift", "Bond", "Pact" };
    } else {
        prefixes = { "Arcane", "Mystical", "Eldritch", "Ancient", "Ethereal" };
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

// Generate names for discovered modifiers
std::string generateModifierName(const std::string& school, int skillLevel)
{
    std::vector<std::string> prefixes = { "Minor", "Standard", "Improved", "Greater", "Master" };
    std::vector<std::string> effects;

    if (school == "mysticism") {
        effects = { "Amplification", "Resonance", "Attunement", "Harmony", "Synergy" };
    } else if (school == "illusion") {
        effects = { "Lingering", "Enduring", "Persistent", "Sustained", "Perpetual" };
    } else {
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
std::string generateUniqueID()
{
    static int counter = 0;
    std::stringstream ss;

    // Get current time
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);

    ss << "spell_" << time << "_" << counter++;
    return ss.str();
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

    SpellExaminationNode(const std::string& name, std::vector<SpellDesign*>& knownSpells)
        : TANode(name)
        , playerSpells(knownSpells)
        , currentExamination(nullptr)
        , learningProgress(0.0f)
    {
    }

    void onEnter(GameContext* context) override
    {
        std::cout << "Welcome to the Arcane Library." << std::endl;
        std::cout << "Here you can study and learn new spells." << std::endl;

        if (currentExamination) {
            std::cout << "\nCurrently studying: " << currentExamination->name << std::endl;
            std::cout << "Learning progress: " << static_cast<int>(learningProgress) << "/100" << std::endl;
        }

        displayAvailableSpells();
    }

    void displayAvailableSpells()
    {
        if (availableSpells.empty()) {
            std::cout << "There are no spells available to study." << std::endl;
            return;
        }

        std::cout << "\nAvailable spells to study:" << std::endl;
        for (size_t i = 0; i < availableSpells.size(); i++) {
            SpellDesign* spell = availableSpells[i];
            bool alreadyKnown = false;

            // Check if player already knows this spell
            for (const SpellDesign* knownSpell : playerSpells) {
                if (knownSpell->id == spell->id) {
                    alreadyKnown = true;
                    break;
                }
            }

            std::cout << (i + 1) << ". " << spell->name;

            if (alreadyKnown) {
                std::cout << " (Already Known)";
            }

            std::cout << std::endl;
        }
    }

    // Select a spell to examine
    void examineSpell(int spellIndex, GameContext* context)
    {
        if (spellIndex < 0 || spellIndex >= static_cast<int>(availableSpells.size())) {
            std::cout << "Invalid spell selection." << std::endl;
            return;
        }

        SpellDesign* spell = availableSpells[spellIndex];

        // Check if player already knows this spell
        for (const SpellDesign* knownSpell : playerSpells) {
            if (knownSpell->id == spell->id) {
                std::cout << "You already know this spell." << std::endl;
                return;
            }
        }

        // Set as current examination
        currentExamination = spell;
        learningProgress = 0.0f;

        std::cout << "You begin studying " << spell->name << "." << std::endl;
        std::cout << spell->getDescription() << std::endl;
    }

    // Study the current spell to make learning progress
    void studySpell(int hoursSpent, GameContext* context)
    {
        if (!currentExamination) {
            std::cout << "You need to select a spell to study first." << std::endl;
            return;
        }

        // Check if player can learn this spell eventually
        if (!currentExamination->canLearn(*context)) {
            std::cout << "This spell is too complex for you to learn with your current skills." << std::endl;
            std::cout << "Required Intelligence: " << (8 + (currentExamination->complexityRating / 5)) << std::endl;
            return;
        }

        // Calculate study progress based on intelligence and relevant skills
        float baseProgress = 5.0f + (context->playerStats.intelligence - 10) * 0.5f;

        // Identify schools used in the spell
        std::set<std::string> schoolsUsed;
        for (const SpellComponent* component : currentExamination->components) {
            for (const auto& [school, _] : component->schoolRequirements) {
                schoolsUsed.insert(school);
            }
        }

        // Apply skill bonuses
        float skillBonus = 0.0f;
        for (const std::string& school : schoolsUsed) {
            int skillLevel = context->playerStats.skills.count(school) ? context->playerStats.skills.at(school) : 0;
            skillBonus += skillLevel * 0.5f;
        }

        if (!schoolsUsed.empty()) {
            skillBonus /= schoolsUsed.size();
        }

        // Calculate total progress for this study session
        float progressGained = (baseProgress + skillBonus) * hoursSpent;

        // Apply complexity penalty
        progressGained *= (100.0f / (50.0f + currentExamination->complexityRating));

        // Update learning progress
        learningProgress += progressGained;

        std::cout << "You study " << currentExamination->name << " for " << hoursSpent << " hours." << std::endl;
        std::cout << "Learning progress: " << static_cast<int>(learningProgress) << "/100" << std::endl;

        // Check if spell is learned
        if (learningProgress >= 100.0f) {
            // Create a copy of the learned spell
            SpellDesign* learnedSpell = new SpellDesign(currentExamination->id, currentExamination->name);
            learnedSpell->description = currentExamination->description;
            learnedSpell->components = currentExamination->components;
            learnedSpell->modifiers = currentExamination->modifiers;
            learnedSpell->delivery = currentExamination->delivery;
            learnedSpell->targetType = currentExamination->targetType;
            learnedSpell->castingVisual = currentExamination->castingVisual;
            learnedSpell->impactVisual = currentExamination->impactVisual;
            learnedSpell->spellIcon = currentExamination->spellIcon;
            learnedSpell->complexityRating = currentExamination->complexityRating;
            learnedSpell->isLearned = true;

            // Add to player's spellbook
            playerSpells.push_back(learnedSpell);

            std::cout << "You've successfully learned " << learnedSpell->name << "!" << std::endl;

            // Reset current examination
            currentExamination = nullptr;
            learningProgress = 0.0f;

            // Gain skill experience in relevant schools
            for (const std::string& school : schoolsUsed) {
                context->playerStats.improveSkill(school, 1);
            }
        }

        // Gain a small amount of skill experience from studying
        for (const std::string& school : schoolsUsed) {
            if (hoursSpent >= 2) {
                context->playerStats.improveSkill(school, 1);
            }
        }
    }

    // Get available actions for spell examination
    std::vector<TAAction> getAvailableActions() override
    {
        std::vector<TAAction> actions = TANode::getAvailableActions();

        // Add examination-specific actions
        if (!currentExamination) {
            actions.push_back(
                { "examine_spell", "Examine a spell", []() -> TAInput {
                     return { "examination_action", { { "action", std::string("examine") } } };
                 } });
        } else {
            actions.push_back(
                { "study_spell", "Study the spell", []() -> TAInput {
                     return { "examination_action", { { "action", std::string("study") } } };
                 } });

            actions.push_back(
                { "stop_studying", "Stop studying this spell", []() -> TAInput {
                     return { "examination_action", { { "action", std::string("stop") } } };
                 } });
        }

        actions.push_back(
            { "exit_examination", "Exit the Arcane Library", []() -> TAInput {
                 return { "examination_action", { { "action", std::string("exit") } } };
             } });

        return actions;
    }

    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override
    {
        if (input.type == "examination_action") {
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

    SpellbookNode(const std::string& name, std::vector<SpellDesign*>& knownSpells)
        : TANode(name)
        , playerSpells(knownSpells)
    {
        // Initialize quick slots to nullptr
        for (auto& slot : quickSlots) {
            slot = nullptr;
        }
    }

    void onEnter(GameContext* context) override
    {
        std::cout << "Opening your spellbook..." << std::endl;

        // Organize spells by school for easier navigation
        organizeSpells();

        // Display spellbook contents
        displaySpellbook();
    }

    void organizeSpells()
    {
        // Clear existing organization
        spellsBySchool.clear();
        favoriteSpells.clear();

        // Categorize spells
        for (SpellDesign* spell : playerSpells) {
            // Determine primary school for the spell
            std::string primarySchool = determinePrimarySchool(spell);

            // Add to school-based category
            spellsBySchool[primarySchool].push_back(spell);

            // Add to favorites if marked
            if (spell->isFavorite) {
                favoriteSpells.push_back(spell);
            }
        }
    }

    std::string determinePrimarySchool(const SpellDesign* spell)
    {
        std::map<std::string, int> schoolCounts;

        // Count components by school
        for (const SpellComponent* component : spell->components) {
            for (const auto& [school, _] : component->schoolRequirements) {
                schoolCounts[school]++;
            }
        }

        // Find the most used school
        std::string primarySchool = "general";
        int maxCount = 0;

        for (const auto& [school, count] : schoolCounts) {
            if (count > maxCount) {
                maxCount = count;
                primarySchool = school;
            }
        }

        return primarySchool;
    }

    void displaySpellbook()
    {
        if (playerSpells.empty()) {
            std::cout << "Your spellbook is empty." << std::endl;
            return;
        }

        std::cout << "\n=== YOUR SPELLBOOK ===" << std::endl;

        // Display quick slots first
        std::cout << "\nQuick Access Slots:" << std::endl;
        for (size_t i = 0; i < quickSlots.size(); i++) {
            std::cout << (i + 1) << ": ";
            if (quickSlots[i]) {
                std::cout << quickSlots[i]->name;
            } else {
                std::cout << "(empty)";
            }
            std::cout << std::endl;
        }

        // Display favorites
        if (!favoriteSpells.empty()) {
            std::cout << "\nFavorites:" << std::endl;
            for (const SpellDesign* spell : favoriteSpells) {
                std::cout << "★ " << spell->name << std::endl;
            }
        }

        // Display by school
        for (const auto& [school, spells] : spellsBySchool) {
            if (!spells.empty()) {
                std::cout << "\n"
                          << capitalizeFirstLetter(school) << " Spells:" << std::endl;
                for (const SpellDesign* spell : spells) {
                    std::cout << "- " << spell->name;
                    if (spell->isFavorite) {
                        std::cout << " ★";
                    }
                    std::cout << std::endl;
                }
            }
        }
    }

    std::string capitalizeFirstLetter(const std::string& str)
    {
        if (str.empty())
            return str;
        std::string result = str;
        result[0] = std::toupper(result[0]);
        return result;
    }

    // View details of a specific spell
    void viewSpellDetails(int spellIndex)
    {
        if (spellIndex < 0 || spellIndex >= static_cast<int>(playerSpells.size())) {
            std::cout << "Invalid spell selection." << std::endl;
            return;
        }

        SpellDesign* spell = playerSpells[spellIndex];
        std::cout << "\n=== SPELL DETAILS ===" << std::endl;
        std::cout << spell->getDescription() << std::endl;
    }

    // Toggle favorite status for a spell
    void toggleFavorite(int spellIndex)
    {
        if (spellIndex < 0 || spellIndex >= static_cast<int>(playerSpells.size())) {
            std::cout << "Invalid spell selection." << std::endl;
            return;
        }

        SpellDesign* spell = playerSpells[spellIndex];
        spell->isFavorite = !spell->isFavorite;

        if (spell->isFavorite) {
            std::cout << spell->name << " added to favorites." << std::endl;
        } else {
            std::cout << spell->name << " removed from favorites." << std::endl;
        }

        // Refresh organization
        organizeSpells();
    }

    // Assign a spell to a quick slot
    void assignToQuickSlot(int spellIndex, int slotIndex)
    {
        if (spellIndex < 0 || spellIndex >= static_cast<int>(playerSpells.size())) {
            std::cout << "Invalid spell selection." << std::endl;
            return;
        }

        if (slotIndex < 0 || slotIndex >= static_cast<int>(quickSlots.size())) {
            std::cout << "Invalid quick slot selection." << std::endl;
            return;
        }

        SpellDesign* spell = playerSpells[spellIndex];
        quickSlots[slotIndex] = spell;

        std::cout << spell->name << " assigned to quick slot " << (slotIndex + 1) << "." << std::endl;
    }

    // Cast a spell from the spellbook
    bool castSpell(int spellIndex, GameContext* context)
    {
        if (spellIndex < 0 || spellIndex >= static_cast<int>(playerSpells.size())) {
            std::cout << "Invalid spell selection." << std::endl;
            return false;
        }

        SpellDesign* spell = playerSpells[spellIndex];
        return spell->cast(context);
    }

    // Cast a spell from a quick slot
    bool castQuickSlot(int slotIndex, GameContext* context)
    {
        if (slotIndex < 0 || slotIndex >= static_cast<int>(quickSlots.size())) {
            std::cout << "Invalid quick slot selection." << std::endl;
            return false;
        }

        if (!quickSlots[slotIndex]) {
            std::cout << "No spell assigned to this quick slot." << std::endl;
            return false;
        }

        return quickSlots[slotIndex]->cast(context);
    }

    // Get available actions for spellbook
    std::vector<TAAction> getAvailableActions() override
    {
        std::vector<TAAction> actions = TANode::getAvailableActions();

        if (!playerSpells.empty()) {
            actions.push_back(
                { "view_spell", "View spell details", []() -> TAInput {
                     return { "spellbook_action", { { "action", std::string("view") } } };
                 } });

            actions.push_back(
                { "cast_spell", "Cast a spell", []() -> TAInput {
                     return { "spellbook_action", { { "action", std::string("cast") } } };
                 } });

            actions.push_back(
                { "toggle_favorite", "Add/remove from favorites", []() -> TAInput {
                     return { "spellbook_action", { { "action", std::string("favorite") } } };
                 } });

            actions.push_back(
                { "assign_quickslot", "Assign to quick slot", []() -> TAInput {
                     return { "spellbook_action", { { "action", std::string("quickslot") } } };
                 } });
        }

        if (std::any_of(quickSlots.begin(), quickSlots.end(), [](SpellDesign* spell) { return spell != nullptr; })) {
            actions.push_back(
                { "cast_quickslot", "Cast from quick slot", []() -> TAInput {
                     return { "spellbook_action", { { "action", std::string("cast_quick") } } };
                 } });
        }

        actions.push_back(
            { "exit_spellbook", "Close spellbook", []() -> TAInput {
                 return { "spellbook_action", { { "action", std::string("exit") } } };
             } });

        return actions;
    }

    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override
    {
        if (input.type == "spellbook_action") {
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

        TrainingSession()
            : school("")
            , duration(0)
            , difficulty(1)
            , inProgress(false)
        {
        }
    } currentSession;

    MagicTrainingNode(const std::string& name)
        : TANode(name)
    {
    }

    void onEnter(GameContext* context) override
    {
        std::cout << "Welcome to the Magic Training Chamber." << std::endl;
        std::cout << "Here you can practice magical skills and improve your spellcasting abilities." << std::endl;

        displaySkills(context);

        if (currentSession.inProgress) {
            std::cout << "\nCurrent training session: "
                      << currentSession.school << " ("
                      << getFocusName(currentSession.focus) << "), "
                      << currentSession.duration << " hours, difficulty "
                      << currentSession.difficulty << std::endl;
        }
    }

    void displaySkills(GameContext* context)
    {
        std::cout << "\nYour magical skills:" << std::endl;

        for (const auto& [skill, level] : context->playerStats.skills) {
            // Only display magical skills
            if (isMagicalSkill(skill)) {
                std::cout << "- " << skill << ": " << level;

                // Show progress to next level
                if (skillProgress.count(skill)) {
                    std::cout << " (" << skillProgress[skill] << "/100)";
                }

                std::cout << std::endl;
            }
        }
    }

    bool isMagicalSkill(const std::string& skill)
    {
        static const std::set<std::string> magicSkills = {
            "destruction", "restoration", "alteration", "illusion",
            "conjuration", "mysticism", "alchemy", "enchanting"
        };

        return magicSkills.find(skill) != magicSkills.end();
    }

    std::string getFocusName(TrainingFocus focus)
    {
        switch (focus) {
        case TrainingFocus::Control:
            return "Control";
        case TrainingFocus::Power:
            return "Power";
        case TrainingFocus::Speed:
            return "Speed";
        case TrainingFocus::Range:
            return "Range";
        case TrainingFocus::Efficiency:
            return "Efficiency";
        default:
            return "Unknown";
        }
    }

    // Start a new training session
    void startTraining(const std::string& school, TrainingFocus focus, int hours, int difficulty, GameContext* context)
    {
        if (currentSession.inProgress) {
            std::cout << "You're already in a training session. Complete or abandon it first." << std::endl;
            return;
        }

        // Check if player has the skill
        int currentSkill = 0;
        if (context->playerStats.skills.count(school)) {
            currentSkill = context->playerStats.skills.at(school);
        }

        // Initialize skill if it doesn't exist yet
        if (currentSkill == 0) {
            context->playerStats.skills[school] = 0;
        }

        // Check if difficulty is appropriate
        if (difficulty > currentSkill + 3) {
            std::cout << "This training is too difficult for your current skill level." << std::endl;
            return;
        }

        // Set up the training session
        currentSession.school = school;
        currentSession.focus = focus;
        currentSession.duration = hours;
        currentSession.difficulty = difficulty;
        currentSession.inProgress = true;

        std::cout << "Beginning " << school << " training with focus on " << getFocusName(focus) << "." << std::endl;
        std::cout << "Training will last " << hours << " hours at difficulty " << difficulty << "." << std::endl;
    }

    // Complete the current training session
    void completeTraining(GameContext* context)
    {
        if (!currentSession.inProgress) {
            std::cout << "You don't have an active training session." << std::endl;
            return;
        }

        std::string school = currentSession.school;
        TrainingFocus focus = currentSession.focus;
        int hours = currentSession.duration;
        int difficulty = currentSession.difficulty;

        // Get current skill level
        int currentSkill = context->playerStats.skills.at(school);

        // Calculate base progress based on time spent and difficulty
        float baseProgress = hours * (difficulty * 0.5f);

        // Adjust for intelligence
        float intMultiplier = 0.8f + (context->playerStats.intelligence * 0.02f);
        baseProgress *= intMultiplier;

        // Adjust for current skill level (diminishing returns at higher levels)
        float skillPenalty = 1.0f - (std::min(50, currentSkill) * 0.01f);
        baseProgress *= skillPenalty;

        // Adjust for focus-specific bonuses
        switch (focus) {
        case TrainingFocus::Control:
            // Improves chance to successfully cast complex spells
            baseProgress *= 1.2f; // Control is harder to master, more rewarding
            context->playerStats.learnFact("magic_control_technique_" + std::to_string(difficulty));
            break;

        case TrainingFocus::Power:
            // Improves spell magnitude
            baseProgress *= 1.1f;
            context->playerStats.learnFact("magic_power_technique_" + std::to_string(difficulty));
            break;

        case TrainingFocus::Speed:
            // Reduces casting time
            baseProgress *= 1.0f;
            context->playerStats.learnFact("magic_speed_technique_" + std::to_string(difficulty));
            break;

        case TrainingFocus::Range:
            // Increases spell range
            baseProgress *= 0.9f;
            context->playerStats.learnFact("magic_range_technique_" + std::to_string(difficulty));
            break;

        case TrainingFocus::Efficiency:
            // Balanced improvement to all aspects
            baseProgress *= 0.8f; // Less focused, less progress, but more well-rounded
            context->playerStats.learnFact("magic_efficiency_technique_" + std::to_string(difficulty));
            break;
        }

        // Apply random factor (80-120%)
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0.8, 1.2);
        float randomFactor = dis(gen);
        baseProgress *= randomFactor;

        // Ensure minimum progress
        int finalProgress = std::max(1, static_cast<int>(baseProgress));

        // Initialize progress tracking if it doesn't exist
        if (!skillProgress.count(school)) {
            skillProgress[school] = 0;
        }

        // Add progress
        skillProgress[school] += finalProgress;

        std::cout << "Training complete! You gained " << finalProgress << " progress in " << school << "." << std::endl;

        // Check for level up
        if (skillProgress[school] >= 100) {
            int levelsGained = skillProgress[school] / 100;
            skillProgress[school] %= 100;

            context->playerStats.improveSkill(school, levelsGained);

            std::cout << "Your " << school << " skill increased by " << levelsGained << " to "
                      << context->playerStats.skills[school] << "!" << std::endl;
        } else {
            std::cout << "Progress to next level: " << skillProgress[school] << "/100" << std::endl;
        }

        // Track completed sessions
        if (!trainingSessionsCompleted.count(school)) {
            trainingSessionsCompleted[school] = 0;
        }
        trainingSessionsCompleted[school]++;

        // Special rewards for milestone sessions
        if (trainingSessionsCompleted[school] == 5) {
            std::cout << "Your dedication to " << school << " has paid off! You've gained a deeper understanding." << std::endl;
            context->playerStats.unlockAbility(school + "_insight");
        } else if (trainingSessionsCompleted[school] == 25) {
            std::cout << "You've achieved mastery in " << school << " training techniques!" << std::endl;
            context->playerStats.unlockAbility(school + "_mastery");
        }

        // End the session
        currentSession.inProgress = false;
    }

    // Abandon the current training session
    void abandonTraining()
    {
        if (!currentSession.inProgress) {
            std::cout << "You don't have an active training session." << std::endl;
            return;
        }

        std::cout << "You've abandoned your " << currentSession.school << " training session." << std::endl;
        currentSession.inProgress = false;
    }

    // Get available actions for magic training
    std::vector<TAAction> getAvailableActions() override
    {
        std::vector<TAAction> actions = TANode::getAvailableActions();

        if (!currentSession.inProgress) {
            actions.push_back(
                { "start_training", "Start a training session", []() -> TAInput {
                     return { "training_action", { { "action", std::string("start") } } };
                 } });
        } else {
            actions.push_back(
                { "complete_training", "Complete training session", []() -> TAInput {
                     return { "training_action", { { "action", std::string("complete") } } };
                 } });

            actions.push_back(
                { "abandon_training", "Abandon training session", []() -> TAInput {
                     return { "training_action", { { "action", std::string("abandon") } } };
                 } });
        }

        actions.push_back(
            { "exit_training", "Exit the Training Chamber", []() -> TAInput {
                 return { "training_action", { { "action", std::string("exit") } } };
             } });

        return actions;
    }

    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override
    {
        if (input.type == "training_action") {
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
};

// Integration function to set up the full spell crafting system in a game
void setupSpellCraftingSystem(TAController& controller, TANode* worldRoot)
{
    std::cout << "Setting up Spell Crafting System..." << std::endl;

    // Create the main spell crafting nodes
    SpellCraftingNode* spellCraftingNode = dynamic_cast<SpellCraftingNode*>(controller.createNode<SpellCraftingNode>("SpellCrafting"));

    SpellExaminationNode* spellExaminationNode = dynamic_cast<SpellExaminationNode*>(
        controller.createNode<SpellExaminationNode>("SpellExamination", spellCraftingNode->knownSpells));

    SpellbookNode* spellbookNode = dynamic_cast<SpellbookNode*>(
        controller.createNode<SpellbookNode>("Spellbook", spellCraftingNode->knownSpells));

    MagicTrainingNode* magicTrainingNode = dynamic_cast<MagicTrainingNode*>(controller.createNode<MagicTrainingNode>("MagicTraining"));

    // Initialize basic components
    // Fire magic
    SpellComponent* fireDamage = new SpellComponent(
        "fire_damage",
        "Flames",
        SpellEffectType::Damage,
        10, // mana cost
        15, // base power
        2 // complexity
    );
    fireDamage->schoolRequirements["destruction"] = 1;
    fireDamage->description = "A basic fire damage effect that burns the target.";

    SpellComponent* fireballEffect = new SpellComponent(
        "fireball_effect",
        "Fireball",
        SpellEffectType::Damage,
        20, // mana cost
        25, // base power
        3 // complexity
    );
    fireballEffect->schoolRequirements["destruction"] = 2;
    fireballEffect->description = "A powerful explosive fire effect that damages the target and surrounding area.";

    // Ice magic
    SpellComponent* frostDamage = new SpellComponent(
        "frost_damage",
        "Frost",
        SpellEffectType::Damage,
        12, // mana cost
        12, // base power
        2 // complexity
    );
    frostDamage->schoolRequirements["destruction"] = 1;
    frostDamage->description = "A cold effect that damages and slows the target.";

    // Healing magic
    SpellComponent* healingTouch = new SpellComponent(
        "healing_touch",
        "Healing Touch",
        SpellEffectType::Healing,
        15, // mana cost
        20, // base power
        2 // complexity
    );
    healingTouch->schoolRequirements["restoration"] = 1;
    healingTouch->description = "A basic healing effect that restores health to the target.";

    SpellComponent* regeneration = new SpellComponent(
        "regeneration",
        "Regeneration",
        SpellEffectType::Healing,
        25, // mana cost
        10, // base power (per tick)
        3 // complexity
    );
    regeneration->schoolRequirements["restoration"] = 2;
    regeneration->description = "A healing effect that restores health gradually over time.";

    // Protection magic
    SpellComponent* wardEffect = new SpellComponent(
        "ward_effect",
        "Arcane Ward",
        SpellEffectType::Protection,
        18, // mana cost
        30, // base power
        3 // complexity
    );
    wardEffect->schoolRequirements["alteration"] = 2;
    wardEffect->description = "Creates a protective barrier that absorbs damage.";

    // Initialize modifiers
    SpellModifier* amplifyPower = new SpellModifier(
        "amplify",
        "Amplify",
        1.5f, // power multiplier
        1.0f, // range multiplier
        1.0f, // duration multiplier
        1.0f, // area multiplier
        1.0f, // casting time multiplier
        1.3f // mana cost multiplier
    );
    amplifyPower->requiredSchool = "mysticism";
    amplifyPower->requiredLevel = 1;
    amplifyPower->description = "Increases the power of the spell effect at the cost of more mana.";

    SpellModifier* extend = new SpellModifier(
        "extend",
        "Extend",
        1.0f, // power multiplier
        1.5f, // range multiplier
        1.5f, // duration multiplier
        1.0f, // area multiplier
        1.0f, // casting time multiplier
        1.2f // mana cost multiplier
    );
    extend->description = "Increases the range and duration of the spell effect.";

    SpellModifier* quickCast = new SpellModifier(
        "quickcast",
        "Quick Cast",
        0.9f, // power multiplier
        1.0f, // range multiplier
        1.0f, // duration multiplier
        1.0f, // area multiplier
        0.7f, // casting time multiplier
        1.1f // mana cost multiplier
    );
    quickCast->requiredSchool = "destruction";
    quickCast->requiredLevel = 3;
    quickCast->description = "Reduces casting time at the cost of slightly reduced power.";

    SpellModifier* widen = new SpellModifier(
        "widen",
        "Widen",
        0.8f, // power multiplier
        1.0f, // range multiplier
        1.0f, // duration multiplier
        2.0f, // area multiplier
        1.1f, // casting time multiplier
        1.3f // mana cost multiplier
    );
    widen->requiredSchool = "destruction";
    widen->requiredLevel = 2;
    widen->description = "Increases the area of effect at the cost of reduced power.";

    // Initialize delivery methods
    SpellDelivery* touchDelivery = new SpellDelivery(
        "touch",
        "Touch",
        SpellDeliveryMethod::Touch,
        0, // mana cost modifier
        0.8f, // casting time modifier
        1.0f // range base
    );
    touchDelivery->description = "Delivers the spell effect through direct contact.";

    SpellDelivery* projectileDelivery = new SpellDelivery(
        "projectile",
        "Projectile",
        SpellDeliveryMethod::Projectile,
        5, // mana cost modifier
        1.0f, // casting time modifier
        20.0f // range base
    );
    projectileDelivery->requiredSchool = "destruction";
    projectileDelivery->requiredLevel = 2;
    projectileDelivery->description = "Launches a magical projectile that delivers the effect on impact.";

    SpellDelivery* aoeDelivery = new SpellDelivery(
        "aoe",
        "Area Effect",
        SpellDeliveryMethod::AreaOfEffect,
        10, // mana cost modifier
        1.2f, // casting time modifier
        10.0f // range base
    );
    aoeDelivery->requiredSchool = "destruction";
    aoeDelivery->requiredLevel = 3;
    aoeDelivery->description = "Creates an effect that covers an area, affecting all targets within.";

    SpellDelivery* selfDelivery = new SpellDelivery(
        "self",
        "Self",
        SpellDeliveryMethod::Self,
        -5, // mana cost modifier (discount)
        0.7f, // casting time modifier
        0.0f // range base
    );
    selfDelivery->description = "Applies the spell effect to yourself.";

    SpellDelivery* runeDelivery = new SpellDelivery(
        "rune",
        "Rune",
        SpellDeliveryMethod::Rune,
        15, // mana cost modifier
        1.5f, // casting time modifier
        5.0f // range base
    );
    runeDelivery->requiredSchool = "alteration";
    runeDelivery->requiredLevel = 3;
    runeDelivery->description = "Creates a magical rune that triggers the spell effect when activated.";

    // Add components to crafting system
    spellCraftingNode->availableComponents.push_back(fireDamage);
    spellCraftingNode->availableComponents.push_back(frostDamage);
    spellCraftingNode->availableComponents.push_back(healingTouch);

    // Add more advanced components based on player skill level
    // These would typically be discovered through gameplay
    GameContext& context = controller.gameContext;
    if (context.playerStats.hasSkill("destruction", 2)) {
        spellCraftingNode->availableComponents.push_back(fireballEffect);
    }

    if (context.playerStats.hasSkill("restoration", 2)) {
        spellCraftingNode->availableComponents.push_back(regeneration);
    }

    if (context.playerStats.hasSkill("alteration", 2)) {
        spellCraftingNode->availableComponents.push_back(wardEffect);
    }

    // Add modifiers to crafting system
    spellCraftingNode->availableModifiers.push_back(amplifyPower);
    spellCraftingNode->availableModifiers.push_back(extend);

    if (context.playerStats.hasSkill("destruction", 3)) {
        spellCraftingNode->availableModifiers.push_back(quickCast);
    }

    if (context.playerStats.hasSkill("destruction", 2)) {
        spellCraftingNode->availableModifiers.push_back(widen);
    }

    // Add delivery methods to crafting system
    spellCraftingNode->availableDeliveryMethods.push_back(touchDelivery);
    spellCraftingNode->availableDeliveryMethods.push_back(selfDelivery);

    if (context.playerStats.hasSkill("destruction", 2)) {
        spellCraftingNode->availableDeliveryMethods.push_back(projectileDelivery);
    }

    if (context.playerStats.hasSkill("destruction", 3)) {
        spellCraftingNode->availableDeliveryMethods.push_back(aoeDelivery);
    }

    if (context.playerStats.hasSkill("alteration", 3)) {
        spellCraftingNode->availableDeliveryMethods.push_back(runeDelivery);
    }

    // Create pre-defined spells for study
    SpellDesign* fireball = new SpellDesign("fireball", "Fireball");
    fireball->components.push_back(fireballEffect);
    fireball->modifiers.push_back(amplifyPower);
    fireball->delivery = projectileDelivery;
    fireball->targetType = SpellTargetType::SingleTarget;
    fireball->description = "A basic offensive spell that launches a ball of fire.";
    fireball->calculateAttributes(context);
    fireball->complexityRating = 3;

    SpellDesign* frostbolt = new SpellDesign("frostbolt", "Frostbolt");
    frostbolt->components.push_back(frostDamage);
    frostbolt->delivery = projectileDelivery;
    frostbolt->targetType = SpellTargetType::SingleTarget;
    frostbolt->description = "A spell that launches a bolt of freezing energy.";
    frostbolt->calculateAttributes(context);
    frostbolt->complexityRating = 2;

    SpellDesign* healingSurge = new SpellDesign("healing_surge", "Healing Surge");
    healingSurge->components.push_back(healingTouch);
    healingSurge->modifiers.push_back(amplifyPower);
    healingSurge->delivery = touchDelivery;
    healingSurge->targetType = SpellTargetType::SingleTarget;
    healingSurge->description = "A powerful healing spell that quickly restores health.";
    healingSurge->calculateAttributes(context);
    healingSurge->complexityRating = 3;

    SpellDesign* arcanicBarrier = new SpellDesign("arcanic_barrier", "Arcanic Barrier");
    arcanicBarrier->components.push_back(wardEffect);
    arcanicBarrier->delivery = selfDelivery;
    arcanicBarrier->targetType = SpellTargetType::Self;
    arcanicBarrier->description = "Creates a protective magical barrier around the caster.";
    arcanicBarrier->calculateAttributes(context);
    arcanicBarrier->complexityRating = 3;

    // Add pre-defined spells to examination node
    spellExaminationNode->availableSpells.push_back(fireball);
    spellExaminationNode->availableSpells.push_back(frostbolt);
    spellExaminationNode->availableSpells.push_back(healingSurge);
    spellExaminationNode->availableSpells.push_back(arcanicBarrier);

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

        CombatState()
            : inCombat(false)
            , castingCooldown(0.0f)
            , currentlyCasting(nullptr)
            , castProgress(0.0f)
        {
        }
    } combatState;

    BattleMagicSystem(std::vector<SpellDesign*>& knownSpells)
        : playerSpells(knownSpells)
    {
        // Initialize battle slots
        for (auto& slot : battleSlots) {
            slot = nullptr;
        }
    }

    // Assign a spell to a battle slot
    bool assignToBattleSlot(SpellDesign* spell, int slotIndex)
    {
        if (slotIndex < 0 || slotIndex >= static_cast<int>(battleSlots.size())) {
            return false;
        }

        battleSlots[slotIndex] = spell;
        return true;
    }

    // Begin casting a spell in combat
    bool beginCasting(SpellDesign* spell, GameContext* context)
    {
        if (combatState.castingCooldown > 0.0f || combatState.currentlyCasting) {
            return false; // Already casting or on cooldown
        }

        if (!spell->canCast(*context)) {
            return false; // Can't cast this spell
        }

        combatState.currentlyCasting = spell;
        combatState.castProgress = 0.0f;

        return true;
    }

    // Begin casting from a battle slot
    bool castFromBattleSlot(int slotIndex, GameContext* context)
    {
        if (slotIndex < 0 || slotIndex >= static_cast<int>(battleSlots.size())) {
            return false;
        }

        if (!battleSlots[slotIndex]) {
            return false;
        }

        return beginCasting(battleSlots[slotIndex], context);
    }

    // Update casting progress (call this each combat frame)
    void updateCasting(float deltaTime, GameContext* context)
    {
        // Update cooldown
        if (combatState.castingCooldown > 0.0f) {
            combatState.castingCooldown -= deltaTime;
            if (combatState.castingCooldown < 0.0f) {
                combatState.castingCooldown = 0.0f;
            }
        }

        // Update spell casting
        if (combatState.currentlyCasting) {
            float castTime = combatState.currentlyCasting->castingTime;

            combatState.castProgress += deltaTime;

            // Check for spell completion
            if (combatState.castProgress >= castTime) {
                // Cast the spell
                bool success = combatState.currentlyCasting->cast(context);

                if (success) {
                    // Set cooldown (could be spell-specific)
                    combatState.castingCooldown = 0.5f; // Global cooldown
                } else {
                    // Failed cast - shorter cooldown
                    combatState.castingCooldown = 0.2f;
                }

                // Reset casting state
                combatState.currentlyCasting = nullptr;
                combatState.castProgress = 0.0f;
            }
        }
    }

    // Cancel the current spell cast
    void cancelCasting()
    {
        if (combatState.currentlyCasting) {
            combatState.currentlyCasting = nullptr;
            combatState.castProgress = 0.0f;
            combatState.castingCooldown = 0.2f; // Small cooldown for canceling
        }
    }

    // Get casting progress as percentage
    float getCastingProgressPercent() const
    {
        if (!combatState.currentlyCasting) {
            return 0.0f;
        }

        return (combatState.castProgress / combatState.currentlyCasting->castingTime) * 100.0f;
    }

    // Check if player can cast any spell (not on cooldown)
    bool canCastSpell() const
    {
        return combatState.castingCooldown <= 0.0f && !combatState.currentlyCasting;
    }

    // Get currently casting spell name
    std::string getCurrentCastingName() const
    {
        if (combatState.currentlyCasting) {
            return combatState.currentlyCasting->name;
        }
        return "";
    }
};

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

    // Create spell crafting node
    SpellCraftingNode* spellCraftingNode = dynamic_cast<SpellCraftingNode*>(controller.createNode<SpellCraftingNode>("SpellCrafting"));

    // Create basic spell components
    SpellComponent* fireDamage = new SpellComponent(
        "fire_damage",
        "Flames",
        SpellEffectType::Damage,
        10, // mana cost
        15, // base power
        2 // complexity
    );
    fireDamage->schoolRequirements["destruction"] = 1;

    SpellComponent* healingTouch = new SpellComponent(
        "healing_touch",
        "Healing Touch",
        SpellEffectType::Healing,
        15, // mana cost
        20, // base power
        2 // complexity
    );
    healingTouch->schoolRequirements["restoration"] = 1;

    SpellComponent* frostDamage = new SpellComponent(
        "frost_damage",
        "Frost",
        SpellEffectType::Damage,
        12, // mana cost
        12, // base power
        2 // complexity
    );
    frostDamage->schoolRequirements["destruction"] = 2;

    // Create modifiers
    SpellModifier* amplifyPower = new SpellModifier(
        "amplify",
        "Amplify",
        1.5f, // power multiplier
        1.0f, // range multiplier
        1.0f, // duration multiplier
        1.0f, // area multiplier
        1.0f, // casting time multiplier
        1.3f // mana cost multiplier
    );
    amplifyPower->requiredSchool = "mysticism";
    amplifyPower->requiredLevel = 1;

    SpellModifier* extend = new SpellModifier(
        "extend",
        "Extend",
        1.0f, // power multiplier
        1.5f, // range multiplier
        1.5f, // duration multiplier
        1.0f, // area multiplier
        1.0f, // casting time multiplier
        1.2f // mana cost multiplier
    );

    // Create delivery methods
    SpellDelivery* touchDelivery = new SpellDelivery(
        "touch",
        "Touch",
        SpellDeliveryMethod::Touch,
        0, // mana cost modifier
        0.8f, // casting time modifier
        1.0f // range base
    );

    SpellDelivery* projectileDelivery = new SpellDelivery(
        "projectile",
        "Projectile",
        SpellDeliveryMethod::Projectile,
        5, // mana cost modifier
        1.0f, // casting time modifier
        20.0f // range base
    );
    projectileDelivery->requiredSchool = "destruction";
    projectileDelivery->requiredLevel = 2;

    SpellDelivery* selfDelivery = new SpellDelivery(
        "self",
        "Self",
        SpellDeliveryMethod::Self,
        -5, // mana cost modifier (discount)
        0.7f, // casting time modifier
        0.0f // range base
    );

    // Set up the spell crafting system
    setupSpellCraftingSystem(controller, worldRoot);

    // Add components to crafting system
    spellCraftingNode->availableComponents.push_back(fireDamage);
    spellCraftingNode->availableComponents.push_back(healingTouch);
    spellCraftingNode->availableComponents.push_back(frostDamage);

    // Add modifiers to crafting system
    spellCraftingNode->availableModifiers.push_back(amplifyPower);
    spellCraftingNode->availableModifiers.push_back(extend);

    // Add delivery methods to crafting system
    spellCraftingNode->availableDeliveryMethods.push_back(touchDelivery);
    spellCraftingNode->availableDeliveryMethods.push_back(projectileDelivery);
    spellCraftingNode->availableDeliveryMethods.push_back(selfDelivery);

    // Create spell examination node
    SpellExaminationNode* examinationNode = dynamic_cast<SpellExaminationNode*>(
        controller.createNode<SpellExaminationNode>("SpellExamination", spellCraftingNode->knownSpells));

    // Create pre-defined spells for study
    SpellDesign* fireball = new SpellDesign("fireball", "Fireball");
    fireball->components.push_back(fireDamage);
    fireball->modifiers.push_back(amplifyPower);
    fireball->delivery = projectileDelivery;
    fireball->targetType = SpellTargetType::SingleTarget;
    fireball->description = "A basic offensive spell that launches a ball of fire.";
    fireball->calculateAttributes(gameContext);
    fireball->complexityRating = 3;

    examinationNode->availableSpells.push_back(fireball);

    // Register the world and spell system
    controller.setSystemRoot("WorldSystem", worldRoot);
    controller.setSystemRoot("SpellSystem", spellCraftingNode);

    // Demo: Enter the world and mages guild
    controller.processInput("WorldSystem", {});
    controller.processInput("WorldSystem",
        { "world_action", { { "action", std::string("enter_spell_system") } } });
    controller.processInput("SpellSystem",
        { "spell_system", { { "destination", std::string("crafting") } } });

    // Scenario 1: Create a Healing Aura spell
    std::cout << "\n=== DEMO: CREATING A HEALING AURA SPELL ===\n"
              << std::endl;
    spellCraftingNode->startNewSpell("Healing Aura", &gameContext);
    spellCraftingNode->addComponent(1, &gameContext);
    spellCraftingNode->addModifier(1, &gameContext);
    spellCraftingNode->setDeliveryMethod(0, &gameContext);
    spellCraftingNode->setTargetType(SpellTargetType::Self, &gameContext);
    spellCraftingNode->finalizeSpell(&gameContext);

    // Scenario 2: Create a Fire Shield spell
    std::cout << "\n=== DEMO: CREATING A FIRE SHIELD SPELL ===\n"
              << std::endl;
    spellCraftingNode->startNewSpell("Fire Shield", &gameContext);
    spellCraftingNode->addComponent(0, &gameContext);
    spellCraftingNode->addModifier(0, &gameContext);
    spellCraftingNode->setDeliveryMethod(2, &gameContext); // Self delivery
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

    // Spell Study
    std::cout << "\n=== DEMO: SPELL STUDY ===\n"
              << std::endl;
    controller.processInput("SpellSystem", { "spellcraft_action", { { "action", std::string("exit") } } });
    std::cout << "Examining 'Fireball' spell..." << std::endl;
    examinationNode->examineSpell(0, &gameContext);
    std::cout << "Studying the spell for 5 hours..." << std::endl;
    examinationNode->studySpell(5, &gameContext);

    // Magic Training
    std::cout << "\n=== DEMO: MAGIC TRAINING ===\n"
              << std::endl;
    controller.processInput("SpellSystem",
        { "spell_system", { { "destination", std::string("training") } } });
    MagicTrainingNode* magicTraining = dynamic_cast<MagicTrainingNode*>(controller.currentNodes["SpellSystem"]);

    if (magicTraining) {
        std::cout << "Starting a destruction magic training session..." << std::endl;
        magicTraining->startTraining("destruction", MagicTrainingNode::TrainingFocus::Power, 3, 2, &gameContext);
        std::cout << "Completing the training session..." << std::endl;
        magicTraining->completeTraining(&gameContext);
    }

    // Battle Magic Demonstration
    std::cout << "\n=== DEMO: BATTLE MAGIC ===\n"
              << std::endl;
    if (!spellCraftingNode->knownSpells.empty()) {
        BattleMagicSystem battleMagic(spellCraftingNode->knownSpells);
        battleMagic.assignToBattleSlot(spellCraftingNode->knownSpells[0], 0);
        std::cout << "Assigned " << spellCraftingNode->knownSpells[0]->name << " to battle slot 1" << std::endl;

        std::cout << "Entering combat..." << std::endl;
        battleMagic.combatState.inCombat = true;

        std::cout << "Casting from battle slot 1..." << std::endl;
        if (battleMagic.castFromBattleSlot(0, &gameContext)) {
            std::cout << "Beginning to cast " << battleMagic.getCurrentCastingName() << "..." << std::endl;

            for (int i = 1; i <= 5; i++) {
                battleMagic.updateCasting(0.2f, &gameContext);
                std::cout << "Casting progress: " << battleMagic.getCastingProgressPercent() << "%" << std::endl;
            }

            battleMagic.updateCasting(2.0f, &gameContext);
            std::cout << "Spell cast complete!" << std::endl;
        }
    }

    // Final summary
    std::cout << "\n=== MAGICAL KNOWLEDGE SUMMARY ===\n"
              << std::endl;
    std::cout << "Intelligence: " << gameContext.playerStats.intelligence << std::endl;

    std::cout << "\nMagical Skills:" << std::endl;
    for (const auto& [skill, level] : gameContext.playerStats.skills) {
        if (skill == "destruction" || skill == "restoration" || skill == "alteration" || skill == "mysticism" || skill == "illusion" || skill == "conjuration") {
            std::cout << "- " << skill << ": " << level << std::endl;
        }
    }

    std::cout << "\nKnown Spells:" << std::endl;
    for (const SpellDesign* spell : spellCraftingNode->knownSpells) {
        std::cout << "- " << spell->name << std::endl;
    }

    std::cout << "\nMagical Knowledge:" << std::endl;
    for (const std::string& fact : gameContext.playerStats.knownFacts) {
        if (fact.find("magic") != std::string::npos) {
            std::cout << "- " << fact << std::endl;
        }
    }

    return 0;
}
