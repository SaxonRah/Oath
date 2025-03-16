// CrimeLawConfig.cpp
#include "CrimeLawConfig.hpp"
#include <fstream>
#include <stdexcept>

// Global config definition (this actually allocates memory for it)
nlohmann::json crimeLawConfig;

void loadCrimeLawConfig()
{
    std::ifstream configFile("resources/config/CrimeLaw.json");
    if (!configFile.is_open()) {
        throw std::runtime_error("Could not open CrimeLaw.json file");
    }
    configFile >> crimeLawConfig;
}