#pragma once

#include "core/TANode.hpp"
#include "systems/religion/ReligiousGameContext.hpp"
#include <string>
#include <vector>

// Forward declarations
class DeityNode;
class RitualNode;

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

    TempleNode(const std::string& id, const std::string& name, const std::string& location, const std::string& deity);
    void loadFromJson(const nlohmann::json& templeData);
    void onEnter(ReligiousGameContext* context);
    DeityNode* findDeityNode(ReligiousGameContext* context) const;
    bool makeOffering(ReligiousGameContext* context, const std::string& itemId, int quantity);
    bool provideHealing(ReligiousGameContext* context, int gold);
    bool removeCurse(ReligiousGameContext* context, int gold);
    std::string performDivination(ReligiousGameContext* context, int gold);
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;
};