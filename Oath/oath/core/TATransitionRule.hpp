#pragma once

#include "TAInput.hpp"

#include <functional>
#include <string>

// Forward declaration
struct TAInput;
class TANode;

// Transition rule
struct TATransitionRule {
    std::function<bool(const TAInput&)> condition;
    TANode* targetNode;
    std::string description;
};