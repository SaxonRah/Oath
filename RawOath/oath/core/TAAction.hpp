// Available actions from a state
struct TAAction {
    std::string name;
    std::string description;
    std::function<TAInput()> createInput;
};