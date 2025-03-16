#pragma once

#include "core/TANode.hpp"
#include "systems/religion/ReligiousGameContext.hpp"
#include <map>
#include <string>

// Forward declaration
class DeityNode;

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

    PrayerNode(const std::string& name, const std::string& deity);
    void initializePrayerResults();
    void onEnter(ReligiousGameContext* context);
    DeityNode* findDeityNode(ReligiousGameContext* context) const;
    PrayerResult getPrayerOutcome(ReligiousGameContext* context, PrayerType type);
    void performPrayer(ReligiousGameContext* context, PrayerType type);
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;
};