#pragma once

#include "../../core/TANode.hpp"
#include "SkillNode.hpp"

#include <map>
#include <set>
#include <string>
#include <vector>

// Class specialization node
class ClassNode : public TANode {
public:
    std::string className;
    std::string description;
    std::map<std::string, int> statBonuses;
    std::set<std::string> startingAbilities;
    std::vector<SkillNode*> classSkills;

    ClassNode(const std::string& name, const std::string& classType);
    void onEnter(GameContext* context) override;
};