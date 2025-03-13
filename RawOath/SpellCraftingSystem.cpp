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
}
;

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

// Main function to test and demonstrate the spell crafting system
int main()
{
    std::cout << "=== Spell Crafting System Demo ===" << std::endl;

    // Create controller
    TAController controller;
    GameContext& gameContext = controller.gameContext;

    // Set up some basic player stats for testing
    gameContext.playerStats.intelligence = 14;
    gameContext.playerStats.improveSkill("destruction", 3);
    gameContext.playerStats.improveSkill("restoration", 2);
    gameContext.playerStats.improveSkill("mysticism", 1);

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

    // Create spell examination node
    SpellExaminationNode* examinationNode = dynamic_cast<SpellExaminationNode*>(
        controller.createNode<SpellExaminationNode>("SpellExamination", spellCraftingNode->knownSpells));

    // Create a pre-defined spell for study
    SpellDesign* fireball = new SpellDesign("fireball", "Fireball");
    fireball->components.push_back(fireDamage);
    fireball->modifiers.push_back(amplifyPower);
    fireball->delivery = projectileDelivery;
    fireball->targetType = SpellTargetType::SingleTarget;
    fireball->description = "A basic offensive spell that launches a ball of fire.";
    fireball->calculateAttributes(gameContext);
    fireball->complexityRating = 3;

    examinationNode->availableSpells.push_back(fireball);

    // Set up transitions between nodes
    spellCraftingNode->addTransition(
        [](const TAInput& input) {
            return input.type == "spellcraft_action" && std::get<std::string>(input.parameters.at("action")) == "exit";
        },
        examinationNode, "Exit to Arcane Library");

    examinationNode->addTransition(
        [](const TAInput& input) {
            return input.type == "examination_action" && std::get<std::string>(input.parameters.at("action")) == "exit";
        },
        spellCraftingNode, "Exit to Spell Crafting");

    // Register the spell crafting system
    controller.setSystemRoot("SpellSystem", spellCraftingNode);

    // Demo: Design and cast a spell
    std::cout << "\n=== DEMO: CREATING A SPELL ===\n"
              << std::endl;

    // Start crafting
    controller.processInput("SpellSystem", {});

    // Start new spell
    std::cout << "Creating a new spell 'Healing Aura'..." << std::endl;
    spellCraftingNode->startNewSpell("Healing Aura", &gameContext);

    // Add healing component
    std::cout << "Adding Healing Touch component..." << std::endl;
    spellCraftingNode->addComponent(1, &gameContext);

    // Add extend modifier
    std::cout << "Adding Extend modifier..." << std::endl;
    spellCraftingNode->addModifier(1, &gameContext);

    // Set touch delivery
    std::cout << "Setting delivery method to Touch..." << std::endl;
    spellCraftingNode->setDeliveryMethod(0, &gameContext);

    // Set target type
    std::cout << "Setting target type to Self..." << std::endl;
    spellCraftingNode->setTargetType(SpellTargetType::Self, &gameContext);

    // Finalize spell
    std::cout << "Finalizing the spell..." << std::endl;
    spellCraftingNode->finalizeSpell(&gameContext);

    // Test casting the spell
    std::cout << "\n=== DEMO: CASTING THE SPELL ===\n"
              << std::endl;
    spellCraftingNode->castSpell(0, &gameContext);

    // Demo: Research a new component
    std::cout << "\n=== DEMO: MAGICAL RESEARCH ===\n"
              << std::endl;
    SpellResearchResult result = spellCraftingNode->conductResearch("destruction", 3, &gameContext);
    std::cout << result.message << std::endl;

    if (result.type == SpellResearchResult::Success || result.type == SpellResearchResult::PartialSuccess) {
        std::cout << "Skill improvement: " << static_cast<int>(result.skillProgress / 10.0f) << " points" << std::endl;
    }

    // Demo: Study an existing spell
    std::cout << "\n=== DEMO: STUDYING A SPELL ===\n"
              << std::endl;

    // Switch to examination node
    controller.processInput("SpellSystem", { "spellcraft_action", { { "action", std::string("exit") } } });

    // Examine the fireball spell
    std::cout << "Examining 'Fireball' spell..." << std::endl;
    examinationNode->examineSpell(0, &gameContext);

    // Study the spell
    std::cout << "Studying the spell for 5 hours..." << std::endl;
    examinationNode->studySpell(5, &gameContext);

    std::cout << "\nFinal player stats:" << std::endl;
    std::cout << "Intelligence: " << gameContext.playerStats.intelligence << std::endl;
    for (const auto& [skill, level] : gameContext.playerStats.skills) {
        std::cout << skill << ": " << level << std::endl;
    }

    std::cout << "\nKnown spells:" << std::endl;
    for (const SpellDesign* spell : spellCraftingNode->knownSpells) {
        std::cout << "- " << spell->name << std::endl;
    }

    return 0;
}