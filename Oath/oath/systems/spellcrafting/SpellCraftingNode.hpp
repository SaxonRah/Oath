#pragma once

#include "../../core/TANode.hpp"
#include "SpellCraftingSystem.hpp"
#include "SpellResearch.hpp"
#include <map>
#include <vector>


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

// Forward declarations for classes that interact with SpellCraftingNode
class SpellExaminationNode;
class SpellbookNode;
class MagicTrainingNode;

// Integration function to set up the full spell crafting system in a game
void setupSpellCraftingSystem(TAController& controller, TANode* worldRoot);

// Helper function to find a node by name in the node hierarchy
TANode* findNodeByName(TANode* root, const std::string& name);