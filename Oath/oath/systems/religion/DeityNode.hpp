#pragma once

#include "core/TANode.hpp"
#include "systems/religion/ReligiousGameContext.hpp"
#include <string>
#include <vector>

// Forward declarations
class BlessingNode;
class TempleNode;

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

    DeityNode(const std::string& name, const std::string& id, const std::string& title);
    void loadFromJson(const nlohmann::json& deityData);
    void onEnter(ReligiousGameContext* context);
    bool isHolyDay(ReligiousGameContext* context) const;
    std::string getFavorLevel(int favor) const;
    bool canGrantBlessing(ReligiousGameContext* context, const std::string& blessingId) const;
    virtual void processDevotionAction(ReligiousGameContext* context, const std::string& actionType);
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;
};