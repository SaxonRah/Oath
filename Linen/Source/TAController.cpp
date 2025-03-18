#include "TAController.hpp"

TANode* TAController::createNode(const std::string& id)
{
    auto node = std::make_shared<TANode>(NodeID(id));
    nodes[id] = node;
    return node.get();
}

template <typename T, typename... Args>
T* TAController::createNode(const std::string& id, Args&&... args)
{
    auto node = std::make_shared<T>(NodeID(id), std::forward<Args>(args)...);
    T* nodePtr = node.get();
    nodes[id] = node;
    return nodePtr;
}

void TAController::setSystemRoot(const std::string& systemName, TANode* rootNode)
{
    systemRoots[systemName] = rootNode;
    currentNodes[systemName] = rootNode;
    nodeChanged[systemName] = true;
}

TANode* TAController::getCurrentNode(const std::string& systemName)
{
    auto it = currentNodes.find(systemName);
    if (it != currentNodes.end()) {
        return it->second;
    }
    return nullptr;
}

bool TAController::hasNodeChanged(const std::string& systemName)
{
    auto it = nodeChanged.find(systemName);
    return it != nodeChanged.end() && it->second;
}

void TAController::resetNodeChangedFlag(const std::string& systemName)
{
    nodeChanged[systemName] = false;
}

std::vector<TAAction> TAController::getAvailableActions(const std::string& systemName)
{
    auto it = currentNodes.find(systemName);
    if (it != currentNodes.end()) {
        return it->second->getAvailableActions();
    }
    return {};
}

bool TAController::processInput(const std::string& systemName, const TAInput& input, TANode*& outNextNode)
{
    auto it = currentNodes.find(systemName);
    if (it == currentNodes.end()) {
        return false;
    }

    TANode* currentNode = it->second;
    if (currentNode->evaluateTransition(input, outNextNode)) {
        if (outNextNode != currentNode) {
            currentNodes[systemName] = outNextNode;
            nodeChanged[systemName] = true;
        }
        return true;
    }

    return false;
}