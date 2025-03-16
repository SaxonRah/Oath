// CrimeSystemNode.hpp
#pragma once

#include "../../core/TANode.hpp"
#include "CrimeLawContext.hpp"
#include <string>

// Forward declarations
struct GameContext;

// Base node for crime system
class CrimeSystemNode : public TANode {
public:
    CrimeSystemNode(const std::string& name);

    // Extended game context
    CrimeLawContext* getLawContext(GameContext* context);

    // Helper to get current region
    std::string getCurrentRegion(GameContext* context);

    // Helper to get current location
    std::string getCurrentLocation(GameContext* context);

    // Check if crime was witnessed based on location and stealth
    bool isCrimeWitnessed(GameContext* context, int stealthModifier);

    // Commits a crime and adds it to the player's record
    void commitCrime(GameContext* context, const std::string& crimeType, int severity, int stealthModifier = 0);
};