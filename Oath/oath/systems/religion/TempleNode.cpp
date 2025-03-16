#include "systems/religion/TempleNode.hpp"
#include "core/TAAction.hpp"
#include "core/TAInput.hpp"
#include "systems/religion/DeityNode.hpp"
#include "systems/religion/RitualNode.hpp"
#include <iostream>
#include <nlohmann/json.hpp>
#include <random>

TempleNode::TempleNode(const std::string& id, const std::string& name, const std::string& location, const std::string& deity)
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

void TempleNode::loadFromJson(const nlohmann::json& templeData)
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

void TempleNode::onEnter(ReligiousGameContext* context)
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

DeityNode* TempleNode::findDeityNode(ReligiousGameContext* context) const
{
    // This would need to be implemented to find the deity node from the controller
    // For now, return nullptr as a placeholder
    return nullptr;
}

bool TempleNode::makeOffering(ReligiousGameContext* context, const std::string& itemId, int quantity)
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

bool TempleNode::provideHealing(ReligiousGameContext* context, int gold)
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

bool TempleNode::removeCurse(ReligiousGameContext* context, int gold)
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

std::string TempleNode::performDivination(ReligiousGameContext* context, int gold)
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

std::vector<TAAction> TempleNode::getAvailableActions()
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

bool TempleNode::evaluateTransition(const TAInput& input, TANode*& outNextNode)
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