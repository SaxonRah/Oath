// An input that can trigger transitions
struct TAInput {
    std::string type;
    std::map<std::string, std::variant<int, float, std::string, bool>> parameters;
};