#pragma once

#include "TAInput.hpp"

#include <functional>
#include <string>


// Available actions from a state
struct TAAction {
    std::string name;
    std::string description;
    std::function<TAInput()> createInput;
};