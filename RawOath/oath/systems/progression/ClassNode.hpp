#pragma once

#include "../../core/TANode.hpp"
#include "../../data/GameContext.hpp"
#include "SkillNode.hpp"

#include <map>
#include <set>
#include <string>
#include <vector>

// Forward declaration
class TANode;
struct GameContext;
class SkillNode;

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