// Oath_JSON.hpp
#ifndef OATH_JSON_HPP
#define OATH_JSON_HPP

#include <algorithm>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <queue>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

// Forward declarations
class TANode;
class TAController;
class Inventory;
class NPC;
class Recipe;

class QuestNode;
class DialogueNode;
class SkillNode;
class ClassNode;
class CraftingNode;
class LocationNode;
class TimeNode;
class RegionNode;

// A unique identifier for nodes
struct NodeID {
    unsigned int data1;
    unsigned int data2;
    unsigned int data3;
    unsigned int data4;
    std::string persistentID; // Path-based or name-based persistent ID

    bool operator==(const NodeID& other) const;
    bool operator<(const NodeID& other) const;
    static NodeID Generate(const std::string& nodePath = "");
    std::string toString() const;
};

// An input that can trigger transitions
struct TAInput {
    std::string type;
    std::map<std::string, std::variant<int, float, std::string, bool>> parameters;
};

// Available actions from a state
struct TAAction {
    std::string name;
    std::string description;
    std::function<TAInput()> createInput;
};

// Transition rule
struct TATransitionRule {
    std::function<bool(const TAInput&)> condition;
    TANode* targetNode;
    std::string description;
};

// Character stats structure for progression and dialogue systems
struct CharacterStats {
    int strength = 10;
    int dexterity = 10;
    int constitution = 10;
    int intelligence = 10;
    int wisdom = 10;
    int charisma = 10;

    std::map<std::string, int> skills;
    std::map<std::string, int> factionReputation;
    std::set<std::string> knownFacts;
    std::set<std::string> unlockedAbilities;

    CharacterStats();
    bool hasSkill(const std::string& skill, int minLevel) const;
    bool hasFactionReputation(const std::string& faction, int minRep) const;
    bool hasKnowledge(const std::string& fact) const;
    bool hasAbility(const std::string& ability) const;
    void learnFact(const std::string& fact);
    void unlockAbility(const std::string& ability);
    void improveSkill(const std::string& skill, int amount);
    void changeFactionRep(const std::string& faction, int amount);
};

// Game world state structure
struct WorldState {
    std::map<std::string, std::string> locationStates;
    std::map<std::string, std::string> factionStates;
    std::map<std::string, bool> worldFlags;
    int daysPassed = 0;
    std::string currentSeason = "spring";

    WorldState();
    bool hasFlag(const std::string& flag) const;
    std::string getLocationState(const std::string& location) const;
    std::string getFactionState(const std::string& faction) const;
    void setLocationState(const std::string& location, const std::string& state);
    void setFactionState(const std::string& faction, const std::string& state);
    void setWorldFlag(const std::string& flag, bool value);
    void advanceDay();
};

// Inventory and item system for crafting
struct Item {
    std::string id;
    std::string name;
    std::string type;
    int value;
    int quantity;
    std::map<std::string, std::variant<int, float, std::string, bool>> properties;

    Item(const std::string& itemId, const std::string& itemName,
        const std::string& itemType, int itemValue = 1, int itemQty = 1);
};

class Inventory {
public:
    std::vector<Item> items;

    bool hasItem(const std::string& itemId, int quantity = 1) const;
    bool addItem(const Item& item);
    bool removeItem(const std::string& itemId, int quantity = 1);
};

// Context for all systems
struct GameContext {
    CharacterStats playerStats;
    WorldState worldState;
    Inventory playerInventory;

    // Add any additional game-specific context here
    std::map<std::string, std::string> questJournal;
    std::map<std::string, std::string> dialogueHistory;
};

// Core node class for tree automata system
class TANode {
public:
    // Unique identifier for this node
    NodeID nodeID;

    // Human-readable name
    std::string nodeName;

    // Current state data - flexible for any system-specific info
    std::map<std::string, std::variant<int, float, std::string, bool>> stateData;

    // Transition rules to other nodes
    std::vector<TATransitionRule> transitionRules;

    // Child nodes (for hierarchical structures)
    std::vector<TANode*> childNodes;

    // Is this a terminal/accepting state?
    bool isAcceptingState;

    TANode(const std::string& name);
    virtual ~TANode() = default;

    // Evaluation function to process inputs
    virtual bool evaluateTransition(const TAInput& input, TANode*& outNextNode);

    // Actions to perform when entering/exiting this node
    virtual void onEnter(GameContext* context);
    virtual void onExit(GameContext* context);

    // Add a transition rule
    void addTransition(const std::function<bool(const TAInput&)>& condition,
        TANode* target, const std::string& description = "");

    // Add a child node
    void addChild(TANode* child);

    // Get available transitions for the current state
    virtual std::vector<TAAction> getAvailableActions();

    // Generate a path-based ID for this node
    void generatePersistentID(const std::string& parentPath = "");

    // Serialize node state
    virtual void serialize(std::ofstream& file) const;

    // Deserialize node state
    virtual bool deserialize(std::ifstream& file);
};

//----------------------------------------
// QUEST SYSTEM
//----------------------------------------

// Quest-specific node implementation
class QuestNode : public TANode {
public:
    // Quest state (Available, Active, Completed, Failed)
    std::string questState;

    // Quest details
    std::string questTitle;
    std::string questDescription;

    // Rewards for completion
    struct QuestReward {
        std::string type;
        int amount;
        std::string itemId;
    };
    std::vector<QuestReward> rewards;

    // Requirements to access this quest
    struct QuestRequirement {
        std::string type; // skill, item, faction, etc.
        std::string target; // skill name, item id, faction name, etc.
        int value; // required value

        bool check(const GameContext& context) const;
    };
    std::vector<QuestRequirement> requirements;

    QuestNode(const std::string& name);

    // Process player action and return next state
    bool processAction(const std::string& playerAction, TANode*& outNextNode);

    // Check if player can access this quest
    bool canAccess(const GameContext& context) const;

    // Activate child quests when this node is entered
    void onEnter(GameContext* context) override;

    // Award rewards when completing quest
    void onExit(GameContext* context) override;

    // Get available actions specific to quests
    std::vector<TAAction> getAvailableActions() override;
};

//----------------------------------------
// DIALOGUE SYSTEM
//----------------------------------------

// Dialogue node for conversation trees
class DialogueNode : public TANode {
public:
    // The text to display for this dialogue node
    std::string speakerName;
    std::string dialogueText;

    // Response options
    struct DialogueResponse {
        std::string text;
        std::function<bool(const GameContext&)> requirement;
        TANode* targetNode;
        std::function<void(GameContext*)> effect;

        DialogueResponse(
            const std::string& responseText, TANode* target,
            std::function<bool(const GameContext&)> req =
                [](const GameContext&) { return true; },
            std::function<void(GameContext*)> eff = [](GameContext*) {});
    };
    std::vector<DialogueResponse> responses;

    // Optional effect to run when this dialogue is shown
    std::function<void(GameContext*)> onShowEffect;

    DialogueNode(const std::string& name, const std::string& speaker,
        const std::string& text);

    void addResponse(
        const std::string& text, TANode* target,
        std::function<bool(const GameContext&)> requirement =
            [](const GameContext&) { return true; },
        std::function<void(GameContext*)> effect = [](GameContext*) {});

    void onEnter(GameContext* context) override;

    // Get available dialogue responses
    std::vector<TAAction> getAvailableActions() override;

    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;
};

// NPC class for dialogue interactions
class NPC {
public:
    std::string name;
    std::string description;
    DialogueNode* rootDialogue;
    DialogueNode* currentDialogue;
    std::map<std::string, DialogueNode*> dialogueNodes;

    // Relationship with player
    int relationshipValue = 0;

    NPC(const std::string& npcName, const std::string& desc);

    void startDialogue(GameContext* context);
    bool processResponse(int responseIndex, GameContext* context);
};

//----------------------------------------
// CHARACTER PROGRESSION SYSTEM
//----------------------------------------

// Skill node for character progression
class SkillNode : public TANode {
public:
    std::string skillName;
    std::string description;
    int level;
    int maxLevel;

    // Requirements to unlock this skill
    struct SkillRequirement {
        std::string type; // "skill", "item", "quest", etc.
        std::string target; // skill name, item id, quest id, etc.
        int level;

        bool check(const GameContext& context) const;
    };
    std::vector<SkillRequirement> requirements;

    // Effects when this skill is learned or improved
    struct SkillEffect {
        std::string type;
        std::string target;
        int value;

        void apply(GameContext* context) const;
    };
    std::vector<SkillEffect> effects;

    // Cost to learn this skill
    struct SkillCost {
        std::string type; // "points", "gold", "item", etc.
        std::string itemId; // if type is "item"
        int amount;

        bool canPay(const GameContext& context) const;
        void pay(GameContext* context) const;
    };
    std::vector<SkillCost> costs;

    SkillNode(const std::string& name, const std::string& skill,
        int initialLevel = 0, int max = 5);

    bool canLearn(const GameContext& context) const;
    void learnSkill(GameContext* context);
    void onEnter(GameContext* context) override;
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;
};

// Class specialization node
class ClassNode : public TANode {
public:
    std::string className;
    std::string description;
    std::map<std::string, int> statBonuses;
    std::set<std::string> startingAbilities;
    std::vector<SkillNode*> classSkills;

    ClassNode(const std::string& name, const std::string& classType);
    void onEnter(GameContext* context) override;
};

//----------------------------------------
// CRAFTING SYSTEM
//----------------------------------------

// Recipe for crafting items
class Recipe {
public:
    std::string recipeId;
    std::string name;
    std::string description;
    bool discovered;

    // Ingredients needed
    struct Ingredient {
        std::string itemId;
        int quantity;
    };
    std::vector<Ingredient> ingredients;

    // Result of crafting
    struct Result {
        std::string itemId;
        std::string name;
        std::string type;
        int quantity;
        std::map<std::string, std::variant<int, float, std::string, bool>> properties;
    };
    Result result;

    // Skill requirements
    std::map<std::string, int> skillRequirements;

    Recipe(const std::string& id, const std::string& recipeName);
    bool canCraft(const GameContext& context) const;
    bool craft(GameContext* context);
};

// Crafting station node
class CraftingNode : public TANode {
public:
    std::string stationType;
    std::string description;
    std::vector<Recipe> availableRecipes;

    CraftingNode(const std::string& name, const std::string& type);
    void onEnter(GameContext* context) override;
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;
    void addRecipe(const Recipe& recipe);
};

//----------------------------------------
// WORLD PROGRESSION SYSTEM
//----------------------------------------

// Location node for world state
class LocationNode : public TANode {
public:
    std::string locationName;
    std::string description;
    std::string currentState;
    std::map<std::string, std::string> stateDescriptions;

    // NPCs at this location
    std::vector<NPC*> npcs;

    // Available activities at this location
    std::vector<TANode*> activities;

    // Conditions to access this location
    struct AccessCondition {
        std::string type;
        std::string target;
        int value;

        bool check(const GameContext& context) const;
    };
    std::vector<AccessCondition> accessConditions;

    LocationNode(const std::string& name, const std::string& location,
        const std::string& initialState = "normal");

    bool canAccess(const GameContext& context) const;
    void onEnter(GameContext* context) override;
    std::vector<TAAction> getAvailableActions() override;
};

// Region node for world map
class RegionNode : public TANode {
public:
    std::string regionName;
    std::string description;
    std::string controllingFaction;

    // Locations in this region
    std::vector<LocationNode*> locations;

    // Connected regions
    std::vector<RegionNode*> connectedRegions;

    // Events that can happen in this region
    struct RegionEvent {
        std::string name;
        std::string description;
        std::function<bool(const GameContext&)> condition;
        std::function<void(GameContext*)> effect;
        double probability;
    };
    std::vector<RegionEvent> possibleEvents;

    RegionNode(const std::string& name, const std::string& region);
    void onEnter(GameContext* context) override;
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;
};

// Time/Season system
class TimeNode : public TANode {
public:
    int day;
    int hour;
    std::string season;
    std::string timeOfDay;

    TimeNode(const std::string& name);
    void advanceHour(GameContext* context);
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;
};

// The main automaton controller
class TAController {
public:
    // The current active node in the automaton
    std::map<std::string, TANode*> currentNodes;

    // Root nodes for different systems (quests, dialogue, skills, etc.)
    std::map<std::string, TANode*> systemRoots;

    // Owned nodes for memory management
    std::vector<std::unique_ptr<TANode>> ownedNodes;

    // Game context
    GameContext gameContext;

    // Game data storage for references
    std::map<std::string, std::map<std::string, NPC*>> gameData;

    // Process an input and potentially transition to a new state
    bool processInput(const std::string& systemName, const TAInput& input);

    void updateCurrentNodePersistentID(const std::string& systemName);

    // Get available actions from current state
    std::vector<TAAction> getAvailableActions(const std::string& systemName);

    // Check if a particular state is reachable from current state
    bool isStateReachable(const NodeID& targetNodeID);

    // Create and register a new node
    template <typename T = TANode, typename... Args>
    T* createNode(const std::string& name, Args&&... args);

    // Set a system root
    void setSystemRoot(const std::string& systemName, TANode* rootNode);

    // Initialize persistent IDs for all nodes
    void initializePersistentIDs();
    std::string findPathToNode(TANode* root, TANode* target, const std::string& basePath);

    // Find a node by its persistent ID
    TANode* findNodeByPersistentID(const std::string& persistentID);
    TANode* findNodeByNameRecursive(TANode* node, const std::string& nodeName);
    TANode* findNodeByNameInHierarchy(TANode* node, const std::string& nodeName);
    void printNodeIDsRecursive(TANode* node, int depth);
    TANode* findNodeByPersistentIDRecursive(TANode* node, const std::string& persistentID);

    // Save/load state
    bool saveState_old(const std::string& filename);
    bool loadState_old(const std::string& filename);
    bool saveState(const std::string& filename);
    bool loadState(const std::string& filename);

private:
    // Helper functions for serializing game context components
    TANode* findNodeById(TANode* startNode, const NodeID& id);
    TANode* findNodeByName(TANode* startNode, const std::string& name);
    bool isNodeReachableFromNode(TANode* startNode, const NodeID& targetId);
};

// JSON serialization functions
json serializeCharacterStats(const CharacterStats& stats);
void deserializeCharacterStats(const json& statsData, CharacterStats& stats);

json serializeWorldState(const WorldState& state);
void deserializeWorldState(const json& worldData, WorldState& state);

json serializeInventory(const Inventory& inventory);
void deserializeInventory(const json& inventoryData, Inventory& inventory);

// Functions to load game data from JSON
bool loadGameData(TAController& controller);
void loadQuestsFromJSON(TAController& controller, const json& questData);
void loadNPCsFromJSON(TAController& controller, const json& npcData);
void loadSkillsFromJSON(TAController& controller, const json& skillsData);
void loadCraftingFromJSON(TAController& controller, const json& craftingData);
void loadWorldFromJSON(TAController& controller, const json& worldData);

// Function to create default JSON files
void createDefaultJSONFiles();

#endif // OATH_JSON_HPP