// System_ReligionDeity_JSON.cpp

#include "System_ReligionDeity_JSON.hpp"

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
class QuestNode;
class DialogueNode;
class SkillNode;
class ClassNode;
class CraftingNode;
class LocationNode;
class TimeNode;
struct NodeID;
struct TAInput;
struct TAAction;
struct TATransitionRule;
struct CharacterStats;
struct WorldState;
struct GameContext;
struct Item;

// New forward declarations for Religion System
class DeityNode;
class TempleNode;
class RitualNode;
class PrayerNode;
class BlessingNode;
class ReligiousQuestNode;

//----------------------------------------
// RELIGION SYSTEM EXTENSIONS
//----------------------------------------

// Add religious fields to CharacterStats
struct ReligiousStats {
    std::map<std::string, int> deityFavor; // Favor level with each deity
    std::map<std::string, int> deityDevotion; // Devotion points spent
    std::map<std::string, bool> deityAlignment; // If true, player is aligned with deity
    std::string primaryDeity; // Current primary deity
    std::set<std::string> completedRituals; // Completed ritual IDs
    std::set<std::string> activeBlessing; // Active blessing effects
    std::map<std::string, int> blessingDuration; // Remaining time on blessings

    ReligiousStats()
    {
        primaryDeity = ""; // No primary deity initially
    }

    void changeFavor(const std::string& deity, int amount)
    {
        if (deityFavor.find(deity) != deityFavor.end()) {
            deityFavor[deity] += amount;

            // Favor is capped between -100 and 100
            deityFavor[deity] = std::max(-100, std::min(100, deityFavor[deity]));

            // Check for opposing deity effects
            if (amount > 0) {
                // The opposing deities will be handled by the JSON data now
                // We'll look these up when processing deity actions
            }
        }
    }

    void addDevotion(const std::string& deity, int points)
    {
        if (deityDevotion.find(deity) != deityDevotion.end()) {
            deityDevotion[deity] += points;
            // Every 10 devotion points adds 1 favor
            changeFavor(deity, points / 10);
        }
    }

    bool hasMinimumFavor(const std::string& deity, int minimumFavor) const
    {
        auto it = deityFavor.find(deity);
        return it != deityFavor.end() && it->second >= minimumFavor;
    }

    bool hasBlessingActive(const std::string& blessing) const
    {
        return activeBlessing.find(blessing) != activeBlessing.end();
    }

    void addBlessing(const std::string& blessing, int duration)
    {
        activeBlessing.insert(blessing);
        blessingDuration[blessing] = duration;
    }

    void removeBlessing(const std::string& blessing)
    {
        activeBlessing.erase(blessing);
        blessingDuration.erase(blessing);
    }

    void updateBlessings()
    {
        std::set<std::string> expiredBlessings;

        // Decrement duration and identify expired blessings
        for (auto& [blessing, duration] : blessingDuration) {
            duration--;
            if (duration <= 0) {
                expiredBlessings.insert(blessing);
            }
        }

        // Remove expired blessings
        for (const auto& blessing : expiredBlessings) {
            removeBlessing(blessing);
        }
    }

    void setPrimaryDeity(const std::string& deity)
    {
        if (deityFavor.find(deity) != deityFavor.end()) {
            // Remove alignment from current primary deity
            if (!primaryDeity.empty()) {
                deityAlignment[primaryDeity] = false;
            }

            primaryDeity = deity;
            deityAlignment[deity] = true;

            // Give an initial favor boost when choosing a primary deity
            changeFavor(deity, 10);
        }
    }

    bool hasCompletedRitual(const std::string& ritualId) const
    {
        return completedRituals.find(ritualId) != completedRituals.end();
    }

    void markRitualCompleted(const std::string& ritualId)
    {
        completedRituals.insert(ritualId);
    }

    void initializeDeities(const std::vector<std::string>& deityIds)
    {
        // Initialize with neutral favor for all deities
        for (const auto& id : deityIds) {
            deityFavor[id] = 0;
            deityDevotion[id] = 0;
            deityAlignment[id] = false;
        }
    }
};

// Extension to GameContext to include religious stats
struct ReligiousGameContext : public GameContext {
    ReligiousStats religiousStats;
    std::map<std::string, std::string> templeJournal; // Records temple visits and rituals
    std::map<int, std::string> holyDayCalendar; // Days of the year for holy celebrations
    std::map<int, std::string> holyDayDeities; // Map days to deity IDs

    ReligiousGameContext()
        : GameContext()
    {
    }

    void loadHolyDays(const json& holyDayData)
    {
        for (auto& [dayStr, holyDay] : holyDayData.items()) {
            int day = std::stoi(dayStr);
            holyDayCalendar[day] = holyDay["name"];
            holyDayDeities[day] = holyDay["deityId"];
        }
    }

    int getCurrentDayOfYear() const
    {
        return (worldState.daysPassed % 365) + 1; // 1-365
    }

    bool isHolyDay() const
    {
        return holyDayCalendar.find(getCurrentDayOfYear()) != holyDayCalendar.end();
    }

    std::string getCurrentHolyDay() const
    {
        auto it = holyDayCalendar.find(getCurrentDayOfYear());
        if (it != holyDayCalendar.end()) {
            return it->second;
        }
        return "";
    }

    std::string getDeityOfCurrentHolyDay() const
    {
        auto it = holyDayDeities.find(getCurrentDayOfYear());
        if (it != holyDayDeities.end()) {
            return it->second;
        }
        return "";
    }
};

// Base class for deity-specific nodes
class DeityNode : public TANode {
public:
    std::string deityId;
    std::string deityName;
    std::string deityTitle;
    std::string deityDescription;
    std::string deityDomain;
    std::string alignmentRequirement; // "good", "neutral", "evil"

    // Opposing deities
    std::vector<std::string> opposingDeities;

    // Favored/disfavored actions
    struct DeityAlignment {
        std::string action;
        int favorChange;
        std::string description;
    };
    std::vector<DeityAlignment> favoredActions;
    std::vector<DeityAlignment> disfavoredActions;

    // Blessings this deity can grant
    std::vector<BlessingNode*> availableBlessings;

    // Temples dedicated to this deity
    std::vector<TempleNode*> temples;

    DeityNode(const std::string& name, const std::string& id, const std::string& title)
        : TANode(name)
        , deityId(id)
        , deityName(name)
        , deityTitle(title)
    {
    }

    void loadFromJson(const json& deityData)
    {
        deityDescription = deityData["description"];
        deityDomain = deityData["domain"];
        alignmentRequirement = deityData["alignment"];

        // Load opposing deities
        opposingDeities.clear();
        for (const auto& opposing : deityData["opposingDeities"]) {
            opposingDeities.push_back(opposing);
        }

        // Load favored actions
        favoredActions.clear();
        for (const auto& action : deityData["favoredActions"]) {
            favoredActions.push_back({ action["action"],
                action["favorChange"],
                action["description"] });
        }

        // Load disfavored actions
        disfavoredActions.clear();
        for (const auto& action : deityData["disfavoredActions"]) {
            disfavoredActions.push_back({ action["action"],
                action["favorChange"],
                action["description"] });
        }
    }

    void onEnter(ReligiousGameContext* context)
    {
        std::cout << "=== " << deityName << ", " << deityTitle << " ===" << std::endl;
        std::cout << deityDescription << std::endl;
        std::cout << "Domain: " << deityDomain << std::endl;
        std::cout << "Alignment: " << alignmentRequirement << std::endl;

        if (context) {
            int favor = context->religiousStats.deityFavor[deityId];
            int devotion = context->religiousStats.deityDevotion[deityId];
            bool isPrimary = (context->religiousStats.primaryDeity == deityId);

            std::cout << "\nYour standing with " << deityName << ":" << std::endl;
            std::cout << "Favor: " << favor << " (" << getFavorLevel(favor) << ")" << std::endl;
            std::cout << "Devotion: " << devotion << std::endl;

            if (isPrimary) {
                std::cout << "This is your primary deity." << std::endl;
            }

            if (isHolyDay(context) && context->religiousStats.hasMinimumFavor(deityId, -10)) {
                std::cout << "\nToday is a holy day for " << deityName << "!" << std::endl;
                std::cout << "Special blessings and rituals are available." << std::endl;
            }
        }

        std::cout << "\nTemples:" << std::endl;
        if (temples.empty()) {
            std::cout << "No known temples." << std::endl;
        } else {
            for (const auto& temple : temples) {
                std::cout << "- " << temple->templeName << " in " << temple->templeLocation << std::endl;
            }
        }
    }

    bool isHolyDay(ReligiousGameContext* context) const
    {
        if (!context)
            return false;
        return context->getDeityOfCurrentHolyDay() == deityId;
    }

    std::string getFavorLevel(int favor) const
    {
        if (favor >= 90)
            return "Exalted";
        if (favor >= 75)
            return "Revered";
        if (favor >= 50)
            return "Honored";
        if (favor >= 25)
            return "Friendly";
        if (favor >= 10)
            return "Liked";
        if (favor >= -10)
            return "Neutral";
        if (favor >= -25)
            return "Disliked";
        if (favor >= -50)
            return "Unfriendly";
        if (favor >= -75)
            return "Hostile";
        return "Hated";
    }

    bool canGrantBlessing(ReligiousGameContext* context, const std::string& blessingId) const
    {
        if (!context)
            return false;

        // Check if player has enough favor
        int favor = context->religiousStats.deityFavor[deityId];

        // Basic blessings require at least 10 favor
        if (favor < 10)
            return false;

        // Greater blessings require at least 50 favor
        if (blessingId.find("greater_") != std::string::npos && favor < 50)
            return false;

        // Divine blessings require at least 75 favor and primary deity status
        if (blessingId.find("divine_") != std::string::npos) {
            if (favor < 75)
                return false;
            if (context->religiousStats.primaryDeity != deityId)
                return false;
        }

        return true;
    }

    virtual void processDevotionAction(ReligiousGameContext* context, const std::string& actionType)
    {
        if (!context)
            return;

        // Process action based on deity's favored/disfavored actions
        for (const auto& action : favoredActions) {
            if (action.action == actionType) {
                context->religiousStats.changeFavor(deityId, action.favorChange);
                context->religiousStats.addDevotion(deityId, action.favorChange > 0 ? action.favorChange : 0);
                std::cout << action.description << std::endl;
                std::cout << "Your favor with " << deityName << " has increased by " << action.favorChange << "." << std::endl;

                // Apply opposing deity effects
                for (const auto& opposingDeity : opposingDeities) {
                    int opposingPenalty = action.favorChange / 2;
                    if (opposingPenalty > 0) {
                        context->religiousStats.changeFavor(opposingDeity, -opposingPenalty);
                        std::cout << "Your favor with the opposing deity has decreased." << std::endl;
                    }
                }
                return;
            }
        }

        for (const auto& action : disfavoredActions) {
            if (action.action == actionType) {
                context->religiousStats.changeFavor(deityId, action.favorChange);
                std::cout << action.description << std::endl;
                std::cout << "Your favor with " << deityName << " has decreased by " << -action.favorChange << "." << std::endl;
                return;
            }
        }
    }

    std::vector<TAAction> getAvailableActions() override
    {
        std::vector<TAAction> actions = TANode::getAvailableActions();

        // Add deity-specific actions
        actions.push_back({ "select_as_primary",
            "Select as primary deity",
            [this]() -> TAInput {
                return { "deity_action", { { "action", std::string("set_primary") }, { "deity", deityId } } };
            } });

        actions.push_back({ "pray_to_deity",
            "Pray to " + deityName,
            [this]() -> TAInput {
                return { "deity_action", { { "action", std::string("pray") }, { "deity", deityId } } };
            } });

        actions.push_back({ "view_blessings",
            "View available blessings",
            [this]() -> TAInput {
                return { "deity_action", { { "action", std::string("view_blessings") }, { "deity", deityId } } };
            } });

        actions.push_back({ "view_temples",
            "Find temples",
            [this]() -> TAInput {
                return { "deity_action", { { "action", std::string("view_temples") }, { "deity", deityId } } };
            } });

        actions.push_back({ "back_to_pantheon",
            "Return to pantheon view",
            [this]() -> TAInput {
                return { "deity_action", { { "action", std::string("back") } } };
            } });

        return actions;
    }

    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override
    {
        if (input.type == "deity_action") {
            std::string action = std::get<std::string>(input.parameters.at("action"));

            if (action == "set_primary") {
                // Stay on same node but update primary deity
                outNextNode = this;
                return true;
            } else if (action == "pray") {
                // Find prayer node if available
                for (TANode* child : childNodes) {
                    if (dynamic_cast<PrayerNode*>(child)) {
                        outNextNode = child;
                        return true;
                    }
                }
                // If no specific prayer node, stay on same node
                outNextNode = this;
                return true;
            } else if (action == "view_blessings") {
                // Find a blessing node to transition to
                if (!availableBlessings.empty()) {
                    outNextNode = availableBlessings[0]; // Go to first blessing view
                    return true;
                }
                // Stay on same node if no blessings
                outNextNode = this;
                return true;
            } else if (action == "view_temples") {
                // Find a temple to transition to
                if (!temples.empty()) {
                    outNextNode = temples[0]; // Go to first temple
                    return true;
                }
                // Stay on same node if no temples
                outNextNode = this;
                return true;
            } else if (action == "back") {
                // Return to parent/pantheon node
                for (const auto& rule : transitionRules) {
                    if (rule.description == "Return to pantheon") {
                        outNextNode = rule.targetNode;
                        return true;
                    }
                }
            }
        }

        return TANode::evaluateTransition(input, outNextNode);
    }
};

// Represents a temple location dedicated to a specific deity
class TempleNode : public TANode {
public:
    std::string templeId;
    std::string templeName;
    std::string templeLocation;
    std::string templeDescription;
    std::string deityId;

    // Temple personnel
    struct TemplePriest {
        std::string name;
        std::string title;
        std::string description;
        int rank; // 1-5, with 5 being highest
    };
    std::vector<TemplePriest> priests;

    // Available rituals at this temple
    std::vector<RitualNode*> availableRituals;

    // Offerings that can be made
    struct TempleOffering {
        std::string name;
        std::string itemId;
        int quantity;
        int favorReward;
        std::string description;
    };
    std::vector<TempleOffering> possibleOfferings;

    // Temple services
    bool providesBlessings;
    bool providesHealing;
    bool providesCurseRemoval;
    bool providesDivination;
    int serviceQuality; // 1-5, affects cost and effectiveness

    TempleNode(const std::string& id, const std::string& name, const std::string& location, const std::string& deity)
        : TANode(name)
        , templeId(id)
        , templeName(name)
        , templeLocation(location)
        , deityId(deity)
        , providesBlessings(true)
        , providesHealing(false)
        , providesCurseRemoval(false)
        , providesDivination(false)
        , serviceQuality(3)
    {
    }

    void loadFromJson(const json& templeData)
    {
        templeDescription = templeData["description"];
        providesBlessings = templeData["providesBlessings"];
        providesHealing = templeData["providesHealing"];
        providesCurseRemoval = templeData["providesCurseRemoval"];
        providesDivination = templeData["providesDivination"];
        serviceQuality = templeData["serviceQuality"];

        // Load priests
        priests.clear();
        for (const auto& priest : templeData["priests"]) {
            priests.push_back({ priest["name"],
                priest["title"],
                priest["description"],
                priest["rank"] });
        }

        // Load offerings
        possibleOfferings.clear();
        for (const auto& offering : templeData["offerings"]) {
            possibleOfferings.push_back({ offering["name"],
                offering["itemId"],
                offering["quantity"],
                offering["favorReward"],
                offering["description"] });
        }
    }

    void onEnter(ReligiousGameContext* context)
    {
        std::cout << "=== " << templeName << " ===" << std::endl;
        std::cout << "Location: " << templeLocation << std::endl;
        std::cout << templeDescription << std::endl;

        if (context) {
            DeityNode* deity = findDeityNode(context);
            if (deity) {
                std::cout << "Dedicated to: " << deity->deityName << ", " << deity->deityTitle << std::endl;

                int favor = context->religiousStats.deityFavor[deityId];
                std::cout << "Your standing: " << deity->getFavorLevel(favor) << std::endl;

                if (deity->isHolyDay(context)) {
                    std::cout << "\nToday is a holy day! Special ceremonies are being conducted." << std::endl;
                }
            }

            // Record temple visit
            context->templeJournal[templeName] = "Visited on day " + std::to_string(context->worldState.daysPassed);
        }

        // List priests
        if (!priests.empty()) {
            std::cout << "\nTemple priests:" << std::endl;
            for (const auto& priest : priests) {
                std::cout << "- " << priest.name << ", " << priest.title << std::endl;
            }
        }

        // List available services
        std::cout << "\nServices offered:" << std::endl;
        if (providesBlessings)
            std::cout << "- Blessings" << std::endl;
        if (providesHealing)
            std::cout << "- Healing" << std::endl;
        if (providesCurseRemoval)
            std::cout << "- Curse Removal" << std::endl;
        if (providesDivination)
            std::cout << "- Divination" << std::endl;

        // List rituals
        if (!availableRituals.empty()) {
            std::cout << "\nAvailable rituals:" << std::endl;
            for (const auto& ritual : availableRituals) {
                std::cout << "- " << ritual->ritualName << std::endl;
            }
        }
    }

    DeityNode* findDeityNode(ReligiousGameContext* context) const
    {
        // This would need to be implemented to find the deity node from the controller
        // For now, return nullptr as a placeholder
        return nullptr;
    }

    bool makeOffering(ReligiousGameContext* context, const std::string& itemId, int quantity)
    {
        if (!context)
            return false;

        // Find the offering in possible offerings
        for (const auto& offering : possibleOfferings) {
            if (offering.itemId == itemId && offering.quantity <= quantity) {
                // Check if player has the item
                if (context->playerInventory.hasItem(itemId, offering.quantity)) {
                    // Remove item from inventory
                    context->playerInventory.removeItem(itemId, offering.quantity);

                    // Grant favor
                    context->religiousStats.changeFavor(deityId, offering.favorReward);

                    std::cout << offering.description << std::endl;
                    std::cout << "Your favor with the deity has increased by " << offering.favorReward << "." << std::endl;

                    // Holy day bonus
                    DeityNode* deity = findDeityNode(context);
                    if (deity && deity->isHolyDay(context)) {
                        int bonus = offering.favorReward / 2;
                        context->religiousStats.changeFavor(deityId, bonus);
                        std::cout << "Holy day bonus: +" << bonus << " additional favor!" << std::endl;
                    }

                    return true;
                } else {
                    std::cout << "You don't have " << offering.quantity << " " << itemId << " to offer." << std::endl;
                    return false;
                }
            }
        }

        std::cout << "That is not an appropriate offering for this temple." << std::endl;
        return false;
    }

    bool provideHealing(ReligiousGameContext* context, int gold)
    {
        if (!context)
            return false;

        int baseCost = 10 * serviceQuality;
        int healAmount = 20 * serviceQuality;

        // Adjust cost based on favor
        int favor = context->religiousStats.deityFavor[deityId];
        if (favor >= 50)
            baseCost = static_cast<int>(baseCost * 0.7); // 30% discount for honored+
        else if (favor >= 25)
            baseCost = static_cast<int>(baseCost * 0.85); // 15% discount for friendly
        else if (favor <= -25)
            baseCost = static_cast<int>(baseCost * 1.5); // 50% surcharge if unfriendly

        if (gold >= baseCost) {
            // Implement healing (would need health system integration)
            std::cout << "The priests heal your wounds and ailments. You feel restored." << std::endl;
            std::cout << "You paid " << baseCost << " gold for healing services." << std::endl;

            // In a real implementation, you would modify the player's health here
            return true;
        } else {
            std::cout << "You cannot afford the " << baseCost << " gold required for healing." << std::endl;
            return false;
        }
    }

    bool removeCurse(ReligiousGameContext* context, int gold)
    {
        if (!context)
            return false;

        int baseCost = 50 * serviceQuality;

        // Adjust cost based on favor
        int favor = context->religiousStats.deityFavor[deityId];
        if (favor >= 50)
            baseCost = static_cast<int>(baseCost * 0.7);
        else if (favor >= 25)
            baseCost = static_cast<int>(baseCost * 0.85);
        else if (favor <= -25)
            baseCost = static_cast<int>(baseCost * 1.5);

        if (gold >= baseCost) {
            // Implement curse removal (would need curse system integration)
            std::cout << "The high priest performs a cleansing ritual, removing any curses affecting you." << std::endl;
            std::cout << "You paid " << baseCost << " gold for the ritual." << std::endl;

            // In a real implementation, you would remove curses here
            return true;
        } else {
            std::cout << "You cannot afford the " << baseCost << " gold required for curse removal." << std::endl;
            return false;
        }
    }

    std::string performDivination(ReligiousGameContext* context, int gold)
    {
        if (!context)
            return "The priests cannot focus their vision.";

        int baseCost = 30 * serviceQuality;

        // Adjust cost based on favor
        int favor = context->religiousStats.deityFavor[deityId];
        if (favor >= 50)
            baseCost = static_cast<int>(baseCost * 0.7);
        else if (favor >= 25)
            baseCost = static_cast<int>(baseCost * 0.85);
        else if (favor <= -25)
            baseCost = static_cast<int>(baseCost * 1.5);

        if (gold >= baseCost) {
            // Generate divination based on deity and favor
            std::string divination;

            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(1, 5);
            int result = dis(gen);

            if (favor >= 50) {
                // Clear, helpful divination for devoted followers
                switch (result) {
                case 1:
                    divination = "The path ahead is clear. Seek the ancient tower to the north.";
                    break;
                case 2:
                    divination = "A powerful ally will soon cross your path. Look for one marked by fire.";
                    break;
                case 3:
                    divination = "The treasure you seek lies beneath the old oak, but beware its guardian.";
                    break;
                case 4:
                    divination = "Your enemy's weakness is revealed: silver will pierce their defenses.";
                    break;
                case 5:
                    divination = "Trust the stranger with the blue cloak. They bring vital information.";
                    break;
                }
            } else if (favor >= 0) {
                // Somewhat vague but still helpful divination
                switch (result) {
                case 1:
                    divination = "Your path is shrouded, but there is light to the north.";
                    break;
                case 2:
                    divination = "An important meeting awaits you. Be prepared for conflict or alliance.";
                    break;
                case 3:
                    divination = "What you seek is hidden, but not beyond reach. Look below.";
                    break;
                case 4:
                    divination = "Your opponent has a weakness, though it remains partially obscured.";
                    break;
                case 5:
                    divination = "Not all strangers are enemies. One may offer unexpected aid.";
                    break;
                }
            } else {
                // Cryptic, potentially misleading divination for those out of favor
                switch (result) {
                case 1:
                    divination = "The mists obscure your destiny. Many paths, many dangers.";
                    break;
                case 2:
                    divination = "Beware those who approach with open hands. Not all gifts are blessings.";
                    break;
                case 3:
                    divination = "What is lost may be found, or perhaps better left forgotten.";
                    break;
                case 4:
                    divination = "Your struggle continues. The gods watch with... interest.";
                    break;
                case 5:
                    divination = "The threads of fate tangle around you. Even we cannot see clearly.";
                    break;
                }
            }

            std::cout << "You paid " << baseCost << " gold for the divination ritual." << std::endl;
            std::cout << "The priest enters a trance and speaks: \"" << divination << "\"" << std::endl;

            return divination;
        } else {
            std::cout << "You cannot afford the " << baseCost << " gold required for divination." << std::endl;
            return "";
        }
    }

    std::vector<TAAction> getAvailableActions() override
    {
        std::vector<TAAction> actions = TANode::getAvailableActions();

        // Add temple-specific actions
        actions.push_back({ "make_offering",
            "Make an offering",
            [this]() -> TAInput {
                return { "temple_action", { { "action", std::string("offer") } } };
            } });

        if (providesHealing) {
            actions.push_back({ "request_healing",
                "Request healing services",
                [this]() -> TAInput {
                    return { "temple_action", { { "action", std::string("heal") } } };
                } });
        }

        if (providesCurseRemoval) {
            actions.push_back({ "remove_curse",
                "Request curse removal",
                [this]() -> TAInput {
                    return { "temple_action", { { "action", std::string("uncurse") } } };
                } });
        }

        if (providesDivination) {
            actions.push_back({ "request_divination",
                "Request divination",
                [this]() -> TAInput {
                    return { "temple_action", { { "action", std::string("divine") } } };
                } });
        }

        // Add ritual actions
        for (size_t i = 0; i < availableRituals.size(); i++) {
            actions.push_back({ "ritual_" + std::to_string(i),
                "Perform ritual: " + availableRituals[i]->ritualName,
                [this, i]() -> TAInput {
                    return { "temple_action",
                        { { "action", std::string("ritual") },
                            { "ritual_index", static_cast<int>(i) } } };
                } });
        }

        actions.push_back({ "pray_at_temple",
            "Pray at the altar",
            [this]() -> TAInput {
                return { "temple_action", { { "action", std::string("pray") } } };
            } });

        actions.push_back({ "talk_to_priest",
            "Speak with a priest",
            [this]() -> TAInput {
                return { "temple_action", { { "action", std::string("talk") } } };
            } });

        actions.push_back({ "leave_temple",
            "Leave the temple",
            [this]() -> TAInput {
                return { "temple_action", { { "action", std::string("leave") } } };
            } });

        return actions;
    }

    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override
    {
        if (input.type == "temple_action") {
            std::string action = std::get<std::string>(input.parameters.at("action"));

            if (action == "ritual" && input.parameters.find("ritual_index") != input.parameters.end()) {
                int ritualIndex = std::get<int>(input.parameters.at("ritual_index"));
                if (ritualIndex >= 0 && ritualIndex < static_cast<int>(availableRituals.size())) {
                    outNextNode = availableRituals[ritualIndex];
                    return true;
                }
            } else if (action == "leave") {
                // Return to location or world map
                for (const auto& rule : transitionRules) {
                    if (rule.description == "Exit temple") {
                        outNextNode = rule.targetNode;
                        return true;
                    }
                }
            } else {
                // For other actions (pray, offer, etc.), stay in temple node
                outNextNode = this;
                return true;
            }
        }

        return TANode::evaluateTransition(input, outNextNode);
    }
};

// Represents a religious ritual that can be performed
class RitualNode : public TANode {
public:
    std::string ritualId;
    std::string ritualName;
    std::string ritualDescription;
    std::string deityId;

    // Requirements to perform the ritual
    int favorRequirement;
    bool requiresHolyDay;
    bool requiresPrimaryDeity;
    std::map<std::string, int> itemRequirements;
    int goldCost;

    // Effects of the ritual
    int favorReward;
    int skillBoost; // Optional skill increase
    std::string skillAffected;
    std::string blessingGranted;
    int blessingDuration;

    // Ritual complexity
    enum RitualComplexity {
        SIMPLE = 1,
        MODERATE = 2,
        COMPLEX = 3,
        ELABORATE = 4,
        GRAND = 5
    };
    RitualComplexity complexity;

    RitualNode(const std::string& id, const std::string& name, const std::string& deity)
        : TANode(name)
        , ritualId(id)
        , ritualName(name)
        , deityId(deity)
        , favorRequirement(0)
        , requiresHolyDay(false)
        , requiresPrimaryDeity(false)
        , goldCost(0)
        , favorReward(10)
        , skillBoost(0)
        , blessingDuration(0)
        , complexity(MODERATE)
    {
    }

    void loadFromJson(const json& ritualData)
    {
        ritualDescription = ritualData["description"];
        favorRequirement = ritualData["favorRequirement"];
        requiresHolyDay = ritualData["requiresHolyDay"];
        requiresPrimaryDeity = ritualData["requiresPrimaryDeity"];
        goldCost = ritualData["goldCost"];
        favorReward = ritualData["favorReward"];
        skillBoost = ritualData["skillBoost"];
        skillAffected = ritualData["skillAffected"];
        blessingGranted = ritualData["blessingGranted"];
        blessingDuration = ritualData["blessingDuration"];
        complexity = static_cast<RitualComplexity>(ritualData["complexity"]);

        // Load item requirements
        itemRequirements.clear();
        for (auto& [item, quantity] : ritualData["itemRequirements"].items()) {
            itemRequirements[item] = quantity;
        }
    }

    void onEnter(ReligiousGameContext* context)
    {
        std::cout << "=== " << ritualName << " Ritual ===" << std::endl;
        std::cout << ritualDescription << std::endl;

        if (context) {
            // Check if ritual has been completed before
            bool completed = context->religiousStats.hasCompletedRitual(ritualId);
            if (completed) {
                std::cout << "You have performed this ritual before." << std::endl;
            }

            // Show requirements
            std::cout << "\nRequirements:" << std::endl;

            if (favorRequirement > 0) {
                std::cout << "- Minimum " << favorRequirement << " favor with deity" << std::endl;
                if (context->religiousStats.deityFavor[deityId] < favorRequirement) {
                    std::cout << "  (You don't have enough favor)" << std::endl;
                }
            }

            if (requiresHolyDay) {
                std::cout << "- Must be performed on deity's holy day" << std::endl;
                DeityNode* deity = findDeityNode(context);
                if (deity && !deity->isHolyDay(context)) {
                    std::cout << "  (Today is not the holy day)" << std::endl;
                }
            }

            if (requiresPrimaryDeity) {
                std::cout << "- Deity must be your primary deity" << std::endl;
                if (context->religiousStats.primaryDeity != deityId) {
                    std::cout << "  (This is not your primary deity)" << std::endl;
                }
            }

            for (const auto& [item, quantity] : itemRequirements) {
                std::cout << "- " << quantity << "x " << item << std::endl;
                if (!context->playerInventory.hasItem(item, quantity)) {
                    std::cout << "  (You don't have enough)" << std::endl;
                }
            }

            if (goldCost > 0) {
                std::cout << "- " << goldCost << " gold donation" << std::endl;
                // Check gold in inventory...
            }

            // Show rewards
            std::cout << "\nRewards:" << std::endl;
            std::cout << "- " << favorReward << " favor with deity" << std::endl;

            if (skillBoost > 0 && !skillAffected.empty()) {
                std::cout << "- +" << skillBoost << " to " << skillAffected << " skill" << std::endl;
            }

            if (!blessingGranted.empty() && blessingDuration > 0) {
                std::cout << "- " << blessingGranted << " blessing (" << blessingDuration << " days)" << std::endl;
            }
        }
    }

    DeityNode* findDeityNode(ReligiousGameContext* context) const
    {
        // This would need to be implemented to find the deity node from the controller
        // For now, return nullptr as a placeholder
        return nullptr;
    }

    bool canPerformRitual(ReligiousGameContext* context) const
    {
        if (!context)
            return false;

        // Check favor requirement
        if (context->religiousStats.deityFavor[deityId] < favorRequirement) {
            return false;
        }

        // Check if holy day is required
        if (requiresHolyDay) {
            DeityNode* deity = findDeityNode(context);
            if (!deity || !deity->isHolyDay(context)) {
                return false;
            }
        }

        // Check if primary deity is required
        if (requiresPrimaryDeity && context->religiousStats.primaryDeity != deityId) {
            return false;
        }

        // Check item requirements
        for (const auto& [item, quantity] : itemRequirements) {
            if (!context->playerInventory.hasItem(item, quantity)) {
                return false;
            }
        }

        // Check gold (would need to be implemented)

        return true;
    }

    bool performRitual(ReligiousGameContext* context)
    {
        if (!context)
            return false;

        if (!canPerformRitual(context)) {
            std::cout << "You cannot perform this ritual. Some requirements are not met." << std::endl;
            return false;
        }

        // Consume required items
        for (const auto& [item, quantity] : itemRequirements) {
            context->playerInventory.removeItem(item, quantity);
        }

        // Consume gold (would need to be implemented)

        // Grant favor reward
        context->religiousStats.changeFavor(deityId, favorReward);
        std::cout << "You have gained " << favorReward << " favor with the deity." << std::endl;

        // Grant skill boost if applicable
        if (skillBoost > 0 && !skillAffected.empty()) {
            context->playerStats.improveSkill(skillAffected, skillBoost);
            std::cout << "Your " << skillAffected << " skill has improved by " << skillBoost << "." << std::endl;
        }

        // Grant blessing if applicable
        if (!blessingGranted.empty() && blessingDuration > 0) {
            context->religiousStats.addBlessing(blessingGranted, blessingDuration);
            std::cout << "You have received the " << blessingGranted << " blessing for " << blessingDuration << " days." << std::endl;
        }

        // Mark ritual as completed
        context->religiousStats.markRitualCompleted(ritualId);

        std::cout << "The ritual has been successfully completed." << std::endl;

        // Ritual effects on the world (could trigger world events)
        switch (complexity) {
        case GRAND:
            std::cout << "The very fabric of reality seems to shift in response to your grand ritual." << std::endl;
            // Implement world-changing effects
            break;
        case ELABORATE:
            std::cout << "The power of your ritual resonates throughout the region." << std::endl;
            // Implement regional effects
            break;
        case COMPLEX:
            std::cout << "The deity's attention has been drawn to your devotion." << std::endl;
            // Special deity interactions
            break;
        case MODERATE:
            std::cout << "You feel the power of the ritual washing over you." << std::endl;
            break;
        case SIMPLE:
            std::cout << "A simple but effective communion with the divine." << std::endl;
            break;
        }

        return true;
    }

    std::vector<TAAction> getAvailableActions() override
    {
        std::vector<TAAction> actions = TANode::getAvailableActions();

        // Add ritual actions
        actions.push_back({ "perform_ritual",
            "Perform the ritual",
            [this]() -> TAInput {
                return { "ritual_action", { { "action", std::string("perform") } } };
            } });

        actions.push_back({ "back_to_temple",
            "Return to temple",
            [this]() -> TAInput {
                return { "ritual_action", { { "action", std::string("back") } } };
            } });

        return actions;
    }
};

// Prayer system for making requests to deities
class PrayerNode : public TANode {
public:
    std::string deityId;
    std::string prayerDescription;

    // Different prayer types
    enum PrayerType {
        GUIDANCE,
        BLESSING,
        PROTECTION,
        HEALING,
        STRENGTH,
        FORTUNE,
        FORGIVENESS
    };

    // Prayer results based on favor levels
    struct PrayerResult {
        std::string description;
        std::map<std::string, int> statEffects;
        std::string blessingGranted;
        int blessingDuration;
        bool curseRemoved;
    };

    std::map<PrayerType, std::map<int, PrayerResult>> prayerResults;

    PrayerNode(const std::string& name, const std::string& deity)
        : TANode(name)
        , deityId(deity)
    {
        // Initialize prayer results for different favor levels
        initializePrayerResults();
    }

    void initializePrayerResults()
    {
        // For each prayer type, define results at different favor thresholds
        // Example for GUIDANCE prayers:
        prayerResults[GUIDANCE][75] = { // Exalted (75+ favor)
            "The deity speaks directly to your mind, offering crystal clear guidance.",
            { { "wisdom", 2 } },
            "divine_insight",
            24,
            false
        };

        prayerResults[GUIDANCE][50] = { // Honored (50+ favor)
            "A vision appears before you, showing a clear path forward.",
            { { "wisdom", 1 } },
            "spiritual_clarity",
            12,
            false
        };

        prayerResults[GUIDANCE][10] = { // Friendly (10+ favor)
            "You receive a vague but helpful nudge in the right direction.",
            {},
            "minor_insight",
            6,
            false
        };

        prayerResults[GUIDANCE][0] = { // Neutral (0+ favor)
            "Your prayer is acknowledged, but no clear answer comes.",
            {},
            "",
            0,
            false
        };

        prayerResults[GUIDANCE][-25] = { // Disliked or worse
            "Your prayer seems to echo emptily. No guidance comes.",
            {},
            "",
            0,
            false
        };

        // Similarly define results for other prayer types...
    }

    void onEnter(ReligiousGameContext* context)
    {
        std::cout << "=== Prayer to the ";

        if (context) {
            DeityNode* deity = findDeityNode(context);
            if (deity) {
                std::cout << deity->deityName << " ===" << std::endl;
            } else {
                std::cout << "Deity ===" << std::endl;
            }
        } else {
            std::cout << "Deity ===" << std::endl;
        }

        std::cout << prayerDescription << std::endl;

        if (context) {
            int favor = context->religiousStats.deityFavor[deityId];
            bool isPrimaryDeity = (context->religiousStats.primaryDeity == deityId);

            std::cout << "\nYour favor: " << favor;
            if (isPrimaryDeity) {
                std::cout << " (Primary Deity)";
            }
            std::cout << std::endl;

            // Check for holy day
            DeityNode* deity = findDeityNode(context);
            if (deity && deity->isHolyDay(context)) {
                std::cout << "Today is this deity's holy day. Your prayers will be more effective." << std::endl;
            }
        }

        std::cout << "\nWhat would you like to pray for?" << std::endl;
    }

    DeityNode* findDeityNode(ReligiousGameContext* context) const
    {
        // This would need to be implemented to find the deity node from the controller
        // For now, return nullptr as a placeholder
        return nullptr;
    }

    PrayerResult getPrayerOutcome(ReligiousGameContext* context, PrayerType type)
    {
        if (!context)
            return {}; // Return empty result if no context

        int favor = context->religiousStats.deityFavor[deityId];
        bool isHolyDay = false;

        // Check for holy day bonus
        DeityNode* deity = findDeityNode(context);
        if (deity) {
            isHolyDay = deity->isHolyDay(context);
        }

        // Apply holy day bonus
        if (isHolyDay) {
            favor += 15; // Temporary bonus for outcome calculation
        }

        // Find the appropriate result based on favor
        if (favor >= 75)
            return prayerResults[type][75];
        if (favor >= 50)
            return prayerResults[type][50];
        if (favor >= 10)
            return prayerResults[type][10];
        if (favor >= 0)
            return prayerResults[type][0];
        return prayerResults[type][-25];
    }

    void performPrayer(ReligiousGameContext* context, PrayerType type)
    {
        if (!context)
            return;

        // Small devotion increase for praying
        context->religiousStats.addDevotion(deityId, 1);

        // Get prayer outcome
        PrayerResult result = getPrayerOutcome(context, type);

        // Display outcome
        std::cout << "\n"
                  << result.description << std::endl;

        // Apply stat effects
        for (const auto& [stat, value] : result.statEffects) {
            if (stat == "strength")
                context->playerStats.strength += value;
            else if (stat == "dexterity")
                context->playerStats.dexterity += value;
            else if (stat == "constitution")
                context->playerStats.constitution += value;
            else if (stat == "intelligence")
                context->playerStats.intelligence += value;
            else if (stat == "wisdom")
                context->playerStats.wisdom += value;
            else if (stat == "charisma")
                context->playerStats.charisma += value;

            if (value != 0) {
                std::cout << "Your " << stat << " has " << (value > 0 ? "increased" : "decreased")
                          << " by " << std::abs(value) << "." << std::endl;
            }
        }

        // Grant blessing if applicable
        if (!result.blessingGranted.empty() && result.blessingDuration > 0) {
            context->religiousStats.addBlessing(result.blessingGranted, result.blessingDuration);
            std::cout << "You have received the " << result.blessingGranted << " blessing for "
                      << result.blessingDuration << " days." << std::endl;
        }

        // Remove curse if applicable
        if (result.curseRemoved) {
            // Would need curse system integration
            std::cout << "You feel a weight lift from your shoulders as a curse is removed." << std::endl;
        }
    }

    std::vector<TAAction> getAvailableActions() override
    {
        std::vector<TAAction> actions = TANode::getAvailableActions();

        // Add prayer actions for different types
        actions.push_back({ "pray_guidance",
            "Pray for guidance",
            [this]() -> TAInput {
                return { "prayer_action", { { "action", std::string("pray") }, { "type", std::string("guidance") } } };
            } });

        actions.push_back({ "pray_blessing",
            "Pray for blessing",
            [this]() -> TAInput {
                return { "prayer_action", { { "action", std::string("pray") }, { "type", std::string("blessing") } } };
            } });

        actions.push_back({ "pray_protection",
            "Pray for protection",
            [this]() -> TAInput {
                return { "prayer_action", { { "action", std::string("pray") }, { "type", std::string("protection") } } };
            } });

        actions.push_back({ "pray_healing",
            "Pray for healing",
            [this]() -> TAInput {
                return { "prayer_action", { { "action", std::string("pray") }, { "type", std::string("healing") } } };
            } });

        actions.push_back({ "pray_strength",
            "Pray for strength",
            [this]() -> TAInput {
                return { "prayer_action", { { "action", std::string("pray") }, { "type", std::string("strength") } } };
            } });

        actions.push_back({ "pray_fortune",
            "Pray for fortune",
            [this]() -> TAInput {
                return { "prayer_action", { { "action", std::string("pray") }, { "type", std::string("fortune") } } };
            } });

        actions.push_back({ "pray_forgiveness",
            "Pray for forgiveness",
            [this]() -> TAInput {
                return { "prayer_action", { { "action", std::string("pray") }, { "type", std::string("forgiveness") } } };
            } });

        actions.push_back({ "return_to_deity",
            "Finish prayer",
            [this]() -> TAInput {
                return { "prayer_action", { { "action", std::string("finish") } } };
            } });

        return actions;
    }

    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override
    {
        if (input.type == "prayer_action") {
            std::string action = std::get<std::string>(input.parameters.at("action"));

            if (action == "pray") {
                // Process prayer but stay on same node
                outNextNode = this;
                return true;
            } else if (action == "finish") {
                // Return to deity view
                for (const auto& rule : transitionRules) {
                    if (rule.description == "Return to deity") {
                        outNextNode = rule.targetNode;
                        return true;
                    }
                }
            }
        }

        return TANode::evaluateTransition(input, outNextNode);
    }
};

//----------------------------------------
// BLESSING SYSTEM
//----------------------------------------

// Blessing effects that can be granted by deities
class BlessingNode : public TANode {
public:
    std::string blessingId;
    std::string blessingName;
    std::string blessingDescription;
    std::string deityId;

    // Blessing tier
    enum BlessingTier {
        MINOR = 1,
        MODERATE = 2,
        MAJOR = 3,
        GREATER = 4,
        DIVINE = 5
    };
    BlessingTier tier;

    // Favor requirement to receive blessing
    int favorRequirement;

    // Duration in game days
    int duration;

    // Effects of the blessing
    struct BlessingEffect {
        std::string type; // "stat", "skill", "protection", "ability", etc.
        std::string target; // Specific stat, skill, damage type, etc.
        int magnitude;
        std::string description;
    };
    std::vector<BlessingEffect> effects;

    BlessingNode(const std::string& id, const std::string& name, const std::string& deity, BlessingTier t)
        : TANode(name)
        , blessingId(id)
        , blessingName(name)
        , deityId(deity)
        , tier(t)
    {
        // Set default values based on tier
        switch (tier) {
        case MINOR:
            favorRequirement = 10;
            duration = 7;
            break;
        case MODERATE:
            favorRequirement = 25;
            duration = 14;
            break;
        case MAJOR:
            favorRequirement = 50;
            duration = 21;
            break;
        case GREATER:
            favorRequirement = 75;
            duration = 30;
            break;
        case DIVINE:
            favorRequirement = 90;
            duration = 60;
            break;
        }
    }

    void loadFromJson(const json& blessingData)
    {
        blessingDescription = blessingData["description"];
        favorRequirement = blessingData["favorRequirement"];
        duration = blessingData["duration"];

        // Load effects
        effects.clear();
        for (const auto& effect : blessingData["effects"]) {
            effects.push_back({ effect["type"],
                effect["target"],
                effect["magnitude"],
                effect["description"] });
        }
    }

    void onEnter(ReligiousGameContext* context)
    {
        std::cout << "=== " << blessingName << " ===\n"
                  << std::endl;
        std::cout << blessingDescription << std::endl;

        std::cout << "\nTier: " << getTierName() << std::endl;
        std::cout << "Duration: " << duration << " days" << std::endl;
        std::cout << "Favor Required: " << favorRequirement << std::endl;

        std::cout << "\nEffects:" << std::endl;
        for (const auto& effect : effects) {
            std::cout << "- " << effect.description << std::endl;
        }

        if (context) {
            bool hasBlessing = context->religiousStats.hasBlessingActive(blessingId);
            int currentFavor = context->religiousStats.deityFavor[deityId];

            if (hasBlessing) {
                int remainingDuration = context->religiousStats.blessingDuration[blessingId];
                std::cout << "\nThis blessing is currently active. " << remainingDuration << " days remaining." << std::endl;
            }

            if (currentFavor < favorRequirement) {
                std::cout << "\nYou need " << (favorRequirement - currentFavor) << " more favor to receive this blessing." << std::endl;
            }
        }
    }

    std::string getTierName() const
    {
        switch (tier) {
        case MINOR:
            return "Minor";
        case MODERATE:
            return "Moderate";
        case MAJOR:
            return "Major";
        case GREATER:
            return "Greater";
        case DIVINE:
            return "Divine";
        default:
            return "Unknown";
        }
    }

    bool canReceiveBlessing(ReligiousGameContext* context) const
    {
        if (!context)
            return false;

        // Check if player has enough favor
        int favor = context->religiousStats.deityFavor[deityId];
        if (favor < favorRequirement) {
            return false;
        }

        // Divine blessings require primary deity status
        if (tier == DIVINE && context->religiousStats.primaryDeity != deityId) {
            return false;
        }

        return true;
    }

    bool grantBlessing(ReligiousGameContext* context)
    {
        if (!context)
            return false;

        if (!canReceiveBlessing(context)) {
            std::cout << "You cannot receive this blessing. You need more favor with the deity." << std::endl;

            if (tier == DIVINE && context->religiousStats.primaryDeity != deityId) {
                std::cout << "Divine blessings can only be granted by your primary deity." << std::endl;
            }

            return false;
        }

        // Add blessing to active blessings
        context->religiousStats.addBlessing(blessingId, duration);

        // Apply immediate effects
        for (const auto& effect : effects) {
            if (effect.type == "stat") {
                if (effect.target == "strength")
                    context->playerStats.strength += effect.magnitude;
                else if (effect.target == "dexterity")
                    context->playerStats.dexterity += effect.magnitude;
                else if (effect.target == "constitution")
                    context->playerStats.constitution += effect.magnitude;
                else if (effect.target == "intelligence")
                    context->playerStats.intelligence += effect.magnitude;
                else if (effect.target == "wisdom")
                    context->playerStats.wisdom += effect.magnitude;
                else if (effect.target == "charisma")
                    context->playerStats.charisma += effect.magnitude;
            } else if (effect.type == "skill") {
                context->playerStats.improveSkill(effect.target, effect.magnitude);
            }
        }

        std::cout << "The " << blessingName << " blessing has been granted to you for " << duration << " days." << std::endl;

        // Small favor cost for receiving the blessing
        int favorCost = tier * 2;
        context->religiousStats.changeFavor(deityId, -favorCost);
        std::cout << "You have spent " << favorCost << " favor to receive this blessing." << std::endl;

        return true;
    }

    std::vector<TAAction> getAvailableActions() override
    {
        std::vector<TAAction> actions = TANode::getAvailableActions();

        // Add blessing actions
        actions.push_back({ "request_blessing",
            "Request this blessing",
            [this]() -> TAInput {
                return { "blessing_action", { { "action", std::string("request") } } };
            } });

        actions.push_back({ "return_to_deity",
            "Return to deity view",
            [this]() -> TAInput {
                return { "blessing_action", { { "action", std::string("back") } } };
            } });

        return actions;
    }
};

// Religious quests related to deities
class ReligiousQuestNode : public QuestNode {
public:
    std::string deityId;
    int favorReward;
    int devotionReward;

    // Effects on the world when completed
    std::map<std::string, std::string> worldStateChanges;

    ReligiousQuestNode(const std::string& name, const std::string& deity)
        : QuestNode(name)
        , deityId(deity)
        , favorReward(20)
        , devotionReward(10)
    {
    }

    void loadFromJson(const json& questData)
    {
        questTitle = questData["title"];
        questDescription = questData["description"];
        favorReward = questData["favorReward"];
        devotionReward = questData["devotionReward"];

        // Load rewards
        rewards.clear();
        for (const auto& reward : questData["rewards"]) {
            rewards.push_back({ reward["type"],
                reward["amount"],
                reward["description"] });
        }

        // Load world state changes
        worldStateChanges.clear();
        for (auto& [key, value] : questData["worldStateChanges"].items()) {
            worldStateChanges[key] = value;
        }
    }

    void onEnter(GameContext* baseContext) override
    {
        QuestNode::onEnter(baseContext);

        // Additional religious quest info
        ReligiousGameContext* context = dynamic_cast<ReligiousGameContext*>(baseContext);
        if (context) {
            std::cout << "\nThis is a religious quest for ";

            DeityNode* deity = findDeityNode(context);
            if (deity) {
                std::cout << deity->deityName << ", " << deity->deityTitle << "." << std::endl;
            } else {
                std::cout << "a deity." << std::endl;
            }

            std::cout << "Favor reward: " << favorReward << std::endl;
            std::cout << "Devotion reward: " << devotionReward << std::endl;

            // Show current favor
            int currentFavor = context->religiousStats.deityFavor[deityId];
            std::cout << "Your current favor: " << currentFavor << std::endl;
        }
    }

    void onExit(GameContext* baseContext) override
    {
        QuestNode::onExit(baseContext);

        // Religious-specific rewards when quest is completed
        if (isAcceptingState) {
            ReligiousGameContext* context = dynamic_cast<ReligiousGameContext*>(baseContext);
            if (context) {
                // Award favor and devotion
                context->religiousStats.changeFavor(deityId, favorReward);
                context->religiousStats.addDevotion(deityId, devotionReward);

                std::cout << "You have gained " << favorReward << " favor and " << devotionReward
                          << " devotion with the deity." << std::endl;

                // Apply world state changes
                for (const auto& [key, value] : worldStateChanges) {
                    context->worldState.setWorldFlag(key, value == "true");
                    std::cout << "The world has changed in response to your actions." << std::endl;
                }
            }
        }
    }

    DeityNode* findDeityNode(ReligiousGameContext* context) const
    {
        // This would need to be implemented to find the deity node from the controller
        // For now, return nullptr as a placeholder
        return nullptr;
    }
}

//----------------------------------------
// RELIGION SYSTEM CONTROLLER
//----------------------------------------

// Extension to TAController for Religion System
class ReligionTAController : public TAController {
public:
    // Root node for religion system
    TANode* religionRoot;

    // All deities in the pantheon
    std::vector<DeityNode*> pantheon;

    // All temples in the world
    std::vector<TempleNode*> temples;

    // All known rituals
    std::vector<RitualNode*> rituals;

    // All available blessings
    std::vector<BlessingNode*> blessings;

    // All religious quests
    std::vector<ReligiousQuestNode*> religiousQuests;

    // Convert base GameContext to ReligiousGameContext
    ReligiousGameContext* getReligiousContext()
    {
        return dynamic_cast<ReligiousGameContext*>(&gameContext);
    }

    // Initialize the religion system from JSON
    void initializeReligionSystem(const std::string& jsonFilePath)
    {
        std::cout << "Initializing Religion System from JSON..." << std::endl;

        // Load JSON data
        json religionData;
        try {
            std::ifstream file(jsonFilePath);
            if (!file.is_open()) {
                std::cerr << "Error: Could not open file " << jsonFilePath << std::endl;
                return;
            }
            file >> religionData;
        } catch (const std::exception& e) {
            std::cerr << "Error parsing JSON: " << e.what() << std::endl;
            return;
        }

        // Create pantheon root node
        religionRoot = createNode("ReligionSystem");

        // Create deities
        std::cout << "Creating pantheon of deities..." << std::endl;
        createDeitiesFromJson(religionData["deities"]);

        // Create temples
        std::cout << "Creating temples..." << std::endl;
        createTemplesFromJson(religionData["temples"]);

        // Create rituals
        std::cout << "Creating rituals..." << std::endl;
        createRitualsFromJson(religionData["rituals"]);

        // Create blessings
        std::cout << "Creating blessings..." << std::endl;
        createBlessingsFromJson(religionData["blessings"]);

        // Create religious quests
        std::cout << "Creating religious quests..." << std::endl;
        createReligiousQuestsFromJson(religionData["religiousQuests"]);

        // Set up holy day calendar
        std::cout << "Setting up holy day calendar..." << std::endl;
        auto* context = getReligiousContext();
        if (context) {
            context->loadHolyDays(religionData["holyDayCalendar"]);

            // Initialize religious stats with all deity IDs
            std::vector<std::string> deityIds;
            for (auto* deity : pantheon) {
                deityIds.push_back(deity->deityId);
            }
            context->religiousStats.initializeDeities(deityIds);
        }

        // Set up hierarchy
        std::cout << "Setting up Religion System hierarchy..." << std::endl;
        setupReligionHierarchy();

        // Register the religion system root
        setSystemRoot("ReligionSystem", religionRoot);

        std::cout << "Religion System initialization complete." << std::endl;
    }

private:
    // Create all deities in the pantheon from JSON
    void createDeitiesFromJson(const json& deitiesData)
    {
        for (const auto& deityData : deitiesData) {
            std::string id = deityData["id"];
            std::string name = deityData["name"];
            std::string title = deityData["title"];

            DeityNode* deity = dynamic_cast<DeityNode*>(
                createNode<DeityNode>(name, id, title));

            deity->loadFromJson(deityData);
            pantheon.push_back(deity);
        }
    }

    // Create temples from JSON
    void createTemplesFromJson(const json& templesData)
    {
        for (const auto& templeData : templesData) {
            std::string id = templeData["id"];
            std::string name = templeData["name"];
            std::string location = templeData["location"];
            std::string deityId = templeData["deityId"];

            TempleNode* temple = dynamic_cast<TempleNode*>(
                createNode<TempleNode>(id, name, location, deityId));

            temple->loadFromJson(templeData);
            temples.push_back(temple);
        }
    }

    // Create rituals from JSON
    void createRitualsFromJson(const json& ritualsData)
    {
        for (const auto& ritualData : ritualsData) {
            std::string id = ritualData["id"];
            std::string name = ritualData["name"];
            std::string deityId = ritualData["deityId"];

            RitualNode* ritual = dynamic_cast<RitualNode*>(
                createNode<RitualNode>(id, name, deityId));

            ritual->loadFromJson(ritualData);
            rituals.push_back(ritual);
        }
    }

    // Create blessings from JSON
    void createBlessingsFromJson(const json& blessingsData)
    {
        for (const auto& blessingData : blessingsData) {
            std::string id = blessingData["id"];
            std::string name = blessingData["name"];
            std::string deityId = blessingData["deityId"];
            int tier = blessingData["tier"];

            BlessingNode* blessing = dynamic_cast<BlessingNode*>(
                createNode<BlessingNode>(id, name, deityId, static_cast<BlessingNode::BlessingTier>(tier)));

            blessing->loadFromJson(blessingData);
            blessings.push_back(blessing);
        }
    }

    // Create religious quests from JSON
    void createReligiousQuestsFromJson(const json& questsData)
    {
        for (const auto& questData : questsData) {
            std::string id = questData["id"];
            std::string deityId = questData["deityId"];

            ReligiousQuestNode* quest = dynamic_cast<ReligiousQuestNode*>(
                createNode<ReligiousQuestNode>(id, deityId));

            quest->loadFromJson(questData);
            religiousQuests.push_back(quest);
        }
    }

    // Set up hierarchy for religion system
    void setupReligionHierarchy()
    {
        // Add all deities to the religion root
        for (auto* deity : pantheon) {
            religionRoot->addChild(deity);

            // Set up blessing connections
            for (auto* blessing : blessings) {
                if (blessing->deityId == deity->deityId) {
                    deity->availableBlessings.push_back(blessing);
                }
            }

            // Set up temple connections
            for (auto* temple : temples) {
                if (temple->deityId == deity->deityId) {
                    deity->temples.push_back(temple);

                    // Add rituals to temples
                    for (auto* ritual : rituals) {
                        if (ritual->deityId == deity->deityId) {
                            temple->availableRituals.push_back(ritual);
                        }
                    }
                }
            }

            // Create prayer node for each deity
            PrayerNode* prayerNode = dynamic_cast<PrayerNode*>(
                createNode<PrayerNode>("Prayer_" + deity->deityId, deity->deityId));
            prayerNode->prayerDescription = "You kneel in prayer before the altar of " + deity->deityName + ".";
            deity->addChild(prayerNode);

            // Set up return transitions
            prayerNode->addTransition(
                [](const TAInput& input) {
                    return input.type == "prayer_action" && std::get<std::string>(input.parameters.at("action")) == "finish";
                },
                deity, "Return to deity");
        }

        // Set up temple exit transitions
        for (auto* temple : temples) {
            // Find corresponding region/location
            temple->addTransition(
                [](const TAInput& input) {
                    return input.type == "temple_action" && std::get<std::string>(input.parameters.at("action")) == "leave";
                },
                religionRoot, "Exit temple");
        }

        // Set up ritual transitions
        for (auto* ritual : rituals) {
            // Find corresponding temple
            for (auto* temple : temples) {
                if (temple->deityId == ritual->deityId) {
                    ritual->addTransition(
                        [](const TAInput& input) {
                            return input.type == "ritual_action" && std::get<std::string>(input.parameters.at("action")) == "back";
                        },
                        temple, "Return to temple");
                    break;
                }
            }
        }

        // Set up deity return transitions
        for (auto* deity : pantheon) {
            deity->addTransition(
                [](const TAInput& input) {
                    return input.type == "deity_action" && std::get<std::string>(input.parameters.at("action")) == "back";
                },
                religionRoot, "Return to pantheon");
        }
    }
};

//----------------------------------------
// DEMO IMPLEMENTATION
//----------------------------------------

int main()
{
    std::cout << "___ Starting Religion Deity System ___" << std::endl;

    // Create the extended controller
    ReligionTAController controller;

    // Create religious game context
    ReligiousGameContext* context = new ReligiousGameContext();
    controller.gameContext = *context;

    // Initialize the religion system from JSON file
    controller.initializeReligionSystem("ReligionDeity.JSON");

    // Demo interaction with the system
    std::cout << "\n___ DEMONSTRATION ___\n"
              << std::endl;

    // Begin at religion system root
    controller.processInput("ReligionSystem", {});

    // Setup some initial state for the player
    dynamic_cast<ReligiousGameContext*>(&controller.gameContext)->religiousStats.changeFavor("sun_god", 30);
    dynamic_cast<ReligiousGameContext*>(&controller.gameContext)->religiousStats.addDevotion("sun_god", 25);
    controller.gameContext.playerInventory.addItem({ "golden_symbol", "Golden Sun Symbol", "religious", 50, 1 });
    controller.gameContext.playerInventory.addItem({ "candle", "Blessed Candle", "religious", 5, 5 });
    controller.gameContext.playerInventory.addItem({ "healing_herbs", "Healing Herbs", "herb", 10, 10 });

    // Examine Sun God
    TAInput selectSunGodInput = {
        "pantheon_action",
        { { "action", std::string("select_deity") }, { "deity", std::string("sun_god") } }
    };
    controller.processInput("ReligionSystem", selectSunGodInput);

    // Set as primary deity
    TAInput setPrimaryInput = {
        "deity_action",
        { { "action", std::string("set_primary") }, { "deity", std::string("sun_god") } }
    };
    controller.processInput("ReligionSystem", setPrimaryInput);

    // Visit temple
    TAInput viewTemplesInput = {
        "deity_action",
        { { "action", std::string("view_temples") }, { "deity", std::string("sun_god") } }
    };
    controller.processInput("ReligionSystem", viewTemplesInput);

    // Make an offering
    TAInput makeOfferingInput = {
        "temple_action",
        { { "action", std::string("offer") }, { "item", std::string("golden_symbol") }, { "quantity", 1 } }
    };
    controller.processInput("ReligionSystem", makeOfferingInput);

    // Perform a ritual
    TAInput selectRitualInput = {
        "temple_action",
        { { "action", std::string("ritual") }, { "ritual_index", 0 } }
    };
    controller.processInput("ReligionSystem", selectRitualInput);

    TAInput performRitualInput = {
        "ritual_action",
        { { "action", std::string("perform") } }
    };
    controller.processInput("ReligionSystem", performRitualInput);

    // Return to temple
    TAInput returnToTempleInput = {
        "ritual_action",
        { { "action", std::string("back") } }
    };
    controller.processInput("ReligionSystem", returnToTempleInput);

    // Pray at temple
    TAInput prayInput = {
        "temple_action",
        { { "action", std::string("pray") } }
    };
    controller.processInput("ReligionSystem", prayInput);

    // Request blessing
    TAInput prayForBlessingInput = {
        "prayer_action",
        { { "action", std::string("pray") }, { "type", std::string("blessing") } }
    };
    controller.processInput("ReligionSystem", prayForBlessingInput);

    // View religious stats
    std::cout << "\n=== PLAYER RELIGIOUS STATS ===\n"
              << std::endl;
    ReligiousGameContext* religContext = dynamic_cast<ReligiousGameContext*>(&controller.gameContext);

    std::cout << "Primary Deity: " << religContext->religiousStats.primaryDeity << std::endl;

    std::cout << "\nDeity Favor:" << std::endl;
    for (const auto& [deity, favor] : religContext->religiousStats.deityFavor) {
        if (favor != 0) {
            std::cout << "- " << deity << ": " << favor << std::endl;
        }
    }

    std::cout << "\nDeity Devotion:" << std::endl;
    for (const auto& [deity, devotion] : religContext->religiousStats.deityDevotion) {
        if (devotion != 0) {
            std::cout << "- " << deity << ": " << devotion << std::endl;
        }
    }

    std::cout << "\nActive Blessings:" << std::endl;
    if (religContext->religiousStats.activeBlessing.empty()) {
        std::cout << "No active blessings." << std::endl;
    } else {
        for (const auto& blessing : religContext->religiousStats.activeBlessing) {
            std::cout << "- " << blessing << " (" << religContext->religiousStats.blessingDuration[blessing] << " days remaining)" << std::endl;
        }
    }

    std::cout << "\nCompleted Rituals:" << std::endl;
    if (religContext->religiousStats.completedRituals.empty()) {
        std::cout << "No completed rituals." << std::endl;
    } else {
        for (const auto& ritual : religContext->religiousStats.completedRituals) {
            std::cout << "- " << ritual << std::endl;
        }
    }

    // Clean up
    delete context;

    // Add this pause before exiting
    std::cout << "\nPress Enter to exit...";
    std::cin.get();

    return 0;
}