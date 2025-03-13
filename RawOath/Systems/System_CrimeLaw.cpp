// System_CrimeLaw.cpp
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

// Forward declarations from RawOathFull.cpp
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

// Enum-like structure for crime types
struct CrimeType {
    static const std::string THEFT;
    static const std::string ASSAULT;
    static const std::string MURDER;
    static const std::string TRESPASSING;
    static const std::string VANDALISM;
    static const std::string PICKPOCKETING;
};

const std::string CrimeType::THEFT = "theft";
const std::string CrimeType::ASSAULT = "assault";
const std::string CrimeType::MURDER = "murder";
const std::string CrimeType::TRESPASSING = "trespassing";
const std::string CrimeType::VANDALISM = "vandalism";
const std::string CrimeType::PICKPOCKETING = "pickpocketing";

// Enum-like structure for guard response types
struct GuardResponseType {
    static const std::string ARREST;
    static const std::string ATTACK;
    static const std::string FINE;
    static const std::string WARN;
    static const std::string IGNORE;
};

const std::string GuardResponseType::ARREST = "arrest";
const std::string GuardResponseType::ATTACK = "attack";
const std::string GuardResponseType::FINE = "fine";
const std::string GuardResponseType::WARN = "warn";
const std::string GuardResponseType::IGNORE = "ignore";

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
        bool wasWitnessed, int crimeSeverity)
        : type(crimeType)
        , region(crimeRegion)
        , location(crimeLocation)
        , witnessed(wasWitnessed)
        , severity(crimeSeverity)
        , bounty(calculateBounty())
        , timestamp(std::time(nullptr))
        , paid(false)
    {
    }

    int calculateBounty() const
    {
        // Base bounty by crime type
        int baseBounty = 0;
        if (type == CrimeType::THEFT)
            baseBounty = 50;
        else if (type == CrimeType::ASSAULT)
            baseBounty = 100;
        else if (type == CrimeType::MURDER)
            baseBounty = 1000;
        else if (type == CrimeType::TRESPASSING)
            baseBounty = 20;
        else if (type == CrimeType::VANDALISM)
            baseBounty = 40;
        else if (type == CrimeType::PICKPOCKETING)
            baseBounty = 30;

        // Adjust by severity
        int adjustedBounty = baseBounty * severity / 5;

        // Adjust if witnessed
        if (witnessed)
            adjustedBounty *= 2;

        return adjustedBounty;
    }

    std::string getDescription() const
    {
        std::string desc = type;
        // Capitalize first letter
        desc[0] = std::toupper(desc[0]);

        desc += " in " + location + ", " + region;
        if (witnessed) {
            desc += " (Witnessed)";
        } else {
            desc += " (Unwitnessed)";
        }

        desc += " - Bounty: " + std::to_string(bounty) + " gold";
        if (paid) {
            desc += " (Paid)";
        }

        return desc;
    }
};

// Tracks the player's criminal status across different regions
class CriminalRecord {
public:
    std::vector<CrimeRecord> crimes;
    std::map<std::string, int> totalBountyByRegion;
    std::map<std::string, int> reputationByRegion; // Criminal reputation (-100 to 100, lower is worse)
    std::map<std::string, bool> wantedStatus; // Whether player is actively wanted by guards

    // Add a new crime to the record
    void addCrime(const CrimeRecord& crime)
    {
        crimes.push_back(crime);

        // Update bounty for the region
        if (!crime.paid) {
            totalBountyByRegion[crime.region] += crime.bounty;
        }

        // Update criminal reputation
        int repLoss = crime.severity * 2;
        if (crime.witnessed)
            repLoss *= 2;

        if (reputationByRegion.find(crime.region) == reputationByRegion.end()) {
            reputationByRegion[crime.region] = 0; // Start at neutral
        }

        reputationByRegion[crime.region] -= repLoss;
        if (reputationByRegion[crime.region] < -100) {
            reputationByRegion[crime.region] = -100; // Cap at -100
        }

        // Update wanted status based on the crime
        if (crime.witnessed && crime.severity > 3) {
            wantedStatus[crime.region] = true;
        }
    }

    // Pay bounty for a specific region
    bool payBounty(const std::string& region, int goldAmount, GameContext* context)
    {
        if (goldAmount < totalBountyByRegion[region]) {
            return false; // Not enough gold
        }

        // Mark all crimes as paid in this region
        for (auto& crime : crimes) {
            if (crime.region == region && !crime.paid) {
                crime.paid = true;
            }
        }

        // Reset bounty for the region
        totalBountyByRegion[region] = 0;

        // Reset wanted status
        wantedStatus[region] = false;

        // Improve reputation slightly
        if (reputationByRegion.find(region) != reputationByRegion.end()) {
            reputationByRegion[region] += 10;
            if (reputationByRegion[region] > 100) {
                reputationByRegion[region] = 100;
            }
        }

        return true;
    }

    // Check if player is wanted in the given region
    bool isWanted(const std::string& region) const
    {
        auto it = wantedStatus.find(region);
        return it != wantedStatus.end() && it->second;
    }

    // Get total bounty for a region
    int getBounty(const std::string& region) const
    {
        auto it = totalBountyByRegion.find(region);
        return (it != totalBountyByRegion.end()) ? it->second : 0;
    }

    // Get criminal reputation for a region
    int getReputation(const std::string& region) const
    {
        auto it = reputationByRegion.find(region);
        return (it != reputationByRegion.end()) ? it->second : 0;
    }

    // Get a list of unpaid crimes in a region
    std::vector<CrimeRecord> getUnpaidCrimes(const std::string& region) const
    {
        std::vector<CrimeRecord> unpaid;
        for (const auto& crime : crimes) {
            if (crime.region == region && !crime.paid) {
                unpaid.push_back(crime);
            }
        }
        return unpaid;
    }

    // Serves jail time and clears some crimes based on time served
    void serveJailSentence(const std::string& region, int days)
    {
        // Mark all crimes as paid in this region
        for (auto& crime : crimes) {
            if (crime.region == region && !crime.paid) {
                crime.paid = true;
            }
        }

        // Reset bounty for the region
        totalBountyByRegion[region] = 0;

        // Reset wanted status
        wantedStatus[region] = false;

        // Improve reputation based on days served
        if (reputationByRegion.find(region) != reputationByRegion.end()) {
            reputationByRegion[region] += days * 5;
            if (reputationByRegion[region] > 100) {
                reputationByRegion[region] = 100;
            }
        }
    }
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

    CrimeLawContext()
    {
        // Initialize with some default values
        for (const auto& region : { "Oakvale Village", "Green Haven Forest", "Stone Peak Mountains" }) {
            guardAlerted[region] = false;
            guardSuspicion[region] = 0;
            jailSentencesByRegion[region] = 0;
        }
    }

    // Calculate jail sentence based on crimes
    int calculateJailSentence(const std::string& region)
    {
        int sentence = 0;
        auto crimes = criminalRecord.getUnpaidCrimes(region);

        for (const auto& crime : crimes) {
            if (crime.type == CrimeType::MURDER) {
                sentence += 30 * crime.severity; // 30 days per murder point
            } else if (crime.type == CrimeType::ASSAULT) {
                sentence += 5 * crime.severity; // 5 days per assault point
            } else if (crime.type == CrimeType::THEFT || crime.type == CrimeType::PICKPOCKETING) {
                sentence += 2 * crime.severity; // 2 days per theft point
            } else {
                sentence += 1 * crime.severity; // 1 day for minor crimes
            }
        }

        // Cap at reasonable values
        return std::min(sentence, 365); // Max 1 year
    }
};

// Base node for crime system
class CrimeSystemNode : public TANode {
public:
    CrimeSystemNode(const std::string& name)
        : TANode(name)
    {
    }

    // Extended game context
    CrimeLawContext* getLawContext(GameContext* context)
    {
        static CrimeLawContext lawContext;
        return &lawContext;
    }

    // Helper to get current region
    std::string getCurrentRegion(GameContext* context)
    {
        // You would get this from your world system
        // This is a placeholder - in a real implementation you'd get the
        // actual current region from the context
        if (context) {
            return context->worldState.getFactionState("current_region");
        }
        return "Oakvale Village"; // Default
    }

    // Helper to get current location
    std::string getCurrentLocation(GameContext* context)
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

    // Check if crime was witnessed based on location and stealth
    bool isCrimeWitnessed(GameContext* context, int stealthModifier)
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
        int witnessChance = 50; // Default 50% chance

        if (location.find("Town") != std::string::npos || location.find("Village") != std::string::npos || location.find("City") != std::string::npos) {
            witnessChance = 80; // 80% in populated areas
        } else if (location.find("Forest") != std::string::npos || location.find("Wilderness") != std::string::npos) {
            witnessChance = 20; // 20% in wilderness
        } else if (location.find("Dungeon") != std::string::npos || location.find("Cave") != std::string::npos) {
            witnessChance = 10; // 10% in dungeons
        }

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

    // Commits a crime and adds it to the player's record
    void commitCrime(GameContext* context, const std::string& crimeType, int severity, int stealthModifier = 0)
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
};

// Theft action node
class TheftNode : public CrimeSystemNode {
public:
    TheftNode(const std::string& name)
        : CrimeSystemNode(name)
    {
    }

    void onEnter(GameContext* context) override
    {
        std::cout << "You are considering stealing something..." << std::endl;

        // Show potential theft targets based on current location
        std::cout << "Potential targets:" << std::endl;
        std::cout << "1. Small valuables (Low risk)" << std::endl;
        std::cout << "2. Merchant goods (Medium risk)" << std::endl;
        std::cout << "3. Valuable artifacts (High risk)" << std::endl;
    }

    std::vector<TAAction> getAvailableActions() override
    {
        std::vector<TAAction> actions;

        actions.push_back({ "steal_small", "Steal small valuables", []() -> TAInput {
                               return { "theft_action", { { "target", std::string("small") } } };
                           } });

        actions.push_back({ "steal_medium", "Steal merchant goods", []() -> TAInput {
                               return { "theft_action", { { "target", std::string("medium") } } };
                           } });

        actions.push_back({ "steal_large", "Steal valuable artifacts", []() -> TAInput {
                               return { "theft_action", { { "target", std::string("large") } } };
                           } });

        actions.push_back({ "cancel_theft", "Cancel theft attempt", []() -> TAInput {
                               return { "theft_action", { { "target", std::string("cancel") } } };
                           } });

        return actions;
    }

    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override
    {
        if (input.type == "theft_action") {
            std::string target = std::get<std::string>(input.parameters.at("target"));

            // Find the target node in the transitions
            for (const auto& rule : transitionRules) {
                if (rule.description.find(target) != std::string::npos) {
                    outNextNode = rule.targetNode;
                    return true;
                }
            }
        }

        return CrimeSystemNode::evaluateTransition(input, outNextNode);
    }
};

// Theft execution node
class TheftExecutionNode : public CrimeSystemNode {
private:
    std::string theftTarget;
    int theftValue;
    int theftSeverity;

public:
    TheftExecutionNode(const std::string& name, const std::string& target, int value, int severity)
        : CrimeSystemNode(name)
        , theftTarget(target)
        , theftValue(value)
        , theftSeverity(severity)
    {
    }

    void onEnter(GameContext* context) override
    {
        std::cout << "Attempting to steal " << theftTarget << "..." << std::endl;

        // Roll for theft success
        bool success = attemptTheft(context);

        if (success) {
            std::cout << "Theft successful! You've stolen " << theftTarget << " worth " << theftValue << " gold." << std::endl;

            // Add stolen goods to inventory
            if (context) {
                std::string itemId = "stolen_" + theftTarget;
                std::string itemName = "Stolen " + theftTarget;
                context->playerInventory.addItem({ itemId, itemName, "stolen", theftValue, 1 });
            }

            // Record the crime
            commitCrime(context, CrimeType::THEFT, theftSeverity, 0);
        } else {
            std::cout << "Theft failed! You couldn't steal the " << theftTarget << "." << std::endl;

            // Check if caught
            CrimeLawContext* lawContext = getLawContext(context);
            std::string region = getCurrentRegion(context);

            bool caught = (lawContext->guardSuspicion[region] > 50);
            if (caught) {
                std::cout << "You've been caught attempting theft!" << std::endl;
                commitCrime(context, CrimeType::THEFT, theftSeverity / 2, -20); // Caught in the act
            } else {
                std::cout << "No one noticed your failed attempt." << std::endl;
            }
        }
    }

    bool attemptTheft(GameContext* context)
    {
        // Base chance based on theft difficulty
        int successChance = 70 - (theftSeverity * 10); // Harder thefts are less likely

        // Adjust based on player skills
        if (context) {
            // Add stealth skill bonus
            auto stealthIt = context->playerStats.skills.find("stealth");
            if (stealthIt != context->playerStats.skills.end()) {
                successChance += stealthIt->second * 3;
            }

            // Add pickpocket/theft skill bonus
            auto theftIt = context->playerStats.skills.find("pickpocket");
            if (theftIt != context->playerStats.skills.end()) {
                successChance += theftIt->second * 5;
            }
        }

        // Ensure reasonable bounds
        successChance = std::max(5, std::min(successChance, 95));

        // Random check
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(1, 100);

        return dis(gen) <= successChance;
    }
};

// Guard encounter node - handles guard interactions when wanted
class GuardEncounterNode : public CrimeSystemNode {
public:
    GuardEncounterNode(const std::string& name)
        : CrimeSystemNode(name)
    {
    }

    void onEnter(GameContext* context) override
    {
        CrimeLawContext* lawContext = getLawContext(context);
        std::string region = getCurrentRegion(context);

        // Check if actually wanted
        if (!lawContext->criminalRecord.isWanted(region)) {
            std::cout << "Guard looks at you but doesn't seem interested." << std::endl;
            return;
        }

        int bounty = lawContext->criminalRecord.getBounty(region);
        int guardResponse = determineGuardResponse(context);

        switch (guardResponse) {
        case 0: // ARREST
            std::cout << "\"Stop right there, criminal scum! You have committed crimes against our region.\"" << std::endl;
            std::cout << "\"Your bounty is " << bounty << " gold. Pay your fine or serve your sentence.\"" << std::endl;
            break;
        case 1: // ATTACK
            std::cout << "\"You're a wanted criminal! Surrender or die!\"" << std::endl;
            std::cout << "The guards draw their weapons and prepare to attack!" << std::endl;
            break;
        case 2: // FINE
            std::cout << "\"You've broken the law. Pay a fine of " << bounty << " gold.\"" << std::endl;
            break;
        case 3: // WARN
            std::cout << "\"We're keeping an eye on you. Don't cause any trouble.\"" << std::endl;
            break;
        default:
            std::cout << "The guard glances at you suspiciously but says nothing." << std::endl;
        }
    }

    int determineGuardResponse(GameContext* context)
    {
        CrimeLawContext* lawContext = getLawContext(context);
        std::string region = getCurrentRegion(context);

        int bounty = lawContext->criminalRecord.getBounty(region);
        int criminalRep = lawContext->criminalRecord.getReputation(region);

        // Determine response based on bounty and reputation
        if (bounty > 1000 || criminalRep < -50) {
            return 1; // ATTACK for serious criminals
        } else if (bounty > 500 || criminalRep < -30) {
            return 0; // ARREST for moderate criminals
        } else if (bounty > 100) {
            return 2; // FINE for minor criminals
        } else {
            return 3; // WARN for petty criminals
        }
    }

    std::vector<TAAction> getAvailableActions() override
    {
        std::vector<TAAction> actions;

        actions.push_back({ "pay_bounty", "Pay bounty", []() -> TAInput {
                               return { "guard_response", { { "action", std::string("pay") } } };
                           } });

        actions.push_back({ "resist_arrest", "Resist arrest", []() -> TAInput {
                               return { "guard_response", { { "action", std::string("resist") } } };
                           } });

        actions.push_back({ "surrender", "Surrender", []() -> TAInput {
                               return { "guard_response", { { "action", std::string("surrender") } } };
                           } });

        actions.push_back({ "flee", "Attempt to flee", []() -> TAInput {
                               return { "guard_response", { { "action", std::string("flee") } } };
                           } });

        return actions;
    }

    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override
    {
        if (input.type == "guard_response") {
            std::string action = std::get<std::string>(input.parameters.at("action"));

            // Find the transition for this action
            for (const auto& rule : transitionRules) {
                if (rule.description.find(action) != std::string::npos) {
                    outNextNode = rule.targetNode;
                    return true;
                }
            }
        }

        return CrimeSystemNode::evaluateTransition(input, outNextNode);
    }
};

// Jail node - handles jail sentences
class JailNode : public CrimeSystemNode {
public:
    JailNode(const std::string& name)
        : CrimeSystemNode(name)
    {
    }

    void onEnter(GameContext* context) override
    {
        CrimeLawContext* lawContext = getLawContext(context);
        std::string region = getCurrentRegion(context);

        // Calculate sentence if not already set
        if (lawContext->jailSentencesByRegion[region] <= 0) {
            lawContext->jailSentencesByRegion[region] = lawContext->calculateJailSentence(region);
        }

        int sentence = lawContext->jailSentencesByRegion[region];

        // Setup jail state
        lawContext->currentJailRegion = region;
        lawContext->currentJailDays = sentence;
        lawContext->inJail = true;

        // Confiscate stolen items
        confiscateItems(context);

        std::cout << "You've been thrown in jail in " << region << " for " << sentence << " days." << std::endl;
        std::cout << "Your stolen items have been confiscated." << std::endl;
    }

    void confiscateItems(GameContext* context)
    {
        if (!context)
            return;

        CrimeLawContext* lawContext = getLawContext(context);
        lawContext->confiscatedItems = Inventory(); // Clear previous

        // Copy stolen items to confiscated inventory and remove from player
        std::vector<Item> stolenItems;
        for (const auto& item : context->playerInventory.items) {
            if (item.type == "stolen") {
                lawContext->confiscatedItems.addItem(item);
                stolenItems.push_back(item);
            }
        }

        // Remove from player inventory
        for (const auto& item : stolenItems) {
            context->playerInventory.removeItem(item.id, item.quantity);
        }
    }

    std::vector<TAAction> getAvailableActions() override
    {
        std::vector<TAAction> actions;

        actions.push_back({ "serve_time", "Serve your sentence", []() -> TAInput {
                               return { "jail_action", { { "action", std::string("serve") } } };
                           } });

        actions.push_back({ "attempt_escape", "Attempt to escape", []() -> TAInput {
                               return { "jail_action", { { "action", std::string("escape") } } };
                           } });

        return actions;
    }

    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override
    {
        if (input.type == "jail_action") {
            std::string action = std::get<std::string>(input.parameters.at("action"));

            if (action == "serve") {
                serveTime(nullptr); // Context not needed here

                // Find the transition for serving time
                for (const auto& rule : transitionRules) {
                    if (rule.description.find("serve") != std::string::npos) {
                        outNextNode = rule.targetNode;
                        return true;
                    }
                }
            } else if (action == "escape") {
                bool escaped = attemptEscape(nullptr); // Context not needed

                // Find appropriate transition based on escape success
                for (const auto& rule : transitionRules) {
                    if (escaped && rule.description.find("escape_success") != std::string::npos) {
                        outNextNode = rule.targetNode;
                        return true;
                    } else if (!escaped && rule.description.find("escape_failure") != std::string::npos) {
                        outNextNode = rule.targetNode;
                        return true;
                    }
                }
            }
        }

        return CrimeSystemNode::evaluateTransition(input, outNextNode);
    }

    void serveTime(GameContext* context)
    {
        CrimeLawContext* lawContext = getLawContext(context);

        // Skip time forward
        if (context) {
            // This would integrate with your time system
            // For example, advancing the day by the jail sentence
            // context->worldState.advanceDay(lawContext->currentJailDays);
            std::cout << "Time passes... " << lawContext->currentJailDays << " days later." << std::endl;
        }

        // Clear criminal record for this region
        lawContext->criminalRecord.serveJailSentence(lawContext->currentJailRegion, lawContext->currentJailDays);

        // Reset jail state
        lawContext->inJail = false;
        lawContext->currentJailDays = 0;
        lawContext->jailSentencesByRegion[lawContext->currentJailRegion] = 0;

        std::cout << "You've served your sentence and are released from jail." << std::endl;
        std::cout << "Your criminal record in " << lawContext->currentJailRegion << " has been cleared." << std::endl;
    }

    bool attemptEscape(GameContext* context)
    {
        // Chance to escape based on skills
        int escapeChance = 20; // Base 20% chance

        if (context) {
            // Add stealth skill bonus
            auto stealthIt = context->playerStats.skills.find("stealth");
            if (stealthIt != context->playerStats.skills.end()) {
                escapeChance += stealthIt->second * 2;
            }

            // Add lockpicking skill bonus
            auto lockpickIt = context->playerStats.skills.find("lockpicking");
            if (lockpickIt != context->playerStats.skills.end()) {
                escapeChance += lockpickIt->second * 3;
            }
        }

        // Longer sentences are harder to escape from
        CrimeLawContext* lawContext = getLawContext(context);
        escapeChance -= lawContext->currentJailDays / 10;

        // Ensure reasonable bounds
        escapeChance = std::max(5, std::min(escapeChance, 90));

        // Random check
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(1, 100);

        bool escaped = dis(gen) <= escapeChance;

        if (escaped) {
            std::cout << "You've successfully escaped from jail!" << std::endl;

            // Update record - escaping increases bounty and makes guards more aggressive
            CrimeLawContext* lawContext = getLawContext(context);

            // Add prison break crime
            CrimeRecord breakout("prison_break", lawContext->currentJailRegion, "Jail", true, 8);
            lawContext->criminalRecord.addCrime(breakout);

            // Reset jail state but keep criminal record
            lawContext->inJail = false;
            lawContext->currentJailDays = 0;

            // Guards will now be more aggressive
            lawContext->guardSuspicion[lawContext->currentJailRegion] = 100;
        } else {
            std::cout << "Your escape attempt has failed! The guards caught you." << std::endl;

            // Increase sentence for attempted escape
            CrimeLawContext* lawContext = getLawContext(context);
            lawContext->currentJailDays += 10;
            lawContext->jailSentencesByRegion[lawContext->currentJailRegion] += 10;

            std::cout << "Your sentence has been extended by 10 days." << std::endl;
        }

        return escaped;
    }
};

// Bounty payment node
class BountyPaymentNode : public CrimeSystemNode {
public:
    BountyPaymentNode(const std::string& name)
        : CrimeSystemNode(name)
    {
    }

    void onEnter(GameContext* context) override
    {
        CrimeLawContext* lawContext = getLawContext(context);
        std::string region = getCurrentRegion(context);

        int bounty = lawContext->criminalRecord.getBounty(region);

        std::cout << "You are at the bounty office in " << region << "." << std::endl;
        std::cout << "Your current bounty is " << bounty << " gold." << std::endl;

        // Show list of crimes
        auto crimes = lawContext->criminalRecord.getUnpaidCrimes(region);
        if (!crimes.empty()) {
            std::cout << "\nYour unpaid crimes in this region:" << std::endl;
            for (size_t i = 0; i < crimes.size(); i++) {
                std::cout << i + 1 << ". " << crimes[i].getDescription() << std::endl;
            }
        }
    }

    std::vector<TAAction> getAvailableActions() override
    {
        std::vector<TAAction> actions;

        actions.push_back({ "pay_full", "Pay full bounty", []() -> TAInput {
                               return { "bounty_action", { { "action", std::string("pay_full") } } };
                           } });

        actions.push_back({ "negotiate", "Attempt to negotiate", []() -> TAInput {
                               return { "bounty_action", { { "action", std::string("negotiate") } } };
                           } });

        actions.push_back({ "leave", "Leave bounty office", []() -> TAInput {
                               return { "bounty_action", { { "action", std::string("leave") } } };
                           } });

        return actions;
    }

    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override
    {
        if (input.type == "bounty_action") {
            std::string action = std::get<std::string>(input.parameters.at("action"));

            if (action == "pay_full") {
                bool paid = payFullBounty(nullptr); // Context would be passed in real implementation

                // Find the right transition
                for (const auto& rule : transitionRules) {
                    if ((paid && rule.description.find("payment_success") != std::string::npos) || (!paid && rule.description.find("payment_failure") != std::string::npos)) {
                        outNextNode = rule.targetNode;
                        return true;
                    }
                }
            } else if (action == "negotiate") {
                bool success = negotiateBounty(nullptr); // Context would be passed

                // Find the right transition
                for (const auto& rule : transitionRules) {
                    if ((success && rule.description.find("negotiate_success") != std::string::npos) || (!success && rule.description.find("negotiate_failure") != std::string::npos)) {
                        outNextNode = rule.targetNode;
                        return true;
                    }
                }
            } else if (action == "leave") {
                // Find the exit transition
                for (const auto& rule : transitionRules) {
                    if (rule.description.find("leave") != std::string::npos) {
                        outNextNode = rule.targetNode;
                        return true;
                    }
                }
            }
        }

        return CrimeSystemNode::evaluateTransition(input, outNextNode);
    }

    bool payFullBounty(GameContext* context)
    {
        CrimeLawContext* lawContext = getLawContext(context);
        std::string region = getCurrentRegion(context);

        int bounty = lawContext->criminalRecord.getBounty(region);

        // Check if player has enough gold
        bool hasEnoughGold = true; // In real implementation, check player gold
        if (context) {
            // For example:
            // hasEnoughGold = (context->playerGold >= bounty);
        }

        if (hasEnoughGold) {
            // Deduct gold
            if (context) {
                // context->playerGold -= bounty;
            }

            // Clear bounty
            lawContext->criminalRecord.payBounty(region, bounty, context);

            std::cout << "You've paid your bounty of " << bounty << " gold." << std::endl;
            std::cout << "Your criminal record in " << region << " has been cleared." << std::endl;
            return true;
        } else {
            std::cout << "You don't have enough gold to pay your bounty." << std::endl;
            return false;
        }
    }

    bool negotiateBounty(GameContext* context)
    {
        CrimeLawContext* lawContext = getLawContext(context);
        std::string region = getCurrentRegion(context);

        // Negotiate based on speech/charisma skill
        int negotiateChance = 30; // Base 30% chance

        if (context) {
            // Add speech skill bonus
            auto speechIt = context->playerStats.skills.find("speech");
            if (speechIt != context->playerStats.skills.end()) {
                negotiateChance += speechIt->second * 3;
            }

            // Add charisma bonus
            negotiateChance += (context->playerStats.charisma - 10) * 2;
        }

        // Criminal reputation affects negotiation
        int criminalRep = lawContext->criminalRecord.getReputation(region);
        negotiateChance += criminalRep / 5; // Better reputation helps

        // Ensure reasonable bounds
        negotiateChance = std::max(5, std::min(negotiateChance, 90));

        // Random check
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(1, 100);

        bool success = dis(gen) <= negotiateChance;

        if (success) {
            int originalBounty = lawContext->criminalRecord.getBounty(region);
            int discountedBounty = originalBounty * 0.7; // 30% discount

            std::cout << "Through skillful negotiation, you've reduced your bounty from "
                      << originalBounty << " to " << discountedBounty << " gold." << std::endl;

            // Check if player has enough gold for discounted bounty
            bool hasEnoughGold = true; // In real implementation, check player gold
            if (context) {
                // For example:
                // hasEnoughGold = (context->playerGold >= discountedBounty);
            }

            if (hasEnoughGold) {
                // Deduct gold
                if (context) {
                    // context->playerGold -= discountedBounty;
                }

                // Clear bounty
                lawContext->criminalRecord.payBounty(region, discountedBounty, context);

                std::cout << "You've paid your reduced bounty of " << discountedBounty << " gold." << std::endl;
                std::cout << "Your criminal record in " << region << " has been cleared." << std::endl;
                return true;
            } else {
                std::cout << "Even with the discount, you don't have enough gold." << std::endl;
                return false;
            }
        } else {
            std::cout << "The official isn't impressed by your negotiation attempt." << std::endl;
            std::cout << "\"Pay the full bounty or face the consequences!\"" << std::endl;
            return false;
        }
    }
};

// Pickpocketing node - For stealing from NPCs
class PickpocketNode : public CrimeSystemNode {
public:
    PickpocketNode(const std::string& name)
        : CrimeSystemNode(name)
    {
    }

    void onEnter(GameContext* context) override
    {
        std::cout << "You scan the area for potential pickpocketing targets..." << std::endl;

        // Get list of NPCs in current location
        std::vector<std::string> targets = getPickpocketTargets(context);

        if (targets.empty()) {
            std::cout << "There's no one suitable to pickpocket here." << std::endl;
            return;
        }

        std::cout << "Potential targets:" << std::endl;
        for (size_t i = 0; i < targets.size(); i++) {
            std::cout << i + 1 << ". " << targets[i] << std::endl;
        }
    }

    std::vector<std::string> getPickpocketTargets(GameContext* context)
    {
        // In a real implementation, this would get NPCs from current location
        // Here we'll just return some sample targets
        return { "Wealthy merchant", "Town guard", "Drunk nobleman", "Peasant farmer" };
    }

    std::vector<TAAction> getAvailableActions() override
    {
        std::vector<TAAction> actions;

        // In a real implementation, this would be generated from actual NPCs
        actions.push_back({ "pickpocket_1", "Pickpocket wealthy merchant", []() -> TAInput {
                               return { "pickpocket_action", { { "target", std::string("merchant") }, { "difficulty", 7 } } };
                           } });

        actions.push_back({ "pickpocket_2", "Pickpocket town guard", []() -> TAInput {
                               return { "pickpocket_action", { { "target", std::string("guard") }, { "difficulty", 9 } } };
                           } });

        actions.push_back({ "pickpocket_3", "Pickpocket drunk nobleman", []() -> TAInput {
                               return { "pickpocket_action", { { "target", std::string("nobleman") }, { "difficulty", 4 } } };
                           } });

        actions.push_back({ "pickpocket_4", "Pickpocket peasant farmer", []() -> TAInput {
                               return { "pickpocket_action", { { "target", std::string("farmer") }, { "difficulty", 3 } } };
                           } });

        actions.push_back({ "cancel", "Cancel pickpocketing", []() -> TAInput {
                               return { "pickpocket_action", { { "target", std::string("cancel") } } };
                           } });

        return actions;
    }

    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override
    {
        if (input.type == "pickpocket_action") {
            std::string target = std::get<std::string>(input.parameters.at("target"));

            if (target == "cancel") {
                // Find the cancel transition
                for (const auto& rule : transitionRules) {
                    if (rule.description.find("cancel") != std::string::npos) {
                        outNextNode = rule.targetNode;
                        return true;
                    }
                }
            } else {
                // Attempt pickpocketing
                int difficulty = std::get<int>(input.parameters.at("difficulty"));
                attemptPickpocket(nullptr, target, difficulty); // Context would be passed

                // Use the success/failure transition
                // For example, if we want to stay in the same node:
                outNextNode = this;
                return true;
            }
        }

        return CrimeSystemNode::evaluateTransition(input, outNextNode);
    }

    void attemptPickpocket(GameContext* context, const std::string& target, int difficulty)
    {
        // Base chance based on difficulty
        int successChance = 80 - (difficulty * 8); // 1-10 difficulty scale

        // Adjust based on player skills
        if (context) {
            // Add stealth skill bonus
            auto stealthIt = context->playerStats.skills.find("stealth");
            if (stealthIt != context->playerStats.skills.end()) {
                successChance += stealthIt->second * 2;
            }

            // Add pickpocket skill bonus
            auto pickpocketIt = context->playerStats.skills.find("pickpocket");
            if (pickpocketIt != context->playerStats.skills.end()) {
                successChance += pickpocketIt->second * 4;
            }
        }

        // Ensure reasonable bounds
        successChance = std::max(5, std::min(successChance, 95));

        // Random check
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(1, 100);

        bool success = dis(gen) <= successChance;

        if (success) {
            std::cout << "You successfully pickpocket the " << target << "!" << std::endl;

            // Determine loot based on target
            int gold = 0;
            std::string itemLoot = "";

            if (target == "merchant") {
                gold = 20 + (rand() % 30);
                itemLoot = "merchant_goods";
            } else if (target == "guard") {
                gold = 10 + (rand() % 15);
                itemLoot = "guard_key";
            } else if (target == "nobleman") {
                gold = 50 + (rand() % 100);
                itemLoot = "jeweled_ring";
            } else if (target == "farmer") {
                gold = 5 + (rand() % 10);
                itemLoot = "food_rations";
            }

            std::cout << "You find " << gold << " gold";
            if (!itemLoot.empty()) {
                std::cout << " and a " << itemLoot;

                // Add to inventory
                if (context) {
                    context->playerInventory.addItem({ itemLoot, itemLoot, "stolen", 0, 1 });
                }
            }
            std::cout << "." << std::endl;

            // Record the crime
            commitCrime(context, CrimeType::PICKPOCKETING, difficulty / 2, 10);

            // Improve skill with successful pickpocketing
            if (context) {
                if (rand() % 100 < 30) { // 30% chance to improve
                    context->playerStats.improveSkill("pickpocket", 1);
                    std::cout << "Your pickpocketing skill has improved!" << std::endl;
                }
            }
        } else {
            std::cout << "You fail to pickpocket the " << target << "!" << std::endl;

            // Determine if caught
            int detectionChance = difficulty * 10;

            // Random check for detection
            bool caught = dis(gen) <= detectionChance;

            if (caught) {
                std::cout << "\"Hey! This person is trying to rob me!\"" << std::endl;
                std::cout << "You've been caught pickpocketing!" << std::endl;

                // Record the crime as witnessed
                commitCrime(context, CrimeType::PICKPOCKETING, difficulty / 2, -20);

                CrimeLawContext* lawContext = getLawContext(context);
                std::string region = getCurrentRegion(context);
                lawContext->guardAlerted[region] = true;
            } else {
                std::cout << "Fortunately, no one noticed your attempt." << std::endl;
            }
        }
    }
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
    TheftExecutionNode* smallTheftNode;
    TheftExecutionNode* mediumTheftNode;
    TheftExecutionNode* largeTheftNode;

public:
    CrimeLawSystem(TAController* controller)
        : controller(controller)
    {
        setupNodes();
        setupTransitions();
        registerSystem();
    }

    void setupNodes()
    {
        // Create all nodes
        criminalStatusNode = dynamic_cast<CrimeSystemNode*>(
            controller->createNode<CrimeSystemNode>("CriminalStatus"));

        theftNode = dynamic_cast<TheftNode*>(
            controller->createNode<TheftNode>("TheftPlanning"));

        guardNode = dynamic_cast<GuardEncounterNode*>(
            controller->createNode<GuardEncounterNode>("GuardEncounter"));

        jailNode = dynamic_cast<JailNode*>(
            controller->createNode<JailNode>("Jail"));

        bountyNode = dynamic_cast<BountyPaymentNode*>(
            controller->createNode<BountyPaymentNode>("BountyPayment"));

        pickpocketNode = dynamic_cast<PickpocketNode*>(
            controller->createNode<PickpocketNode>("Pickpocket"));

        smallTheftNode = dynamic_cast<TheftExecutionNode*>(
            controller->createNode<TheftExecutionNode>("SmallTheft", "small valuables", 20, 2));

        mediumTheftNode = dynamic_cast<TheftExecutionNode*>(
            controller->createNode<TheftExecutionNode>("MediumTheft", "merchant goods", 50, 5));

        largeTheftNode = dynamic_cast<TheftExecutionNode*>(
            controller->createNode<TheftExecutionNode>("LargeTheft", "valuable artifacts", 200, 8));
    }

    void setupTransitions()
    {
        // From criminal status to various crime options
        criminalStatusNode->addTransition(
            [](const TAInput& input) {
                return input.type == "crime_choice" && std::get<std::string>(input.parameters.at("choice")) == "theft";
            },
            theftNode, "Plan theft");

        criminalStatusNode->addTransition(
            [](const TAInput& input) {
                return input.type == "crime_choice" && std::get<std::string>(input.parameters.at("choice")) == "pickpocket";
            },
            pickpocketNode, "Attempt pickpocketing");

        criminalStatusNode->addTransition(
            [](const TAInput& input) {
                return input.type == "crime_choice" && std::get<std::string>(input.parameters.at("choice")) == "pay_bounty";
            },
            bountyNode, "Pay bounty");

        // From theft planning to execution
        theftNode->addTransition(
            [](const TAInput& input) {
                return input.type == "theft_action" && std::get<std::string>(input.parameters.at("target")) == "small";
            },
            smallTheftNode, "Steal small items");

        theftNode->addTransition(
            [](const TAInput& input) {
                return input.type == "theft_action" && std::get<std::string>(input.parameters.at("target")) == "medium";
            },
            mediumTheftNode, "Steal medium items");

        theftNode->addTransition(
            [](const TAInput& input) {
                return input.type == "theft_action" && std::get<std::string>(input.parameters.at("target")) == "large";
            },
            largeTheftNode, "Steal large items");

        theftNode->addTransition(
            [](const TAInput& input) {
                return input.type == "theft_action" && std::get<std::string>(input.parameters.at("target")) == "cancel";
            },
            criminalStatusNode, "Cancel theft");

        // From theft execution back to status
        smallTheftNode->addTransition(
            [](const TAInput& input) { return true; },
            criminalStatusNode, "Return after theft");

        mediumTheftNode->addTransition(
            [](const TAInput& input) { return true; },
            criminalStatusNode, "Return after theft");

        largeTheftNode->addTransition(
            [](const TAInput& input) { return true; },
            criminalStatusNode, "Return after theft");

        // Guard encounters
        guardNode->addTransition(
            [](const TAInput& input) {
                return input.type == "guard_response" && std::get<std::string>(input.parameters.at("action")) == "pay";
            },
            bountyNode, "Pay bounty");

        guardNode->addTransition(
            [](const TAInput& input) {
                return input.type == "guard_response" && std::get<std::string>(input.parameters.at("action")) == "surrender";
            },
            jailNode, "Go to jail");

        guardNode->addTransition(
            [](const TAInput& input) {
                return input.type == "guard_response" && std::get<std::string>(input.parameters.at("action")) == "resist";
            },
            criminalStatusNode, "Combat with guards"); // Would link to combat system

        guardNode->addTransition(
            [](const TAInput& input) {
                return input.type == "guard_response" && std::get<std::string>(input.parameters.at("action")) == "flee";
            },
            criminalStatusNode, "Attempt to flee"); // Would have chance to escape

        // Jail transitions
        jailNode->addTransition(
            [](const TAInput& input) {
                return input.type == "jail_action" && std::get<std::string>(input.parameters.at("action")) == "serve";
            },
            criminalStatusNode, "Serve jail time");

        jailNode->addTransition(
            [](const TAInput& input) {
                return input.type == "jail_action" && std::get<std::string>(input.parameters.at("action")) == "escape";
            },
            criminalStatusNode, "escape_success");

        jailNode->addTransition(
            [](const TAInput& input) {
                return input.type == "jail_action" && std::get<std::string>(input.parameters.at("action")) == "escape";
            },
            jailNode, "escape_failure");

        // Bounty office transitions
        bountyNode->addTransition(
            [](const TAInput& input) {
                return input.type == "bounty_action" && std::get<std::string>(input.parameters.at("action")) == "pay_full";
            },
            criminalStatusNode, "payment_success");

        bountyNode->addTransition(
            [](const TAInput& input) {
                return input.type == "bounty_action" && std::get<std::string>(input.parameters.at("action")) == "pay_full";
            },
            bountyNode, "payment_failure");

        bountyNode->addTransition(
            [](const TAInput& input) {
                return input.type == "bounty_action" && std::get<std::string>(input.parameters.at("action")) == "negotiate";
            },
            criminalStatusNode, "negotiate_success");

        bountyNode->addTransition(
            [](const TAInput& input) {
                return input.type == "bounty_action" && std::get<std::string>(input.parameters.at("action")) == "negotiate";
            },
            bountyNode, "negotiate_failure");

        bountyNode->addTransition(
            [](const TAInput& input) {
                return input.type == "bounty_action" && std::get<std::string>(input.parameters.at("action")) == "leave";
            },
            criminalStatusNode, "leave");

        // Pickpocket transitions
        pickpocketNode->addTransition(
            [](const TAInput& input) {
                return input.type == "pickpocket_action" && std::get<std::string>(input.parameters.at("target")) == "cancel";
            },
            criminalStatusNode, "cancel");
    }

    void registerSystem()
    {
        // Register the crime system with the controller
        controller->setSystemRoot("CrimeLawSystem", criminalStatusNode);

        // Set up guard encounter triggering
        setupGuardEncounters();
    }

    void setupGuardEncounters()
    {
        // This would integrate with your world system to trigger guard encounters
        // when the player enters certain locations and has a wanted status

        // For example, you might add a condition to location transitions that
        // checks if the player is wanted in that region, and if so, redirects
        // to the guard encounter node
    }

    // Method to be called by game to commit a crime programmatically
    void commitCrime(GameContext* context, const std::string& crimeType, int severity, const std::string& region, const std::string& location)
    {
        // Get the law context
        CrimeLawContext* lawContext = getLawContextFromController();

        // Determine if witnessed based on location and other factors
        bool witnessed = (rand() % 100) < 50; // 50% chance for demonstration

        // Create and record the crime
        CrimeRecord crime(crimeType, region, location, witnessed, severity);
        lawContext->criminalRecord.addCrime(crime);

        std::cout << "Crime committed: " << crime.getDescription() << std::endl;

        if (witnessed) {
            lawContext->guardAlerted[region] = true;
        }
    }

    CrimeLawContext* getLawContextFromController()
    {
        // In a real implementation, this would get the context from the controller
        // Here we're simplifying by creating a static instance
        static CrimeLawContext lawContext;
        return &lawContext;
    }
};

// Main function to test the crime system
int main()
{
    std::cout << "___ Starting Crime & Law System ___" << std::endl;

    // Create controller
    TAController controller;

    // Create and set up crime system
    CrimeLawSystem crimeSystem(&controller);

    // Test the system
    std::cout << "\n=== CRIME SYSTEM DEMONSTRATION ===\n"
              << std::endl;

    // Initialize
    controller.processInput("CrimeLawSystem", {});

    // Test theft
    TAInput theftInput = {
        "crime_choice",
        { { "choice", std::string("theft") } }
    };
    controller.processInput("CrimeLawSystem", theftInput);

    // Choose small theft
    TAInput smallTheftInput = {
        "theft_action",
        { { "target", std::string("small") } }
    };
    controller.processInput("CrimeLawSystem", smallTheftInput);

    // Test pickpocketing
    TAInput pickpocketInput = {
        "crime_choice",
        { { "choice", std::string("pickpocket") } }
    };
    controller.processInput("CrimeLawSystem", pickpocketInput);

    // Choose a target
    TAInput pickTargetInput = {
        "pickpocket_action",
        { { "target", std::string("nobleman") }, { "difficulty", 4 } }
    };
    controller.processInput("CrimeLawSystem", pickTargetInput);

    // Test paying bounty
    TAInput bountyInput = {
        "crime_choice",
        { { "choice", std::string("pay_bounty") } }
    };
    controller.processInput("CrimeLawSystem", bountyInput);

    // Pay full bounty
    TAInput payInput = {
        "bounty_action",
        { { "action", std::string("pay_full") } }
    };
    controller.processInput("CrimeLawSystem", payInput);

    std::cout << "\nCrime System demonstration complete." << std::endl;

    return 0;
}