// Region node for world map
class RegionNode : public TANode {
public:
    std::string regionName;
    std::string description;
    std::string controllingFaction;

    // Locations in this region
    std::vector<LocationNode*> locations;

    // Connected regions
    std::vector<RegionNode*> connectedRegions;

    // Events that can happen in this region
    struct RegionEvent {
        std::string name;
        std::string description;
        std::function<bool(const GameContext&)> condition;
        std::function<void(GameContext*)> effect;
        double probability;
    };
    std::vector<RegionEvent> possibleEvents;

    RegionNode(const std::string& name, const std::string& region);
    void onEnter(GameContext* context) override;
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;
};