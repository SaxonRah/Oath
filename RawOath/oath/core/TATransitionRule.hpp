// Transition rule
struct TATransitionRule {
    std::function<bool(const TAInput&)> condition;
    TANode* targetNode;
    std::string description;
};