// Oath_RPGSystems_New.cpp

#include "Oath_RPGSystems_New.hpp"

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

// A unique identifier for nodes (replaces FGuid)
struct NodeID {
    unsigned int data1;
    unsigned int data2;
    unsigned int data3;
    unsigned int data4;
    std::string persistentID; // Path-based or name-based persistent ID

    bool operator==(const NodeID& other) const
    {
        // Check persistent ID first if available
        if (!persistentID.empty() && !other.persistentID.empty()) {
            return persistentID == other.persistentID;
        }
        // Fall back to numeric ID comparison
        return data1 == other.data1 && data2 == other.data2 && data3 == other.data3 && data4 == other.data4;
    }

    bool operator<(const NodeID& other) const
    {
        // Use persistent ID for comparison if available
        if (!persistentID.empty() && !other.persistentID.empty()) {
            return persistentID < other.persistentID;
        }
        // Fall back to numeric comparison
        if (data1 != other.data1)
            return data1 < other.data1;
        if (data2 != other.data2)
            return data2 < other.data2;
        if (data3 != other.data3)
            return data3 < other.data3;
        return data4 < other.data4;
    }

    static NodeID Generate(const std::string& nodePath = "")
    {
        static unsigned int counter = 0;
        NodeID id = { ++counter, 0, 0, 0 };
        id.persistentID = nodePath;
        return id;
    }

    std::string toString() const
    {
        if (!persistentID.empty()) {
            return persistentID;
        }
        std::stringstream ss;
        ss << data1 << "-" << data2 << "-" << data3 << "-" << data4;
        return ss.str();
    }
};

// An input that can trigger transitions
struct TAInput {
    std::string type;
    std::map<std::string, std::variant<int, float, std::string, bool>>
        parameters;
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

    CharacterStats()
    {
        // Initialize basic skills
        skills["combat"] = 0;
        skills["stealth"] = 0;
        skills["persuasion"] = 0;
        skills["survival"] = 0;
        skills["alchemy"] = 0;
        skills["crafting"] = 0;
        skills["magic"] = 0;

        // Initialize some factions
        factionReputation["villagers"] = 0;
        factionReputation["merchants"] = 0;
        factionReputation["nobility"] = 0;
        factionReputation["bandits"] = -50;
    }

    bool hasSkill(const std::string& skill, int minLevel) const
    {
        auto it = skills.find(skill);
        return it != skills.end() && it->second >= minLevel;
    }

    bool hasFactionReputation(const std::string& faction, int minRep) const
    {
        auto it = factionReputation.find(faction);
        return it != factionReputation.end() && it->second >= minRep;
    }

    bool hasKnowledge(const std::string& fact) const
    {
        return knownFacts.find(fact) != knownFacts.end();
    }

    bool hasAbility(const std::string& ability) const
    {
        return unlockedAbilities.find(ability) != unlockedAbilities.end();
    }

    void learnFact(const std::string& fact) { knownFacts.insert(fact); }

    void unlockAbility(const std::string& ability)
    {
        unlockedAbilities.insert(ability);
    }

    void improveSkill(const std::string& skill, int amount)
    {
        skills[skill] += amount;
    }

    void changeFactionRep(const std::string& faction, int amount)
    {
        factionReputation[faction] += amount;
    }
};

// Game world state structure
struct WorldState {
    std::map<std::string, std::string> locationStates;
    std::map<std::string, std::string> factionStates;
    std::map<std::string, bool> worldFlags;
    int daysPassed = 0;
    std::string currentSeason = "spring";

    WorldState()
    {
        // Initialize some locations
        locationStates["village"] = "peaceful";
        locationStates["forest"] = "wild";
        locationStates["mountain"] = "unexplored";
        locationStates["castle"] = "occupied";

        // Initialize faction states
        factionStates["villagers"] = "normal";
        factionStates["bandits"] = "aggressive";
        factionStates["merchants"] = "traveling";

        // Initialize world flags
        worldFlags["war_active"] = false;
        worldFlags["plague_spreading"] = false;
        worldFlags["dragons_returned"] = false;
    }

    bool hasFlag(const std::string& flag) const
    {
        auto it = worldFlags.find(flag);
        return it != worldFlags.end() && it->second;
    }

    std::string getLocationState(const std::string& location) const
    {
        auto it = locationStates.find(location);
        return (it != locationStates.end()) ? it->second : "unknown";
    }

    std::string getFactionState(const std::string& faction) const
    {
        auto it = factionStates.find(faction);
        return (it != factionStates.end()) ? it->second : "unknown";
    }

    void setLocationState(const std::string& location, const std::string& state)
    {
        locationStates[location] = state;
    }

    void setFactionState(const std::string& faction, const std::string& state)
    {
        factionStates[faction] = state;
    }

    void setWorldFlag(const std::string& flag, bool value)
    {
        worldFlags[flag] = value;
    }

    void advanceDay()
    {
        daysPassed++;

        // Update season every 90 days
        if (daysPassed % 90 == 0) {
            if (currentSeason == "spring")
                currentSeason = "summer";
            else if (currentSeason == "summer")
                currentSeason = "autumn";
            else if (currentSeason == "autumn")
                currentSeason = "winter";
            else if (currentSeason == "winter")
                currentSeason = "spring";
        }
    }
};

// Inventory and item system for crafting
struct Item {
    std::string id;
    std::string name;
    std::string type;
    int value;
    int quantity;
    std::map<std::string, std::variant<int, float, std::string, bool>>
        properties;

    Item(const std::string& itemId, const std::string& itemName,
        const std::string& itemType, int itemValue = 1, int itemQty = 1)
        : id(itemId)
        , name(itemName)
        , type(itemType)
        , value(itemValue)
        , quantity(itemQty)
    {
    }
};

class Inventory {
public:
    std::vector<Item> items;

    bool hasItem(const std::string& itemId, int quantity = 1) const
    {
        for (const auto& item : items) {
            if (item.id == itemId && item.quantity >= quantity) {
                return true;
            }
        }
        return false;
    }

    bool addItem(const Item& item)
    {
        // Check if item already exists
        for (auto& existingItem : items) {
            if (existingItem.id == item.id) {
                existingItem.quantity += item.quantity;
                return true;
            }
        }

        // Add new item
        items.push_back(item);
        return true;
    }

    bool removeItem(const std::string& itemId, int quantity = 1)
    {
        for (auto it = items.begin(); it != items.end(); ++it) {
            if (it->id == itemId) {
                if (it->quantity > quantity) {
                    it->quantity -= quantity;
                    return true;
                } else if (it->quantity == quantity) {
                    items.erase(it);
                    return true;
                } else {
                    return false; // Not enough quantity
                }
            }
        }
        return false; // Item not found
    }
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

    TANode(const std::string& name)
        : nodeID(NodeID::Generate())
        , nodeName(name)
        , isAcceptingState(false)
    {
    }

    virtual ~TANode() = default;

    // Evaluation function to process inputs
    virtual bool evaluateTransition(const TAInput& input, TANode*& outNextNode)
    {
        for (const auto& rule : transitionRules) {
            if (rule.condition(input)) {
                outNextNode = rule.targetNode;
                return true;
            }
        }
        return false;
    }

    // Actions to perform when entering/exiting this node
    virtual void onEnter(GameContext* context)
    {
        std::cout << "Entered node: " << nodeName << std::endl;
    }

    virtual void onExit(GameContext* context)
    {
        std::cout << "Exited node: " << nodeName << std::endl;
    }

    // Add a transition rule
    void addTransition(const std::function<bool(const TAInput&)>& condition,
        TANode* target, const std::string& description = "")
    {
        transitionRules.push_back({ condition, target, description });
    }

    // Add a child node
    void addChild(TANode* child) { childNodes.push_back(child); }

    // Get available transitions for the current state
    virtual std::vector<TAAction> getAvailableActions()
    {
        std::vector<TAAction> actions;
        for (size_t i = 0; i < transitionRules.size(); i++) {
            const auto& rule = transitionRules[i];
            actions.push_back(
                { "transition_" + std::to_string(i), rule.description,
                    [this, i]() -> TAInput {
                        return { "transition", { { "index", static_cast<int>(i) } } };
                    } });
        }
        return actions;
    }

    // Generate a path-based ID for this node
    void generatePersistentID(const std::string& parentPath = "")
    {
        std::string path;
        if (parentPath.empty()) {
            // Root node case
            path = nodeName;
        } else {
            // Child node case - include parent path
            path = parentPath + "/" + nodeName;
        }

        nodeID.persistentID = path;

        // Update child nodes recursively
        for (TANode* child : childNodes) {
            child->generatePersistentID(path);
        }
    }

    // Serialize node state
    virtual void serialize(std::ofstream& file) const
    {
        // Write state data
        size_t stateDataSize = stateData.size();
        if (stateDataSize > 1000) {
            std::cerr << "Warning: Large state data size: " << stateDataSize << std::endl;
            stateDataSize = 1000; // Limit to prevent huge allocations
        }
        file.write(reinterpret_cast<const char*>(&stateDataSize), sizeof(stateDataSize));

        size_t count = 0;
        for (const auto& [key, value] : stateData) {
            if (count >= stateDataSize)
                break;

            // Skip empty keys or extremely long keys
            if (key.empty() || key.length() > 1000) {
                std::cerr << "Warning: Skipping invalid key with length " << key.length() << std::endl;
                continue;
            }

            // Write key
            size_t keyLength = key.length();
            file.write(reinterpret_cast<const char*>(&keyLength), sizeof(keyLength));
            file.write(key.c_str(), keyLength);

            // Write variant type and value
            if (std::holds_alternative<int>(value)) {
                char type = 'i';
                file.write(&type, 1);
                int val = std::get<int>(value);
                file.write(reinterpret_cast<const char*>(&val), sizeof(val));
            } else if (std::holds_alternative<float>(value)) {
                char type = 'f';
                file.write(&type, 1);
                float val = std::get<float>(value);
                file.write(reinterpret_cast<const char*>(&val), sizeof(val));
            } else if (std::holds_alternative<std::string>(value)) {
                char type = 's';
                file.write(&type, 1);
                std::string val = std::get<std::string>(value);

                // Limit string length
                if (val.length() > 10000) {
                    std::cerr << "Warning: Truncating long string value for key " << key << std::endl;
                    val = val.substr(0, 10000);
                }

                size_t valLength = val.length();
                file.write(reinterpret_cast<const char*>(&valLength), sizeof(valLength));
                file.write(val.c_str(), valLength);
            } else if (std::holds_alternative<bool>(value)) {
                char type = 'b';
                file.write(&type, 1);
                bool val = std::get<bool>(value);
                file.write(reinterpret_cast<const char*>(&val), sizeof(val));
            }

            count++;
        }
    }

    // Deserialize node state
    virtual bool deserialize(std::ifstream& file)
    {
        // Read state data
        size_t stateDataSize;
        if (!file.read(reinterpret_cast<char*>(&stateDataSize), sizeof(stateDataSize))) {
            std::cerr << "Failed to read state data size" << std::endl;
            return false;
        }

        // Sanity check
        const size_t MAX_STATE_SIZE = 1000; // Reasonable maximum
        if (stateDataSize > MAX_STATE_SIZE) {
            std::cerr << "Invalid state data size: " << stateDataSize << std::endl;
            return false;
        }

        stateData.clear();
        for (size_t i = 0; i < stateDataSize; i++) {
            // Check file position is valid
            if (file.eof()) {
                std::cerr << "Unexpected end of file during node deserialization" << std::endl;
                return false;
            }

            // Read key length
            size_t keyLength;
            if (!file.read(reinterpret_cast<char*>(&keyLength), sizeof(keyLength))) {
                std::cerr << "Failed to read key length for item " << i << std::endl;
                return false;
            }

            // Sanity check - enforce reasonable maximum
            const size_t MAX_KEY_LENGTH = 1000;
            if (keyLength == 0 || keyLength > MAX_KEY_LENGTH) {
                std::cerr << "Invalid key length: " << keyLength << std::endl;
                return false;
            }

            // Read key with verified length
            std::string key(keyLength, ' ');
            if (!file.read(&key[0], keyLength)) {
                std::cerr << "Failed to read key with length " << keyLength << std::endl;
                return false;
            }

            // Verify type is valid
            char type;
            if (!file.read(&type, 1)) {
                std::cerr << "Failed to read type for key " << key << std::endl;
                return false;
            }

            // Only accept valid type characters
            if (type != 'i' && type != 'f' && type != 's' && type != 'b') {
                std::cerr << "Invalid type character: " << type << " for key " << key << std::endl;
                return false;
            }

            // Handle each type with validation
            if (type == 'i') {
                int val;
                if (!file.read(reinterpret_cast<char*>(&val), sizeof(val))) {
                    std::cerr << "Failed to read int value for key " << key << std::endl;
                    return false;
                }
                stateData[key] = val;
            } else if (type == 'f') {
                float val;
                if (!file.read(reinterpret_cast<char*>(&val), sizeof(val))) {
                    std::cerr << "Failed to read float value for key " << key << std::endl;
                    return false;
                }
                stateData[key] = val;
            } else if (type == 's') {
                size_t valLength;
                if (!file.read(reinterpret_cast<char*>(&valLength), sizeof(valLength))) {
                    std::cerr << "Failed to read string length for key " << key << std::endl;
                    return false;
                }

                // Sanity check
                const size_t MAX_STRING_LENGTH = 10000;
                if (valLength > MAX_STRING_LENGTH) {
                    std::cerr << "Invalid string length: " << valLength << " for key " << key << std::endl;
                    return false;
                }

                std::string val(valLength, ' ');
                if (!file.read(&val[0], valLength)) {
                    std::cerr << "Failed to read string value for key " << key << std::endl;
                    return false;
                }
                stateData[key] = val;
            } else if (type == 'b') {
                bool val;
                if (!file.read(reinterpret_cast<char*>(&val), sizeof(val))) {
                    std::cerr << "Failed to read bool value for key " << key << std::endl;
                    return false;
                }
                stateData[key] = val;
            }
        }

        return true;
    }
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

        bool check(const GameContext& context) const
        {
            if (type == "skill") {
                return context.playerStats.hasSkill(target, value);
            } else if (type == "item") {
                return context.playerInventory.hasItem(target, value);
            } else if (type == "faction") {
                return context.playerStats.hasFactionReputation(target, value);
            } else if (type == "knowledge") {
                return context.playerStats.hasKnowledge(target);
            } else if (type == "worldflag") {
                return context.worldState.hasFlag(target);
            }
            return false;
        }
    };
    std::vector<QuestRequirement> requirements;

    QuestNode(const std::string& name)
        : TANode(name)
        , questState("Available")
    {
    }

    // Process player action and return next state
    bool processAction(const std::string& playerAction, TANode*& outNextNode)
    {
        return evaluateTransition({ "action", { { "name", playerAction } } },
            outNextNode);
    }

    // Check if player can access this quest
    bool canAccess(const GameContext& context) const
    {
        for (const auto& req : requirements) {
            if (!req.check(context)) {
                return false;
            }
        }
        return true;
    }

    // Activate child quests when this node is entered
    void onEnter(GameContext* context) override
    {
        // Mark quest as active
        questState = "Active";

        // Activate all child quests/objectives
        for (TANode* childNode : childNodes) {
            if (auto* questChild = dynamic_cast<QuestNode*>(childNode)) {
                questChild->questState = "Available";
            }
        }

        // Update quest journal
        if (context) {
            context->questJournal[nodeName] = "Active";
        }

        std::cout << "Quest activated: " << questTitle << std::endl;
        std::cout << questDescription << std::endl;
    }

    // Award rewards when completing quest
    void onExit(GameContext* context) override
    {
        // Only award rewards if moving to a completion state
        if (isAcceptingState) {
            questState = "Completed";

            if (context) {
                context->questJournal[nodeName] = "Completed";

                // Award rewards to player
                std::cout << "Quest completed: " << questTitle << std::endl;
                std::cout << "Rewards:" << std::endl;

                for (const auto& reward : rewards) {
                    if (reward.type == "experience") {
                        std::cout << "  " << reward.amount << " experience points"
                                  << std::endl;
                    } else if (reward.type == "gold") {
                        std::cout << "  " << reward.amount << " gold coins" << std::endl;
                    } else if (reward.type == "item") {
                        context->playerInventory.addItem(Item(reward.itemId, reward.itemId,
                            "quest_reward", 1,
                            reward.amount));
                        std::cout << "  " << reward.amount << "x " << reward.itemId
                                  << std::endl;
                    } else if (reward.type == "faction") {
                        context->playerStats.changeFactionRep(reward.itemId, reward.amount);
                        std::cout << "  " << reward.amount << " reputation with "
                                  << reward.itemId << std::endl;
                    } else if (reward.type == "skill") {
                        context->playerStats.improveSkill(reward.itemId, reward.amount);
                        std::cout << "  " << reward.amount << " points in " << reward.itemId
                                  << " skill" << std::endl;
                    }
                }
            }
        } else if (questState == "Failed") {
            if (context) {
                context->questJournal[nodeName] = "Failed";
            }
            std::cout << "Quest failed: " << questTitle << std::endl;
        }
    }

    // Get available actions specific to quests
    std::vector<TAAction> getAvailableActions() override
    {
        std::vector<TAAction> actions = TANode::getAvailableActions();

        // Add quest-specific actions
        actions.push_back(
            { "abandon_quest", "Abandon this quest", []() -> TAInput {
                 return { "quest_action", { { "action", std::string("abandon") } } };
             } });

        return actions;
    }
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
            std::function<void(GameContext*)> eff = [](GameContext*) {})
            : text(responseText)
            , requirement(req)
            , targetNode(target)
            , effect(eff)
        {
        }
    };
    std::vector<DialogueResponse> responses;

    // Optional effect to run when this dialogue is shown
    std::function<void(GameContext*)> onShowEffect;

    DialogueNode(const std::string& name, const std::string& speaker,
        const std::string& text)
        : TANode(name)
        , speakerName(speaker)
        , dialogueText(text)
    {
    }

    void addResponse(
        const std::string& text, TANode* target,
        std::function<bool(const GameContext&)> requirement =
            [](const GameContext&) { return true; },
        std::function<void(GameContext*)> effect = [](GameContext*) {})
    {
        responses.push_back(DialogueResponse(text, target, requirement, effect));
    }

    void onEnter(GameContext* context) override
    {
        // Display the dialogue
        std::cout << speakerName << ": " << dialogueText << std::endl;

        // Run any effects
        if (onShowEffect && context) {
            onShowEffect(context);
        }

        // Store in dialogue history
        if (context) {
            context->dialogueHistory[nodeName] = dialogueText;
        }
    }

    // Get available dialogue responses
    std::vector<TAAction> getAvailableActions() override
    {
        std::vector<TAAction> actions;

        for (size_t i = 0; i < responses.size(); i++) {
            actions.push_back(
                { "response_" + std::to_string(i), responses[i].text,
                    [this, i]() -> TAInput {
                        return { "dialogue_response", { { "index", static_cast<int>(i) } } };
                    } });
        }

        return actions;
    }

    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override
    {
        if (input.type == "dialogue_response") {
            int index = std::get<int>(input.parameters.at("index"));
            if (index >= 0 && index < static_cast<int>(responses.size())) {
                outNextNode = responses[index].targetNode;
                return true;
            }
        }
        return false;
    }
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

    NPC(const std::string& npcName, const std::string& desc)
        : name(npcName)
        , description(desc)
        , rootDialogue(nullptr)
        , currentDialogue(nullptr)
    {
    }

    void startDialogue(GameContext* context)
    {
        if (rootDialogue) {
            currentDialogue = rootDialogue;
            currentDialogue->onEnter(context);
        }
    }

    bool processResponse(int responseIndex, GameContext* context)
    {
        if (!currentDialogue)
            return false;

        if (responseIndex >= 0 && responseIndex < static_cast<int>(currentDialogue->responses.size())) {
            auto& response = currentDialogue->responses[responseIndex];

            // Check if requirement is met
            if (!response.requirement(*context)) {
                std::cout << "You cannot select that response." << std::endl;
                return false;
            }

            // Execute effect
            if (response.effect) {
                response.effect(context);
            }

            // Move to next dialogue node
            currentDialogue = dynamic_cast<DialogueNode*>(response.targetNode);
            if (currentDialogue) {
                currentDialogue->onEnter(context);
                return true;
            }
        }

        return false;
    }
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

        bool check(const GameContext& context) const
        {
            if (type == "skill") {
                return context.playerStats.hasSkill(target, level);
            } else if (type == "item") {
                return context.playerInventory.hasItem(target, level);
            } else if (type == "knowledge") {
                return context.playerStats.hasKnowledge(target);
            }
            return false;
        }
    };
    std::vector<SkillRequirement> requirements;

    // Effects when this skill is learned or improved
    struct SkillEffect {
        std::string type;
        std::string target;
        int value;

        void apply(GameContext* context) const
        {
            if (!context)
                return;

            if (type == "stat") {
                if (target == "strength")
                    context->playerStats.strength += value;
                else if (target == "dexterity")
                    context->playerStats.dexterity += value;
                else if (target == "constitution")
                    context->playerStats.constitution += value;
                else if (target == "intelligence")
                    context->playerStats.intelligence += value;
                else if (target == "wisdom")
                    context->playerStats.wisdom += value;
                else if (target == "charisma")
                    context->playerStats.charisma += value;
            } else if (type == "skill") {
                context->playerStats.improveSkill(target, value);
            } else if (type == "ability") {
                context->playerStats.unlockAbility(target);
            }
        }
    };
    std::vector<SkillEffect> effects;

    // Cost to learn this skill
    struct SkillCost {
        std::string type; // "points", "gold", "item", etc.
        std::string itemId; // if type is "item"
        int amount;

        bool canPay(const GameContext& context) const
        {
            if (type == "item") {
                return context.playerInventory.hasItem(itemId, amount);
            }
            // Other types would be checked here
            return true;
        }

        void pay(GameContext* context) const
        {
            if (!context)
                return;

            if (type == "item") {
                context->playerInventory.removeItem(itemId, amount);
            }
            // Other payment types would be handled here
        }
    };
    std::vector<SkillCost> costs;

    SkillNode(const std::string& name, const std::string& skill,
        int initialLevel = 0, int max = 5)
        : TANode(name)
        , skillName(skill)
        , description("")
        , level(initialLevel)
        , maxLevel(max)
    {
    }

    bool canLearn(const GameContext& context) const
    {
        // Check all requirements
        for (const auto& req : requirements) {
            if (!req.check(context)) {
                return false;
            }
        }

        // Check costs
        for (const auto& cost : costs) {
            if (!cost.canPay(context)) {
                return false;
            }
        }

        return level < maxLevel;
    }

    void learnSkill(GameContext* context)
    {
        if (!context || !canLearn(*context)) {
            return;
        }

        // Pay costs
        for (const auto& cost : costs) {
            cost.pay(context);
        }

        // Apply effects
        for (const auto& effect : effects) {
            effect.apply(context);
        }

        // Increase level
        level++;

        // Update player stats
        context->playerStats.improveSkill(skillName, 1);

        // If reached max level, mark as accepting state
        if (level >= maxLevel) {
            isAcceptingState = true;
        }

        std::cout << "Learned " << skillName << " (Level " << level << "/"
                  << maxLevel << ")" << std::endl;
    }

    void onEnter(GameContext* context) override
    {
        std::cout << "Viewing skill: " << skillName << " (Level " << level << "/"
                  << maxLevel << ")" << std::endl;
        std::cout << description << std::endl;

        if (context && canLearn(*context)) {
            std::cout << "This skill can be learned/improved." << std::endl;
        }
    }

    std::vector<TAAction> getAvailableActions() override
    {
        std::vector<TAAction> actions = TANode::getAvailableActions();

        // Add learn skill action if not at max level
        if (level < maxLevel) {
            actions.push_back(
                { "learn_skill", "Learn/Improve " + skillName, [this]() -> TAInput {
                     return { "skill_action",
                         { { "action", std::string("learn") }, { "skill", skillName } } };
                 } });
        }

        return actions;
    }

    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override
    {
        if (input.type == "skill_action") {
            std::string action = std::get<std::string>(input.parameters.at("action"));
            if (action == "learn") {
                // Stay in same node after learning
                outNextNode = this;
                return true;
            }
        }

        return TANode::evaluateTransition(input, outNextNode);
    }
};

// Class specialization node
class ClassNode : public TANode {
public:
    std::string className;
    std::string description;
    std::map<std::string, int> statBonuses;
    std::set<std::string> startingAbilities;
    std::vector<SkillNode*> classSkills;

    ClassNode(const std::string& name, const std::string& classType)
        : TANode(name)
        , className(classType)
    {
    }

    void onEnter(GameContext* context) override
    {
        std::cout << "Selected class: " << className << std::endl;
        std::cout << description << std::endl;

        if (context) {
            // Apply stat bonuses
            for (const auto& [stat, bonus] : statBonuses) {
                if (stat == "strength")
                    context->playerStats.strength += bonus;
                else if (stat == "dexterity")
                    context->playerStats.dexterity += bonus;
                else if (stat == "constitution")
                    context->playerStats.constitution += bonus;
                else if (stat == "intelligence")
                    context->playerStats.intelligence += bonus;
                else if (stat == "wisdom")
                    context->playerStats.wisdom += bonus;
                else if (stat == "charisma")
                    context->playerStats.charisma += bonus;
            }

            // Grant starting abilities
            for (const auto& ability : startingAbilities) {
                context->playerStats.unlockAbility(ability);
            }
        }
    }
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
        std::map<std::string, std::variant<int, float, std::string, bool>>
            properties;
    };
    Result result;

    // Skill requirements
    std::map<std::string, int> skillRequirements;

    Recipe(const std::string& id, const std::string& recipeName)
        : recipeId(id)
        , name(recipeName)
        , discovered(false)
    {
    }

    bool canCraft(const GameContext& context) const
    {
        // Check skill requirements
        for (const auto& [skill, level] : skillRequirements) {
            if (!context.playerStats.hasSkill(skill, level)) {
                return false;
            }
        }

        // Check ingredients
        for (const auto& ingredient : ingredients) {
            if (!context.playerInventory.hasItem(ingredient.itemId,
                    ingredient.quantity)) {
                return false;
            }
        }

        return true;
    }

    bool craft(GameContext* context)
    {
        if (!context || !canCraft(*context)) {
            return false;
        }

        // Consume ingredients
        for (const auto& ingredient : ingredients) {
            context->playerInventory.removeItem(ingredient.itemId,
                ingredient.quantity);
        }

        // Create result item
        Item craftedItem(result.itemId, result.name, result.type, 1,
            result.quantity);
        craftedItem.properties = result.properties;

        // Add to inventory
        context->playerInventory.addItem(craftedItem);

        // Mark as discovered
        discovered = true;

        std::cout << "Crafted " << result.quantity << "x " << result.name
                  << std::endl;
        return true;
    }
};

// Crafting station node
class CraftingNode : public TANode {
public:
    std::string stationType;
    std::string description;
    std::vector<Recipe> availableRecipes;

    CraftingNode(const std::string& name, const std::string& type)
        : TANode(name)
        , stationType(type)
    {
    }

    void onEnter(GameContext* context) override
    {
        std::cout << "At " << stationType << " station." << std::endl;
        std::cout << description << std::endl;

        // Show available recipes
        std::cout << "Available recipes:" << std::endl;
        for (size_t i = 0; i < availableRecipes.size(); i++) {
            const auto& recipe = availableRecipes[i];
            if (recipe.discovered) {
                std::cout << i + 1 << ". " << recipe.name;
                if (context && recipe.canCraft(*context)) {
                    std::cout << " (Can craft)";
                }
                std::cout << std::endl;
            } else {
                std::cout << i + 1 << ". ???" << std::endl;
            }
        }
    }

    std::vector<TAAction> getAvailableActions() override
    {
        std::vector<TAAction> actions = TANode::getAvailableActions();

        // Add crafting actions for discovered recipes
        for (size_t i = 0; i < availableRecipes.size(); i++) {
            if (availableRecipes[i].discovered) {
                actions.push_back({ "craft_" + std::to_string(i),
                    "Craft " + availableRecipes[i].name,
                    [this, i]() -> TAInput {
                        return { "crafting_action",
                            { { "action", std::string("craft") },
                                { "recipe_index", static_cast<int>(i) } } };
                    } });
            }
        }

        // Add exit action
        actions.push_back(
            { "exit_crafting", "Exit crafting station", [this]() -> TAInput {
                 return { "crafting_action", { { "action", std::string("exit") } } };
             } });

        return actions;
    }

    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override
    {
        if (input.type == "crafting_action") {
            std::string action = std::get<std::string>(input.parameters.at("action"));

            if (action == "craft") {
                int recipeIndex = std::get<int>(input.parameters.at("recipe_index"));
                if (recipeIndex >= 0 && recipeIndex < static_cast<int>(availableRecipes.size())) {
                    // Stay in same node after crafting
                    outNextNode = this;
                    return true;
                }
            } else if (action == "exit") {
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

    void addRecipe(const Recipe& recipe) { availableRecipes.push_back(recipe); }
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

        bool check(const GameContext& context) const
        {
            if (type == "item") {
                return context.playerInventory.hasItem(target, value);
            } else if (type == "skill") {
                return context.playerStats.hasSkill(target, value);
            } else if (type == "faction") {
                return context.playerStats.hasFactionReputation(target, value);
            } else if (type == "worldflag") {
                return context.worldState.hasFlag(target);
            }
            return false;
        }
    };
    std::vector<AccessCondition> accessConditions;

    LocationNode(const std::string& name, const std::string& location,
        const std::string& initialState = "normal")
        : TANode(name)
        , locationName(location)
        , currentState(initialState)
    {
    }

    bool canAccess(const GameContext& context) const
    {
        for (const auto& condition : accessConditions) {
            if (!condition.check(context)) {
                return false;
            }
        }
        return true;
    }

    void onEnter(GameContext* context) override
    {
        std::cout << "Arrived at " << locationName << std::endl;

        // Show description based on current state
        if (stateDescriptions.find(currentState) != stateDescriptions.end()) {
            std::cout << stateDescriptions.at(currentState) << std::endl;
        } else {
            std::cout << description << std::endl;
        }

        // Update world state
        if (context) {
            context->worldState.setLocationState(locationName, currentState);
        }

        // List NPCs
        if (!npcs.empty()) {
            std::cout << "People here:" << std::endl;
            for (const auto& npc : npcs) {
                std::cout << "- " << npc->name << std::endl;
            }
        }

        // List activities
        if (!activities.empty()) {
            std::cout << "Available activities:" << std::endl;
            for (const auto& activity : activities) {
                std::cout << "- " << activity->nodeName << std::endl;
            }
        }
    }

    std::vector<TAAction> getAvailableActions() override
    {
        std::vector<TAAction> actions = TANode::getAvailableActions();

        // Add NPC interaction actions
        for (size_t i = 0; i < npcs.size(); i++) {
            actions.push_back({ "talk_to_npc_" + std::to_string(i),
                "Talk to " + npcs[i]->name, [this, i]() -> TAInput {
                    return { "location_action",
                        { { "action", std::string("talk") },
                            { "npc_index", static_cast<int>(i) } } };
                } });
        }

        // Add activity actions
        for (size_t i = 0; i < activities.size(); i++) {
            actions.push_back({ "do_activity_" + std::to_string(i),
                "Do " + activities[i]->nodeName,
                [this, i]() -> TAInput {
                    return { "location_action",
                        { { "action", std::string("activity") },
                            { "activity_index", static_cast<int>(i) } } };
                } });
        }

        return actions;
    }
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

    RegionNode(const std::string& name, const std::string& region)
        : TANode(name)
        , regionName(region)
    {
    }

    void onEnter(GameContext* context) override
    {
        std::cout << "Entered region: " << regionName << std::endl;
        std::cout << description << std::endl;

        if (!controllingFaction.empty()) {
            std::cout << "Controlled by: " << controllingFaction << std::endl;
        }

        // List locations
        if (!locations.empty()) {
            std::cout << "Locations in this region:" << std::endl;
            for (const auto& location : locations) {
                std::cout << "- " << location->locationName;
                if (context && !location->canAccess(*context)) {
                    std::cout << " (Inaccessible)";
                }
                std::cout << std::endl;
            }
        }

        // List connected regions
        if (!connectedRegions.empty()) {
            std::cout << "Connected regions:" << std::endl;
            for (const auto& region : connectedRegions) {
                std::cout << "- " << region->regionName << std::endl;
            }
        }

        // Check for random events
        if (context) {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<> dis(0.0, 1.0);

            for (const auto& event : possibleEvents) {
                if (event.condition(*context) && dis(gen) < event.probability) {
                    std::cout << "\nEvent: " << event.name << std::endl;
                    std::cout << event.description << std::endl;
                    event.effect(context);
                    break;
                }
            }
        }
    }

    std::vector<TAAction> getAvailableActions() override
    {
        std::vector<TAAction> actions = TANode::getAvailableActions();

        // Add location travel actions
        for (size_t i = 0; i < locations.size(); i++) {
            actions.push_back({ "travel_to_location_" + std::to_string(i),
                "Travel to " + locations[i]->locationName,
                [this, i]() -> TAInput {
                    return { "region_action",
                        { { "action", std::string("travel_location") },
                            { "location_index", static_cast<int>(i) } } };
                } });
        }

        // Add region travel actions
        for (size_t i = 0; i < connectedRegions.size(); i++) {
            actions.push_back({ "travel_to_region_" + std::to_string(i),
                "Travel to " + connectedRegions[i]->regionName,
                [this, i]() -> TAInput {
                    return { "region_action",
                        { { "action", std::string("travel_region") },
                            { "region_index", static_cast<int>(i) } } };
                } });
        }

        return actions;
    }

    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override
    {
        if (input.type == "region_action") {
            std::string action = std::get<std::string>(input.parameters.at("action"));

            if (action == "travel_location") {
                int locationIndex = std::get<int>(input.parameters.at("location_index"));
                if (locationIndex >= 0 && locationIndex < static_cast<int>(locations.size())) {
                    // Set the persistent ID for the location to include the region path
                    locations[locationIndex]->nodeID.persistentID = "WorldSystem/" + locations[locationIndex]->nodeName;

                    outNextNode = locations[locationIndex];
                    return true;
                }
            } else if (action == "travel_region") {
                int regionIndex = std::get<int>(input.parameters.at("region_index"));
                if (regionIndex >= 0 && regionIndex < static_cast<int>(connectedRegions.size())) {
                    outNextNode = connectedRegions[regionIndex];
                    return true;
                }
            }
        }

        return TANode::evaluateTransition(input, outNextNode);
    }
};

// Time/Season system
class TimeNode : public TANode {
public:
    int day;
    int hour;
    std::string season;
    std::string timeOfDay;

    TimeNode(const std::string& name)
        : TANode(name)
        , day(1)
        , hour(6)
        , season("spring")
        , timeOfDay("morning")
    {
    }

    void advanceHour(GameContext* context)
    {
        hour++;

        if (hour >= 24) {
            hour = 0;
            day++;

            // Update season every 90 days
            if (day % 90 == 0) {
                if (season == "spring")
                    season = "summer";
                else if (season == "summer")
                    season = "autumn";
                else if (season == "autumn")
                    season = "winter";
                else if (season == "winter")
                    season = "spring";
            }

            if (context) {
                context->worldState.advanceDay();
            }
        }

        // Update time of day
        if (hour >= 5 && hour < 12)
            timeOfDay = "morning";
        else if (hour >= 12 && hour < 17)
            timeOfDay = "afternoon";
        else if (hour >= 17 && hour < 21)
            timeOfDay = "evening";
        else
            timeOfDay = "night";

        std::cout << "Time: Day " << day << ", " << hour << ":00, " << timeOfDay
                  << " (" << season << ")" << std::endl;
    }

    std::vector<TAAction> getAvailableActions() override
    {
        std::vector<TAAction> actions = TANode::getAvailableActions();

        actions.push_back({ "wait_1_hour", "Wait 1 hour", [this]() -> TAInput {
                               return {
                                   "time_action",
                                   { { "action", std::string("wait") }, { "hours", 1 } }
                               };
                           } });

        actions.push_back(
            { "wait_until_morning", "Wait until morning", [this]() -> TAInput {
                 return { "time_action",
                     { { "action", std::string("wait_until") },
                         { "time", std::string("morning") } } };
             } });

        return actions;
    }

    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override
    {
        if (input.type == "time_action") {
            std::string action = std::get<std::string>(input.parameters.at("action"));

            if (action == "wait") {
                int hours = std::get<int>(input.parameters.at("hours"));
                // In a real game, this would trigger events, status changes, etc.
                std::cout << "Waiting for " << hours << " hours..." << std::endl;

                // Stay in same node after waiting
                outNextNode = this;
                return true;
            } else if (action == "wait_until") {
                std::string targetTime = std::get<std::string>(input.parameters.at("time"));
                // Calculate hours to wait
                int hoursToWait = 0;

                if (targetTime == "morning" && timeOfDay != "morning") {
                    if (hour < 5)
                        hoursToWait = 5 - hour;
                    else
                        hoursToWait = 24 - (hour - 5);
                }
                // Add other time of day calculations

                std::cout << "Waiting until " << targetTime << " (" << hoursToWait
                          << " hours)..." << std::endl;

                // Stay in same node after waiting
                outNextNode = this;
                return true;
            }
        }

        return TANode::evaluateTransition(input, outNextNode);
    }
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
    std::map<std::string, std::map<std::string, void*>> gameData;

    // Process an input and potentially transition to a new state
    bool processInput(const std::string& systemName, const TAInput& input)
    {
        if (systemRoots.find(systemName) == systemRoots.end()) {
            std::cerr << "System not found: " << systemName << std::endl;
            return false;
        }

        if (currentNodes.find(systemName) == currentNodes.end()) {
            // Initialize with the system root if no current node
            currentNodes[systemName] = systemRoots[systemName];
            currentNodes[systemName]->onEnter(&gameContext);
        }

        TANode* nextNode = nullptr;
        if (currentNodes[systemName]->evaluateTransition(input, nextNode)) {
            if (nextNode != currentNodes[systemName]) {
                currentNodes[systemName]->onExit(&gameContext);
                currentNodes[systemName] = nextNode;

                // Update the persistent ID to reflect the actual path
                updateCurrentNodePersistentID(systemName);

                nextNode->onEnter(&gameContext);
                return true;
            }
        }
        return false;
    }

    void updateCurrentNodePersistentID(const std::string& systemName)
    {
        if (currentNodes.find(systemName) == currentNodes.end() || systemRoots.find(systemName) == systemRoots.end()) {
            return;
        }

        TANode* current = currentNodes[systemName];

        // Set a persistent ID that includes the system name and node name
        // This ensures it can be found even if the hierarchy isn't perfectly matched
        current->nodeID.persistentID = systemName + "/" + current->nodeName;

        std::cout << "Updated node ID for " << current->nodeName
                  << " to: " << current->nodeID.persistentID << std::endl;
    }

    // Get available actions from current state
    std::vector<TAAction> getAvailableActions(const std::string& systemName)
    {
        if (currentNodes.find(systemName) == currentNodes.end()) {
            return {};
        }

        return currentNodes[systemName]->getAvailableActions();
    }

    // Create and register a new node
    template <typename T = TANode, typename... Args>
    T* createNode(const std::string& name, Args&&... args)
    {
        auto node = std::make_unique<T>(name, std::forward<Args>(args)...);
        T* nodePtr = node.get();
        ownedNodes.push_back(std::move(node));
        return nodePtr;
    }

    // Set a system root
    void setSystemRoot(const std::string& systemName, TANode* rootNode)
    {
        systemRoots[systemName] = rootNode;
    }

    // Initialize persistent IDs for all nodes
    void initializePersistentIDs()
    {
        for (const auto& [systemName, rootNode] : systemRoots) {
            rootNode->generatePersistentID(systemName);

            // Also initialize persistent IDs for currentNodes, especially if they're not
            // directly in the hierarchy
            if (currentNodes.find(systemName) != currentNodes.end() && currentNodes[systemName] != rootNode) {
                // Find the path from root to this node
                std::string path = findPathToNode(rootNode, currentNodes[systemName], systemName);
                if (!path.empty()) {
                    currentNodes[systemName]->nodeID.persistentID = path;
                    std::cout << "Set current node ID for " << systemName << ": " << path << std::endl;
                }
            }
        }
    }

    std::string findPathToNode(TANode* root, TANode* target, const std::string& basePath)
    {
        if (root == target) {
            return basePath;
        }

        for (TANode* child : root->childNodes) {
            std::string childPath = basePath + "/" + child->nodeName;

            if (child == target) {
                return childPath;
            }

            std::string path = findPathToNode(child, target, childPath);
            if (!path.empty()) {
                return path;
            }
        }

        return ""; // Not found in this branch
    }

    // Find a node by its persistent ID
    TANode* findNodeByPersistentID(const std::string& persistentID)
    {
        // First try direct match with recursive search
        for (const auto& [systemName, rootNode] : systemRoots) {
            TANode* node = findNodeByPersistentIDRecursive(rootNode, persistentID);
            if (node)
                return node;
        }

        // If not found, try to extract just the node name
        std::string nodeName = persistentID;
        size_t lastSlash = persistentID.find_last_of('/');
        if (lastSlash != std::string::npos) {
            nodeName = persistentID.substr(lastSlash + 1);
        }

        // Try to find by name
        TANode* foundNode = nullptr;
        for (const auto& [systemName, rootNode] : systemRoots) {
            foundNode = findNodeByNameRecursive(rootNode, nodeName);
            if (foundNode) {
                // Update its persistent ID to match what was expected
                foundNode->nodeID.persistentID = persistentID;
                return foundNode;
            }
        }

        // Check if this is a location node (special case for WorldSystem)
        if (persistentID.find("WorldSystem/") == 0) {
            // Try to find locations in each region
            if (systemRoots.find("WorldSystem") != systemRoots.end()) {
                TANode* worldRoot = systemRoots["WorldSystem"];
                RegionNode* regionNode = dynamic_cast<RegionNode*>(worldRoot);

                if (regionNode) {
                    // Check direct locations in this region
                    for (LocationNode* location : regionNode->locations) {
                        if (location->nodeName == nodeName) {
                            location->nodeID.persistentID = persistentID;
                            return location;
                        }
                    }
                }

                // Try sub-regions if main region didn't have it
                for (TANode* child : worldRoot->childNodes) {
                    RegionNode* subRegion = dynamic_cast<RegionNode*>(child);
                    if (subRegion) {
                        for (LocationNode* location : subRegion->locations) {
                            if (location->nodeName == nodeName) {
                                location->nodeID.persistentID = persistentID;
                                return location;
                            }
                        }
                    }
                }
            }
        }

        return nullptr;
    }

    TANode* findNodeByNameRecursive(TANode* node, const std::string& nodeName)
    {
        if (node->nodeName == nodeName) {
            return node;
        }

        for (TANode* child : node->childNodes) {
            TANode* result = findNodeByNameRecursive(child, nodeName);
            if (result)
                return result;
        }

        return nullptr;
    }

    void printNodeIDsRecursive(TANode* node, int depth)
    {
        for (int i = 0; i < depth; i++)
            std::cerr << "  ";
        std::cerr << "- " << node->nodeName << ": '" << node->nodeID.persistentID << "'" << std::endl;

        for (TANode* child : node->childNodes) {
            printNodeIDsRecursive(child, depth + 1);
        }
    }

    TANode* findNodeByPersistentIDRecursive(TANode* node,
        const std::string& persistentID)
    {
        if (node->nodeID.persistentID == persistentID) {
            return node;
        }

        for (TANode* child : node->childNodes) {
            TANode* result = findNodeByPersistentIDRecursive(child, persistentID);
            if (result)
                return result;
        }

        return nullptr;
    }

    // Enhanced save state to file
    bool saveState(const std::string& filename)
    {
        // Ensure all nodes have persistent IDs
        initializePersistentIDs();

        try {
            json saveData;

            // Save systems and current nodes
            json systemsData = json::array();
            for (const auto& [name, root] : systemRoots) {
                json systemEntry;
                systemEntry["name"] = name;

                // Save current node if it exists
                if (currentNodes.find(name) != currentNodes.end()) {
                    systemEntry["hasCurrentNode"] = true;
                    systemEntry["currentNodeID"] = currentNodes[name]->nodeID.persistentID;

                    // Serialize node state
                    json stateData;
                    for (const auto& [key, value] : currentNodes[name]->stateData) {
                        if (std::holds_alternative<int>(value)) {
                            stateData[key] = std::get<int>(value);
                        } else if (std::holds_alternative<float>(value)) {
                            stateData[key] = std::get<float>(value);
                        } else if (std::holds_alternative<std::string>(value)) {
                            stateData[key] = std::get<std::string>(value);
                        } else if (std::holds_alternative<bool>(value)) {
                            stateData[key] = std::get<bool>(value);
                        }
                    }
                    systemEntry["nodeState"] = stateData;
                } else {
                    systemEntry["hasCurrentNode"] = false;
                }

                systemsData.push_back(systemEntry);
            }
            saveData["systems"] = systemsData;

            // Save game context
            saveData["playerStats"] = serializeCharacterStats(gameContext.playerStats);
            saveData["worldState"] = serializeWorldState(gameContext.worldState);
            saveData["inventory"] = serializeInventory(gameContext.playerInventory);
            saveData["questJournal"] = gameContext.questJournal;
            saveData["dialogueHistory"] = gameContext.dialogueHistory;

            // Write to file
            std::ofstream outFile(filename);
            if (!outFile.is_open()) {
                std::cerr << "Failed to open file for saving: " << filename << std::endl;
                return false;
            }

            outFile << std::setw(4) << saveData << std::endl;
            outFile.close();

            std::cout << "Game state successfully saved to " << filename << std::endl;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error saving game state: " << e.what() << std::endl;
            return false;
        }
    }

    bool loadState(const std::string& filename)
    {
        try {
            // Ensure all nodes have persistent IDs before loading
            initializePersistentIDs();

            std::ifstream inFile(filename);
            if (!inFile.is_open()) {
                std::cerr << "Failed to open save file: " << filename << std::endl;
                return false;
            }

            json saveData = json::parse(inFile);

            // Clear current state
            currentNodes.clear();

            // Load systems and current nodes
            for (const auto& systemEntry : saveData["systems"]) {
                std::string name = systemEntry["name"];

                if (systemRoots.find(name) == systemRoots.end()) {
                    std::cerr << "System not found during load: " << name << std::endl;
                    continue;
                }

                bool hasCurrentNode = systemEntry["hasCurrentNode"];
                if (hasCurrentNode) {
                    std::string persistentID = systemEntry["currentNodeID"];

                    // Find the node with this ID
                    TANode* node = findNodeByPersistentID(persistentID);

                    if (node) {
                        // Deserialize node state
                        if (systemEntry.contains("nodeState")) {
                            node->stateData.clear();
                            for (auto& [key, value] : systemEntry["nodeState"].items()) {
                                if (value.is_number_integer()) {
                                    node->stateData[key] = value.get<int>();
                                } else if (value.is_number_float()) {
                                    node->stateData[key] = value.get<float>();
                                } else if (value.is_string()) {
                                    node->stateData[key] = value.get<std::string>();
                                } else if (value.is_boolean()) {
                                    node->stateData[key] = value.get<bool>();
                                }
                            }
                        }

                        currentNodes[name] = node;
                        std::cout << "Successfully restored node " << node->nodeName
                                  << " for system " << name << std::endl;
                    } else {
                        std::cerr << "Node not found during load: " << persistentID << std::endl;
                        currentNodes[name] = systemRoots[name]; // Default to root
                    }
                }
            }

            // Load game context
            deserializeCharacterStats(saveData["playerStats"], gameContext.playerStats);
            deserializeWorldState(saveData["worldState"], gameContext.worldState);
            deserializeInventory(saveData["inventory"], gameContext.playerInventory);

            gameContext.questJournal = saveData["questJournal"].get<std::map<std::string, std::string>>();
            gameContext.dialogueHistory = saveData["dialogueHistory"].get<std::map<std::string, std::string>>();

            inFile.close();

            std::cout << "Game state successfully loaded from " << filename << std::endl;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error loading game state: " << e.what() << std::endl;
            return false;
        }
    }

private:
    // Helper functions for serializing game context components
    json serializeCharacterStats(const CharacterStats& stats)
    {
        json statsData;

        // Basic stats
        statsData["strength"] = stats.strength;
        statsData["dexterity"] = stats.dexterity;
        statsData["constitution"] = stats.constitution;
        statsData["intelligence"] = stats.intelligence;
        statsData["wisdom"] = stats.wisdom;
        statsData["charisma"] = stats.charisma;

        // Skills
        statsData["skills"] = stats.skills;

        // Faction reputation
        statsData["factionReputation"] = stats.factionReputation;

        // Known facts
        statsData["knownFacts"] = json::array();
        for (const auto& fact : stats.knownFacts) {
            statsData["knownFacts"].push_back(fact);
        }

        // Unlocked abilities
        statsData["unlockedAbilities"] = json::array();
        for (const auto& ability : stats.unlockedAbilities) {
            statsData["unlockedAbilities"].push_back(ability);
        }

        return statsData;
    }

    void deserializeCharacterStats(const json& statsData, CharacterStats& stats)
    {
        // Basic stats
        stats.strength = statsData["strength"];
        stats.dexterity = statsData["dexterity"];
        stats.constitution = statsData["constitution"];
        stats.intelligence = statsData["intelligence"];
        stats.wisdom = statsData["wisdom"];
        stats.charisma = statsData["charisma"];

        // Skills
        stats.skills = statsData["skills"].get<std::map<std::string, int>>();

        // Faction reputation
        stats.factionReputation = statsData["factionReputation"].get<std::map<std::string, int>>();

        // Known facts
        stats.knownFacts.clear();
        for (const auto& fact : statsData["knownFacts"]) {
            stats.knownFacts.insert(fact);
        }

        // Unlocked abilities
        stats.unlockedAbilities.clear();
        for (const auto& ability : statsData["unlockedAbilities"]) {
            stats.unlockedAbilities.insert(ability);
        }
    }

    json serializeWorldState(const WorldState& state)
    {
        json worldData;

        worldData["locationStates"] = state.locationStates;
        worldData["factionStates"] = state.factionStates;
        worldData["worldFlags"] = state.worldFlags;
        worldData["daysPassed"] = state.daysPassed;
        worldData["currentSeason"] = state.currentSeason;

        return worldData;
    }

    void deserializeWorldState(const json& worldData, WorldState& state)
    {
        state.locationStates = worldData["locationStates"].get<std::map<std::string, std::string>>();
        state.factionStates = worldData["factionStates"].get<std::map<std::string, std::string>>();
        state.worldFlags = worldData["worldFlags"].get<std::map<std::string, bool>>();
        state.daysPassed = worldData["daysPassed"];
        state.currentSeason = worldData["currentSeason"];
    }

    json serializeInventory(const Inventory& inventory)
    {
        json inventoryData = json::array();

        for (const auto& item : inventory.items) {
            json itemData;
            itemData["id"] = item.id;
            itemData["name"] = item.name;
            itemData["type"] = item.type;
            itemData["value"] = item.value;
            itemData["quantity"] = item.quantity;

            json properties;
            for (const auto& [key, value] : item.properties) {
                if (std::holds_alternative<int>(value)) {
                    properties[key] = std::get<int>(value);
                } else if (std::holds_alternative<float>(value)) {
                    properties[key] = std::get<float>(value);
                } else if (std::holds_alternative<std::string>(value)) {
                    properties[key] = std::get<std::string>(value);
                } else if (std::holds_alternative<bool>(value)) {
                    properties[key] = std::get<bool>(value);
                }
            }
            itemData["properties"] = properties;

            inventoryData.push_back(itemData);
        }

        return inventoryData;
    }

    void deserializeInventory(const json& inventoryData, Inventory& inventory)
    {
        inventory.items.clear();

        for (const auto& itemData : inventoryData) {
            Item item(
                itemData["id"],
                itemData["name"],
                itemData["type"],
                itemData["value"],
                itemData["quantity"]);

            // Load properties
            for (const auto& [key, value] : itemData["properties"].items()) {
                if (value.is_number_integer()) {
                    item.properties[key] = value.get<int>();
                } else if (value.is_number_float()) {
                    item.properties[key] = value.get<float>();
                } else if (value.is_string()) {
                    item.properties[key] = value.get<std::string>();
                } else if (value.is_boolean()) {
                    item.properties[key] = value.get<bool>();
                }
            }

            inventory.items.push_back(item);
        }
    }
};

// Forward declare loading functions to resolve circular dependencies
void loadQuestsFromJSON(TAController& controller, const json& questData);
void loadNPCsFromJSON(TAController& controller, const json& npcData);
void loadSkillsFromJSON(TAController& controller, const json& skillsData);
void loadCraftingFromJSON(TAController& controller, const json& craftingData);
void loadWorldFromJSON(TAController& controller, const json& worldData);

// Function to load game data from JSON files
bool loadGameData(TAController& controller)
{
    try {
        // Make sure the data directory exists
        if (!std::filesystem::exists("data")) {
            std::filesystem::create_directory("data");
            std::cout << "Created data directory" << std::endl;
            createDefaultJSONFiles();
        }

        // Load quests
        std::ifstream questFile("data/quests.json");
        if (questFile.is_open()) {
            json questData = json::parse(questFile);
            loadQuestsFromJSON(controller, questData);
            std::cout << "Loaded quest data" << std::endl;
        } else {
            std::cerr << "Failed to open data/quests.json" << std::endl;
        }

        // Load NPCs and dialogue
        std::ifstream npcFile("data/npcs.json");
        if (npcFile.is_open()) {
            json npcData = json::parse(npcFile);
            loadNPCsFromJSON(controller, npcData);
            std::cout << "Loaded NPC data" << std::endl;
        } else {
            std::cerr << "Failed to open data/npcs.json" << std::endl;
        }

        // Load skills and progression
        std::ifstream skillsFile("data/skills.json");
        if (skillsFile.is_open()) {
            json skillsData = json::parse(skillsFile);
            loadSkillsFromJSON(controller, skillsData);
            std::cout << "Loaded skills data" << std::endl;
        } else {
            std::cerr << "Failed to open data/skills.json" << std::endl;
        }

        // Load crafting recipes
        std::ifstream craftingFile("data/crafting.json");
        if (craftingFile.is_open()) {
            json craftingData = json::parse(craftingFile);
            loadCraftingFromJSON(controller, craftingData);
            std::cout << "Loaded crafting data" << std::endl;
        } else {
            std::cerr << "Failed to open data/crafting.json" << std::endl;
        }

        // Load world data
        std::ifstream worldFile("data/world.json");
        if (worldFile.is_open()) {
            json worldData = json::parse(worldFile);
            loadWorldFromJSON(controller, worldData);
            std::cout << "Loaded world data" << std::endl;
        } else {
            std::cerr << "Failed to open data/world.json" << std::endl;
        }

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading game data: " << e.what() << std::endl;
        return false;
    }
}

// Function to create default JSON files if they don't exist
void createDefaultJSONFiles()
{
    // Create quests.json
    std::ofstream questFile("data/quests.json");
    if (questFile.is_open()) {
        json questData;
        questData["quests"] = json::array({ { { "id", "MainQuest" },
            { "title", "Defend the Village" },
            { "description", "The village is under threat. Prepare its defenses!" },
            { "state", "Available" },
            { "isAcceptingState", false },
            { "rewards", json::array({ { { "type", "experience" }, { "amount", 500 }, { "itemId", "" } }, { { "type", "gold" }, { "amount", 200 }, { "itemId", "" } }, { { "type", "faction" }, { "amount", 25 }, { "itemId", "villagers" } }, { { "type", "item" }, { "amount", 1 }, { "itemId", "defenders_shield" } } }) },
            { "requirements", json::array() },
            { "subquests", json::array({ { { "id", "RepairWalls" }, { "title", "Repair the Walls" }, { "description", "The village walls are in disrepair. Fix them!" }, { "state", "Available" }, { "isAcceptingState", false }, { "rewards", json::array({ { { "type", "experience" }, { "amount", 100 }, { "itemId", "" } }, { { "type", "gold" }, { "amount", 50 }, { "itemId", "" } }, { { "type", "faction" }, { "amount", 10 }, { "itemId", "villagers" } } }) }, { "requirements", json::array({ { { "type", "skill" }, { "target", "crafting" }, { "value", 1 } } }) }, { "transitions", json::array({ { { "action", "repair_complete" }, { "target", "MainQuest" }, { "description", "Complete wall repairs" } } }) } }, { { "id", "TrainMilitia" }, { "title", "Train the Militia" }, { "description", "The villagers need combat training." }, { "state", "Available" }, { "isAcceptingState", false }, { "rewards", json::array({ { { "type", "experience" }, { "amount", 150 }, { "itemId", "" } }, { { "type", "skill" }, { "amount", 1 }, { "itemId", "combat" } } }) }, { "requirements", json::array({ { { "type", "skill" }, { "target", "combat" }, { "value", 2 } } }) }, { "transitions", json::array({ { { "action", "training_complete" }, { "target", "MainQuest" }, { "description", "Complete militia training" } } }) } }, { { "id", "GatherSupplies" }, { "title", "Gather Supplies" }, { "description", "The village needs food and resources." }, { "state", "Available" }, { "isAcceptingState", false }, { "rewards", json::array({ { { "type", "experience" }, { "amount", 100 }, { "itemId", "" } }, { { "type", "item" }, { "amount", 1 }, { "itemId", "rare_herb" } } }) }, { "requirements", json::array({ { { "type", "skill" }, { "target", "survival" }, { "value", 1 } } }) }, { "transitions", json::array({ { { "action", "supplies_gathered" }, { "target", "MainQuest" }, { "description", "Finish gathering supplies" } } }) } } }) } } });
        questFile << std::setw(4) << questData << std::endl;
        questFile.close();
    }

    // Create npcs.json
    std::ofstream npcFile("data/npcs.json");
    if (npcFile.is_open()) {
        json npcData;
        npcData["npcs"] = json::array({ { { "id", "elder_marius" },
            { "name", "Elder Marius" },
            { "description", "The wise leader of the village" },
            { "relationshipValue", 0 },
            { "rootDialogue", "ElderGreeting" },
            { "dialogueNodes", json::array({ { { "id", "ElderGreeting" }, { "speakerName", "Elder Marius" }, { "dialogueText", "Greetings, traveler. Our village faces difficult times." }, { "responses", json::array({ { { "text", "What threat does the village face?" }, { "targetNode", "AskThreat" }, { "requirements", json::array() }, { "effects", json::array() } }, { { "text", "Is there something I can help with?" }, { "targetNode", "AskHelp" }, { "requirements", json::array() }, { "effects", json::array() } }, { { "text", "I need to go. Farewell." }, { "targetNode", "Farewell" }, { "requirements", json::array() }, { "effects", json::array() } } }) } }, { { "id", "AskThreat" }, { "speakerName", "Elder Marius" }, { "dialogueText", "Bandits have been raiding nearby settlements. I fear we're next." }, { "responses", json::array({ { { "text", "How can I help against these bandits?" }, { "targetNode", "AskHelp" }, { "requirements", json::array() }, { "effects", json::array() } }, { { "text", "I'll be on my way." }, { "targetNode", "Farewell" }, { "requirements", json::array() }, { "effects", json::array() } } }) } }, { { "id", "AskHelp" }, { "speakerName", "Elder Marius" }, { "dialogueText", "We need someone skilled to help prepare our defenses." }, { "responses", json::array({ { { "text", "I'll help defend the village." }, { "targetNode", "AcceptQuest" }, { "requirements", json::array() }, { "effects", json::array({ { { "type", "quest" }, { "action", "activate" }, { "target", "MainQuest" } }, { { "type", "knowledge" }, { "action", "add" }, { "target", "village_under_threat" } }, { { "type", "faction" }, { "action", "change" }, { "target", "villagers" }, { "amount", 5 } } }) } }, { { "text", "I'm not interested in helping." }, { "targetNode", "RejectQuest" }, { "requirements", json::array() }, { "effects", json::array() } }, { { "text", "I need to think about it." }, { "targetNode", "Farewell" }, { "requirements", json::array() }, { "effects", json::array() } } }) } }, { { "id", "AcceptQuest" }, { "speakerName", "Elder Marius" }, { "dialogueText", "Thank you! This means a lot to our community. We need the walls repaired, the militia trained, and supplies gathered." }, { "responses", json::array({ { { "text", "I'll get started right away." }, { "targetNode", "Farewell" }, { "requirements", json::array() }, { "effects", json::array() } } }) } }, { { "id", "RejectQuest" }, { "speakerName", "Elder Marius" }, { "dialogueText", "I understand. Perhaps you'll reconsider when you have time." }, { "responses", json::array({ { { "text", "Goodbye." }, { "targetNode", "Farewell" }, { "requirements", json::array() }, { "effects", json::array() } } }) } }, { { "id", "Farewell" }, { "speakerName", "Elder Marius" }, { "dialogueText", "Safe travels, friend. Return if you need anything." }, { "responses", json::array() } } }) } } });
        npcFile << std::setw(4) << npcData << std::endl;
        npcFile.close();
    }

    // Create skills.json
    std::ofstream skillsFile("data/skills.json");
    if (skillsFile.is_open()) {
        json skillsData;
        // Add skills data
        skillsData["skills"] = json::array({ { { "id", "CombatBasics" },
                                                 { "skillName", "combat" },
                                                 { "description", "Basic combat techniques and weapon handling." },
                                                 { "level", 0 },
                                                 { "maxLevel", 5 },
                                                 { "requirements", json::array() },
                                                 { "effects", json::array({ { { "type", "stat" }, { "target", "strength" }, { "value", 1 } } }) },
                                                 { "costs", json::array() },
                                                 { "childSkills", json::array({ { { "id", "Swordsmanship" }, { "skillName", "swordsmanship" }, { "description", "Advanced sword techniques for greater damage and defense." }, { "level", 0 }, { "maxLevel", 5 }, { "requirements", json::array({ { { "type", "skill" }, { "target", "combat" }, { "level", 2 } } }) }, { "effects", json::array({ { { "type", "ability" }, { "target", "power_attack" }, { "value", 0 } } }) } }, { { "id", "Archery" }, { "skillName", "archery" }, { "description", "Precision with bows and other ranged weapons." }, { "level", 0 }, { "maxLevel", 5 }, { "requirements", json::array({ { { "type", "skill" }, { "target", "combat" }, { "level", 2 } } }) }, { "effects", json::array({ { { "type", "ability" }, { "target", "precise_shot" }, { "value", 0 } } }) } } }) } },
            { { "id", "SurvivalBasics" },
                { "skillName", "survival" },
                { "description", "Basic survival skills for harsh environments." },
                { "level", 0 },
                { "maxLevel", 5 },
                { "requirements", json::array() },
                { "effects", json::array({ { { "type", "stat" }, { "target", "constitution" }, { "value", 1 } } }) },
                { "costs", json::array() },
                { "childSkills", json::array({ { { "id", "Herbalism" }, { "skillName", "herbalism" }, { "description", "Knowledge of medicinal and poisonous plants." }, { "level", 0 }, { "maxLevel", 5 }, { "requirements", json::array({ { { "type", "skill" }, { "target", "survival" }, { "level", 2 } } }) }, { "effects", json::array({ { { "type", "ability" }, { "target", "herbal_remedy" }, { "value", 0 } } }) } }, { { "id", "Tracking" }, { "skillName", "tracking" }, { "description", "Follow trails and find creatures in the wilderness." }, { "level", 0 }, { "maxLevel", 5 }, { "requirements", json::array({ { { "type", "skill" }, { "target", "survival" }, { "level", 1 } } }) }, { "effects", json::array({ { { "type", "ability" }, { "target", "track_prey" }, { "value", 0 } } }) } } }) } },
            { { "id", "CraftingBasics" },
                { "skillName", "crafting" },
                { "description", "Basic crafting and repair techniques." },
                { "level", 0 },
                { "maxLevel", 5 },
                { "requirements", json::array() },
                { "effects", json::array({ { { "type", "stat" }, { "target", "dexterity" }, { "value", 1 } } }) },
                { "costs", json::array() },
                { "childSkills", json::array({ { { "id", "Blacksmithing" }, { "skillName", "blacksmithing" }, { "description", "Forge and improve metal weapons and armor." }, { "level", 0 }, { "maxLevel", 5 }, { "requirements", json::array({ { { "type", "skill" }, { "target", "crafting" }, { "level", 2 } } }) }, { "effects", json::array({ { { "type", "ability" }, { "target", "forge_weapon" }, { "value", 0 } } }) } }, { { "id", "Alchemy" }, { "skillName", "alchemy" }, { "description", "Create potions and elixirs with magical effects." }, { "level", 0 }, { "maxLevel", 5 }, { "requirements", json::array({ { { "type", "skill" }, { "target", "crafting" }, { "level", 1 } }, { { "type", "skill" }, { "target", "herbalism" }, { "level", 1 } } }) }, { "effects", json::array({ { { "type", "ability" }, { "target", "brew_potion" }, { "value", 0 } } }) } } }) } } });

        // Add classes data
        skillsData["classes"] = json::array({ { { "id", "Warrior" },
                                                  { "className", "Warrior" },
                                                  { "description", "Masters of combat, strong and resilient." },
                                                  { "statBonuses", { { "strength", 3 }, { "constitution", 2 } } },
                                                  { "startingAbilities", { "weapon_specialization" } },
                                                  { "classSkills", { "CombatBasics", "Swordsmanship" } } },
            { { "id", "Ranger" },
                { "className", "Ranger" },
                { "description", "Wilderness experts, skilled with bow and blade." },
                { "statBonuses", { { "dexterity", 2 }, { "wisdom", 2 } } },
                { "startingAbilities", { "animal_companion" } },
                { "classSkills", { "Archery", "Tracking" } } },
            { { "id", "Alchemist" },
                { "className", "Alchemist" },
                { "description", "Masters of potions and elixirs." },
                { "statBonuses", { { "intelligence", 3 }, { "dexterity", 1 } } },
                { "startingAbilities", { "potion_mastery" } },
                { "classSkills", { "Herbalism", "Alchemy" } } } });

        skillsFile << std::setw(4) << skillsData << std::endl;
        skillsFile.close();
    }

    // Create crafting.json
    std::ofstream craftingFile("data/crafting.json");
    if (craftingFile.is_open()) {
        json craftingData;
        craftingData["craftingStations"] = json::array({ { { "id", "BlacksmithStation" },
                                                             { "stationType", "Blacksmith" },
                                                             { "description", "A forge with anvil, hammers, and other metalworking tools." },
                                                             { "recipes", json::array({ { { "recipeId", "sword_recipe" }, { "name", "Iron Sword" }, { "description", "A standard iron sword, good for combat." }, { "discovered", true }, { "ingredients", json::array({ { { "itemId", "iron_ingot" }, { "quantity", 2 } }, { { "itemId", "leather_strips" }, { "quantity", 1 } } }) }, { "skillRequirements", { { "blacksmithing", 1 } } }, { "result", { { "itemId", "iron_sword" }, { "name", "Iron Sword" }, { "type", "weapon" }, { "quantity", 1 }, { "properties", { { "damage", 10 } } } } } }, { { "recipeId", "armor_recipe" }, { "name", "Leather Armor" }, { "description", "Basic protective gear made from leather." }, { "discovered", true }, { "ingredients", json::array({ { { "itemId", "leather" }, { "quantity", 5 } }, { { "itemId", "metal_studs" }, { "quantity", 10 } } }) }, { "skillRequirements", { { "crafting", 2 } } }, { "result", { { "itemId", "leather_armor" }, { "name", "Leather Armor" }, { "type", "armor" }, { "quantity", 1 }, { "properties", { { "defense", 5 } } } } } } }) } },
            { { "id", "AlchemyStation" },
                { "stationType", "Alchemy" },
                { "description", "A workbench with alembics, mortars, and various containers for brewing." },
                { "recipes", json::array({ { { "recipeId", "health_potion_recipe" }, { "name", "Minor Healing Potion" }, { "description", "A potion that restores a small amount of health." }, { "discovered", true }, { "ingredients", json::array({ { { "itemId", "red_herb" }, { "quantity", 2 } }, { { "itemId", "water_flask" }, { "quantity", 1 } } }) }, { "skillRequirements", { { "alchemy", 1 } } }, { "result", { { "itemId", "minor_healing_potion" }, { "name", "Minor Healing Potion" }, { "type", "potion" }, { "quantity", 1 }, { "properties", { { "heal_amount", 25 } } } } } } }) } },
            { { "id", "CookingStation" },
                { "stationType", "Cooking" },
                { "description", "A firepit with cooking pots and utensils." },
                { "recipes", json::array({ { { "recipeId", "stew_recipe" }, { "name", "Hearty Stew" }, { "description", "A filling meal that provides temporary stat bonuses." }, { "discovered", true }, { "ingredients", json::array({ { { "itemId", "meat" }, { "quantity", 2 } }, { { "itemId", "vegetables" }, { "quantity", 3 } } }) }, { "skillRequirements", { { "cooking", 1 } } }, { "result", { { "itemId", "hearty_stew" }, { "name", "Hearty Stew" }, { "type", "food" }, { "quantity", 2 }, { "properties", { { "effect_duration", 300 } } } } } } }) } } });
        craftingFile << std::setw(4) << craftingData << std::endl;
        craftingFile.close();
    }

    // Create world.json
    std::ofstream worldFile("data/world.json");
    if (worldFile.is_open()) {
        json worldData;
        worldData["regions"] = json::array({ { { "id", "VillageRegion" },
                                                 { "regionName", "Oakvale Village" },
                                                 { "description", "A peaceful farming village surrounded by wooden palisades." },
                                                 { "controllingFaction", "villagers" },
                                                 { "connectedRegions", { "ForestRegion", "MountainRegion" } },
                                                 { "locations", json::array({ { { "id", "VillageCenter" }, { "locationName", "Village Center" }, { "description", "The bustling center of the village with a market and well." }, { "currentState", "normal" }, { "stateDescriptions", { { "damaged", "The village center shows signs of damage from bandit raids." }, { "rebuilt", "The village center has been rebuilt stronger than before." } } }, { "accessConditions", json::array() }, { "npcs", { "elder_marius" } }, { "activities", { "MainQuest" } } }, { { "id", "VillageInn" }, { "locationName", "The Sleeping Dragon Inn" }, { "description", "A cozy inn where travelers find rest and information." }, { "currentState", "normal" }, { "stateDescriptions", {} }, { "accessConditions", json::array() }, { "npcs", json::array() }, { "activities", json::array() } }, { { "id", "VillageForge" }, { "locationName", "Blacksmith's Forge" }, { "description", "The local blacksmith's workshop with a roaring forge." }, { "currentState", "normal" }, { "stateDescriptions", {} }, { "accessConditions", json::array() }, { "npcs", json::array() }, { "activities", { "BlacksmithStation" } } } }) },
                                                 { "possibleEvents", json::array({ { { "name", "Bandit Raid" }, { "description", "A small group of bandits is attacking the village outskirts!" }, { "condition", { { "type", "worldflag" }, { "flag", "village_defended" }, { "value", false } } }, { "effect", { { "type", "location" }, { "target", "village" }, { "state", "under_attack" } } }, { "probability", 0.2 } } }) } },
            { { "id", "ForestRegion" },
                { "regionName", "Green Haven Forest" },
                { "description", "A dense forest with ancient trees and hidden paths." },
                { "controllingFaction", "forest guardians" },
                { "connectedRegions", { "VillageRegion", "MountainRegion" } },
                { "locations", json::array({ { { "id", "ForestClearing" }, { "locationName", "Forest Clearing" }, { "description", "A peaceful clearing in the heart of the forest." }, { "currentState", "normal" }, { "stateDescriptions", {} }, { "accessConditions", json::array() }, { "npcs", json::array() }, { "activities", json::array() } }, { { "id", "AncientGroves" }, { "locationName", "Ancient Groves" }, { "description", "An area with trees older than any human memory." }, { "currentState", "normal" }, { "stateDescriptions", {} }, { "accessConditions", json::array({ { { "type", "skill" }, { "target", "survival" }, { "value", 2 } } }) }, { "npcs", json::array() }, { "activities", json::array() } } }) },
                { "possibleEvents", json::array({ { { "name", "Rare Herb Sighting" }, { "description", "You spot a patch of rare medicinal herbs growing nearby." }, { "condition", { { "type", "skill" }, { "skill", "herbalism" }, { "value", 1 } } }, { "effect", { { "type", "item" }, { "item", "rare_herb" }, { "quantity", 1 } } }, { "probability", 0.3 } } }) } },
            { { "id", "MountainRegion" },
                { "regionName", "Stone Peak Mountains" },
                { "description", "Rugged mountains with treacherous paths and hidden caves." },
                { "controllingFaction", "mountainfolk" },
                { "connectedRegions", { "VillageRegion", "ForestRegion" } },
                { "locations", json::array({ { { "id", "MountainPass" }, { "locationName", "Mountain Pass" }, { "description", "A winding path through the mountains." }, { "currentState", "normal" }, { "stateDescriptions", {} }, { "accessConditions", json::array() }, { "npcs", json::array() }, { "activities", json::array() } }, { { "id", "AbandonedMine" }, { "locationName", "Abandoned Mine" }, { "description", "An old mine, no longer in use. Rumors say something lurks within." }, { "currentState", "normal" }, { "stateDescriptions", {} }, { "accessConditions", json::array({ { { "type", "item" }, { "target", "torch" }, { "value", 1 } } }) }, { "npcs", json::array() }, { "activities", json::array() } } }) },
                { "possibleEvents", json::array() } } });

        worldData["timeSystem"] = {
            { "day", 1 },
            { "hour", 6 },
            { "season", "spring" },
            { "timeOfDay", "morning" }
        };

        worldFile << std::setw(4) << worldData << std::endl;
        worldFile.close();
    }
}

// Load quests from JSON
void loadQuestsFromJSON(TAController& controller, const json& questData)
{
    for (const auto& questEntry : questData["quests"]) {
        // Create main quest node
        QuestNode* quest = dynamic_cast<QuestNode*>(
            controller.createNode<QuestNode>(questEntry["id"]));

        quest->questTitle = questEntry["title"];
        quest->questDescription = questEntry["description"];
        quest->questState = questEntry["state"];
        quest->isAcceptingState = questEntry["isAcceptingState"];

        // Load rewards
        for (const auto& rewardData : questEntry["rewards"]) {
            QuestNode::QuestReward reward;
            reward.type = rewardData["type"];
            reward.amount = rewardData["amount"];
            reward.itemId = rewardData["itemId"];
            quest->rewards.push_back(reward);
        }

        // Load requirements
        for (const auto& reqData : questEntry["requirements"]) {
            QuestNode::QuestRequirement req;
            req.type = reqData["type"];
            req.target = reqData["target"];
            req.value = reqData["value"];
            quest->requirements.push_back(req);
        }

        // Create and link subquests
        std::map<std::string, QuestNode*> questNodes;
        questNodes[quest->nodeName] = quest;

        // First, create all subquest nodes
        for (const auto& subquestData : questEntry["subquests"]) {
            QuestNode* subquest = dynamic_cast<QuestNode*>(
                controller.createNode<QuestNode>(subquestData["id"]));

            subquest->questTitle = subquestData["title"];
            subquest->questDescription = subquestData["description"];
            subquest->questState = subquestData["state"];
            subquest->isAcceptingState = subquestData["isAcceptingState"];

            // Load rewards
            for (const auto& rewardData : subquestData["rewards"]) {
                QuestNode::QuestReward reward;
                reward.type = rewardData["type"];
                reward.amount = rewardData["amount"];
                reward.itemId = rewardData["itemId"];
                subquest->rewards.push_back(reward);
            }

            // Load requirements
            for (const auto& reqData : subquestData["requirements"]) {
                QuestNode::QuestRequirement req;
                req.type = reqData["type"];
                req.target = reqData["target"];
                req.value = reqData["value"];
                subquest->requirements.push_back(req);
            }

            questNodes[subquest->nodeName] = subquest;
            quest->addChild(subquest);
        }

        // Now set up transitions (after all nodes are created)
        for (const auto& subquestData : questEntry["subquests"]) {
            QuestNode* subquest = questNodes[subquestData["id"]];

            // Add transitions
            for (const auto& transData : subquestData["transitions"]) {
                std::string action = transData["action"];
                std::string targetId = transData["target"];
                std::string description = transData["description"];

                subquest->addTransition(
                    [action](const TAInput& input) {
                        return input.type == "action" && std::get<std::string>(input.parameters.at("name")) == action;
                    },
                    questNodes[targetId],
                    description);
            }
        }

        // Register the quest system
        if (questEntry["id"] == "MainQuest") {
            controller.setSystemRoot("QuestSystem", quest);
        }
    }
}

// Load NPCs and dialogue from JSON
void loadNPCsFromJSON(TAController& controller, const json& npcData)
{
    std::map<std::string, NPC*> npcs;
    std::map<std::string, DialogueNode*> dialogueNodes;

    for (const auto& npcEntry : npcData["npcs"]) {
        // Create NPC
        NPC* npc = new NPC(npcEntry["name"], npcEntry["description"]);
        npc->relationshipValue = npcEntry["relationshipValue"];

        // Create all dialogue nodes first
        for (const auto& dialogueData : npcEntry["dialogueNodes"]) {
            DialogueNode* dialogueNode = dynamic_cast<DialogueNode*>(
                controller.createNode<DialogueNode>(
                    dialogueData["id"],
                    dialogueData["speakerName"],
                    dialogueData["dialogueText"]));

            dialogueNodes[dialogueData["id"]] = dialogueNode;
            npc->dialogueNodes[dialogueData["id"]] = dialogueNode;
        }

        // Set root dialogue
        npc->rootDialogue = dialogueNodes[npcEntry["rootDialogue"]];

        // Now connect responses after all nodes are created
        for (const auto& dialogueData : npcEntry["dialogueNodes"]) {
            DialogueNode* currentNode = dialogueNodes[dialogueData["id"]];

            for (const auto& responseData : dialogueData["responses"]) {
                // Create response function for requirements and effects
                std::function<bool(const GameContext&)> reqFunc = [](const GameContext&) { return true; };

                if (responseData.contains("requirements") && !responseData["requirements"].empty()) {
                    reqFunc = [responseData](const GameContext& ctx) {
                        for (const auto& req : responseData["requirements"]) {
                            // Implement requirement checking based on req type
                            if (req["type"] == "skill") {
                                if (!ctx.playerStats.hasSkill(req["skill"], req["level"])) {
                                    return false;
                                }
                            } else if (req["type"] == "item") {
                                if (!ctx.playerInventory.hasItem(req["item"], req["amount"])) {
                                    return false;
                                }
                            }
                            // Add other requirement types as needed
                        }
                        return true;
                    };
                }

                std::function<void(GameContext*)> effectFunc = [](GameContext*) {};

                if (responseData.contains("effects") && !responseData["effects"].empty()) {
                    effectFunc = [responseData](GameContext* ctx) {
                        for (const auto& effect : responseData["effects"]) {
                            // Implement effects based on type
                            if (effect["type"] == "quest" && effect["action"] == "activate") {
                                ctx->questJournal[effect["target"]] = "Active";
                                std::cout << "Quest activated: " << effect["target"] << std::endl;
                            } else if (effect["type"] == "knowledge" && effect["action"] == "add") {
                                ctx->playerStats.learnFact(effect["target"]);
                            } else if (effect["type"] == "faction" && effect["action"] == "change") {
                                ctx->playerStats.changeFactionRep(effect["target"], effect["amount"]);
                            }
                            // Add other effect types as needed
                        }
                    };
                }

                // Add the response with its target, requirements, and effects
                currentNode->addResponse(
                    responseData["text"],
                    dialogueNodes[responseData["targetNode"]],
                    reqFunc,
                    effectFunc);
            }
        }

        npcs[npcEntry["id"]] = npc;
    }

    // Create a dialogue controller node
    TANode* dialogueControllerNode = controller.createNode("DialogueController");
    controller.setSystemRoot("DialogueSystem", dialogueControllerNode);

    // Store NPCs for later reference
    controller.gameData["npcs"] = reinterpret_cast<std::map<std::string, void*>&>(npcs);
}

// Load skills and progression from JSON
void loadSkillsFromJSON(TAController& controller, const json& skillsData)
{
    // Create skill tree root
    TANode* skillTreeRoot = controller.createNode("SkillTreeRoot");

    std::map<std::string, SkillNode*> skillNodes;

    // First pass: create all skill nodes
    for (const auto& skillEntry : skillsData["skills"]) {
        SkillNode* skillNode = dynamic_cast<SkillNode*>(
            controller.createNode<SkillNode>(
                skillEntry["id"],
                skillEntry["skillName"],
                skillEntry.value("level", 0),
                skillEntry.value("maxLevel", 5)));

        skillNode->description = skillEntry["description"];

        // Load requirements
        for (const auto& reqData : skillEntry["requirements"]) {
            SkillNode::SkillRequirement req;
            req.type = reqData["type"];
            req.target = reqData["target"];
            req.level = reqData["level"];
            skillNode->requirements.push_back(req);
        }

        // Load effects
        for (const auto& effectData : skillEntry["effects"]) {
            SkillNode::SkillEffect effect;
            effect.type = effectData["type"];
            effect.target = effectData["target"];
            effect.value = effectData["value"];
            skillNode->effects.push_back(effect);
        }

        // Load costs if any
        if (skillEntry.contains("costs")) {
            for (const auto& costData : skillEntry["costs"]) {
                SkillNode::SkillCost cost;
                cost.type = costData["type"];
                if (costData.contains("itemId")) {
                    cost.itemId = costData["itemId"];
                }
                cost.amount = costData["amount"];
                skillNode->costs.push_back(cost);
            }
        }

        skillNodes[skillEntry["id"]] = skillNode;
        skillTreeRoot->addChild(skillNode);
    }

    // Second pass: connect child skills
    for (const auto& skillEntry : skillsData["skills"]) {
        if (skillEntry.contains("childSkills")) {
            for (const auto& childData : skillEntry["childSkills"]) {
                SkillNode* childNode = dynamic_cast<SkillNode*>(
                    controller.createNode<SkillNode>(
                        childData["id"],
                        childData["skillName"],
                        childData.value("level", 0),
                        childData.value("maxLevel", 5)));

                childNode->description = childData["description"];

                // Load requirements
                for (const auto& reqData : childData["requirements"]) {
                    SkillNode::SkillRequirement req;
                    req.type = reqData["type"];
                    req.target = reqData["target"];
                    req.level = reqData["level"];
                    childNode->requirements.push_back(req);
                }

                // Load effects
                for (const auto& effectData : childData["effects"]) {
                    SkillNode::SkillEffect effect;
                    effect.type = effectData["type"];
                    effect.target = effectData["target"];
                    effect.value = effectData["value"];
                    childNode->effects.push_back(effect);
                }

                skillNodes[childData["id"]] = childNode;
                skillNodes[skillEntry["id"]]->addChild(childNode);
            }
        }
    }

    // Create character classes
    TANode* classSelectionNode = controller.createNode("ClassSelection");

    for (const auto& classEntry : skillsData["classes"]) {
        ClassNode* classNode = dynamic_cast<ClassNode*>(
            controller.createNode<ClassNode>(
                classEntry["id"],
                classEntry["className"]));

        classNode->description = classEntry["description"];

        // Load stat bonuses
        for (const auto& [stat, bonus] : classEntry["statBonuses"].items()) {
            classNode->statBonuses[stat] = bonus;
        }

        // Load starting abilities
        for (const auto& ability : classEntry["startingAbilities"]) {
            classNode->startingAbilities.insert(ability);
        }

        // Link class skills
        for (const auto& skillId : classEntry["classSkills"]) {
            if (skillNodes.count(skillId)) {
                classNode->classSkills.push_back(skillNodes[skillId]);
            }
        }

        classSelectionNode->addChild(classNode);
    }

    // Register systems
    controller.setSystemRoot("ProgressionSystem", skillTreeRoot);
    controller.setSystemRoot("ClassSystem", classSelectionNode);
}

// Load crafting from JSON
void loadCraftingFromJSON(TAController& controller, const json& craftingData)
{
    TANode* craftingRoot = controller.createNode("CraftingRoot");

    for (const auto& stationData : craftingData["craftingStations"]) {
        CraftingNode* station = dynamic_cast<CraftingNode*>(
            controller.createNode<CraftingNode>(
                stationData["id"],
                stationData["stationType"]));

        station->description = stationData["description"];

        // Load recipes
        for (const auto& recipeData : stationData["recipes"]) {
            Recipe recipe(recipeData["recipeId"], recipeData["name"]);
            recipe.description = recipeData["description"];
            recipe.discovered = recipeData["discovered"];

            // Load ingredients
            for (const auto& ingredientData : recipeData["ingredients"]) {
                Recipe::Ingredient ingredient;
                ingredient.itemId = ingredientData["itemId"];
                ingredient.quantity = ingredientData["quantity"];
                recipe.ingredients.push_back(ingredient);
            }

            // Load skill requirements
            for (const auto& [skill, level] : recipeData["skillRequirements"].items()) {
                recipe.skillRequirements[skill] = level;
            }

            // Load result
            recipe.result.itemId = recipeData["result"]["itemId"];
            recipe.result.name = recipeData["result"]["name"];
            recipe.result.type = recipeData["result"]["type"];
            recipe.result.quantity = recipeData["result"]["quantity"];

            // Load result properties
            for (const auto& [key, value] : recipeData["result"]["properties"].items()) {
                if (value.is_number_integer()) {
                    recipe.result.properties[key] = value.get<int>();
                } else if (value.is_number_float()) {
                    recipe.result.properties[key] = value.get<float>();
                } else if (value.is_string()) {
                    recipe.result.properties[key] = value.get<std::string>();
                } else if (value.is_boolean()) {
                    recipe.result.properties[key] = value.get<bool>();
                }
            }

            station->addRecipe(recipe);
        }

        craftingRoot->addChild(station);
    }

    controller.setSystemRoot("CraftingSystem", craftingRoot);
}

// Load world from JSON
void loadWorldFromJSON(TAController& controller, const json& worldData)
{
    std::map<std::string, RegionNode*> regions;
    std::map<std::string, LocationNode*> locations;

    // First pass: create all region nodes
    for (const auto& regionData : worldData["regions"]) {
        RegionNode* region = dynamic_cast<RegionNode*>(
            controller.createNode<RegionNode>(
                regionData["id"],
                regionData["regionName"]));

        region->description = regionData["description"];
        region->controllingFaction = regionData["controllingFaction"];

        // Create locations in this region
        for (const auto& locationData : regionData["locations"]) {
            LocationNode* location = dynamic_cast<LocationNode*>(
                controller.createNode<LocationNode>(
                    locationData["id"],
                    locationData["locationName"],
                    locationData["currentState"]));

            location->description = locationData["description"];

            // Load state descriptions
            for (const auto& [state, desc] : locationData["stateDescriptions"].items()) {
                location->stateDescriptions[state] = desc;
            }

            // Load access conditions
            for (const auto& conditionData : locationData["accessConditions"]) {
                LocationNode::AccessCondition condition;
                condition.type = conditionData["type"];
                condition.target = conditionData["target"];
                condition.value = conditionData["value"];
                location->accessConditions.push_back(condition);
            }

            locations[locationData["id"]] = location;
            region->locations.push_back(location);
        }

        // Load possible events
        for (const auto& eventData : regionData["possibleEvents"]) {
            RegionNode::RegionEvent event;
            event.name = eventData["name"];
            event.description = eventData["description"];
            event.probability = eventData["probability"];

            // Set up condition function
            event.condition = [eventData](const GameContext& ctx) {
                if (eventData["condition"]["type"] == "worldflag") {
                    return ctx.worldState.hasFlag(eventData["condition"]["flag"]) == eventData["condition"]["value"].get<bool>();
                } else if (eventData["condition"]["type"] == "skill") {
                    return ctx.playerStats.hasSkill(
                        eventData["condition"]["skill"],
                        eventData["condition"]["value"]);
                }
                return true;
            };

            // Set up effect function
            event.effect = [eventData](GameContext* ctx) {
                if (!ctx)
                    return;

                if (eventData["effect"]["type"] == "location") {
                    ctx->worldState.setLocationState(
                        eventData["effect"]["target"],
                        eventData["effect"]["state"]);
                } else if (eventData["effect"]["type"] == "item") {
                    ctx->playerInventory.addItem(
                        Item(
                            eventData["effect"]["item"],
                            eventData["effect"]["item"],
                            "event_reward",
                            1,
                            eventData["effect"]["quantity"]));
                }
            };

            region->possibleEvents.push_back(event);
        }

        regions[regionData["id"]] = region;
    }

    // Second pass: connect regions
    for (const auto& regionData : worldData["regions"]) {
        RegionNode* region = regions[regionData["id"]];

        // Connect regions
        for (const auto& connectedId : regionData["connectedRegions"]) {
            if (regions.count(connectedId)) {
                region->connectedRegions.push_back(regions[connectedId]);
            }
        }
    }

    // Set up NPCs in locations and activities
    auto& npcMap = *reinterpret_cast<std::map<std::string, NPC*>*>(&controller.gameData["npcs"]);

    for (const auto& regionData : worldData["regions"]) {
        for (const auto& locationData : regionData["locations"]) {
            LocationNode* location = locations[locationData["id"]];

            // Add NPCs
            if (locationData.contains("npcs")) {
                for (const auto& npcId : locationData["npcs"]) {
                    if (npcMap.count(npcId)) {
                        location->npcs.push_back(npcMap[npcId]);
                    }
                }
            }

            // Add activities (quests, crafting, etc.)
            if (locationData.contains("activities")) {
                for (const auto& activityId : locationData["activities"]) {
                    // Try to find the activity in various systems
                    TANode* activity = nullptr;

                    if (controller.systemRoots.count("QuestSystem") && controller.systemRoots["QuestSystem"]->nodeName == activityId) {
                        activity = controller.systemRoots["QuestSystem"];
                    } else if (controller.systemRoots.count("CraftingSystem")) {
                        // Search in crafting children
                        for (TANode* child : controller.systemRoots["CraftingSystem"]->childNodes) {
                            if (child->nodeName == activityId) {
                                activity = child;
                                break;
                            }
                        }
                    }

                    if (activity) {
                        location->activities.push_back(activity);
                    }
                }
            }
        }
    }

    // Load time system
    TimeNode* timeSystem = dynamic_cast<TimeNode*>(
        controller.createNode<TimeNode>("TimeSystem"));

    timeSystem->day = worldData["timeSystem"]["day"];
    timeSystem->hour = worldData["timeSystem"]["hour"];
    timeSystem->season = worldData["timeSystem"]["season"];
    timeSystem->timeOfDay = worldData["timeSystem"]["timeOfDay"];

    // Register the world and time systems
    controller.setSystemRoot("WorldSystem", regions["VillageRegion"]); // Start in village
    controller.setSystemRoot("TimeSystem", timeSystem);
}

// Main function demonstrating all systems
int main()
{
    std::cout << "___ Starting Raw Oath ___" << std::endl;

    // Create the automaton controller
    TAController controller;

    // Load all game data from JSON files
    std::cout << "___ LOADING GAME DATA FROM JSON FILES ___" << std::endl;
    if (!loadGameData(controller)) {
        std::cerr << "Failed to load game data. Exiting." << std::endl;
        return 1;
    }

    std::cout << "\n___ GAME DATA LOADED SUCCESSFULLY ___\n"
              << std::endl;

    // Initialize player inventory with some items
    controller.gameContext.playerInventory.addItem(
        { "iron_ingot", "Iron Ingot", "material", 10, 5 });
    controller.gameContext.playerInventory.addItem(
        { "leather_strips", "Leather Strips", "material", 5, 10 });
    controller.gameContext.playerInventory.addItem(
        { "torch", "Torch", "tool", 2, 3 });
    controller.gameContext.playerInventory.addItem(
        { "red_herb", "Red Herb", "herb", 3, 5 });
    controller.gameContext.playerInventory.addItem(
        { "water_flask", "Water Flask", "container", 1, 2 });

    // Set initial skills
    controller.gameContext.playerStats.improveSkill("combat", 2);
    controller.gameContext.playerStats.improveSkill("survival", 1);
    controller.gameContext.playerStats.improveSkill("crafting", 1);

    // Example: Start at village and talk to elder
    std::cout << "\n=== WORLD AND DIALOGUE EXAMPLE ===\n"
              << std::endl;

    // Start in village region
    controller.processInput("WorldSystem", {});

    // Travel to village center
    TAInput travelInput = {
        "region_action",
        { { "action", std::string("travel_location") }, { "location_index", 0 } }
    };
    controller.processInput("WorldSystem", travelInput);

    // Get the Elder Marius NPC from the game data
    std::map<std::string, NPC*>& npcMap = *reinterpret_cast<std::map<std::string, NPC*>*>(&controller.gameData["npcs"]);
    NPC* elderMarius = npcMap["elder_marius"];

    // Talk to the village elder
    if (elderMarius) {
        std::cout << "\nTalking to Elder Marius...\n"
                  << std::endl;
        elderMarius->startDialogue(&controller.gameContext);

        // Example: Choose dialogue option 0 (Ask about threat)
        elderMarius->processResponse(0, &controller.gameContext);

        // Example: Choose dialogue option 0 (Ask how to help)
        elderMarius->processResponse(0, &controller.gameContext);

        // Example: Choose dialogue option 0 (Accept quest)
        elderMarius->processResponse(0, &controller.gameContext);

        // Example: Choose dialogue option 0 (I'll get started)
        elderMarius->processResponse(0, &controller.gameContext);
    }

    // Example: Try crafting
    std::cout << "\n=== CRAFTING EXAMPLE ===\n"
              << std::endl;

    // Access the crafting system
    controller.processInput("CraftingSystem", {});

    // Navigate to blacksmith station (first child)
    TAInput craftingInput = {
        "crafting_action",
        { { "action", std::string("select_station") }, { "index", 0 } }
    };
    controller.processInput("CraftingSystem", craftingInput);

    // Craft a sword (recipe index 0)
    TAInput craftSwordInput = {
        "crafting_action",
        { { "action", std::string("craft") }, { "recipe_index", 0 } }
    };
    controller.processInput("CraftingSystem", craftSwordInput);

    // Example: Skill progression
    std::cout << "\n=== SKILL PROGRESSION EXAMPLE ===\n"
              << std::endl;

    // Initialize progression system
    controller.processInput("ProgressionSystem", {});

    // Example: Complete part of the main quest
    std::cout << "\n=== QUEST PROGRESSION EXAMPLE ===\n"
              << std::endl;

    // Initialize quest system with main quest
    controller.processInput("QuestSystem", {});

    // Find the repair walls subquest
    QuestNode* mainQuest = dynamic_cast<QuestNode*>(controller.systemRoots["QuestSystem"]);
    QuestNode* repairWalls = nullptr;

    for (TANode* child : mainQuest->childNodes) {
        if (child->nodeName == "RepairWalls") {
            repairWalls = dynamic_cast<QuestNode*>(child);
            break;
        }
    }

    // Access and complete the repair walls subquest
    if (repairWalls) {
        // Make this the current quest node
        controller.currentNodes["QuestSystem"] = repairWalls;
        repairWalls->onEnter(&controller.gameContext);

        // Complete the repair walls quest
        TAInput completeQuestInput = { "action", { { "name", std::string("repair_complete") } } };
        controller.processInput("QuestSystem", completeQuestInput);
    }

    // Track quest progress
    std::cout << "\nQuest journal:" << std::endl;
    for (const auto& [quest, status] : controller.gameContext.questJournal) {
        std::cout << "- " << quest << ": " << status << std::endl;
    }

    // Example: Time passage
    std::cout << "\n=== TIME SYSTEM EXAMPLE ===\n"
              << std::endl;

    // Initialize time system
    controller.processInput("TimeSystem", {});

    // Wait 5 hours
    TimeNode* timeSystem = dynamic_cast<TimeNode*>(controller.currentNodes["TimeSystem"]);
    if (timeSystem) {
        for (int i = 0; i < 5; i++) {
            timeSystem->advanceHour(&controller.gameContext);
        }
    }

    // Initialize persistent IDs before saving
    controller.initializePersistentIDs();

    // Save the game state
    controller.saveState("game_save.json");
    std::cout << "\nGame state saved to game_save.json" << std::endl;

    // Load the state from the saved file
    std::cout << "\n=== LOADING SAVED GAME ===\n"
              << std::endl;

    // Check if file exists first
    std::ifstream checkFile("game_save.json");
    if (!checkFile.good()) {
        std::cout << "Save file doesn't exist or can't be opened." << std::endl;
        return 0;
    }
    checkFile.close();

    try {
        bool load_result = controller.loadState("game_save.json");
        if (load_result) {
            std::cout << "Game state loaded successfully!" << std::endl;

            // Display the loaded state information
            std::cout << "\nLoaded game information:" << std::endl;

            // Display current time
            TimeNode* loadedTime = dynamic_cast<TimeNode*>(controller.currentNodes["TimeSystem"]);
            if (loadedTime) {
                std::cout << "Time: Day " << loadedTime->day << ", " << loadedTime->hour
                          << ":00, " << loadedTime->timeOfDay << " ("
                          << loadedTime->season << ")" << std::endl;
            }

            // Display world state
            std::cout << "\nWorld state:" << std::endl;
            std::cout << "Days passed: " << controller.gameContext.worldState.daysPassed
                      << std::endl;
            std::cout << "Current season: "
                      << controller.gameContext.worldState.currentSeason << std::endl;

            // Display quest journal
            std::cout << "\nQuest journal:" << std::endl;
            for (const auto& [quest, status] : controller.gameContext.questJournal) {
                std::cout << "- " << quest << ": " << status << std::endl;
            }

            // Display player stats
            std::cout << "\nPlayer stats:" << std::endl;
            std::cout << "Strength: " << controller.gameContext.playerStats.strength
                      << std::endl;
            std::cout << "Dexterity: " << controller.gameContext.playerStats.dexterity
                      << std::endl;
            std::cout << "Constitution: "
                      << controller.gameContext.playerStats.constitution << std::endl;
            std::cout << "Intelligence: "
                      << controller.gameContext.playerStats.intelligence << std::endl;
            std::cout << "Wisdom: " << controller.gameContext.playerStats.wisdom
                      << std::endl;
            std::cout << "Charisma: " << controller.gameContext.playerStats.charisma
                      << std::endl;

            // Display skills
            std::cout << "\nSkills:" << std::endl;
            for (const auto& [skill, level] :
                controller.gameContext.playerStats.skills) {
                std::cout << "- " << skill << ": " << level << std::endl;
            }

            // Display known facts
            std::cout << "\nKnown facts:" << std::endl;
            for (const auto& fact : controller.gameContext.playerStats.knownFacts) {
                std::cout << "- " << fact << std::endl;
            }

            // Display inventory
            std::cout << "\nInventory:" << std::endl;
            for (const auto& item : controller.gameContext.playerInventory.items) {
                std::cout << "- " << item.name << " (" << item.quantity << ")"
                          << std::endl;
            }
        } else {
            std::cout << "Failed to load game state." << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "Exception during load: " << e.what() << std::endl;
    } catch (...) {
        std::cout << "Unknown error during loading process." << std::endl;
    }

    // Add this pause before exiting
    std::cout << "\nPress Enter to exit...";
    std::cin.get();

    return 0;
}