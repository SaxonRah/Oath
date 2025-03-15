#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "../data/GameContext.hpp"
#include "TANode.hpp"


class TAController {
public:
    // Current active nodes
    std::map<std::string, TANode*> currentNodes;

    // Root nodes for different systems
    std::map<std::string, TANode*> systemRoots;

    // Owned nodes for memory management
    std::vector<std::unique_ptr<TANode>> ownedNodes;

    // Game context
    GameContext gameContext;

    // Game data storage
    std::map<std::string, std::map<std::string, NPC*>> gameData;

    // Methods as defined in the original implementation
    // ...
};