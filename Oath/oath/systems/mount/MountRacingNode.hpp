#pragma once

#include "../../core/TANode.hpp"
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

class Mount;
class GameContext;
struct TAInput;
struct TAAction;

// Mount racing event node
class MountRacingNode : public TANode {
public:
    std::string trackName;
    float trackLength;
    int difficulty;
    int entryFee;
    int firstPrize;
    int secondPrize;
    int thirdPrize;

    struct RaceCompetitor {
        std::string name;
        int speed;
        int stamina;
        int skill;
    };
    std::vector<RaceCompetitor> competitors;

    MountRacingNode(const std::string& name, const std::string& track, float length, int diff);
    static MountRacingNode* createFromJson(const std::string& name, const nlohmann::json& j);
    void generateCompetitors(int count, const std::vector<std::string>& names = {},
        const std::vector<std::string>& lastNames = {}, int baseDifficulty = 0);
    void onEnter(GameContext* context) override;

    struct RaceResult {
        std::string name;
        float time;
        int position;
    };

    std::vector<RaceResult> simulateRace(Mount* playerMount);
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;
};