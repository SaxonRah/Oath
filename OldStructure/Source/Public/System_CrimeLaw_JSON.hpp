// System_CrimeLaw_JSON.hpp
#ifndef CRIME_LAW_SYSTEM_JSON_HPP
#define CRIME_LAW_SYSTEM_JSON_HPP

#include <algorithm>
#include <ctime>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <queue>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

// Forward declarations
class TANode;
class TAController;
class Inventory;
class NPC;
class Recipe;
class GameContext;
class WorldState;
class CharacterStats;
struct NodeID;
struct TAInput;
struct TAAction;
struct TATransitionRule;

//----------------------------------------
// CRIME AND LAW SYSTEM
//----------------------------------------

// Struct for accessing crime types
struct CrimeType {
    static std::string get(const std::string& type);

    static std::string THEFT();
    static std::string ASSAULT();
    static std::string MURDER();
    static std::string TRESPASSING();
    static std::string VANDALISM();
    static std::string PICKPOCKETING();
    static std::string PRISON_BREAK();
};

// Struct for accessing guard response types
struct GuardResponseType {
    static std::string get(const std::string& type);

    static std::string ARREST();
    static std::string ATTACK();
    static std::string FINE();
    static std::string WARN();
    static std::string IGNORE();
};

// Record of a committed crime
struct CrimeRecord {
    std::string type; // Type of crime (from CrimeType)
    std::string region; // Region where crime was committed
    std::string location; // Specific location
    bool witnessed; // Whether the crime was witnessed
    int severity; // 1-10 scale of severity
    int bounty; // Gold bounty assigned
    time_t timestamp; // When the crime was committed
    bool paid; // Whether the bounty has been paid

    CrimeRecord(const std::string& crimeType, const std::string& crimeRegion, const std::string& crimeLocation,
        bool wasWitnessed, int crimeSeverity);

    int calculateBounty() const;
    std::string getDescription() const;
};

// Tracks the player's criminal status across different regions
class CriminalRecord {
public:
    std::vector<CrimeRecord> crimes;
    std::map<std::string, int> totalBountyByRegion;
    std::map<std::string, int> reputationByRegion; // Criminal reputation (-100 to 100, lower is worse)
    std::map<std::string, bool> wantedStatus; // Whether player is actively wanted by guards

    // Add a new crime to the record
    void addCrime(const CrimeRecord& crime);

    // Pay bounty for a specific region
    bool payBounty(const std::string& region, int goldAmount, GameContext* context);

    // Check if player is wanted in the given region
    bool isWanted(const std::string& region) const;

    // Get total bounty for a region
    int getBounty(const std::string& region) const;

    // Get criminal reputation for a region
    int getReputation(const std::string& region) const;

    // Get a list of unpaid crimes in a region
    std::vector<CrimeRecord> getUnpaidCrimes(const std::string& region) const;

    // Serves jail time and clears some crimes based on time served
    void serveJailSentence(const std::string& region, int days);
};

// Extend GameContext to include criminal record
struct CrimeLawContext {
    CriminalRecord criminalRecord;

    // Existing guard status
    std::map<std::string, bool> guardAlerted;
    std::map<std::string, int> guardSuspicion; // 0-100
    std::map<std::string, int> jailSentencesByRegion; // Days of jail time

    // Jail properties
    int currentJailDays = 0;
    std::string currentJailRegion = "";
    bool inJail = false;

    // Items confiscated during arrest
    Inventory confiscatedItems;

    CrimeLawContext();

    // Calculate jail sentence based on crimes
    int calculateJailSentence(const std::string& region);
};

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

// Theft action node
class TheftNode : public CrimeSystemNode {
public:
    TheftNode(const std::string& name);

    void onEnter(GameContext* context) override;
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;
};

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

// Guard encounter node - handles guard interactions when wanted
class GuardEncounterNode : public CrimeSystemNode {
public:
    GuardEncounterNode(const std::string& name);

    void onEnter(GameContext* context) override;
    int determineGuardResponse(GameContext* context);
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;
};

// Jail node - handles jail sentences
class JailNode : public CrimeSystemNode {
public:
    JailNode(const std::string& name);

    void onEnter(GameContext* context) override;
    void confiscateItems(GameContext* context);
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;
    void serveTime(GameContext* context);
    bool attemptEscape(GameContext* context);
};

// Bounty payment node
class BountyPaymentNode : public CrimeSystemNode {
public:
    BountyPaymentNode(const std::string& name);

    void onEnter(GameContext* context) override;
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;
    bool payFullBounty(GameContext* context);
    bool negotiateBounty(GameContext* context);
};

// Pickpocketing node - For stealing from NPCs
class PickpocketNode : public CrimeSystemNode {
public:
    PickpocketNode(const std::string& name);

    void onEnter(GameContext* context) override;
    std::vector<json> getPickpocketTargets(GameContext* context);
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;
    void attemptPickpocket(GameContext* context, const std::string& target, int difficulty);
};

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

public:
    CrimeLawSystem(TAController* controller);

    void setupNodes();
    void setupTransitions();
    void registerSystem();
    void setupGuardEncounters();

    // Method to be called by game to commit a crime programmatically
    void commitCrime(GameContext* context, const std::string& crimeType, int severity, const std::string& region, const std::string& location);

    CrimeLawContext* getLawContextFromController();
};

// Function to load the JSON configuration
void loadCrimeLawConfig();

#endif // CRIME_LAW_SYSTEM_JSON_HPP