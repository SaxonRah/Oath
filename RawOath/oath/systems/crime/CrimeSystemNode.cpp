// CrimeSystemNode.cpp
#include "CrimeSystemNode.hpp"
#include "../../data/GameContext.hpp"
#include "CrimeLawConfig.hpp"
#include "CrimeType.hpp"
#include <iostream>
#include <random>


CrimeSystemNode::CrimeSystemNode(const std::string& name)
    : TANode(name)
{
}

CrimeLawContext* CrimeSystemNode::getLawContext(GameContext* context)
{
    static CrimeLawContext lawContext;
    return &lawContext;
}

std::string CrimeSystemNode::getCurrentRegion(GameContext* context)
{
    // You would get this from your world system
    // This is a placeholder - in a real implementation you'd get the
    // actual current region from the context
    if (context) {
        return context->worldState.getFactionState("current_region");
    }
    return crimeLawConfig["regions"][0]; // Default to first region
}

std::string CrimeSystemNode::getCurrentLocation(GameContext* context)
{
    // You would get this from your world system
    // This is a placeholder - real implementation would get from context
    if (context) {
        for (const auto& [location, status] : context->worldState.locationStates) {
            if (status == "current") {
                return location;
            }
        }
    }
    return "Village Center"; // Default
}

bool CrimeSystemNode::isCrimeWitnessed(GameContext* context, int stealthModifier)
{
    // Get player stealth skill
    int stealthSkill = 0;
    if (context) {
        auto it = context->playerStats.skills.find("stealth");
        if (it != context->playerStats.skills.end()) {
            stealthSkill = it->second;
        }
    }

    // Determine base witness chance by location
    std::string location = getCurrentLocation(context);
    std::string locationType = "default";

    // Check location keywords to determine type
    for (const auto& type : { "town", "village", "city", "forest", "wilderness", "dungeon", "cave" }) {
        if (location.find(type) != std::string::npos) {
            locationType = type;
            break;
        }
    }

    int witnessChance = crimeLawConfig["witnessChances"][locationType].get<int>();

    // Apply stealth skill and modifier
    witnessChance -= (stealthSkill * 2) + stealthModifier;

    // Ensure bounds
    witnessChance = std::max(5, std::min(witnessChance, 95));

    // Random check
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 100);

    return dis(gen) <= witnessChance;
}

void CrimeSystemNode::commitCrime(GameContext* context, const std::string& crimeType, int severity, int stealthModifier)
{
    CrimeLawContext* lawContext = getLawContext(context);
    std::string region = getCurrentRegion(context);
    std::string location = getCurrentLocation(context);

    bool witnessed = isCrimeWitnessed(context, stealthModifier);

    CrimeRecord crime(crimeType, region, location, witnessed, severity);
    lawContext->criminalRecord.addCrime(crime);

    if (witnessed) {
        std::cout << "Your crime was witnessed!" << std::endl;
        lawContext->guardAlerted[region] = true;
        lawContext->guardSuspicion[region] += severity * 10;
    } else {
        std::cout << "You committed a crime unseen." << std::endl;
        lawContext->guardSuspicion[region] += severity * 2;
    }

    std::cout << "Crime added: " << crime.getDescription() << std::endl;
}