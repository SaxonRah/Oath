// ReligionDeitySystem.cpp
// Extension for RawOathFull.cpp implementing a Religion and Deity System

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
        // Initialize with neutral favor for all deities
        deityFavor["sun_god"] = 0;
        deityFavor["moon_goddess"] = 0;
        deityFavor["earth_mother"] = 0;
        deityFavor["war_god"] = 0;
        deityFavor["trade_god"] = 0;
        deityFavor["death_goddess"] = 0;
        deityFavor["trickster"] = 0;
        deityFavor["sea_god"] = 0;
        deityFavor["wisdom_goddess"] = 0;

        // Initialize devotion points at 0
        for (const auto& [deity, _] : deityFavor) {
            deityDevotion[deity] = 0;
            deityAlignment[deity] = false;
        }

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
                if (deity == "sun_god" && deityFavor.find("moon_goddess") != deityFavor.end()) {
                    deityFavor["moon_goddess"] -= amount / 2;
                } else if (deity == "moon_goddess" && deityFavor.find("sun_god") != deityFavor.end()) {
                    deityFavor["sun_god"] -= amount / 2;
                } else if (deity == "war_god" && deityFavor.find("trade_god") != deityFavor.end()) {
                    deityFavor["trade_god"] -= amount / 3;
                } else if (deity == "trickster" && deityFavor.find("wisdom_goddess") != deityFavor.end()) {
                    deityFavor["wisdom_goddess"] -= amount / 2;
                }
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
};

// Extension to GameContext to include religious stats
struct ReligiousGameContext : public GameContext {
    ReligiousStats religiousStats;
    std::map<std::string, std::string> templeJournal; // Records temple visits and rituals
    std::map<std::string, int> holyDayCalendar; // Days of the year for holy celebrations

    ReligiousGameContext()
        : GameContext()
    {
        // Initialize holy days calendar (day of year -> deity)
        holyDayCalendar[60] = 1; // Spring Equinox - Earth Mother (day 60)
        holyDayCalendar[172] = 2; // Summer Solstice - Sun God (day 172)
        holyDayCalendar[265] = 3; // Fall Equinox - Death Goddess (day 265)
        holyDayCalendar[356] = 4; // Winter Solstice - Moon Goddess (day 356)
        holyDayCalendar[100] = 5; // War God's Day (day 100)
        holyDayCalendar[200] = 6; // Trade Festival (day 200)
        holyDayCalendar[300] = 7; // Trickster's Eve (day 300)
        holyDayCalendar[25] = 8; // Sea God's Blessing (day 25)
        holyDayCalendar[225] = 9; // Wisdom Goddess Enlightenment (day 225)
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
            switch (it->second) {
            case 1:
                return "Spring Blessing (Earth Mother)";
            case 2:
                return "Sun Festival (Sun God)";
            case 3:
                return "Remembrance Day (Death Goddess)";
            case 4:
                return "Night of Stars (Moon Goddess)";
            case 5:
                return "Warrior's Triumph (War God)";
            case 6:
                return "Market Festival (Trade God)";
            case 7:
                return "Fool's Day (Trickster)";
            case 8:
                return "Tide Blessing (Sea God)";
            case 9:
                return "Day of Reflection (Wisdom Goddess)";
            default:
                return "Unknown Holy Day";
            }
        }
        return "";
    }

    std::string getDeityOfCurrentHolyDay() const
    {
        auto it = holyDayCalendar.find(getCurrentDayOfYear());
        if (it != holyDayCalendar.end()) {
            switch (it->second) {
            case 1:
                return "earth_mother";
            case 2:
                return "sun_god";
            case 3:
                return "death_goddess";
            case 4:
                return "moon_goddess";
            case 5:
                return "war_god";
            case 6:
                return "trade_god";
            case 7:
                return "trickster";
            case 8:
                return "sea_god";
            case 9:
                return "wisdom_goddess";
            default:
                return "";
            }
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

    TempleNode(const std::string& name, const std::string& location, const std::string& deity)
        : TANode(name)
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
};

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

    // Initialize the religion system
    void initializeReligionSystem()
    {
        std::cout << "Initializing Religion System..." << std::endl;

        // Create pantheon root node
        religionRoot = createNode("ReligionSystem");

        // Create deities
        std::cout << "Creating pantheon of deities..." << std::endl;
        createDeities();

        // Create temples
        std::cout << "Creating temples..." << std::endl;
        createTemples();

        // Create rituals
        std::cout << "Creating rituals..." << std::endl;
        createRituals();

        // Create blessings
        std::cout << "Creating blessings..." << std::endl;
        createBlessings();

        // Create religious quests
        std::cout << "Creating religious quests..." << std::endl;
        createReligiousQuests();

        // Set up hierarchy
        std::cout << "Setting up Religion System hierarchy..." << std::endl;
        setupReligionHierarchy();

        // Register the religion system root
        setSystemRoot("ReligionSystem", religionRoot);

        std::cout << "Religion System initialization complete." << std::endl;
    }

private:
    // Create all deities in the pantheon
    void createDeities()
    {
        // Sun God
        DeityNode* sunGod = dynamic_cast<DeityNode*>(
            createNode<DeityNode>("SunGod", "sun_god", "The Radiant One"));
        sunGod->deityDescription = "God of light, truth, and healing. Brings warmth and clarity to all.";
        sunGod->deityDomain = "Light, Truth, Healing";
        sunGod->alignmentRequirement = "good";

        // Add favored actions
        sunGod->favoredActions.push_back({ "heal_others", 5, "Your acts of healing please the Sun God." });
        sunGod->favoredActions.push_back({ "reveal_truth", 3, "Revealing hidden truths brings you favor with the Sun God." });
        sunGod->favoredActions.push_back({ "defeat_undead", 4, "Destroying the undead is a service to the light." });

        // Add disfavored actions
        sunGod->disfavoredActions.push_back({ "use_poison", -3, "The use of poison displeases the Sun God." });
        sunGod->disfavoredActions.push_back({ "steal", -2, "Theft is an affront to truth and honor." });
        sunGod->disfavoredActions.push_back({ "necromancy", -5, "Necromancy is abhorrent to the Sun God." });

        // Opposing deities
        sunGod->opposingDeities.push_back("moon_goddess");
        sunGod->opposingDeities.push_back("trickster");

        pantheon.push_back(sunGod);

        // Moon Goddess
        DeityNode* moonGoddess = dynamic_cast<DeityNode*>(
            createNode<DeityNode>("MoonGoddess", "moon_goddess", "Lady of Shadows"));
        moonGoddess->deityDescription = "Goddess of night, dreams, and intuition. She guides travelers through darkness.";
        moonGoddess->deityDomain = "Night, Dreams, Magic";
        moonGoddess->alignmentRequirement = "neutral";

        // Add favored actions
        moonGoddess->favoredActions.push_back({ "night_exploration", 3, "Exploring by moonlight pleases the Moon Goddess." });
        moonGoddess->favoredActions.push_back({ "dream_magic", 5, "Your connection to dream magic is noticed by the Lady of Shadows." });
        moonGoddess->favoredActions.push_back({ "mercy_to_enemies", 4, "Showing mercy resonates with the Moon Goddess's compassion." });

        // Add disfavored actions
        moonGoddess->disfavoredActions.push_back({ "destroy_night_creatures", -3, "Destroying creatures of the night displeases the Moon Goddess." });
        moonGoddess->disfavoredActions.push_back({ "burn_forest", -4, "Destroying natural sanctuaries angers the Moon Goddess." });

        // Opposing deities
        moonGoddess->opposingDeities.push_back("sun_god");

        pantheon.push_back(moonGoddess);

        // Earth Mother
        DeityNode* earthMother = dynamic_cast<DeityNode*>(
            createNode<DeityNode>("EarthMother", "earth_mother", "The Nurturing Earth"));
        earthMother->deityDescription = "Goddess of nature, fertility, and growth. She blesses crops and new life.";
        earthMother->deityDomain = "Nature, Fertility, Growth";
        earthMother->alignmentRequirement = "good";

        // Add favored actions
        earthMother->favoredActions.push_back({ "plant_trees", 3, "Planting trees and nurturing growth pleases the Earth Mother." });
        earthMother->favoredActions.push_back({ "protect_nature", 4, "Protecting natural places brings the Earth Mother's favor." });
        earthMother->favoredActions.push_back({ "help_births", 5, "Assisting with births and new life is sacred to the Earth Mother." });

        // Add disfavored actions
        earthMother->disfavoredActions.push_back({ "destroy_forest", -5, "Needless destruction of forests deeply offends the Earth Mother." });
        earthMother->disfavoredActions.push_back({ "kill_animals", -3, "Killing animals without need or respect displeases the Earth Mother." });

        pantheon.push_back(earthMother);

        // War God
        DeityNode* warGod = dynamic_cast<DeityNode*>(
            createNode<DeityNode>("WarGod", "war_god", "The Unyielding Blade"));
        warGod->deityDescription = "God of battle, strategy, and honor in combat. He favors the courageous.";
        warGod->deityDomain = "War, Strength, Victory";
        warGod->alignmentRequirement = "neutral";

        // Add favored actions
        warGod->favoredActions.push_back({ "honorable_combat", 5, "Defeating worthy opponents in honorable combat pleases the War God." });
        warGod->favoredActions.push_back({ "military_strategy", 3, "Demonstrating strategic brilliance in battle brings the War God's favor." });
        warGod->favoredActions.push_back({ "protect_weak", 4, "Using your strength to protect those who cannot protect themselves is honored." });

        // Add disfavored actions
        warGod->disfavoredActions.push_back({ "cowardice", -5, "Fleeing from battle without cause is despised by the War God." });
        warGod->disfavoredActions.push_back({ "dishonorable_tactics", -4, "Using poison or attacking from behind dishonors the War God." });

        // Opposing deities
        warGod->opposingDeities.push_back("trade_god");

        pantheon.push_back(warGod);

        // Trade God
        DeityNode* tradeGod = dynamic_cast<DeityNode*>(
            createNode<DeityNode>("TradeGod", "trade_god", "Master of Exchange"));
        tradeGod->deityDescription = "God of commerce, wealth, and fair exchange. He oversees all transactions.";
        tradeGod->deityDomain = "Trade, Wealth, Travel";
        tradeGod->alignmentRequirement = "neutral";

        // Add favored actions
        tradeGod->favoredActions.push_back({ "fair_trade", 3, "Conducting fair business pleases the Trade God." });
        tradeGod->favoredActions.push_back({ "establish_trade_route", 5, "Establishing new trade routes brings the Trade God's favor." });
        tradeGod->favoredActions.push_back({ "generous_donation", 4, "Generosity with wealth is valued by the Master of Exchange." });

        // Add disfavored actions
        tradeGod->disfavoredActions.push_back({ "theft", -4, "Stealing violates the principles of fair exchange." });
        tradeGod->disfavoredActions.push_back({ "destroy_market", -5, "Destroying marketplaces is abhorrent to the Trade God." });

        pantheon.push_back(tradeGod);

        // Death Goddess
        DeityNode* deathGoddess = dynamic_cast<DeityNode*>(
            createNode<DeityNode>("DeathGoddess", "death_goddess", "Keeper of Souls"));
        deathGoddess->deityDescription = "Goddess of death, rebirth, and the passage of souls. Neither cruel nor kind, but necessary.";
        deathGoddess->deityDomain = "Death, Rebirth, Transition";
        deathGoddess->alignmentRequirement = "neutral";

        // Add favored actions
        deathGoddess->favoredActions.push_back({ "proper_burial", 4, "Ensuring proper burial rites pleases the Death Goddess." });
        deathGoddess->favoredActions.push_back({ "release_suffering", 3, "Releasing those in endless suffering is a mercy in her eyes." });
        deathGoddess->favoredActions.push_back({ "honor_ancestors", 5, "Honoring ancestors and the dead brings the Death Goddess's favor." });

        // Add disfavored actions
        deathGoddess->disfavoredActions.push_back({ "necromancy", -5, "Using necromancy to trap souls is an abomination to the Death Goddess." });
        deathGoddess->disfavoredActions.push_back({ "desecrate_tombs", -4, "Desecrating tombs and disturbing the dead displeases the Keeper of Souls." });

        pantheon.push_back(deathGoddess);

        // Trickster
        DeityNode* trickster = dynamic_cast<DeityNode*>(
            createNode<DeityNode>("Trickster", "trickster", "The Laughing Shadow"));
        trickster->deityDescription = "Deity of mischief, cunning, and change. Neither good nor evil, but unpredictable.";
        trickster->deityDomain = "Trickery, Luck, Change";
        trickster->alignmentRequirement = "chaotic";

        // Add favored actions
        trickster->favoredActions.push_back({ "clever_deception", 3, "Using clever deception to achieve just ends amuses the Trickster." });
        trickster->favoredActions.push_back({ "humiliate_proud", 4, "Humbling the proud and arrogant delights the Laughing Shadow." });
        trickster->favoredActions.push_back({ "unexpected_change", 5, "Creating unexpected but positive change brings the Trickster's favor." });

        // Add disfavored actions
        trickster->disfavoredActions.push_back({ "predictable_actions", -3, "Being utterly predictable bores the Trickster." });
        trickster->disfavoredActions.push_back({ "humorless_cruelty", -4, "Cruelty without wit or purpose is beneath the Trickster's interest." });

        // Opposing deities
        trickster->opposingDeities.push_back("wisdom_goddess");
        trickster->opposingDeities.push_back("sun_god");

        pantheon.push_back(trickster);

        // Sea God
        DeityNode* seaGod = dynamic_cast<DeityNode*>(
            createNode<DeityNode>("SeaGod", "sea_god", "Lord of Tides"));
        seaGod->deityDescription = "God of oceans, storms, and voyages. His moods shift like the tides.";
        seaGod->deityDomain = "Sea, Weather, Travel";
        seaGod->alignmentRequirement = "neutral";

        // Add favored actions
        seaGod->favoredActions.push_back({ "seafaring", 4, "Successfully navigating the seas pleases the Sea God." });
        seaGod->favoredActions.push_back({ "respect_ocean", 3, "Showing respect to the ocean and its creatures brings the Sea God's favor." });
        seaGod->favoredActions.push_back({ "rescue_drowning", 5, "Rescuing those at risk of drowning honors the Lord of Tides." });

        // Add disfavored actions
        seaGod->disfavoredActions.push_back({ "pollute_water", -5, "Polluting waters deeply offends the Sea God." });
        seaGod->disfavoredActions.push_back({ "wanton_fishing", -3, "Excessive or wasteful fishing displeases the Sea God." });

        pantheon.push_back(seaGod);

        // Wisdom Goddess
        DeityNode* wisdomGoddess = dynamic_cast<DeityNode*>(
            createNode<DeityNode>("WisdomGoddess", "wisdom_goddess", "The Knowing Eye"));
        wisdomGoddess->deityDescription = "Goddess of wisdom, knowledge, and foresight. She values learning and prudence.";
        wisdomGoddess->deityDomain = "Wisdom, Knowledge, Foresight";
        wisdomGoddess->alignmentRequirement = "lawful";

        // Add favored actions
        wisdomGoddess->favoredActions.push_back({ "preserve_knowledge", 5, "Preserving ancient knowledge pleases the Wisdom Goddess." });
        wisdomGoddess->favoredActions.push_back({ "teach_others", 4, "Teaching others and sharing wisdom brings the Wisdom Goddess's favor." });
        wisdomGoddess->favoredActions.push_back({ "careful_planning", 3, "Making thoughtful, careful plans honors the Knowing Eye." });

        // Add disfavored actions
        wisdomGoddess->disfavoredActions.push_back({ "destroy_books", -5, "Destroying books and knowledge is abhorrent to the Wisdom Goddess." });
        wisdomGoddess->disfavoredActions.push_back({ "reckless_action", -3, "Acting without thought or consideration displeases the Wisdom Goddess." });

        // Opposing deities
        wisdomGoddess->opposingDeities.push_back("trickster");

        pantheon.push_back(wisdomGoddess);
    }

    // Create temples for various deities
    void createTemples()
    {
        // Sun Temple
        TempleNode* sunTemple = dynamic_cast<TempleNode*>(
            createNode<TempleNode>("SunTemple", "Capital City", "sun_god"));
        sunTemple->templeName = "Temple of Radiance";
        sunTemple->templeDescription = "A grand temple with golden domes that catch the sunlight. The interior is spacious and filled with light through crystal windows.";
        sunTemple->providesHealing = true;
        sunTemple->providesBlessings = true;
        sunTemple->serviceQuality = 5;

        // Add priests
        sunTemple->priests.push_back({ "Elara Sunborn", "High Priestess", "A woman with golden robes and a serene expression.", 5 });
        sunTemple->priests.push_back({ "Tomar Brightshield", "Temple Guardian", "A middle-aged man with a sun emblem on his shield.", 3 });

        // Add offerings
        sunTemple->possibleOfferings.push_back({ "Golden Symbol", "golden_symbol", 1, 10, "You place the golden symbol on the altar, where it catches the sunlight." });
        sunTemple->possibleOfferings.push_back({ "Healing Herbs", "healing_herbs", 5, 5, "You offer the healing herbs, which are used in the temple's rituals." });

        temples.push_back(sunTemple);

        // Moon Temple
        TempleNode* moonTemple = dynamic_cast<TempleNode*>(
            createNode<TempleNode>("MoonTemple", "Northern Forest", "moon_goddess"));
        moonTemple->templeName = "Sanctuary of Dreams";
        moonTemple->templeDescription = "A mystical stone temple open to the night sky. Silver symbols adorn the walls, and soft blue light emanates from enchanted crystals.";
        moonTemple->providesDivination = true;
        moonTemple->providesBlessings = true;
        moonTemple->serviceQuality = 4;

        // Add priests
        moonTemple->priests.push_back({ "Lysara Nightveil", "Dream Oracle", "A woman with silver hair who speaks in riddles.", 5 });
        moonTemple->priests.push_back({ "Thorne Shadowwalker", "Night Guardian", "A quiet man who tends the temple grounds at night.", 3 });

        // Add offerings
        moonTemple->possibleOfferings.push_back({ "Silver Mirror", "silver_mirror", 1, 10, "You place the silver mirror on the altar, where it reflects the moonlight." });
        moonTemple->possibleOfferings.push_back({ "Dream Essence", "dream_essence", 3, 8, "You pour the dream essence into the offering bowl, where it glows with an ethereal light." });

        temples.push_back(moonTemple);

        // Earth Temple
        TempleNode* earthTemple = dynamic_cast<TempleNode*>(
            createNode<TempleNode>("EarthTemple", "Green Valley", "earth_mother"));
        earthTemple->templeName = "Grove of Renewal";
        earthTemple->templeDescription = "Less a building and more a sacred grove of ancient trees. The center contains a stone altar covered in moss and wildflowers.";
        earthTemple->providesHealing = true;
        earthTemple->providesBlessings = true;
        earthTemple->serviceQuality = 4;

        // Add priests
        earthTemple->priests.push_back({ "Orana Greentouch", "Grove Keeper", "An elderly woman whose fingers seem to make plants grow at her touch.", 5 });
        earthTemple->priests.push_back({ "Bryn Earthson", "Nature Warden", "A sturdy man who protects the grove from any who would harm it.", 4 });

        // Add offerings
        earthTemple->possibleOfferings.push_back({ "Rare Seeds", "rare_seeds", 5, 8, "You plant the rare seeds in the sacred soil near the altar." });
        earthTemple->possibleOfferings.push_back({ "Pure Water", "pure_water", 3, 5, "You pour the pure water around the base of the ancient tree." });

        temples.push_back(earthTemple);

        // War Temple
        TempleNode* warTemple = dynamic_cast<TempleNode*>(
            createNode<TempleNode>("WarTemple", "Fortress City", "war_god"));
        warTemple->templeName = "Hall of Valor";
        warTemple->templeDescription = "A stone fortress-temple with walls adorned with weapons and battle standards. Training grounds surround the main structure.";
        warTemple->providesBlessings = true;
        warTemple->serviceQuality = 5;

        // Add priests
        warTemple->priests.push_back({ "Korvan Ironheart", "Battle Priest", "A scarred veteran who leads warriors in prayer before battle.", 5 });
        warTemple->priests.push_back({ "Mara Steelblade", "War Priestess", "A stern woman who tests the worthiness of those seeking the War God's blessing.", 4 });

        // Add offerings
        warTemple->possibleOfferings.push_back({ "Trophy Weapon", "trophy_weapon", 1, 10, "You place the weapon taken from a worthy opponent on the altar." });
        warTemple->possibleOfferings.push_back({ "War Banner", "war_banner", 1, 8, "You hang the banner among the many that adorn the temple walls." });

        temples.push_back(warTemple);

        // Add more temples for other deities...
    }

    // Create rituals for different deities
    void createRituals()
    {
        // Sun God Rituals
        RitualNode* sunRitual = dynamic_cast<RitualNode*>(
            createNode<RitualNode>("SunRitual", "Ritual of Dawn's Light", "sun_god"));
        sunRitual->ritualDescription = "A ceremony performed at dawn to honor the Sun God and request his blessing for the day ahead.";
        sunRitual->favorRequirement = 20;
        sunRitual->requiresHolyDay = false;
        sunRitual->goldCost = 30;
        sunRitual->itemRequirements["candle"] = 3;
        sunRitual->favorReward = 15;
        sunRitual->blessingGranted = "sun_protection";
        sunRitual->blessingDuration = 7;
        sunRitual->complexity = RitualNode::MODERATE;

        rituals.push_back(sunRitual);

        RitualNode* sunHolyRitual = dynamic_cast<RitualNode*>(
            createNode<RitualNode>("SunHolyRitual", "Grand Solar Cleansing", "sun_god"));
        sunHolyRitual->ritualDescription = "A powerful ritual performed only on the summer solstice, cleansing an area of undead influence and dark magic.";
        sunHolyRitual->favorRequirement = 50;
        sunHolyRitual->requiresHolyDay = true;
        sunHolyRitual->goldCost = 100;
        sunHolyRitual->itemRequirements["golden_symbol"] = 1;
        sunHolyRitual->itemRequirements["purification_oil"] = 3;
        sunHolyRitual->favorReward = 30;
        sunHolyRitual->blessingGranted = "solar_radiance";
        sunHolyRitual->blessingDuration = 30;
        sunHolyRitual->complexity = RitualNode::ELABORATE;

        rituals.push_back(sunHolyRitual);

        // Moon Goddess Rituals
        RitualNode* moonRitual = dynamic_cast<RitualNode*>(
            createNode<RitualNode>("MoonRitual", "Lunar Communion", "moon_goddess"));
        moonRitual->ritualDescription = "A meditative ritual performed under moonlight to connect with the Goddess's dream realm.";
        moonRitual->favorRequirement = 20;
        moonRitual->requiresHolyDay = false;
        moonRitual->goldCost = 25;
        moonRitual->itemRequirements["moon_crystal"] = 1;
        moonRitual->itemRequirements["dream_herb"] = 2;
        moonRitual->favorReward = 15;
        moonRitual->skillBoost = 1;
        moonRitual->skillAffected = "magic";
        moonRitual->blessingGranted = "night_vision";
        moonRitual->blessingDuration = 5;
        moonRitual->complexity = RitualNode::MODERATE;

        rituals.push_back(moonRitual);

        // Earth Mother Rituals
        RitualNode* earthRitual = dynamic_cast<RitualNode*>(
            createNode<RitualNode>("EarthRitual", "Seed Blessing", "earth_mother"));
        earthRitual->ritualDescription = "A ritual to bless seeds for planting, ensuring bountiful harvests.";
        earthRitual->favorRequirement = 15;
        earthRitual->requiresHolyDay = false;
        earthRitual->goldCost = 20;
        earthRitual->itemRequirements["fertile_soil"] = 1;
        earthRitual->itemRequirements["grain_seeds"] = 10;
        earthRitual->favorReward = 10;
        earthRitual->blessingGranted = "earth_bounty";
        earthRitual->blessingDuration = 14;
        earthRitual->complexity = RitualNode::SIMPLE;

        rituals.push_back(earthRitual);

        // War God Rituals
        RitualNode* warRitual = dynamic_cast<RitualNode*>(
            createNode<RitualNode>("WarRitual", "Warrior's Oath", "war_god"));
        warRitual->ritualDescription = "A blood oath sworn to the War God before a significant battle.";
        warRitual->favorRequirement = 25;
        warRitual->requiresHolyDay = false;
        warRitual->goldCost = 50;
        warRitual->itemRequirements["weapon"] = 1;
        warRitual->favorReward = 20;
        warRitual->skillBoost = 2;
        warRitual->skillAffected = "combat";
        warRitual->blessingGranted = "battle_fury";
        warRitual->blessingDuration = 3;
        warRitual->complexity = RitualNode::COMPLEX;

        rituals.push_back(warRitual);

        // Add more rituals for other deities...
    }

    // Create blessings for different deities
    void createBlessings()
    {
        // Sun God Blessings
        BlessingNode* sunMinorBlessing = dynamic_cast<BlessingNode*>(
            createNode<BlessingNode>("SunMinorBlessing", "Blessing of Light", "sun_god", BlessingNode::MINOR));
        sunMinorBlessing->blessingDescription = "A minor blessing from the Sun God that grants improved vision in darkness and resistance to fear.";
        sunMinorBlessing->effects.push_back({ "stat", "wisdom", 1, "Wisdom +1 while active" });
        sunMinorBlessing->effects.push_back({ "protection", "fear", 25, "25% resistance to fear effects" });

        blessings.push_back(sunMinorBlessing);

        BlessingNode* sunMajorBlessing = dynamic_cast<BlessingNode*>(
            createNode<BlessingNode>("SunMajorBlessing", "Radiant Protection", "sun_god", BlessingNode::MAJOR));
        sunMajorBlessing->blessingDescription = "A powerful blessing that provides significant protection against undead and grants healing abilities.";
        sunMajorBlessing->effects.push_back({ "protection", "undead", 50, "50% damage reduction against undead" });
        sunMajorBlessing->effects.push_back({ "ability", "minor_heal", 0, "Ability to cast Minor Heal once per day" });

        blessings.push_back(sunMajorBlessing);

        BlessingNode* sunDivineBlessing = dynamic_cast<BlessingNode*>(
            createNode<BlessingNode>("SunDivineBlessing", "Sun's Champion", "sun_god", BlessingNode::DIVINE));
        sunDivineBlessing->blessingDescription = "The highest blessing of the Sun God, marking you as his chosen champion with powerful light-based abilities.";
        sunDivineBlessing->effects.push_back({ "stat", "strength", 3, "Strength +3 while active" });
        sunDivineBlessing->effects.push_back({ "stat", "charisma", 3, "Charisma +3 while active" });
        sunDivineBlessing->effects.push_back({ "ability", "divine_radiance", 0, "Ability to emit a powerful burst of sunlight once per day" });

        blessings.push_back(sunDivineBlessing);

        // Moon Goddess Blessings
        BlessingNode* moonMinorBlessing = dynamic_cast<BlessingNode*>(
            createNode<BlessingNode>("MoonMinorBlessing", "Night's Embrace", "moon_goddess", BlessingNode::MINOR));
        moonMinorBlessing->blessingDescription = "A minor blessing that improves stealth at night and enhances dream recall.";
        moonMinorBlessing->effects.push_back({ "skill", "stealth", 2, "+2 to Stealth skill at night" });

        blessings.push_back(moonMinorBlessing);

        // Earth Mother Blessings
        BlessingNode* earthMinorBlessing = dynamic_cast<BlessingNode*>(
            createNode<BlessingNode>("EarthMinorBlessing", "Nature's Touch", "earth_mother", BlessingNode::MINOR));
        earthMinorBlessing->blessingDescription = "A connection to nature that allows easier foraging and identification of plants.";
        earthMinorBlessing->effects.push_back({ "skill", "survival", 2, "+2 to Survival skill when foraging" });

        blessings.push_back(earthMinorBlessing);

        // War God Blessings
        BlessingNode* warMinorBlessing = dynamic_cast<BlessingNode*>(
            createNode<BlessingNode>("WarMinorBlessing", "Warrior's Courage", "war_god", BlessingNode::MINOR));
        warMinorBlessing->blessingDescription = "A blessing that grants courage in battle and improves combat reflexes.";
        warMinorBlessing->effects.push_back({ "stat", "strength", 1, "Strength +1 while active" });
        warMinorBlessing->effects.push_back({ "protection", "fear", 50, "50% resistance to fear in combat" });

        blessings.push_back(warMinorBlessing);

        // Add more blessings for other deities...
    }

    // Create religious quests
    void createReligiousQuests()
    {
        // Sun God Quest
        ReligiousQuestNode* sunQuest = dynamic_cast<ReligiousQuestNode*>(
            createNode<ReligiousQuestNode>("SunQuest", "sun_god"));
        sunQuest->questTitle = "Cleanse the Darkness";
        sunQuest->questDescription = "A crypt near the village has become infested with undead. The Sun God calls upon you to cleanse it with his holy light.";
        sunQuest->favorReward = 25;
        sunQuest->devotionReward = 20;

        // Add rewards
        sunQuest->rewards.push_back({ "experience", 200, "" });
        sunQuest->rewards.push_back({ "gold", 100, "" });
        sunQuest->rewards.push_back({ "item", 1, "sunlight_amulet" });

        // Add world state changes
        sunQuest->worldStateChanges["crypt_cleansed"] = "true";

        religiousQuests.push_back(sunQuest);

        // Moon Goddess Quest
        ReligiousQuestNode* moonQuest = dynamic_cast<ReligiousQuestNode*>(
            createNode<ReligiousQuestNode>("MoonQuest", "moon_goddess"));
        moonQuest->questTitle = "Recover the Lunar Fragments";
        moonQuest->questDescription = "Pieces of a sacred lunar artifact have been scattered. Find them under the light of the full moon and return them to the Moon Goddess's temple.";
        moonQuest->favorReward = 30;
        moonQuest->devotionReward = 25;

        // Add rewards
        moonQuest->rewards.push_back({ "experience", 250, "" });
        moonQuest->rewards.push_back({ "gold", 75, "" });
        moonQuest->rewards.push_back({ "item", 1, "moon_crystal" });
        moonQuest->rewards.push_back({ "skill", 2, "magic" });

        // Add world state changes
        moonQuest->worldStateChanges["lunar_artifact_restored"] = "true";

        religiousQuests.push_back(moonQuest);

        // Add more religious quests for other deities...
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

    // Initialize the religion system
    controller.initializeReligionSystem();

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