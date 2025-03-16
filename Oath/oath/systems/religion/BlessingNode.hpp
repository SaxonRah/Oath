#pragma once

#include "core/TANode.hpp"
#include "systems/religion/ReligiousGameContext.hpp"
#include <string>
#include <vector>

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

    BlessingNode(const std::string& id, const std::string& name, const std::string& deity, BlessingTier t);
    void loadFromJson(const nlohmann::json& blessingData);
    void onEnter(ReligiousGameContext* context);
    std::string getTierName() const;
    bool canReceiveBlessing(ReligiousGameContext* context) const;
    bool grantBlessing(ReligiousGameContext* context);
    std::vector<TAAction> getAvailableActions() override;
};