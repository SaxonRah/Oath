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

// Event handlers for gameplay integration

// Handle player entering a magical location
void onEnterMagicalLocation(GameContext* context, const std::string& locationType)
{
    // Discover magical properties and potential for spell components
    if (locationType == "leyline") {
        context->playerStats.learnFact("leyline_magic_properties");
        std::cout << "You feel magical energy coursing through this area. "
                  << "This would be an excellent place for magical research." << std::endl;
    } else if (locationType == "ancient_ruin") {
        context->playerStats.learnFact("ancient_magic_techniques");
        std::cout << "The remnants of ancient magical workings still linger here. "
                  << "You might be able to learn forgotten spells." << std::endl;
    } else if (locationType == "elemental_node") {
        context->playerStats.learnFact("elemental_magic_source");
        std::cout << "Pure elemental energy saturates this location. "
                  << "Spells aligned with this element would be particularly effective here." << std::endl;
    }
}

// Handle finding a spell tome or scroll
SpellDesign* onFindSpellTome(GameContext* context, const std::string& spellId, SpellExaminationNode* examinationNode)
{
    // Check if the tome contains a spell we already have available for study
    for (SpellDesign* spell : examinationNode->availableSpells) {
        if (spell->id == spellId) {
            std::cout << "You found a spell tome containing the " << spell->name << " spell." << std::endl;
            return spell;
        }
    }

    // If not found, it could be a newly discovered spell
    std::cout << "You found an ancient spell tome. It requires further study." << std::endl;
    context->playerStats.learnFact("found_spell_tome_" + spellId);

    return nullptr;
}

// Handle magical catastrophe events
void onMagicalCatastrophe(GameContext* context, const std::string& catastropheType)
{
    if (catastropheType == "spell_surge") {
        std::cout << "A surge of wild magic sweeps through the area!" << std::endl;

        // Random magical effects based on player's spell knowledge
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(1, 5);
        int effect = dis(gen);

        switch (effect) {
        case 1:
            std::cout << "The magical energies temporarily enhance your spellcasting abilities." << std::endl;
            // Apply a buff
            break;
        case 2:
            std::cout << "The chaotic magic interferes with your spellcasting." << std::endl;
            // Apply a debuff
            break;
        case 3:
            std::cout << "The magical surge reveals hidden magical properties of the area." << std::endl;
            context->playerStats.learnFact("magical_properties_revealed");
            break;
        case 4:
            std::cout << "The surge of energy attracts magical creatures to the area." << std::endl;
            // Spawn magical creatures
            break;
        case 5:
            std::cout << "The magical energies coalesce into a temporary portal!" << std::endl;
            // Create a temporary portal to another location
            break;
        }
    } else if (catastropheType == "spell_backfire") {
        std::cout << "Your spell catastrophically backfires!" << std::endl;

        // Effects based on the spell school
        std::string primarySchool = "destruction"; // Should come from the actually cast spell

        if (primarySchool == "destruction") {
            std::cout << "A wave of destructive energy bursts forth, damaging everything nearby!" << std::endl;
            // Apply damage to player and surroundings
        } else if (primarySchool == "alteration") {
            std::cout << "Reality warps around you in unpredictable ways!" << std::endl;
            // Apply random transmutation effects
        } else if (primarySchool == "restoration") {
            std::cout << "The healing energies invert, draining vitality from living things!" << std::endl;
            // Apply damage over time to living things
        }
    }
}

// Main program for testing the spell crafting system
int main()
{
    std::cout << "=== Daggerfall-Style Spell Crafting System ===" << std::endl;

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

    // Set up the spell crafting system
    setupSpellCraftingSystem(controller, worldRoot);

    // Register the world
    controller.setSystemRoot("WorldSystem", worldRoot);

    // Demo: Enter the mages guild and create a spell
    std::cout << "\n=== DEMO: VISITING THE MAGES GUILD ===\n"
              << std::endl;

    // Start in the world
    controller.processInput("WorldSystem", {});

    // Enter the mages guild
    std::cout << "Entering the Mages Guild..." << std::endl;
    controller.processInput("WorldSystem",
        { "world_action", { { "action", std::string("enter_spell_system") } } });

    // Go to spell crafting
    std::cout << "Going to the Spell Crafting Chamber..." << std::endl;
    controller.processInput("SpellSystem",
        { "spell_system", { { "destination", std::string("crafting") } } });

    // Access the spell crafting node
    SpellCraftingNode* spellCrafting = dynamic_cast<SpellCraftingNode*>(controller.currentNodes["SpellSystem"]);

    if (spellCrafting) {
        // Create a new spell
        std::cout << "Creating a new spell 'Fire Shield'..." << std::endl;
        spellCrafting->startNewSpell("Fire Shield", &gameContext);

        // Add components and modifiers
        std::cout << "Adding components and modifiers..." << std::endl;

        // Assuming the first component is fire damage for this example
        spellCrafting->addComponent(0, &gameContext);

        // Assuming the first modifier is amplify
        spellCrafting->addModifier(0, &gameContext);

        // Set delivery to self
        spellCrafting->setDeliveryMethod(1, &gameContext); // Self delivery

        // Set target type
        spellCrafting->setTargetType(SpellTargetType::Self, &gameContext);

        // Finalize spell
        std::cout << "Finalizing the spell..." << std::endl;
        spellCrafting->finalizeSpell(&gameContext);

        // Test casting the spell
        std::cout << "\n=== CASTING THE NEW SPELL ===\n"
                  << std::endl;
        if (!spellCrafting->knownSpells.empty()) {
            spellCrafting->castSpell(0, &gameContext);
        }
    }

    // Visit the Magic Training Chamber
    std::cout << "\n=== VISITING THE MAGIC TRAINING CHAMBER ===\n"
              << std::endl;
    controller.processInput("SpellSystem",
        { "spell_system", { { "destination", std::string("training") } } });

    // Access the magic training node
    MagicTrainingNode* magicTraining = dynamic_cast<MagicTrainingNode*>(controller.currentNodes["SpellSystem"]);

    if (magicTraining) {
        // Start a training session
        std::cout << "Starting a destruction magic training session..." << std::endl;
        magicTraining->startTraining("destruction", MagicTrainingNode::TrainingFocus::Power, 3, 2, &gameContext);

        // Complete the training
        std::cout << "Completing the training session..." << std::endl;
        magicTraining->completeTraining(&gameContext);
    }

    // Demonstrate battle magic in a simulated combat scenario
    std::cout << "\n=== BATTLE MAGIC DEMONSTRATION ===\n"
              << std::endl;

    if (!spellCrafting->knownSpells.empty()) {
        BattleMagicSystem battleMagic(spellCrafting->knownSpells);

        // Assign spell to battle slot
        battleMagic.assignToBattleSlot(spellCrafting->knownSpells[0], 0);
        std::cout << "Assigned " << spellCrafting->knownSpells[0]->name << " to battle slot 1" << std::endl;

        // Simulate combat
        std::cout << "Entering combat..." << std::endl;
        battleMagic.combatState.inCombat = true;

        // Cast from battle slot
        std::cout << "Casting from battle slot 1..." << std::endl;
        if (battleMagic.castFromBattleSlot(0, &gameContext)) {
            std::cout << "Beginning to cast " << battleMagic.getCurrentCastingName() << "..." << std::endl;

            // Simulate time passing
            for (int i = 1; i <= 5; i++) {
                battleMagic.updateCasting(0.2f, &gameContext);
                std::cout << "Casting progress: " << battleMagic.getCastingProgressPercent() << "%" << std::endl;
            }

            // Final update to complete the cast
            battleMagic.updateCasting(2.0f, &gameContext);
            std::cout << "Spell cast complete!" << std::endl;
        }
    }

    // Final display of player's magical knowledge
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
    for (const SpellDesign* spell : spellCrafting->knownSpells) {
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