#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "../data/GameContext.hpp"
#include "TANode.hpp"

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