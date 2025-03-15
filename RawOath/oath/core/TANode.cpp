#include <TANode.hpp>

TANode::TANode(const std::string& name)
    : nodeID(NodeID::Generate())
    , nodeName(name)
    , isAcceptingState(false)
{
}

bool TANode::evaluateTransition(const TAInput& input, TANode*& outNextNode)
{
    for (const auto& rule : transitionRules) {
        if (rule.condition(input)) {
            outNextNode = rule.targetNode;
            return true;
        }
    }
    return false;
}

void TANode::onEnter(GameContext* context)
{
    std::cout << "Entered node: " << nodeName << std::endl;
}

void TANode::onExit(GameContext* context)
{
    std::cout << "Exited node: " << nodeName << std::endl;
}

void TANode::addTransition(const std::function<bool(const TAInput&)>& condition,
    TANode* target, const std::string& description)
{
    transitionRules.push_back({ condition, target, description });
}

void TANode::addChild(TANode* child)
{
    childNodes.push_back(child);
}

std::vector<TAAction> TANode::getAvailableActions()
{
    std::vector<TAAction> actions;
    for (size_t i = 0; i < transitionRules.size(); i++) {
        const auto& rule = transitionRules[i];
        actions.push_back(
            { "transition_" + std::to_string(i), rule.description,
                [this, i]() -> TAInput {
                    return { "transition", { { "index", static_cast<int>(i) } } };
                } });
    }
    return actions;
}

void TANode::generatePersistentID(const std::string& parentPath)
{
    std::string path;
    if (parentPath.empty()) {
        // Root node case
        path = nodeName;
    } else {
        // Child node case - include parent path
        path = parentPath + "/" + nodeName;
    }

    nodeID.persistentID = path;

    // Update child nodes recursively
    for (TANode* child : childNodes) {
        child->generatePersistentID(path);
    }
}

void TANode::serialize(std::ofstream& file) const
{
    // Write state data
    size_t stateDataSize = stateData.size();
    if (stateDataSize > 1000) {
        std::cerr << "Warning: Large state data size: " << stateDataSize << std::endl;
        stateDataSize = 1000; // Limit to prevent huge allocations
    }
    file.write(reinterpret_cast<const char*>(&stateDataSize), sizeof(stateDataSize));

    size_t count = 0;
    for (const auto& [key, value] : stateData) {
        if (count >= stateDataSize)
            break;

        // Skip empty keys or extremely long keys
        if (key.empty() || key.length() > 1000) {
            std::cerr << "Warning: Skipping invalid key with length " << key.length() << std::endl;
            continue;
        }

        // Write key
        size_t keyLength = key.length();
        file.write(reinterpret_cast<const char*>(&keyLength), sizeof(keyLength));
        file.write(key.c_str(), keyLength);

        // Write variant type and value
        if (std::holds_alternative<int>(value)) {
            char type = 'i';
            file.write(&type, 1);
            int val = std::get<int>(value);
            file.write(reinterpret_cast<const char*>(&val), sizeof(val));
        } else if (std::holds_alternative<float>(value)) {
            char type = 'f';
            file.write(&type, 1);
            float val = std::get<float>(value);
            file.write(reinterpret_cast<const char*>(&val), sizeof(val));
        } else if (std::holds_alternative<std::string>(value)) {
            char type = 's';
            file.write(&type, 1);
            std::string val = std::get<std::string>(value);

            // Limit string length
            if (val.length() > 10000) {
                std::cerr << "Warning: Truncating long string value for key " << key << std::endl;
                val = val.substr(0, 10000);
            }

            size_t valLength = val.length();
            file.write(reinterpret_cast<const char*>(&valLength), sizeof(valLength));
            file.write(val.c_str(), valLength);
        } else if (std::holds_alternative<bool>(value)) {
            char type = 'b';
            file.write(&type, 1);
            bool val = std::get<bool>(value);
            file.write(reinterpret_cast<const char*>(&val), sizeof(val));
        }

        count++;
    }
}

bool TANode::deserialize(std::ifstream& file)
{
    // Read state data
    size_t stateDataSize;
    if (!file.read(reinterpret_cast<char*>(&stateDataSize), sizeof(stateDataSize))) {
        std::cerr << "Failed to read state data size" << std::endl;
        return false;
    }

    // Sanity check
    const size_t MAX_STATE_SIZE = 1000; // Reasonable maximum
    if (stateDataSize > MAX_STATE_SIZE) {
        std::cerr << "Invalid state data size: " << stateDataSize << std::endl;
        return false;
    }

    stateData.clear();
    for (size_t i = 0; i < stateDataSize; i++) {
        // Check file position is valid
        if (file.eof()) {
            std::cerr << "Unexpected end of file during node deserialization" << std::endl;
            return false;
        }

        // Read key length
        size_t keyLength;
        if (!file.read(reinterpret_cast<char*>(&keyLength), sizeof(keyLength))) {
            std::cerr << "Failed to read key length for item " << i << std::endl;
            return false;
        }

        // Sanity check - enforce reasonable maximum
        const size_t MAX_KEY_LENGTH = 1000;
        if (keyLength == 0 || keyLength > MAX_KEY_LENGTH) {
            std::cerr << "Invalid key length: " << keyLength << std::endl;
            return false;
        }

        // Read key with verified length
        std::string key(keyLength, ' ');
        if (!file.read(&key[0], keyLength)) {
            std::cerr << "Failed to read key with length " << keyLength << std::endl;
            return false;
        }

        // Verify type is valid
        char type;
        if (!file.read(&type, 1)) {
            std::cerr << "Failed to read type for key " << key << std::endl;
            return false;
        }

        // Only accept valid type characters
        if (type != 'i' && type != 'f' && type != 's' && type != 'b') {
            std::cerr << "Invalid type character: " << type << " for key " << key << std::endl;
            return false;
        }

        // Handle each type with validation
        if (type == 'i') {
            int val;
            if (!file.read(reinterpret_cast<char*>(&val), sizeof(val))) {
                std::cerr << "Failed to read int value for key " << key << std::endl;
                return false;
            }
            stateData[key] = val;
        } else if (type == 'f') {
            float val;
            if (!file.read(reinterpret_cast<char*>(&val), sizeof(val))) {
                std::cerr << "Failed to read float value for key " << key << std::endl;
                return false;
            }
            stateData[key] = val;
        } else if (type == 's') {
            size_t valLength;
            if (!file.read(reinterpret_cast<char*>(&valLength), sizeof(valLength))) {
                std::cerr << "Failed to read string length for key " << key << std::endl;
                return false;
            }

            // Sanity check
            const size_t MAX_STRING_LENGTH = 10000;
            if (valLength > MAX_STRING_LENGTH) {
                std::cerr << "Invalid string length: " << valLength << " for key " << key << std::endl;
                return false;
            }

            std::string val(valLength, ' ');
            if (!file.read(&val[0], valLength)) {
                std::cerr << "Failed to read string value for key " << key << std::endl;
                return false;
            }
            stateData[key] = val;
        } else if (type == 'b') {
            bool val;
            if (!file.read(reinterpret_cast<char*>(&val), sizeof(val))) {
                std::cerr << "Failed to read bool value for key " << key << std::endl;
                return false;
            }
            stateData[key] = val;
        }
    }

    return true;
}