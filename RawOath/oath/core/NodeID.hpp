// A unique identifier for nodes
struct NodeID {
    unsigned int data1;
    unsigned int data2;
    unsigned int data3;
    unsigned int data4;
    std::string persistentID; // Path-based or name-based persistent ID

    bool operator==(const NodeID& other) const;
    bool operator<(const NodeID& other) const;
    static NodeID Generate(const std::string& nodePath = "");
    std::string toString() const;
};