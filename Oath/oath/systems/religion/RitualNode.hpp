#pragma once

#include "core/TANode.hpp"
#include "systems/religion/ReligiousGameContext.hpp"
#include <map>
#include <string>

// Forward declaration
class DeityNode;

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

    RitualNode(const std::string& id, const std::string& name, const std::string& deity);
    void loadFromJson(const nlohmann::json& ritualData);
    void onEnter(ReligiousGameContext* context);
    DeityNode* findDeityNode(ReligiousGameContext* context) const;
    bool canPerformRitual(ReligiousGameContext* context) const;
    bool performRitual(ReligiousGameContext* context);
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;
};