#include <NodeID.hpp>

bool NodeID::operator==(const NodeID& other) const
{
    // Check persistent ID first if available
    if (!persistentID.empty() && !other.persistentID.empty()) {
        return persistentID == other.persistentID;
    }
    // Fall back to numeric ID comparison
    return data1 == other.data1 && data2 == other.data2 && data3 == other.data3 && data4 == other.data4;
}

bool NodeID::operator<(const NodeID& other) const
{
    // Use persistent ID for comparison if available
    if (!persistentID.empty() && !other.persistentID.empty()) {
        return persistentID < other.persistentID;
    }
    // Fall back to numeric comparison
    if (data1 != other.data1)
        return data1 < other.data1;
    if (data2 != other.data2)
        return data2 < other.data2;
    if (data3 != other.data3)
        return data3 < other.data3;
    return data4 < other.data4;
}

NodeID NodeID::Generate(const std::string& nodePath)
{
    static unsigned int counter = 0;
    NodeID id = { ++counter, 0, 0, 0 };
    id.persistentID = nodePath;
    return id;
}

std::string NodeID::toString() const
{
    if (!persistentID.empty()) {
        return persistentID;
    }
    std::stringstream ss;
    ss << data1 << "-" << data2 << "-" << data3 << "-" << data4;
    return ss.str();
}