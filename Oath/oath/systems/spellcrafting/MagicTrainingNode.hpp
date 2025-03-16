#pragma once

#include "../../core/TANode.hpp"
#include <string>
#include <vector>


class MagicTrainingNode : public TANode {
private:
    std::vector<std::string> availableSchools;

public:
    MagicTrainingNode(const std::string& name);

    void onEnter(GameContext* context) override;

    void listAvailableSchools();
    void trainSchool(const std::string& school, int hours, GameContext* context);

    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;
};