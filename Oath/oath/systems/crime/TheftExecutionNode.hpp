// TheftExecutionNode.hpp
#pragma once

#include "CrimeSystemNode.hpp"
#include <string>

// Theft execution node
class TheftExecutionNode : public CrimeSystemNode {
private:
    std::string theftTarget;
    int theftValue;
    int theftSeverity;

public:
    TheftExecutionNode(const std::string& name, const std::string& target);

    void onEnter(GameContext* context) override;
    bool attemptTheft(GameContext* context);
};