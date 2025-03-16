// CrimeLawConfig.hpp
#pragma once

#include <nlohmann/json.hpp>

// Declare the global config (just the declaration, not definition)
extern nlohmann::json crimeLawConfig;

// Function to load the config
void loadCrimeLawConfig();