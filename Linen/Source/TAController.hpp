#pragma once

#include "TACore.hpp"
#include <iostream>
#include <map>
#include <memory>
#include <string>

class GameContext;

// Controller for the tree automata system
class TAController {
public:
    TAController() { }

    template <typename T, typename... Args>
    T* createNode(const std::string& name, Args&&... args)
    {
        auto node = new T(name, std::forward<Args>(args)...);
        nodes_[name] = node;
        return node;
    }

    void setSystemRoot(const std::string& systemName, TANode* rootNode)
    {
        systemRoots[systemName] = rootNode;
        currentNodes[systemName] = rootNode;
        nodeChangedFlags[systemName] = true;
    }

    TANode* getNode(const std::string& name)
    {
        auto it = nodes_.find(name);
        return it != nodes_.end() ? it->second : nullptr;
    }

    TANode* getCurrentNode(const std::string& systemName)
    {
        auto it = currentNodes.find(systemName);
        return it != currentNodes.end() ? it->second : nullptr;
    }

    bool hasNodeChanged(const std::string& systemName)
    {
        auto it = nodeChangedFlags.find(systemName);
        return it != nodeChangedFlags.end() && it->second;
    }

    void resetNodeChangedFlag(const std::string& systemName)
    {
        nodeChangedFlags[systemName] = false;
    }

    bool processInput(const std::string& systemName, const TAInput& input, TANode*& outNextNode)
    {
        if (systemRoots.find(systemName) == systemRoots.end()) {
            std::cerr << "System not found: " << systemName << std::endl;
            return false;
        }

        if (currentNodes.find(systemName) == currentNodes.end()) {
            // Initialize with the system root if no current node
            currentNodes[systemName] = systemRoots[systemName];
            nodeChangedFlags[systemName] = true;
        }

        // Find appropriate transition
        for (const auto& rule : currentNodes[systemName]->transitionRules) {
            if (rule.condition(input)) {
                // Found matching transition
                TANode* prevNode = currentNodes[systemName];

                // Update the current node
                currentNodes[systemName] = rule.targetNode;
                outNextNode = rule.targetNode;

                // Mark node as changed
                nodeChangedFlags[systemName] = true;

                // Call exit and enter methods
                prevNode->onExit(nullptr);
                rule.targetNode->onEnter(nullptr);

                return true;
            }
        }

        return false;
    }

    std::vector<TAAction> getAvailableActions(const std::string& systemName)
    {
        if (currentNodes.find(systemName) == currentNodes.end()) {
            return {};
        }

        return currentNodes[systemName]->getAvailableActions();
    }

private:
    std::map<std::string, TANode*> nodes_;
    std::map<std::string, TANode*> systemRoots;
    std::map<std::string, TANode*> currentNodes;
    std::map<std::string, bool> nodeChangedFlags;
};
