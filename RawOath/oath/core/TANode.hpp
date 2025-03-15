// Core node class for tree automata system
class TANode {
public:
    // Unique identifier for this node
    NodeID nodeID;

    // Human-readable name
    std::string nodeName;

    // Current state data - flexible for any system-specific info
    std::map<std::string, std::variant<int, float, std::string, bool>> stateData;

    // Transition rules to other nodes
    std::vector<TATransitionRule> transitionRules;

    // Child nodes (for hierarchical structures)
    std::vector<TANode*> childNodes;

    // Is this a terminal/accepting state?
    bool isAcceptingState;

    TANode(const std::string& name);
    virtual ~TANode() = default;

    // Evaluation function to process inputs
    virtual bool evaluateTransition(const TAInput& input, TANode*& outNextNode);

    // Actions to perform when entering/exiting this node
    virtual void onEnter(GameContext* context);
    virtual void onExit(GameContext* context);

    // Add a transition rule
    void addTransition(const std::function<bool(const TAInput&)>& condition,
        TANode* target, const std::string& description = "");

    // Add a child node
    void addChild(TANode* child);

    // Get available transitions for the current state
    virtual std::vector<TAAction> getAvailableActions();

    // Generate a path-based ID for this node
    void generatePersistentID(const std::string& parentPath = "");

    // Serialize node state
    virtual void serialize(std::ofstream& file) const;

    // Deserialize node state
    virtual bool deserialize(std::ifstream& file);
};