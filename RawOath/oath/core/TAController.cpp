#include "TAController.hpp"
#include "../utils/JSONSerializer.hpp"

#include "../../include/nlohmann/json.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>

template <typename T, typename... Args>
T* TAController::createNode(const std::string& name, Args&&... args)
{
    auto node = std::make_unique<T>(name, std::forward<Args>(args)...);
    T* nodePtr = node.get();
    ownedNodes.push_back(std::move(node));
    return nodePtr;
}

bool TAController::processInput(const std::string& systemName, const TAInput& input)
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

void TAController::updateCurrentNodePersistentID(const std::string& systemName)
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

std::vector<TAAction> TAController::getAvailableActions(const std::string& systemName)
{
    if (currentNodes.find(systemName) == currentNodes.end()) {
        return {};
    }

    return currentNodes[systemName]->getAvailableActions();
}

bool TAController::isStateReachable(const NodeID& targetNodeID)
{
    for (const auto& [name, rootNode] : systemRoots) {
        if (isNodeReachableFromNode(rootNode, targetNodeID)) {
            return true;
        }
    }
    return false;
}

void TAController::setSystemRoot(const std::string& systemName, TANode* rootNode)
{
    systemRoots[systemName] = rootNode;
}

void TAController::initializePersistentIDs()
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

std::string TAController::findPathToNode(TANode* root, TANode* target, const std::string& basePath)
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

TANode* TAController::findNodeByPersistentID(const std::string& persistentID)
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
            std::cout << "Found node '" << nodeName << "' by name instead of by full ID" << std::endl;

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
                        std::cout << "Found location '" << nodeName << "' in region "
                                  << regionNode->regionName << std::endl;
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
                            std::cout << "Found location '" << nodeName << "' in sub-region "
                                      << subRegion->regionName << std::endl;
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

TANode* TAController::findNodeByNameRecursive(TANode* node, const std::string& nodeName)
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

TANode* TAController::findNodeByNameInHierarchy(TANode* node, const std::string& nodeName)
{
    if (node->nodeName == nodeName) {
        return node;
    }

    for (TANode* child : node->childNodes) {
        TANode* result = findNodeByNameInHierarchy(child, nodeName);
        if (result)
            return result;
    }

    return nullptr;
}

void TAController::printNodeIDsRecursive(TANode* node, int depth)
{
    for (int i = 0; i < depth; i++)
        std::cerr << "  ";
    std::cerr << "- " << node->nodeName << ": '" << node->nodeID.persistentID << "'" << std::endl;

    for (TANode* child : node->childNodes) {
        printNodeIDsRecursive(child, depth + 1);
    }
}

TANode* TAController::findNodeByPersistentIDRecursive(TANode* node, const std::string& persistentID)
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

bool TAController::saveState_old(const std::string& filename)
{
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
        file.write(reinterpret_cast<const char*>(&nameLength),
            sizeof(nameLength));
        file.write(name.c_str(), nameLength);

        // Write if there's a current node
        bool hasCurrentNode = currentNodes.find(name) != currentNodes.end();
        file.write(reinterpret_cast<const char*>(&hasCurrentNode),
            sizeof(hasCurrentNode));

        if (hasCurrentNode) {
            // Write node ID
            file.write(reinterpret_cast<const char*>(&currentNodes[name]->nodeID),
                sizeof(NodeID));

            // Also write node name for better lookup capability
            size_t nodeNameLength = currentNodes[name]->nodeName.length();
            file.write(reinterpret_cast<const char*>(&nodeNameLength),
                sizeof(nodeNameLength));
            file.write(currentNodes[name]->nodeName.c_str(), nodeNameLength);
        }
    }

    // Save game context (simplified)
    file.write(
        reinterpret_cast<const char*>(&gameContext.worldState.daysPassed),
        sizeof(int));

    size_t knownFactsSize = gameContext.playerStats.knownFacts.size();
    file.write(reinterpret_cast<const char*>(&knownFactsSize),
        sizeof(knownFactsSize));

    for (const auto& fact : gameContext.playerStats.knownFacts) {
        size_t factLength = fact.length();
        file.write(reinterpret_cast<const char*>(&factLength),
            sizeof(factLength));
        file.write(fact.c_str(), factLength);
    }

    return true;
}

bool TAController::loadState_old(const std::string& filename)
{
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
        file.read(reinterpret_cast<char*>(&hasCurrentNode),
            sizeof(hasCurrentNode));

        if (hasCurrentNode) {
            // Read node ID
            NodeID nodeID;
            file.read(reinterpret_cast<char*>(&nodeID), sizeof(NodeID));

            // Read node name
            size_t nodeNameLength;
            file.read(reinterpret_cast<char*>(&nodeNameLength),
                sizeof(nodeNameLength));
            std::string nodeName(nodeNameLength, ' ');
            file.read(&nodeName[0], nodeNameLength);

            // First try to find by ID (faster)
            TANode* node = findNodeById(systemRoots[name], nodeID);

            // If not found by ID, try by name as fallback
            if (!node) {
                std::cout << "Node ID not matched for system " << name
                          << ", trying to find by name..." << std::endl;
                node = findNodeByName(systemRoots[name], nodeName);
            }

            if (node) {
                currentNodes[name] = node;
                std::cout << "Successfully restored node " << nodeName
                          << " for system " << name << std::endl;
            } else {
                std::cerr << "Node not found during load for system: " << name
                          << std::endl;
                currentNodes[name] = systemRoots[name]; // Default to root
                std::cout << "Falling back to system root: "
                          << systemRoots[name]->nodeName << std::endl;
            }
        }
    }

    // Load game context (simplified)
    file.read(reinterpret_cast<char*>(&gameContext.worldState.daysPassed),
        sizeof(int));

    size_t knownFactsSize;
    file.read(reinterpret_cast<char*>(&knownFactsSize),
        sizeof(knownFactsSize));

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

bool TAController::saveState(const std::string& filename)
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

bool TAController::loadState(const std::string& filename)
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

TANode* TAController::findNodeById(TANode* startNode, const NodeID& id)
{
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

TANode* TAController::findNodeByName(TANode* startNode, const std::string& name)
{
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

bool TAController::isNodeReachableFromNode(TANode* startNode, const NodeID& targetId)
{
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