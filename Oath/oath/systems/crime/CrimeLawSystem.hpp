// CrimeLawSystem.hpp
#pragma once

#include "../../core/TAController.hpp"
#include "BountyPaymentNode.hpp"
#include "CrimeLawConfig.hpp"
#include "GuardEncounterNode.hpp"
#include "JailNode.hpp"
#include "PickpocketNode.hpp"
#include "TheftExecutionNode.hpp"
#include "TheftNode.hpp"

#include <map>
#include <string>

// Create and set up a complete Crime and Law System
class CrimeLawSystem {
private:
    TAController* controller;
    CrimeSystemNode* criminalStatusNode;
    TheftNode* theftNode;
    GuardEncounterNode* guardNode;
    JailNode* jailNode;
    BountyPaymentNode* bountyNode;
    PickpocketNode* pickpocketNode;
    std::map<std::string, TheftExecutionNode*> theftExecutionNodes;

    static CrimeLawSystem* instance;

public:
    CrimeLawSystem(TAController* controller);

    void setupNodes();
    void setupTransitions();
    void registerSystem();
    void setupGuardEncounters();

    // Method to be called by game to commit a crime programmatically
    void commitCrime(GameContext* context, const std::string& crimeType, int severity, const std::string& region, const std::string& location);

    CrimeLawContext* getLawContextFromController();

    static CrimeLawSystem* getInstance()
    {
        return instance;
    }

    CrimeLawContext* getLawContext()
    {
        return getLawContextFromController();
    }
};
