#pragma once

#include "../../core/TANode.hpp"
#include "../../data/GameContext.hpp"

#include <string>

// Time/Season system
class TimeNode : public TANode {
public:
    int day;
    int hour;
    std::string season;
    std::string timeOfDay;

    TimeNode(const std::string& name);
    void advanceHour(GameContext* context);
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;
};