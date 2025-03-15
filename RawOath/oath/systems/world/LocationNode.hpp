// Location node for world state
class LocationNode : public TANode {
public:
    std::string locationName;
    std::string description;
    std::string currentState;
    std::map<std::string, std::string> stateDescriptions;

    // NPCs at this location
    std::vector<NPC*> npcs;

    // Available activities at this location
    std::vector<TANode*> activities;

    // Conditions to access this location
    struct AccessCondition {
        std::string type;
        std::string target;
        int value;

        bool check(const GameContext& context) const;
    };
    std::vector<AccessCondition> accessConditions;

    LocationNode(const std::string& name, const std::string& location,
        const std::string& initialState = "normal");

    bool canAccess(const GameContext& context) const;
    void onEnter(GameContext* context) override;
    std::vector<TAAction> getAvailableActions() override;
};