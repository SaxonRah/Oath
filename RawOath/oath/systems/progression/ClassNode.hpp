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