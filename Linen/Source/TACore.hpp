#pragma once

#include <Include/nlohmann/json.hpp>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

using json = nlohmann::json;

// Base Node ID class for all nodes in the system
class NodeID {
public:
    NodeID(const std::string& id)
        : id_(id)
    {
    }
    const std::string& getId() const { return id_; }

private:
    std::string id_;
};

// Input structure for transitions
struct TAInput {
    std::string type;
    std::map<std::string, std::variant<int, float, std::string, bool>> parameters;
};

// Action structure for user interface
struct TAAction {
    std::string id;
    std::string description;
    std::function<TAInput()> inputGenerator;
};

// Transition rule structure
struct TATransitionRule {
    std::function<bool(const TAInput&)> condition;
    TANode* targetNode;
    std::string description;
};

// Tree Automata Node - the foundation of our entire system
class TANode {
public:
    TANode(const std::string& name)
        : id(NodeID(name))
        , nodeName(name)
    {
    }
    virtual ~TANode() = default;

    // Add child nodes
    void addChild(TANode* child);

    // Virtual methods for state transitions
    virtual void onEnter(void* context) { }
    virtual void onExit(void* context) { }

    virtual std::vector<TAAction> getAvailableActions();

    bool evaluateTransition(const TAInput& input, TANode*& outNextNode);

    void TANode::addTransition(const std::function<bool(const TAInput&)>& condition, TANode* target, const std::string& description);

    const NodeID& getId() const { return id; }
    const std::string& getName() const { return nodeName; }

    // State data storage
    std::map<std::string, std::variant<int, float, std::string, bool>> stateData;

protected:
    NodeID id_;
    std::string nodeName;
    std::vector<TANode*> children_;
};
