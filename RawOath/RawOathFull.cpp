#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <iostream>
#include <variant>
#include <fstream>
#include <algorithm>
#include <set>
#include <queue>
#include <random>
#include <ctime>
#include <sstream>

// Forward declarations
class TANode;
class TAController;

// A unique identifier for nodes (replaces FGuid)
struct NodeID {
    unsigned int data1;
    unsigned int data2;
    unsigned int data3;
    unsigned int data4;
    
    bool operator==(const NodeID& other) const {
        return data1 == other.data1 && 
               data2 == other.data2 && 
               data3 == other.data3 && 
               data4 == other.data4;
    }
    
    // For map usage
    bool operator<(const NodeID& other) const {
        if (data1 != other.data1) return data1 < other.data1;
        if (data2 != other.data2) return data2 < other.data2;
        if (data3 != other.data3) return data3 < other.data3;
        return data4 < other.data4;
    }
    
    static NodeID Generate() {
        // Simple implementation - would use a proper UUID library in production
        static unsigned int counter = 0;
        return {++counter, 0, 0, 0};
    }
    
    std::string toString() const {
        std::stringstream ss;
        ss << data1 << "-" << data2 << "-" << data3 << "-" << data4;
        return ss.str();
    }
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
    
    CharacterStats() {
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
    
    bool hasSkill(const std::string& skill, int minLevel) const {
        auto it = skills.find(skill);
        return it != skills.end() && it->second >= minLevel;
    }
    
    bool hasFactionReputation(const std::string& faction, int minRep) const {
        auto it = factionReputation.find(faction);
        return it != factionReputation.end() && it->second >= minRep;
    }
    
    bool hasKnowledge(const std::string& fact) const {
        return knownFacts.find(fact) != knownFacts.end();
    }
    
    bool hasAbility(const std::string& ability) const {
        return unlockedAbilities.find(ability) != unlockedAbilities.end();
    }
    
    void learnFact(const std::string& fact) {
        knownFacts.insert(fact);
    }
    
    void unlockAbility(const std::string& ability) {
        unlockedAbilities.insert(ability);
    }
    
    void improveSkill(const std::string& skill, int amount) {
        skills[skill] += amount;
    }
    
    void changeFactionRep(const std::string& faction, int amount) {
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
    
    WorldState() {
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
    
    bool hasFlag(const std::string& flag) const {
        auto it = worldFlags.find(flag);
        return it != worldFlags.end() && it->second;
    }
    
    std::string getLocationState(const std::string& location) const {
        auto it = locationStates.find(location);
        return (it != locationStates.end()) ? it->second : "unknown";
    }
    
    std::string getFactionState(const std::string& faction) const {
        auto it = factionStates.find(faction);
        return (it != factionStates.end()) ? it->second : "unknown";
    }
    
    void setLocationState(const std::string& location, const std::string& state) {
        locationStates[location] = state;
    }
    
    void setFactionState(const std::string& faction, const std::string& state) {
        factionStates[faction] = state;
    }
    
    void setWorldFlag(const std::string& flag, bool value) {
        worldFlags[flag] = value;
    }
    
    void advanceDay() {
        daysPassed++;
        
        // Update season every 90 days
        if (daysPassed % 90 == 0) {
            if (currentSeason == "spring") currentSeason = "summer";
            else if (currentSeason == "summer") currentSeason = "autumn";
            else if (currentSeason == "autumn") currentSeason = "winter";
            else if (currentSeason == "winter") currentSeason = "spring";
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
    std::map<std::string, std::variant<int, float, std::string, bool>> properties;
    
    Item(const std::string& itemId, const std::string& itemName, 
         const std::string& itemType, int itemValue = 1, int itemQty = 1) : 
        id(itemId), name(itemName), type(itemType), value(itemValue), quantity(itemQty) {}
};

class Inventory {
public:
    std::vector<Item> items;
    
    bool hasItem(const std::string& itemId, int quantity = 1) const {
        for (const auto& item : items) {
            if (item.id == itemId && item.quantity >= quantity) {
                return true;
            }
        }
        return false;
    }
    
    bool addItem(const Item& item) {
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
    
    bool removeItem(const std::string& itemId, int quantity = 1) {
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
    
    TANode(const std::string& name) : 
        nodeID(NodeID::Generate()),
        nodeName(name),
        isAcceptingState(false) {}
    
    virtual ~TANode() = default;
    
    // Evaluation function to process inputs
    virtual bool evaluateTransition(const TAInput& input, TANode*& outNextNode) {
        for (const auto& rule : transitionRules) {
            if (rule.condition(input)) {
                outNextNode = rule.targetNode;
                return true;
            }
        }
        return false;
    }
    
    // Actions to perform when entering/exiting this node
    virtual void onEnter(GameContext* context) {
        std::cout << "Entered node: " << nodeName << std::endl;
    }
    
    virtual void onExit(GameContext* context) {
        std::cout << "Exited node: " << nodeName << std::endl;
    }
    
    // Add a transition rule
    void addTransition(const std::function<bool(const TAInput&)>& condition, 
                       TANode* target, 
                       const std::string& description = "") {
        transitionRules.push_back({condition, target, description});
    }
    
    // Add a child node
    void addChild(TANode* child) {
        childNodes.push_back(child);
    }
    
    // Get available transitions for the current state
    virtual std::vector<TAAction> getAvailableActions() {
        std::vector<TAAction> actions;
        for (size_t i = 0; i < transitionRules.size(); i++) {
            const auto& rule = transitionRules[i];
            actions.push_back({
                "transition_" + std::to_string(i),
                rule.description,
                [this, i]() -> TAInput {
                    return {"transition", {{"index", static_cast<int>(i)}}};
                }
            });
        }
        return actions;
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
    
    // Process an input and potentially transition to a new state
    bool processInput(const std::string& systemName, const TAInput& input) {
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
                nextNode->onEnter(&gameContext);
                return true;
            }
        }
        return false;
    }
    
    // Get available actions from current state
    std::vector<TAAction> getAvailableActions(const std::string& systemName) {
        if (currentNodes.find(systemName) == currentNodes.end()) {
            return {};
        }
        
        return currentNodes[systemName]->getAvailableActions();
    }
    
    // Check if a particular state is reachable from current state
    bool isStateReachable(const NodeID& targetNodeID) {
        for (const auto& [name, rootNode] : systemRoots) {
            if (isNodeReachableFromNode(rootNode, targetNodeID)) {
                return true;
            }
        }
        return false;
    }
    
    // Create and register a new node
    template<typename T = TANode, typename... Args>
    T* createNode(const std::string& name, Args&&... args) {
        auto node = std::make_unique<T>(name, std::forward<Args>(args)...);
        T* nodePtr = node.get();
        ownedNodes.push_back(std::move(node));
        return nodePtr;
    }

    // Set a system root
    void setSystemRoot(const std::string& systemName, TANode* rootNode) {
        systemRoots[systemName] = rootNode;
    }
    
    // Save state to file
    bool saveState(const std::string& filename) {
        std::ofstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }
        
        // Write number of systems
        size_t numSystems = systemRoots.size();
        file.write(reinterpret_cast<const char*>(&numSystems), sizeof(numSystems));
        
        // Write each system
        for (const auto& [name, root] : systemRoots) {
            // Write system name
            size_t nameLength = name.length();
            file.write(reinterpret_cast<const char*>(&nameLength), sizeof(nameLength));
            file.write(name.c_str(), nameLength);
            
            // Write if there's a current node
            bool hasCurrentNode = currentNodes.find(name) != currentNodes.end();
            file.write(reinterpret_cast<const char*>(&hasCurrentNode), sizeof(hasCurrentNode));
            
            if (hasCurrentNode) {
                // Write node ID
                file.write(reinterpret_cast<const char*>(&currentNodes[name]->nodeID), sizeof(NodeID));
                
                // Also write node name for better lookup capability
                size_t nodeNameLength = currentNodes[name]->nodeName.length();
                file.write(reinterpret_cast<const char*>(&nodeNameLength), sizeof(nodeNameLength));
                file.write(currentNodes[name]->nodeName.c_str(), nodeNameLength);
            }
        }
        
        // Save game context (simplified)
        file.write(reinterpret_cast<const char*>(&gameContext.worldState.daysPassed), sizeof(int));
        
        size_t knownFactsSize = gameContext.playerStats.knownFacts.size();
        file.write(reinterpret_cast<const char*>(&knownFactsSize), sizeof(knownFactsSize));
        
        for (const auto& fact : gameContext.playerStats.knownFacts) {
            size_t factLength = fact.length();
            file.write(reinterpret_cast<const char*>(&factLength), sizeof(factLength));
            file.write(fact.c_str(), factLength);
        }
        
        return true;
    }
    
    
    // Load state from file
    bool loadState(const std::string& filename) {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }
        
        // Clear current state
        currentNodes.clear();
        
        // Read number of systems
        size_t numSystems;
        file.read(reinterpret_cast<char*>(&numSystems), sizeof(numSystems));
        
        // Read each system
        for (size_t i = 0; i < numSystems; i++) {
            // Read system name
            size_t nameLength;
            file.read(reinterpret_cast<char*>(&nameLength), sizeof(nameLength));
            std::string name(nameLength, ' ');
            file.read(&name[0], nameLength);
            
            // Check if this system exists
            if (systemRoots.find(name) == systemRoots.end()) {
                std::cerr << "System not found during load: " << name << std::endl;
                continue;
            }
            
            // Read if there's a current node
            bool hasCurrentNode;
            file.read(reinterpret_cast<char*>(&hasCurrentNode), sizeof(hasCurrentNode));
            
            if (hasCurrentNode) {
                // Read node ID
                NodeID nodeID;
                file.read(reinterpret_cast<char*>(&nodeID), sizeof(NodeID));
                
                // Read node name
                size_t nodeNameLength;
                file.read(reinterpret_cast<char*>(&nodeNameLength), sizeof(nodeNameLength));
                std::string nodeName(nodeNameLength, ' ');
                file.read(&nodeName[0], nodeNameLength);
                
                // First try to find by ID (faster)
                TANode* node = findNodeById(systemRoots[name], nodeID);
                
                // If not found by ID, try by name as fallback
                if (!node) {
                    std::cout << "Node ID not matched for system " << name << ", trying to find by name..." << std::endl;
                    node = findNodeByName(systemRoots[name], nodeName);
                }
                
                if (node) {
                    currentNodes[name] = node;
                    std::cout << "Successfully restored node " << nodeName << " for system " << name << std::endl;
                } else {
                    std::cerr << "Node not found during load for system: " << name << std::endl;
                    currentNodes[name] = systemRoots[name]; // Default to root
                    std::cout << "Falling back to system root: " << systemRoots[name]->nodeName << std::endl;
                }
            }
        }
        
        // Load game context (simplified)
        file.read(reinterpret_cast<char*>(&gameContext.worldState.daysPassed), sizeof(int));
        
        size_t knownFactsSize;
        file.read(reinterpret_cast<char*>(&knownFactsSize), sizeof(knownFactsSize));
        
        gameContext.playerStats.knownFacts.clear();
        for (size_t i = 0; i < knownFactsSize; i++) {
            size_t factLength;
            file.read(reinterpret_cast<char*>(&factLength), sizeof(factLength));
            std::string fact(factLength, ' ');
            file.read(&fact[0], factLength);
            gameContext.playerStats.knownFacts.insert(fact);
        }
        
        return true;
    }

private:
    // Helper function to find a node by ID
    TANode* findNodeById(TANode* startNode, const NodeID& id) {
        if (startNode->nodeID == id) {
            return startNode;
        }
        
        for (TANode* child : startNode->childNodes) {
            TANode* result = findNodeById(child, id);
            if (result) {
                return result;
            }
        }
        
        return nullptr;
    }

    TANode* findNodeByName(TANode* startNode, const std::string& name) {
        if (startNode->nodeName == name) {
            return startNode;
        }
        
        for (TANode* child : startNode->childNodes) {
            TANode* result = findNodeByName(child, name);
            if (result) {
                return result;
            }
        }
        
        return nullptr;
    }
    
    // Helper to check if a node is reachable (simplified)
    bool isNodeReachableFromNode(TANode* startNode, const NodeID& targetId) {
        if (startNode->nodeID == targetId) {
            return true;
        }
        
        for (TANode* child : startNode->childNodes) {
            if (isNodeReachableFromNode(child, targetId)) {
                return true;
            }
        }
        
        return false;
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
        
        bool check(const GameContext& context) const {
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
    
    QuestNode(const std::string& name) : TANode(name), questState("Available") {}
    
    // Process player action and return next state
    bool processAction(const std::string& playerAction, TANode*& outNextNode) {
        return evaluateTransition({"action", {{"name", playerAction}}}, outNextNode);
    }
    
    // Check if player can access this quest
    bool canAccess(const GameContext& context) const {
        for (const auto& req : requirements) {
            if (!req.check(context)) {
                return false;
            }
        }
        return true;
    }
    
    // Activate child quests when this node is entered
    void onEnter(GameContext* context) override {
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
    void onExit(GameContext* context) override {
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
                        std::cout << "  " << reward.amount << " experience points" << std::endl;
                    } else if (reward.type == "gold") {
                        std::cout << "  " << reward.amount << " gold coins" << std::endl;
                    } else if (reward.type == "item") {
                        context->playerInventory.addItem(Item(reward.itemId, reward.itemId, "quest_reward", 1, reward.amount));
                        std::cout << "  " << reward.amount << "x " << reward.itemId << std::endl;
                    } else if (reward.type == "faction") {
                        context->playerStats.changeFactionRep(reward.itemId, reward.amount);
                        std::cout << "  " << reward.amount << " reputation with " << reward.itemId << std::endl;
                    } else if (reward.type == "skill") {
                        context->playerStats.improveSkill(reward.itemId, reward.amount);
                        std::cout << "  " << reward.amount << " points in " << reward.itemId << " skill" << std::endl;
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
    std::vector<TAAction> getAvailableActions() override {
        std::vector<TAAction> actions = TANode::getAvailableActions();
        
        // Add quest-specific actions
        actions.push_back({
            "abandon_quest",
            "Abandon this quest",
            []() -> TAInput {
                return {"quest_action", {{"action", std::string("abandon")}}};
            }
        });
        
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
        
        DialogueResponse(const std::string& responseText, 
                         TANode* target,
                         std::function<bool(const GameContext&)> req = [](const GameContext&) { return true; },
                         std::function<void(GameContext*)> eff = [](GameContext*) {}) :
            text(responseText), requirement(req), targetNode(target), effect(eff) {}
    };
    std::vector<DialogueResponse> responses;
    
    // Optional effect to run when this dialogue is shown
    std::function<void(GameContext*)> onShowEffect;
    
    DialogueNode(const std::string& name, const std::string& speaker, const std::string& text) :
        TANode(name), speakerName(speaker), dialogueText(text) {}
    
    void addResponse(const std::string& text, 
                     TANode* target, 
                     std::function<bool(const GameContext&)> requirement = [](const GameContext&) { return true; },
                     std::function<void(GameContext*)> effect = [](GameContext*) {}) {
        responses.push_back(DialogueResponse(text, target, requirement, effect));
    }
    
    void onEnter(GameContext* context) override {
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
    std::vector<TAAction> getAvailableActions() override {
        std::vector<TAAction> actions;
        
        for (size_t i = 0; i < responses.size(); i++) {
            actions.push_back({
                "response_" + std::to_string(i),
                responses[i].text,
                [this, i]() -> TAInput {
                    return {"dialogue_response", {{"index", static_cast<int>(i)}}};
                }
            });
        }
        
        return actions;
    }
    
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override {
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
    
    NPC(const std::string& npcName, const std::string& desc) :
        name(npcName), description(desc), rootDialogue(nullptr), currentDialogue(nullptr) {}
    
    void startDialogue(GameContext* context) {
        if (rootDialogue) {
            currentDialogue = rootDialogue;
            currentDialogue->onEnter(context);
        }
    }
    
    bool processResponse(int responseIndex, GameContext* context) {
        if (!currentDialogue) return false;
        
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
        
        bool check(const GameContext& context) const {
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
        
        void apply(GameContext* context) const {
            if (!context) return;
            
            if (type == "stat") {
                if (target == "strength") context->playerStats.strength += value;
                else if (target == "dexterity") context->playerStats.dexterity += value;
                else if (target == "constitution") context->playerStats.constitution += value;
                else if (target == "intelligence") context->playerStats.intelligence += value;
                else if (target == "wisdom") context->playerStats.wisdom += value;
                else if (target == "charisma") context->playerStats.charisma += value;
            }
            else if (type == "skill") {
                context->playerStats.improveSkill(target, value);
            }
            else if (type == "ability") {
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
        
        bool canPay(const GameContext& context) const {
            if (type == "item") {
                return context.playerInventory.hasItem(itemId, amount);
            }
            // Other types would be checked here
            return true;
        }
        
        void pay(GameContext* context) const {
            if (!context) return;
            
            if (type == "item") {
                context->playerInventory.removeItem(itemId, amount);
            }
            // Other payment types would be handled here
        }
    };
    std::vector<SkillCost> costs;
    
    SkillNode(const std::string& name, const std::string& skill, int initialLevel = 0, int max = 5) :
        TANode(name), skillName(skill), description(""), level(initialLevel), maxLevel(max) {}
    
    bool canLearn(const GameContext& context) const {
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
    
    void learnSkill(GameContext* context) {
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
        
        std::cout << "Learned " << skillName << " (Level " << level << "/" << maxLevel << ")" << std::endl;
    }
    
    void onEnter(GameContext* context) override {
        std::cout << "Viewing skill: " << skillName << " (Level " << level << "/" << maxLevel << ")" << std::endl;
        std::cout << description << std::endl;
        
        if (context && canLearn(*context)) {
            std::cout << "This skill can be learned/improved." << std::endl;
        }
    }
    
    std::vector<TAAction> getAvailableActions() override {
        std::vector<TAAction> actions = TANode::getAvailableActions();
        
        // Add learn skill action if not at max level
        if (level < maxLevel) {
            actions.push_back({
                "learn_skill",
                "Learn/Improve " + skillName,
                [this]() -> TAInput {
                    return {"skill_action", {{"action", std::string("learn")}, {"skill", skillName}}};
                }
            });
        }
        
        return actions;
    }
    
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override {
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
    
    ClassNode(const std::string& name, const std::string& classType) :
        TANode(name), className(classType) {}
    
    void onEnter(GameContext* context) override {
        std::cout << "Selected class: " << className << std::endl;
        std::cout << description << std::endl;
        
        if (context) {
            // Apply stat bonuses
            for (const auto& [stat, bonus] : statBonuses) {
                if (stat == "strength") context->playerStats.strength += bonus;
                else if (stat == "dexterity") context->playerStats.dexterity += bonus;
                else if (stat == "constitution") context->playerStats.constitution += bonus;
                else if (stat == "intelligence") context->playerStats.intelligence += bonus;
                else if (stat == "wisdom") context->playerStats.wisdom += bonus;
                else if (stat == "charisma") context->playerStats.charisma += bonus;
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
        std::map<std::string, std::variant<int, float, std::string, bool>> properties;
    };
    Result result;
    
    // Skill requirements
    std::map<std::string, int> skillRequirements;
    
    Recipe(const std::string& id, const std::string& recipeName) :
        recipeId(id), name(recipeName), discovered(false) {}
    
    bool canCraft(const GameContext& context) const {
        // Check skill requirements
        for (const auto& [skill, level] : skillRequirements) {
            if (!context.playerStats.hasSkill(skill, level)) {
                return false;
            }
        }
        
        // Check ingredients
        for (const auto& ingredient : ingredients) {
            if (!context.playerInventory.hasItem(ingredient.itemId, ingredient.quantity)) {
                return false;
            }
        }
        
        return true;
    }
    
    bool craft(GameContext* context) {
        if (!context || !canCraft(*context)) {
            return false;
        }
        
        // Consume ingredients
        for (const auto& ingredient : ingredients) {
            context->playerInventory.removeItem(ingredient.itemId, ingredient.quantity);
        }
        
        // Create result item
        Item craftedItem(result.itemId, result.name, result.type, 1, result.quantity);
        craftedItem.properties = result.properties;
        
        // Add to inventory
        context->playerInventory.addItem(craftedItem);
        
        // Mark as discovered
        discovered = true;
        
        std::cout << "Crafted " << result.quantity << "x " << result.name << std::endl;
        return true;
    }
};

// Crafting station node
class CraftingNode : public TANode {
public:
    std::string stationType;
    std::string description;
    std::vector<Recipe> availableRecipes;
    
    CraftingNode(const std::string& name, const std::string& type) :
        TANode(name), stationType(type) {}
    
    void onEnter(GameContext* context) override {
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
    
    std::vector<TAAction> getAvailableActions() override {
        std::vector<TAAction> actions = TANode::getAvailableActions();
        
        // Add crafting actions for discovered recipes
        for (size_t i = 0; i < availableRecipes.size(); i++) {
            if (availableRecipes[i].discovered) {
                actions.push_back({
                    "craft_" + std::to_string(i),
                    "Craft " + availableRecipes[i].name,
                    [this, i]() -> TAInput {
                        return {"crafting_action", {{"action", std::string("craft")}, {"recipe_index", static_cast<int>(i)}}};
                    }
                });
            }
        }
        
        // Add exit action
        actions.push_back({
            "exit_crafting",
            "Exit crafting station",
            [this]() -> TAInput {
                return {"crafting_action", {{"action", std::string("exit")}}};
            }
        });
        
        return actions;
    }
    
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override {
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
    
    void addRecipe(const Recipe& recipe) {
        availableRecipes.push_back(recipe);
    }
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
        
        bool check(const GameContext& context) const {
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
    
    LocationNode(const std::string& name, const std::string& location, const std::string& initialState = "normal") :
        TANode(name), locationName(location), currentState(initialState) {}
    
    bool canAccess(const GameContext& context) const {
        for (const auto& condition : accessConditions) {
            if (!condition.check(context)) {
                return false;
            }
        }
        return true;
    }
    
    void onEnter(GameContext* context) override {
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
    
    std::vector<TAAction> getAvailableActions() override {
        std::vector<TAAction> actions = TANode::getAvailableActions();
        
        // Add NPC interaction actions
        for (size_t i = 0; i < npcs.size(); i++) {
            actions.push_back({
                "talk_to_npc_" + std::to_string(i),
                "Talk to " + npcs[i]->name,
                [this, i]() -> TAInput {
                    return {"location_action", {{"action", std::string("talk")}, {"npc_index", static_cast<int>(i)}}};
                }
            });
        }
        
        // Add activity actions
        for (size_t i = 0; i < activities.size(); i++) {
            actions.push_back({
                "do_activity_" + std::to_string(i),
                "Do " + activities[i]->nodeName,
                [this, i]() -> TAInput {
                    return {"location_action", {{"action", std::string("activity")}, {"activity_index", static_cast<int>(i)}}};
                }
            });
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
    
    RegionNode(const std::string& name, const std::string& region) :
        TANode(name), regionName(region) {}
    
    void onEnter(GameContext* context) override {
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
    
    std::vector<TAAction> getAvailableActions() override {
        std::vector<TAAction> actions = TANode::getAvailableActions();
        
        // Add location travel actions
        for (size_t i = 0; i < locations.size(); i++) {
            actions.push_back({
                "travel_to_location_" + std::to_string(i),
                "Travel to " + locations[i]->locationName,
                [this, i]() -> TAInput {
                    return {"region_action", {{"action", std::string("travel_location")}, {"location_index", static_cast<int>(i)}}};
                }
            });
        }
        
        // Add region travel actions
        for (size_t i = 0; i < connectedRegions.size(); i++) {
            actions.push_back({
                "travel_to_region_" + std::to_string(i),
                "Travel to " + connectedRegions[i]->regionName,
                [this, i]() -> TAInput {
                    return {"region_action", {{"action", std::string("travel_region")}, {"region_index", static_cast<int>(i)}}};
                }
            });
        }
        
        return actions;
    }
    
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override {
        if (input.type == "region_action") {
            std::string action = std::get<std::string>(input.parameters.at("action"));
            
            if (action == "travel_location") {
                int locationIndex = std::get<int>(input.parameters.at("location_index"));
                if (locationIndex >= 0 && locationIndex < static_cast<int>(locations.size())) {
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
    
    TimeNode(const std::string& name) :
        TANode(name), day(1), hour(6), season("spring"), timeOfDay("morning") {}
    
    void advanceHour(GameContext* context) {
        hour++;
        
        if (hour >= 24) {
            hour = 0;
            day++;
            
            // Update season every 90 days
            if (day % 90 == 0) {
                if (season == "spring") season = "summer";
                else if (season == "summer") season = "autumn";
                else if (season == "autumn") season = "winter";
                else if (season == "winter") season = "spring";
            }
            
            if (context) {
                context->worldState.advanceDay();
            }
        }
        
        // Update time of day
        if (hour >= 5 && hour < 12) timeOfDay = "morning";
        else if (hour >= 12 && hour < 17) timeOfDay = "afternoon";
        else if (hour >= 17 && hour < 21) timeOfDay = "evening";
        else timeOfDay = "night";
        
        std::cout << "Time: Day " << day << ", " << hour << ":00, " << timeOfDay << " (" << season << ")" << std::endl;
    }
    
    std::vector<TAAction> getAvailableActions() override {
        std::vector<TAAction> actions = TANode::getAvailableActions();
        
        actions.push_back({
            "wait_1_hour",
            "Wait 1 hour",
            [this]() -> TAInput {
                return {"time_action", {{"action", std::string("wait")}, {"hours", 1}}};
            }
        });
        
        actions.push_back({
            "wait_until_morning",
            "Wait until morning",
            [this]() -> TAInput {
                return {"time_action", {{"action", std::string("wait_until")}, {"time", std::string("morning")}}};
            }
        });
        
        return actions;
    }
    
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override {
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
                    if (hour < 5) hoursToWait = 5 - hour;
                    else hoursToWait = 24 - (hour - 5);
                }
                // Add other time of day calculations
                
                std::cout << "Waiting until " << targetTime << " (" << hoursToWait << " hours)..." << std::endl;
                
                // Stay in same node after waiting
                outNextNode = this;
                return true;
            }
        }
        
        return TANode::evaluateTransition(input, outNextNode);
    }
};

// Main function demonstrating all systems
int main() {
    std::cout << "___ Starting Raw Oath ___" << std::endl;

    // Create the automaton controller
    TAController controller;
    
    //----------------------------------------
    // QUEST SYSTEM SETUP
    //----------------------------------------
    
    std::cout << "___ QUEST SYSTEM SETUP ___" << std::endl;

    std::cout << "Create mainQuest QuestNode" << std::endl;
    QuestNode* mainQuest = dynamic_cast<QuestNode*>(controller.createNode<QuestNode>("MainQuest"));
    mainQuest->questTitle = "Defend the Village";
    mainQuest->questDescription = "The village is under threat. Prepare its defenses!";
    
    // Create sub-quests
    std::cout << "Create repairWalls QuestNode" << std::endl;
    QuestNode* repairWalls = dynamic_cast<QuestNode*>(controller.createNode<QuestNode>("RepairWalls"));
    repairWalls->questTitle = "Repair the Walls";
    repairWalls->questDescription = "The village walls are in disrepair. Fix them!";
    repairWalls->requirements.push_back({"skill", "crafting", 1});
    
    std::cout << "Create trainMilitia QuestNode" << std::endl;
    QuestNode* trainMilitia = dynamic_cast<QuestNode*>(controller.createNode<QuestNode>("TrainMilitia"));
    trainMilitia->questTitle = "Train the Militia";
    trainMilitia->questDescription = "The villagers need combat training.";
    trainMilitia->requirements.push_back({"skill", "combat", 2});
    
    std::cout << "Create gatherSupplies QuestNode" << std::endl;
    QuestNode* gatherSupplies = dynamic_cast<QuestNode*>(controller.createNode<QuestNode>("GatherSupplies"));
    gatherSupplies->questTitle = "Gather Supplies";
    gatherSupplies->questDescription = "The village needs food and resources.";
    gatherSupplies->requirements.push_back({"skill", "survival", 1});
    
    // Set up the hierarchy
    std::cout << "Set up the mainQuest hierarchy" << std::endl;
    std::cout << "mainQuest->repairWalls" << std::endl;
    std::cout << "mainQuest->trainMilitia" << std::endl;
    std::cout << "mainQuest->gatherSupplies" << std::endl;
    mainQuest->addChild(repairWalls);
    mainQuest->addChild(trainMilitia);
    mainQuest->addChild(gatherSupplies);
    
    // Add transitions
    std::cout << "Add transitions: repairWalls->repair_complete" << std::endl;
    repairWalls->addTransition(
        [](const TAInput& input) { 
            return input.type == "action" && 
                    std::get<std::string>(input.parameters.at("name")) == "repair_complete"; 
        }, 
        mainQuest,
        "Complete wall repairs"
    );
    
    std::cout << "Add transitions: trainMilitia->training_complete" << std::endl;
    trainMilitia->addTransition(
        [](const TAInput& input) { 
            return input.type == "action" && 
                    std::get<std::string>(input.parameters.at("name")) == "training_complete"; 
        }, 
        mainQuest,
        "Complete militia training"
    );
    
    std::cout << "Add transitions: gatherSupplies->supplies_gathered" << std::endl;
    gatherSupplies->addTransition(
        [](const TAInput& input) { 
            return input.type == "action" && 
                    std::get<std::string>(input.parameters.at("name")) == "supplies_gathered"; 
        }, 
        mainQuest,
        "Finish gathering supplies"
    );
    
    // Add rewards
    std::cout << "Add repairWalls rewards: experience, gold, faction" << std::endl;
    repairWalls->rewards.push_back({"experience", 100, ""});
    repairWalls->rewards.push_back({"gold", 50, ""});
    repairWalls->rewards.push_back({"faction", 10, "villagers"});
    
    std::cout << "Add trainMilitia rewards: experience, skill" << std::endl;
    trainMilitia->rewards.push_back({"experience", 150, ""});
    trainMilitia->rewards.push_back({"skill", 1, "combat"});
    
    std::cout << "Add gatherSupplies rewards: experience, item" << std::endl;
    gatherSupplies->rewards.push_back({"experience", 100, ""});
    gatherSupplies->rewards.push_back({"item", 1, "rare_herb"});
    
    std::cout << "Add mainQuest rewards: experience, gold, faction, item" << std::endl;
    mainQuest->rewards.push_back({"experience", 500, ""});
    mainQuest->rewards.push_back({"gold", 200, ""});
    mainQuest->rewards.push_back({"faction", 25, "villagers"});
    mainQuest->rewards.push_back({"item", 1, "defenders_shield"});
    
    // Register the quest system
    std::cout << "Register the quest system" << std::endl;
    controller.setSystemRoot("QuestSystem", mainQuest);
    
    //----------------------------------------
    // DIALOGUE SYSTEM SETUP
    //----------------------------------------
    
    std::cout << "___ DIALOGUE SYSTEM SETUP ___" << std::endl;

    // Create a village elder NPC
    std::cout << "Create a village elder NPC 'Elder Marius'" << std::endl;
    NPC elderNPC("Elder Marius", "The wise leader of the village");
    
    // Create dialogue nodes
    
    std::cout << "Create dialogue nodes for 'Elder Marius'" << std::endl;
    DialogueNode* greeting = dynamic_cast<DialogueNode*>(controller.createNode<DialogueNode>(
        "ElderGreeting", "Elder Marius", "Greetings, traveler. Our village faces difficult times."
    ));
    
    DialogueNode* askThreat = dynamic_cast<DialogueNode*>(controller.createNode<DialogueNode>(
        "AskThreat", "Elder Marius", "Bandits have been raiding nearby settlements. I fear we're next."
    ));
    
    DialogueNode* askHelp = dynamic_cast<DialogueNode*>(controller.createNode<DialogueNode>(
        "AskHelp", "Elder Marius", "We need someone skilled to help prepare our defenses."
    ));
    
    DialogueNode* acceptQuest = dynamic_cast<DialogueNode*>(controller.createNode<DialogueNode>(
        "AcceptQuest", "Elder Marius", "Thank you! This means a lot to our community. We need the walls repaired, the militia trained, and supplies gathered."
    ));
    
    DialogueNode* rejectQuest = dynamic_cast<DialogueNode*>(controller.createNode<DialogueNode>(
        "RejectQuest", "Elder Marius", "I understand. Perhaps you'll reconsider when you have time."
    ));
    
    DialogueNode* farewell = dynamic_cast<DialogueNode*>(controller.createNode<DialogueNode>(
        "Farewell", "Elder Marius", "Safe travels, friend. Return if you need anything."
    ));
    
    // Add responses
    std::cout << "Add responses to 'Elder Marius'" << std::endl;
    greeting->addResponse("What threat does the village face?", askThreat);
    greeting->addResponse("Is there something I can help with?", askHelp);
    greeting->addResponse("I need to go. Farewell.", farewell);
    
    askThreat->addResponse("How can I help against these bandits?", askHelp);
    askThreat->addResponse("I'll be on my way.", farewell);
    
    askHelp->addResponse("I'll help defend the village.", acceptQuest, 
                    [](const GameContext& ctx) { return true; }, // No requirements
                    [mainQuest](GameContext* ctx) { 
                        // Give the main quest
                        if (ctx) {
                            ctx->questJournal[mainQuest->nodeName] = "Active";
                            ctx->playerStats.learnFact("village_under_threat");
                            ctx->playerStats.changeFactionRep("villagers", 5);
                        }
                        std::cout << "Quest accepted: Defend the Village" << std::endl;
                    });

                    askHelp->addResponse("I'll help defend the village.", acceptQuest, 
                        [](const GameContext& ctx) { return true; }, // No requirements
                        [mainQuest](GameContext* ctx) { 
                            // Give the main quest
                            if (ctx) {
                                ctx->questJournal[mainQuest->nodeName] = "Active";
                                ctx->playerStats.learnFact("village_under_threat");
                                ctx->playerStats.changeFactionRep("villagers", 5);
                            }
                            std::cout << "Quest accepted: Defend the Village" << std::endl;
                        });
    
    askHelp->addResponse("I'm not interested in helping.", rejectQuest);
    askHelp->addResponse("I need to think about it.", farewell);
    
    acceptQuest->addResponse("I'll get started right away.", farewell);
    rejectQuest->addResponse("Goodbye.", farewell);
    
    // Set up NPC dialogue tree
    std::cout << "Set up NPC dialogue tree for 'Elder Marius'" << std::endl;
    elderNPC.rootDialogue = greeting;
    elderNPC.dialogueNodes["greeting"] = greeting;
    elderNPC.dialogueNodes["askThreat"] = askThreat;
    elderNPC.dialogueNodes["askHelp"] = askHelp;
    elderNPC.dialogueNodes["acceptQuest"] = acceptQuest;
    elderNPC.dialogueNodes["rejectQuest"] = rejectQuest;
    elderNPC.dialogueNodes["farewell"] = farewell;
    
    // Create a dialogue controller node
    std::cout << "Create a dialogue controller node" << std::endl;
    TANode* dialogueControllerNode = controller.createNode("DialogueController");
    controller.setSystemRoot("DialogueSystem", dialogueControllerNode);
    
    //----------------------------------------
    // CHARACTER PROGRESSION SYSTEM SETUP
    //----------------------------------------
    
    std::cout << "___ CHARACTER PROGRESSION SYSTEM SETUP ___" << std::endl;

    // Create skill tree root
    std::cout << "Create skill tree root" << std::endl;
    TANode* skillTreeRoot = controller.createNode("SkillTreeRoot");
    
    // Create combat branch
    std::cout << "Create combat branch: combatBasics" << std::endl;
    SkillNode* combatBasics = dynamic_cast<SkillNode*>(controller.createNode<SkillNode>("CombatBasics", "combat"));
    combatBasics->description = "Basic combat techniques and weapon handling.";
    combatBasics->effects.push_back({"stat", "strength", 1});
    
    std::cout << "Create combat branch: swordsmanship" << std::endl;
    SkillNode* swordsmanship = dynamic_cast<SkillNode*>(controller.createNode<SkillNode>("Swordsmanship", "swordsmanship"));
    swordsmanship->description = "Advanced sword techniques for greater damage and defense.";
    swordsmanship->requirements.push_back({"skill", "combat", 2});
    swordsmanship->effects.push_back({"ability", "power_attack", 0});
    
    std::cout << "Create combat branch: archery" << std::endl;
    SkillNode* archery = dynamic_cast<SkillNode*>(controller.createNode<SkillNode>("Archery", "archery"));
    archery->description = "Precision with bows and other ranged weapons.";
    archery->requirements.push_back({"skill", "combat", 2});
    archery->effects.push_back({"ability", "precise_shot", 0});
    
    // Create survival branch
    std::cout << "Create survival branch: survivalBasics" << std::endl;
    SkillNode* survivalBasics = dynamic_cast<SkillNode*>(controller.createNode<SkillNode>("SurvivalBasics", "survival"));
    survivalBasics->description = "Basic survival skills for harsh environments.";
    survivalBasics->effects.push_back({"stat", "constitution", 1});
    
    std::cout << "Create survival branch: herbalism" << std::endl;
    SkillNode* herbalism = dynamic_cast<SkillNode*>(controller.createNode<SkillNode>("Herbalism", "herbalism"));
    herbalism->description = "Knowledge of medicinal and poisonous plants.";
    herbalism->requirements.push_back({"skill", "survival", 2});
    herbalism->effects.push_back({"ability", "herbal_remedy", 0});
    
    std::cout << "Create survival branch: tracking" << std::endl;
    SkillNode* tracking = dynamic_cast<SkillNode*>(controller.createNode<SkillNode>("Tracking", "tracking"));
    tracking->description = "Follow trails and find creatures in the wilderness.";
    tracking->requirements.push_back({"skill", "survival", 1});
    tracking->effects.push_back({"ability", "track_prey", 0});
    
    // Create crafting branch
    std::cout << "Create crafting branch: craftingBasics" << std::endl;
    SkillNode* craftingBasics = dynamic_cast<SkillNode*>(controller.createNode<SkillNode>("CraftingBasics", "crafting"));
    craftingBasics->description = "Basic crafting and repair techniques.";
    craftingBasics->effects.push_back({"stat", "dexterity", 1});
    
    std::cout << "Create crafting branch: blacksmithing" << std::endl;
    SkillNode* blacksmithing = dynamic_cast<SkillNode*>(controller.createNode<SkillNode>("Blacksmithing", "blacksmithing"));
    blacksmithing->description = "Forge and improve metal weapons and armor.";
    blacksmithing->requirements.push_back({"skill", "crafting", 2});
    blacksmithing->effects.push_back({"ability", "forge_weapon", 0});
    
    std::cout << "Create crafting branch: alchemy" << std::endl;
    SkillNode* alchemy = dynamic_cast<SkillNode*>(controller.createNode<SkillNode>("Alchemy", "alchemy"));
    alchemy->description = "Create potions and elixirs with magical effects.";
    alchemy->requirements.push_back({"skill", "crafting", 1});
    alchemy->requirements.push_back({"skill", "herbalism", 1});
    alchemy->effects.push_back({"ability", "brew_potion", 0});
    
    // Set up skill tree hierarchy
    std::cout << "Set up skill tree hierarchy: skillTreeRoot->combatBasics" << std::endl;
    std::cout << "Set up skill tree hierarchy: skillTreeRoot->survivalBasics" << std::endl;
    std::cout << "Set up skill tree hierarchy: skillTreeRoot->craftingBasics" << std::endl;
    skillTreeRoot->addChild(combatBasics);
    skillTreeRoot->addChild(survivalBasics);
    skillTreeRoot->addChild(craftingBasics);
    
    std::cout << "Set up skill tree hierarchy: combatBasics->swordsmanship" << std::endl;
    std::cout << "Set up skill tree hierarchy: combatBasics->archery" << std::endl;
    combatBasics->addChild(swordsmanship);
    combatBasics->addChild(archery);
    
    std::cout << "Set up skill tree hierarchy: survivalBasics->herbalism" << std::endl;
    std::cout << "Set up skill tree hierarchy: survivalBasics->tracking" << std::endl;
    survivalBasics->addChild(herbalism);
    survivalBasics->addChild(tracking);
    
    std::cout << "Set up skill tree hierarchy: craftingBasics->blacksmithing" << std::endl;
    std::cout << "Set up skill tree hierarchy: craftingBasics->alchemy" << std::endl;
    craftingBasics->addChild(blacksmithing);
    craftingBasics->addChild(alchemy);
    
    // Create character classes
    std::cout << "Create character classes: warriorClass" << std::endl;
    ClassNode* warriorClass = dynamic_cast<ClassNode*>(controller.createNode<ClassNode>("Warrior", "Warrior"));
    warriorClass->description = "Masters of combat, strong and resilient.";
    warriorClass->statBonuses["strength"] = 3;
    warriorClass->statBonuses["constitution"] = 2;
    warriorClass->startingAbilities.insert("weapon_specialization");
    warriorClass->classSkills.push_back(combatBasics);
    warriorClass->classSkills.push_back(swordsmanship);
    
    std::cout << "Create character classes: rangerClass" << std::endl;
    ClassNode* rangerClass = dynamic_cast<ClassNode*>(controller.createNode<ClassNode>("Ranger", "Ranger"));
    rangerClass->description = "Wilderness experts, skilled with bow and blade.";
    rangerClass->statBonuses["dexterity"] = 2;
    rangerClass->statBonuses["wisdom"] = 2;
    rangerClass->startingAbilities.insert("animal_companion");
    rangerClass->classSkills.push_back(archery);
    rangerClass->classSkills.push_back(tracking);
    
    std::cout << "Create character classes: alchemistClass" << std::endl;
    ClassNode* alchemistClass = dynamic_cast<ClassNode*>(controller.createNode<ClassNode>("Alchemist", "Alchemist"));
    alchemistClass->description = "Masters of potions and elixirs.";
    alchemistClass->statBonuses["intelligence"] = 3;
    alchemistClass->statBonuses["dexterity"] = 1;
    alchemistClass->startingAbilities.insert("potion_mastery");
    alchemistClass->classSkills.push_back(herbalism);
    alchemistClass->classSkills.push_back(alchemy);
    
    // Create a class selection node
    std::cout << "Create a class selection node: classSelectionNode->warriorClass" << std::endl;
    std::cout << "Create a class selection node: classSelectionNode->rangerClass" << std::endl;
    std::cout << "Create a class selection node: classSelectionNode->alchemistClass" << std::endl;
    TANode* classSelectionNode = controller.createNode("ClassSelection");
    classSelectionNode->addChild(warriorClass);
    classSelectionNode->addChild(rangerClass);
    classSelectionNode->addChild(alchemistClass);
    
    // Register the character progression system
    std::cout << "Register the character progression system" << std::endl;
    controller.setSystemRoot("ProgressionSystem", skillTreeRoot);
    controller.setSystemRoot("ClassSystem", classSelectionNode);
    
    //----------------------------------------
    // CRAFTING SYSTEM SETUP
    //----------------------------------------
    
    std::cout << "___ CRAFTING SYSTEM SETUP ___" << std::endl;

    // Create crafting stations
    std::cout << "Create crafting stations: blacksmithStation" << std::endl;
    CraftingNode* blacksmithStation = dynamic_cast<CraftingNode*>(controller.createNode<CraftingNode>("BlacksmithStation", "Blacksmith"));
    blacksmithStation->description = "A forge with anvil, hammers, and other metalworking tools.";
    
    std::cout << "Create crafting stations: alchemyStation" << std::endl;
    CraftingNode* alchemyStation = dynamic_cast<CraftingNode*>(controller.createNode<CraftingNode>("AlchemyStation", "Alchemy"));
    alchemyStation->description = "A workbench with alembics, mortars, and various containers for brewing.";
    
    std::cout << "Create crafting stations: cookingStation" << std::endl;
    CraftingNode* cookingStation = dynamic_cast<CraftingNode*>(controller.createNode<CraftingNode>("CookingStation", "Cooking"));
    cookingStation->description = "A firepit with cooking pots and utensils.";
    
    // Create recipes
    std::cout << "Create recipes: swordRecipe" << std::endl;
    Recipe swordRecipe("sword_recipe", "Iron Sword");
    swordRecipe.description = "A standard iron sword, good for combat.";
    swordRecipe.ingredients.push_back({"iron_ingot", 2});
    swordRecipe.ingredients.push_back({"leather_strips", 1});
    swordRecipe.skillRequirements["blacksmithing"] = 1;
    swordRecipe.result = {"iron_sword", "Iron Sword", "weapon", 1, {{"damage", 10}}};
    swordRecipe.discovered = true;
    
    std::cout << "Create recipes: armor_recipe" << std::endl;
    Recipe armorRecipe("armor_recipe", "Leather Armor");
    armorRecipe.description = "Basic protective gear made from leather.";
    armorRecipe.ingredients.push_back({"leather", 5});
    armorRecipe.ingredients.push_back({"metal_studs", 10});
    armorRecipe.skillRequirements["crafting"] = 2;
    armorRecipe.result = {"leather_armor", "Leather Armor", "armor", 1, {{"defense", 5}}};
    armorRecipe.discovered = true;
    
    std::cout << "Create recipes: healthPotionRecipe" << std::endl;
    Recipe healthPotionRecipe("health_potion_recipe", "Minor Healing Potion");
    healthPotionRecipe.description = "A potion that restores a small amount of health.";
    healthPotionRecipe.ingredients.push_back({"red_herb", 2});
    healthPotionRecipe.ingredients.push_back({"water_flask", 1});
    healthPotionRecipe.skillRequirements["alchemy"] = 1;
    healthPotionRecipe.result = {"minor_healing_potion", "Minor Healing Potion", "potion", 1, {{"heal_amount", 25}}};
    healthPotionRecipe.discovered = true;
    
    std::cout << "Create recipes: stewRecipe" << std::endl;
    Recipe stewRecipe("stew_recipe", "Hearty Stew");
    stewRecipe.description = "A filling meal that provides temporary stat bonuses.";
    stewRecipe.ingredients.push_back({"meat", 2});
    stewRecipe.ingredients.push_back({"vegetables", 3});
    stewRecipe.skillRequirements["cooking"] = 1;
    stewRecipe.result = {"hearty_stew", "Hearty Stew", "food", 2, {{"effect_duration", 300}}};
    stewRecipe.discovered = true;
    
    // Add recipes to stations
    std::cout << "Add recipes to stations: blacksmithStation->swordRecipe" << std::endl;
    std::cout << "Add recipes to stations: blacksmithStation->armorRecipe" << std::endl;
    blacksmithStation->addRecipe(swordRecipe);
    blacksmithStation->addRecipe(armorRecipe);
    std::cout << "Add recipes to stations: alchemyStation->healthPotionRecipe" << std::endl;
    alchemyStation->addRecipe(healthPotionRecipe);
    std::cout << "Add recipes to stations: cookingStation->stewRecipe" << std::endl;
    cookingStation->addRecipe(stewRecipe);
    
    // Register the crafting system
    std::cout << "Register the crafting system" << std::endl;
    TANode* craftingRoot = controller.createNode("CraftingRoot");
    craftingRoot->addChild(blacksmithStation);
    craftingRoot->addChild(alchemyStation);
    craftingRoot->addChild(cookingStation);
    controller.setSystemRoot("CraftingSystem", craftingRoot);
    
    //----------------------------------------
    // WORLD PROGRESSION SYSTEM SETUP
    //----------------------------------------
    
    std::cout << "___ WORLD PROGRESSION SYSTEM SETUP ___" << std::endl;

    // Create regions
    std::cout << "Create regions: villageRegion" << std::endl;
    RegionNode* villageRegion = dynamic_cast<RegionNode*>(controller.createNode<RegionNode>("VillageRegion", "Oakvale Village"));
    villageRegion->description = "A peaceful farming village surrounded by wooden palisades.";
    villageRegion->controllingFaction = "villagers";
    
    std::cout << "Create regions: forestRegion" << std::endl;
    RegionNode* forestRegion = dynamic_cast<RegionNode*>(controller.createNode<RegionNode>("ForestRegion", "Green Haven Forest"));
    forestRegion->description = "A dense forest with ancient trees and hidden paths.";
    forestRegion->controllingFaction = "forest guardians";
    
    std::cout << "Create regions: mountainRegion" << std::endl;
    RegionNode* mountainRegion = dynamic_cast<RegionNode*>(controller.createNode<RegionNode>("MountainRegion", "Stone Peak Mountains"));
    mountainRegion->description = "Rugged mountains with treacherous paths and hidden caves.";
    mountainRegion->controllingFaction = "mountainfolk";
    
    // Connect regions
    std::cout << "Create regions: villageRegion->forestRegion" << std::endl;
    std::cout << "Create regions: villageRegion->mountainRegion" << std::endl;
    villageRegion->connectedRegions.push_back(forestRegion);
    villageRegion->connectedRegions.push_back(mountainRegion);

    std::cout << "Create regions: forestRegion->villageRegion" << std::endl;
    std::cout << "Create regions: forestRegion->mountainRegion" << std::endl;
    forestRegion->connectedRegions.push_back(villageRegion);
    forestRegion->connectedRegions.push_back(mountainRegion);

    std::cout << "Create regions: mountainRegion->villageRegion" << std::endl;
    std::cout << "Create regions: mountainRegion->forestRegion" << std::endl;
    mountainRegion->connectedRegions.push_back(villageRegion);
    mountainRegion->connectedRegions.push_back(forestRegion);
    
    // Create locations in village region
    std::cout << "Create locations in village region: VillageCenter" << std::endl;
    LocationNode* villageCenterLocation = dynamic_cast<LocationNode*>(controller.createNode<LocationNode>("VillageCenter", "Village Center"));
    villageCenterLocation->description = "The bustling center of the village with a market and well.";
    villageCenterLocation->stateDescriptions["damaged"] = "The village center shows signs of damage from bandit raids.";
    villageCenterLocation->stateDescriptions["rebuilt"] = "The village center has been rebuilt stronger than before.";
    
    std::cout << "Create locations in village region: villageInnLocation" << std::endl;
    LocationNode* villageInnLocation = dynamic_cast<LocationNode*>(controller.createNode<LocationNode>("VillageInn", "The Sleeping Dragon Inn"));
    villageInnLocation->description = "A cozy inn where travelers find rest and information.";
    
    std::cout << "Create locations in village region: villageForgeLocation" << std::endl;
    LocationNode* villageForgeLocation = dynamic_cast<LocationNode*>(controller.createNode<LocationNode>("VillageForge", "Blacksmith's Forge"));
    villageForgeLocation->description = "The local blacksmith's workshop with a roaring forge.";
    
    // Create locations in forest region
    std::cout << "Create locations in forest region: forestClearingLocation" << std::endl;
    LocationNode* forestClearingLocation = dynamic_cast<LocationNode*>(controller.createNode<LocationNode>("ForestClearing", "Forest Clearing"));
    forestClearingLocation->description = "A peaceful clearing in the heart of the forest.";
    
    std::cout << "Create locations in forest region: ancientGrovesLocation" << std::endl;
    LocationNode* ancientGrovesLocation = dynamic_cast<LocationNode*>(controller.createNode<LocationNode>("AncientGroves", "Ancient Groves"));
    ancientGrovesLocation->description = "An area with trees older than any human memory.";
    ancientGrovesLocation->accessConditions.push_back({"skill", "survival", 2});
    
    // Create locations in mountain region
    std::cout << "Create locations in mountain region: mountainPassLocation" << std::endl;
    LocationNode* mountainPassLocation = dynamic_cast<LocationNode*>(controller.createNode<LocationNode>("MountainPass", "Mountain Pass"));
    mountainPassLocation->description = "A winding path through the mountains.";
    
    std::cout << "Create locations in mountain region: abandonedMineLocation" << std::endl;
    LocationNode* abandonedMineLocation = dynamic_cast<LocationNode*>(controller.createNode<LocationNode>("AbandonedMine", "Abandoned Mine"));
    abandonedMineLocation->description = "An old mine, no longer in use. Rumors say something lurks within.";
    abandonedMineLocation->accessConditions.push_back({"item", "torch", 1});
    
    // Add locations to regions
    std::cout << "Add locations to regions" << std::endl;
    villageRegion->locations.push_back(villageCenterLocation);
    villageRegion->locations.push_back(villageInnLocation);
    villageRegion->locations.push_back(villageForgeLocation);
    forestRegion->locations.push_back(forestClearingLocation);
    forestRegion->locations.push_back(ancientGrovesLocation);
    mountainRegion->locations.push_back(mountainPassLocation);
    mountainRegion->locations.push_back(abandonedMineLocation);
    
    // Add NPCs to locations
    std::cout << "Add NPCs to locations" << std::endl;
    villageCenterLocation->npcs.push_back(&elderNPC);
    
    // Add activities to locations
    std::cout << "Add activities to locations" << std::endl;
    villageCenterLocation->activities.push_back(mainQuest); // Main quest is accessible here
    villageForgeLocation->activities.push_back(blacksmithStation); // Blacksmith crafting
    
    // Create regional events
    std::cout << "Create regional events" << std::endl;
    villageRegion->possibleEvents.push_back({
        "Bandit Raid",
        "A small group of bandits is attacking the village outskirts!",
        [](const GameContext& ctx) { return !ctx.worldState.hasFlag("village_defended"); },
        [](GameContext* ctx) { 
            if (ctx) {
                ctx->worldState.setLocationState("village", "under_attack");
                ctx->playerStats.learnFact("bandits_attacked");
            }
        },
        0.2 // 20% chance
    });
    
    forestRegion->possibleEvents.push_back({
        "Rare Herb Sighting",
        "You spot a patch of rare medicinal herbs growing nearby.",
        [](const GameContext& ctx) { return ctx.playerStats.hasSkill("herbalism", 1); },
        [](GameContext* ctx) {
            if (ctx) {
                ctx->playerInventory.addItem({"rare_herb", "Rare Herb", "herb", 5, 1});
                std::cout << "Added 1 rare herb to inventory" << std::endl;
            }
        },
        0.3 // 30% chance
    });
    
    // Create time system
    std::cout << "Create time system" << std::endl;
    TimeNode* timeSystem = dynamic_cast<TimeNode*>(controller.createNode<TimeNode>("TimeSystem"));
    
    // Register the world system
    std::cout << "Register the world system" << std::endl;
    controller.setSystemRoot("WorldSystem", villageRegion);
    controller.setSystemRoot("TimeSystem", timeSystem);
    
    //----------------------------------------
    // DEMONSTRATION
    //----------------------------------------
    
    std::cout << "\n___ DEMONSTRATION ___\n" << std::endl;
    std::cout << "\n___ DEMONSTRATION ___\n" << std::endl;
    std::cout << "\n___ DEMONSTRATION ___\n" << std::endl;

    std::cout << "=== RPG GAME SYSTEMS USING TREE AUTOMATA ===" << std::endl;
    std::cout << "A comprehensive implementation of hierarchical game systems\n" << std::endl;
    
    // Initialize player inventory with some items
    controller.gameContext.playerInventory.addItem({"iron_ingot", "Iron Ingot", "material", 10, 5});
    controller.gameContext.playerInventory.addItem({"leather_strips", "Leather Strips", "material", 5, 10});
    controller.gameContext.playerInventory.addItem({"torch", "Torch", "tool", 2, 3});
    controller.gameContext.playerInventory.addItem({"red_herb", "Red Herb", "herb", 3, 5});
    controller.gameContext.playerInventory.addItem({"water_flask", "Water Flask", "container", 1, 2});
    
    // Set initial skills
    controller.gameContext.playerStats.improveSkill("combat", 2);
    controller.gameContext.playerStats.improveSkill("survival", 1);
    controller.gameContext.playerStats.improveSkill("crafting", 1);
    
    // Example: Start at village and talk to elder
    std::cout << "\n=== WORLD AND DIALOGUE EXAMPLE ===\n" << std::endl;
    
    // Start in village region
    controller.processInput("WorldSystem", {});
    
    // Travel to village center
    TAInput travelInput = {"region_action", {{"action", std::string("travel_location")}, {"location_index", 0}}};
    controller.processInput("WorldSystem", travelInput);
    
    // Talk to the village elder (NPC index 0)
    std::cout << "\nTalking to Elder Marius...\n" << std::endl;
    elderNPC.startDialogue(&controller.gameContext);
    
    // Example: Choose dialogue option 0 (Ask about threat)
    elderNPC.processResponse(0, &controller.gameContext);
    
    // Example: Choose dialogue option 0 (Ask how to help)
    elderNPC.processResponse(0, &controller.gameContext);
    
    // Example: Choose dialogue option 0 (Accept quest)
    elderNPC.processResponse(0, &controller.gameContext);
    
    // Example: Choose dialogue option 0 (I'll get started)
    elderNPC.processResponse(0, &controller.gameContext);
    
    // Example: Try crafting
    std::cout << "\n=== CRAFTING EXAMPLE ===\n" << std::endl;
    
    // Access the blacksmith station
    controller.processInput("CraftingSystem", {});
    
    // Select blacksmith station (child index 0)
    TAInput craftingInput = {"crafting_action", {{"action", std::string("select_station")}, {"index", 0}}};
    controller.processInput("CraftingSystem", craftingInput);
    
    // Craft a sword (recipe index 0)
    TAInput craftSwordInput = {"crafting_action", {{"action", std::string("craft")}, {"recipe_index", 0}}};
    controller.processInput("CraftingSystem", craftSwordInput);
    
    // Example: Skill progression
    std::cout << "\n=== SKILL PROGRESSION EXAMPLE ===\n" << std::endl;
    
    // Initialize progression system
    controller.processInput("ProgressionSystem", {});
    
    // Learn combat basics skill
    SkillNode* combatNode = dynamic_cast<SkillNode*>(controller.currentNodes["ProgressionSystem"]->childNodes[0]);
    if (combatNode && combatNode->canLearn(controller.gameContext)) {
        combatNode->learnSkill(&controller.gameContext);
    }
    
    // Learn swordsmanship (assumes we've navigated to it)
    if (swordsmanship->canLearn(controller.gameContext)) {
        swordsmanship->learnSkill(&controller.gameContext);
        std::cout << "Unlocked ability: power_attack" << std::endl;
    } else {
        std::cout << "Cannot learn swordsmanship yet - requirements not met" << std::endl;
    }
    
    // Example: Complete part of the main quest
    std::cout << "\n=== QUEST PROGRESSION EXAMPLE ===\n" << std::endl;
    
    // Initialize quest system with main quest
    controller.processInput("QuestSystem", {});
    
    // Access the repair walls subquest
    TANode* nextNode = nullptr;
    if (dynamic_cast<QuestNode*>(controller.currentNodes["QuestSystem"])->processAction("access_subquest", nextNode)) {
        controller.currentNodes["QuestSystem"] = repairWalls;
        repairWalls->onEnter(&controller.gameContext);
    }
    
    // Complete the repair walls quest
    TAInput completeQuestInput = {"action", {{"name", std::string("repair_complete")}}};
    controller.processInput("QuestSystem", completeQuestInput);
    
    // Track quest progress
    std::cout << "\nQuest journal:" << std::endl;
    for (const auto& [quest, status] : controller.gameContext.questJournal) {
        std::cout << "- " << quest << ": " << status << std::endl;
    }
    
    // Example: Time passage
    std::cout << "\n=== TIME SYSTEM EXAMPLE ===\n" << std::endl;
    
    // Initialize time system
    controller.processInput("TimeSystem", {});
    
    // Wait 5 hours
    for (int i = 0; i < 5; i++) {
        timeSystem->advanceHour(&controller.gameContext);
    }
    
    // Save the game state
    controller.saveState("game_save.dat");
    std::cout << "\nGame state saved to game_save.dat" << std::endl;

    // Display the contents of game_save
    std::ifstream savedFile("game_save.dat", std::ios::binary);
    if (savedFile.is_open()) {
        std::cout << "File created with size: " << savedFile.tellg() << " bytes" << std::endl;
        savedFile.close();
    }

    // Load the state from the saved file
    std::cout << "\n=== LOADING SAVED GAME ===\n" << std::endl;
    if (controller.loadState("game_save.dat")) {
        std::cout << "Game state loaded successfully!" << std::endl;
        
        // Display the loaded state information
        std::cout << "\nLoaded game information:" << std::endl;
        
        // Display current time
        TimeNode* loadedTime = dynamic_cast<TimeNode*>(controller.currentNodes["TimeSystem"]);
        if (loadedTime) {
            std::cout << "Time: Day " << loadedTime->day << ", " 
                    << loadedTime->hour << ":00, " 
                    << loadedTime->timeOfDay << " (" 
                    << loadedTime->season << ")" << std::endl;
        }
        
        // Display world state
        std::cout << "\nWorld state:" << std::endl;
        std::cout << "Days passed: " << controller.gameContext.worldState.daysPassed << std::endl;
        std::cout << "Current season: " << controller.gameContext.worldState.currentSeason << std::endl;
        
        // Display quest journal
        std::cout << "\nQuest journal:" << std::endl;
        for (const auto& [quest, status] : controller.gameContext.questJournal) {
            std::cout << "- " << quest << ": " << status << std::endl;
        }
        
        // Display player stats
        std::cout << "\nPlayer stats:" << std::endl;
        std::cout << "Strength: " << controller.gameContext.playerStats.strength << std::endl;
        std::cout << "Dexterity: " << controller.gameContext.playerStats.dexterity << std::endl;
        std::cout << "Constitution: " << controller.gameContext.playerStats.constitution << std::endl;
        std::cout << "Intelligence: " << controller.gameContext.playerStats.intelligence << std::endl;
        std::cout << "Wisdom: " << controller.gameContext.playerStats.wisdom << std::endl;
        std::cout << "Charisma: " << controller.gameContext.playerStats.charisma << std::endl;
        
        // Display skills
        std::cout << "\nSkills:" << std::endl;
        for (const auto& [skill, level] : controller.gameContext.playerStats.skills) {
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
            std::cout << "- " << item.name << " (" << item.quantity << ")" << std::endl;
        }
    } else {
        std::cout << "Failed to load game state." << std::endl;
    }

    // Add this pause before exiting
    std::cout << "\nPress Enter to exit...";
    std::cin.get();

    return 0;
}