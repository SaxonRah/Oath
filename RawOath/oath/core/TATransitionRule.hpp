#pragma once

#include "TAInput.hpp"

#include <functional>
#include <string>

// Transition rule
struct TATransitionRule {
    std::function<bool(const TAInput&)> condition;
    TANode* targetNode;
    std::string description;
};