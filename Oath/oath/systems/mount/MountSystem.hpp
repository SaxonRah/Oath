#pragma once

#include <string>

class TAController;

// Mount system setup function
void setupMountSystem(TAController& controller, const std::string& configPath = "resources/json/Mount.json");