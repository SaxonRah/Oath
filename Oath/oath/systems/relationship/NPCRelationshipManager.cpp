#include "NPCRelationshipManager.hpp"
#include "RelationshipConfig.hpp"
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>

NPCRelationshipManager::NPCRelationshipManager()
    : currentGameDay(0)
{
    loadNPCsFromConfig();
}

void NPCRelationshipManager::loadNPCsFromConfig()
{
    RelationshipConfig& config = RelationshipConfig::getInstance();

    // Load NPC definitions
    const nlohmann::json& npcsData = config.getNPCs();
    for (const auto& npcData : npcsData) {
        RelationshipNPC npc(npcData);
        registerNPC(npc);
    }

    // Set up default relationships
    const nlohmann::json& defaultRelationships = config.getDefaultRelationships();
    for (const auto& rel : defaultRelationships) {
        std::string npcId = rel["npcId"];
        int value = rel["value"];
        RelationshipType type = config.getRelationshipTypeFromString(rel["type"]);

        playerRelationships[npcId] = value;
        playerRelationshipTypes[npcId] = type;
    }
}

void NPCRelationshipManager::registerNPC(const RelationshipNPC& npc)
{
    npcs[npc.id] = npc;
    if (playerRelationships.find(npc.id) == playerRelationships.end()) {
        playerRelationships[npc.id] = 0; // Start neutral
        playerRelationshipTypes[npc.id] = RelationshipType::None;
        playerRelationshipStates[npc.id] = RelationshipState::Neutral;
        lastGiftDay[npc.id] = -RelationshipConfig::getInstance().getMinDaysBetweenGifts(); // Allow immediate gift
    }
}

RelationshipNPC* NPCRelationshipManager::getNPC(const std::string& npcId)
{
    if (npcs.find(npcId) != npcs.end()) {
        return &npcs[npcId];
    }
    return nullptr;
}

void NPCRelationshipManager::changeRelationship(const std::string& npcId, int amount)
{
    if (npcs.find(npcId) == npcs.end())
        return;

    RelationshipConfig& config = RelationshipConfig::getInstance();

    // Apply personality trait modifiers
    const RelationshipNPC& npc = npcs[npcId];

    // Vengeful NPCs remember negative actions more
    if (amount < 0 && npc.personalityTraits.find(PersonalityTrait::Vengeful) != npc.personalityTraits.end()) {
        amount = static_cast<int>(amount * 1.5f);
    }

    // Forgiving NPCs are less affected by negative actions
    if (amount < 0 && npc.personalityTraits.find(PersonalityTrait::Merciful) != npc.personalityTraits.end()) {
        amount = static_cast<int>(amount * 0.5f);
    }

    // Update relationship value with limits
    playerRelationships[npcId] = std::max(
        config.getMinRelationship(),
        std::min(config.getMaxRelationship(), playerRelationships[npcId] + amount));

    // Update relationship type based on new value
    updateRelationshipType(npcId);
}

void NPCRelationshipManager::updateRelationshipType(const std::string& npcId)
{
    RelationshipConfig& config = RelationshipConfig::getInstance();
    int value = playerRelationships[npcId];

    // Update type based on value thresholds
    if (value <= config.getHatredThreshold()) {
        playerRelationshipTypes[npcId] = RelationshipType::Enemy;
    } else if (value <= config.getDislikeThreshold()) {
        playerRelationshipTypes[npcId] = RelationshipType::Rival;
    } else if (value <= config.getNeutralThreshold()) {
        playerRelationshipTypes[npcId] = RelationshipType::None;
    } else if (value <= config.getFriendlyThreshold()) {
        playerRelationshipTypes[npcId] = RelationshipType::Acquaintance;
    } else if (value <= config.getCloseThreshold()) {
        playerRelationshipTypes[npcId] = RelationshipType::Friend;
    } else if (value <= config.getIntimateThreshold()) {
        playerRelationshipTypes[npcId] = RelationshipType::CloseFriend;
    } else {
        playerRelationshipTypes[npcId] = RelationshipType::BestFriend;
    }
}

bool NPCRelationshipManager::giveGift(const std::string& npcId, const std::string& itemId, GiftCategory category, int itemValue)
{
    if (npcs.find(npcId) == npcs.end())
        return false;

    RelationshipConfig& config = RelationshipConfig::getInstance();

    // Check if enough time has passed since last gift
    if (currentGameDay - lastGiftDay[npcId] < config.getMinDaysBetweenGifts()) {
        return false; // Too soon for another gift
    }

    RelationshipNPC& npc = npcs[npcId];

    // Calculate gift impact based on NPC preferences and item value
    float reactionMultiplier = npc.getGiftReaction(itemId, category);
    int relationshipChange = static_cast<int>(itemValue * reactionMultiplier * 0.1f);

    // Update relationship
    changeRelationship(npcId, relationshipChange);

    // Update gift timestamp
    lastGiftDay[npcId] = currentGameDay;

    // Update state based on gift reaction
    if (reactionMultiplier >= config.getFavoriteGiftMultiplier()) {
        playerRelationshipStates[npcId] = RelationshipState::Happy;
    } else if (reactionMultiplier <= config.getHatedGiftMultiplier()) {
        playerRelationshipStates[npcId] = RelationshipState::Disappointed;
    } else if (reactionMultiplier >= config.getLikedGiftMultiplier()) {
        playerRelationshipStates[npcId] = RelationshipState::Grateful;
    } else if (reactionMultiplier <= config.getDislikedGiftMultiplier()) {
        playerRelationshipStates[npcId] = RelationshipState::Disappointed;
    }

    return true;
}

void NPCRelationshipManager::handleConversation(const std::string& npcId, const std::string& topic, bool isPositive)
{
    if (npcs.find(npcId) == npcs.end())
        return;

    RelationshipNPC& npc = npcs[npcId];
    int relationshipChange = 0;

    // Check if topic is liked or disliked
    if (npc.conversationTopics.find(topic) != npc.conversationTopics.end()) {
        relationshipChange = isPositive ? 3 : 1;
    } else if (npc.tabooTopics.find(topic) != npc.tabooTopics.end()) {
        relationshipChange = isPositive ? -1 : -5;
    } else {
        relationshipChange = isPositive ? 1 : -1;
    }

    // Apply change
    changeRelationship(npcId, relationshipChange);

    // Update state based on conversation
    if (isPositive) {
        if (npc.conversationTopics.find(topic) != npc.conversationTopics.end()) {
            playerRelationshipStates[npcId] = RelationshipState::Happy;
        } else {
            playerRelationshipStates[npcId] = RelationshipState::Neutral;
        }
    } else {
        if (npc.tabooTopics.find(topic) != npc.tabooTopics.end()) {
            playerRelationshipStates[npcId] = RelationshipState::Angry;
        } else {
            playerRelationshipStates[npcId] = RelationshipState::Disappointed;
        }
    }
}

void NPCRelationshipManager::advanceDay()
{
    RelationshipConfig& config = RelationshipConfig::getInstance();
    currentGameDay++;

    // Natural decay in relationships over time if not maintained
    for (auto& [npcId, relationshipValue] : playerRelationships) {
        // Skip decay for close relationships
        if (relationshipValue > config.getCloseThreshold())
            continue;

        // Apply small decay
        relationshipValue += config.getDailyDecayAmount();

        // Ensure it doesn't fall below a minimum
        if (relationshipValue < config.getMinRelationship()) {
            relationshipValue = config.getMinRelationship();
        }

        // Update type if needed
        updateRelationshipType(npcId);
    }

    // Reset temporary states back to neutral after a few days
    for (auto& [npcId, state] : playerRelationshipStates) {
        if (state != RelationshipState::Neutral && std::rand() % 3 == 0) {
            state = RelationshipState::Neutral;
        }
    }
}

void NPCRelationshipManager::handleTaskCompletion(const std::string& npcId, int importance)
{
    if (npcs.find(npcId) == npcs.end())
        return;

    // Importance ranges from 1 (minor) to 10 (life-changing)
    int relationshipChange = importance * 2;
    changeRelationship(npcId, relationshipChange);

    // Update state
    if (importance >= 8) {
        playerRelationshipStates[npcId] = RelationshipState::Grateful;
    } else if (importance >= 4) {
        playerRelationshipStates[npcId] = RelationshipState::Happy;
    } else {
        playerRelationshipStates[npcId] = RelationshipState::Impressed;
    }
}

void NPCRelationshipManager::handleBetrayal(const std::string& npcId, int severity)
{
    if (npcs.find(npcId) == npcs.end())
        return;

    // Severity ranges from 1 (minor) to 10 (unforgivable)
    int relationshipChange = -severity * 3;
    changeRelationship(npcId, relationshipChange);

    // Update state
    if (severity >= 8) {
        playerRelationshipStates[npcId] = RelationshipState::Angry;
    } else if (severity >= 4) {
        playerRelationshipStates[npcId] = RelationshipState::Disappointed;
    } else {
        playerRelationshipStates[npcId] = RelationshipState::Sad;
    }
}

std::string NPCRelationshipManager::getRelationshipDescription(const std::string& npcId)
{
    if (npcs.find(npcId) == npcs.end())
        return "Unknown";

    RelationshipConfig& config = RelationshipConfig::getInstance();
    int value = playerRelationships[npcId];
    RelationshipType type = playerRelationshipTypes[npcId];
    RelationshipState state = playerRelationshipStates[npcId];

    std::string description;

    // Base description on relationship type
    description = config.getRelationshipTypeString(type);

    // Add current state as modifier
    if (state != RelationshipState::Neutral) {
        description += " (" + config.getRelationshipStateString(state) + ")";
    }

    return description;
}

std::vector<std::string> NPCRelationshipManager::getNPCsAtLocation(const std::string& location, int day, int hour)
{
    std::vector<std::string> presentNPCs;

    for (auto& [npcId, npc] : npcs) {
        auto schedule = npc.getCurrentSchedule(day, hour);
        if (schedule.location == location) {
            presentNPCs.push_back(npcId);
        }
    }

    return presentNPCs;
}

bool NPCRelationshipManager::changeRelationshipType(const std::string& npcId, RelationshipType newType, bool force)
{
    if (npcs.find(npcId) == npcs.end())
        return false;

    RelationshipConfig& config = RelationshipConfig::getInstance();
    int value = playerRelationships[npcId];

    // Check requirements for each type, unless forcing
    if (!force) {
        switch (newType) {
        case RelationshipType::Friend:
            if (value < config.getFriendlyThreshold())
                return false;
            break;
        case RelationshipType::CloseFriend:
            if (value < config.getCloseThreshold())
                return false;
            break;
        case RelationshipType::BestFriend:
            if (value < config.getIntimateThreshold())
                return false;
            break;
        case RelationshipType::Partner:
            if (value < config.getIntimateThreshold())
                return false;
            break;
        case RelationshipType::Spouse:
            if (value < config.getIntimateThreshold())
                return false;
            // Check if already married to someone else
            for (auto& [id, type] : playerRelationshipTypes) {
                if (type == RelationshipType::Spouse && id != npcId)
                    return false;
            }
            break;
        default:
            break;
        }
    }

    // Apply the change
    playerRelationshipTypes[npcId] = newType;

    // Boost relationship value for positive type changes
    if (newType == RelationshipType::Partner || newType == RelationshipType::Spouse || newType == RelationshipType::BestFriend) {
        playerRelationships[npcId] = config.getMaxRelationship();
    }

    return true;
}

int NPCRelationshipManager::getRelationshipValue(const std::string& npcId)
{
    if (playerRelationships.find(npcId) != playerRelationships.end()) {
        return playerRelationships[npcId];
    }
    return 0;
}

RelationshipType NPCRelationshipManager::getRelationshipType(const std::string& npcId)
{
    if (playerRelationshipTypes.find(npcId) != playerRelationshipTypes.end()) {
        return playerRelationshipTypes[npcId];
    }
    return RelationshipType::None;
}

RelationshipState NPCRelationshipManager::getRelationshipState(const std::string& npcId)
{
    if (playerRelationshipStates.find(npcId) != playerRelationshipStates.end()) {
        return playerRelationshipStates[npcId];
    }
    return RelationshipState::Neutral;
}

bool NPCRelationshipManager::saveRelationships(const std::string& filename)
{
    try {
        // Create JSON structure for saving
        nlohmann::json saveData;

        // Save current day
        saveData["currentGameDay"] = currentGameDay;

        // Save NPCs
        nlohmann::json npcsData = nlohmann::json::array();
        for (const auto& [npcId, npc] : npcs) {
            npcsData.push_back(npc.toJson());
        }
        saveData["npcs"] = npcsData;

        // Save player relationships
        nlohmann::json relationships = nlohmann::json::array();
        for (const auto& [npcId, value] : playerRelationships) {
            RelationshipConfig& config = RelationshipConfig::getInstance();

            nlohmann::json rel;
            rel["npcId"] = npcId;
            rel["value"] = value;
            rel["type"] = config.getRelationshipTypeString(playerRelationshipTypes[npcId]);
            rel["state"] = config.getRelationshipStateString(playerRelationshipStates[npcId]);
            rel["lastGiftDay"] = lastGiftDay[npcId];
            relationships.push_back(rel);
        }
        saveData["playerRelationships"] = relationships;

        // Write to file
        std::ofstream file(filename);
        if (!file.is_open()) {
            return false;
        }

        file << saveData.dump(4); // Pretty print with 4-space indentation
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error saving relationships: " << e.what() << std::endl;
        return false;
    }
}

bool NPCRelationshipManager::loadRelationships(const std::string& filename)
{
    try {
        // Open and parse the save file
        std::ifstream file(filename);
        if (!file.is_open()) {
            return false;
        }

        nlohmann::json saveData;
        file >> saveData;

        // Load current day
        currentGameDay = saveData["currentGameDay"];

        // Clear current data
        npcs.clear();
        playerRelationships.clear();
        playerRelationshipTypes.clear();
        playerRelationshipStates.clear();
        lastGiftDay.clear();

        // Load NPCs
        for (const auto& npcData : saveData["npcs"]) {
            RelationshipNPC npc(npcData);
            npcs[npc.id] = npc;
        }

        // Load player relationships
        RelationshipConfig& config = RelationshipConfig::getInstance();
        for (const auto& rel : saveData["playerRelationships"]) {
            std::string npcId = rel["npcId"];
            int value = rel["value"];
            RelationshipType type = config.getRelationshipTypeFromString(rel["type"]);
            RelationshipState state = config.getRelationshipStateFromString(rel["state"]);
            int lastGift = rel["lastGiftDay"];

            playerRelationships[npcId] = value;
            playerRelationshipTypes[npcId] = type;
            playerRelationshipStates[npcId] = state;
            lastGiftDay[npcId] = lastGift;
        }

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading relationships: " << e.what() << std::endl;
        return false;
    }
}