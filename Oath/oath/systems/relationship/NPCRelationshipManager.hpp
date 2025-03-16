#pragma once

#include "RelationshipNPC.hpp"
#include "RelationshipTypes.hpp"
#include <map>
#include <string>
#include <vector>


class NPCRelationshipManager {
private:
    std::map<std::string, RelationshipNPC> npcs;
    std::map<std::string, int> playerRelationships; // NPC ID -> relationship value
    std::map<std::string, RelationshipType> playerRelationshipTypes;
    std::map<std::string, RelationshipState> playerRelationshipStates;
    std::map<std::string, int> lastGiftDay; // NPC ID -> game day when last gift was given
    int currentGameDay;

public:
    NPCRelationshipManager();

    void loadNPCsFromConfig();
    void registerNPC(const RelationshipNPC& npc);
    RelationshipNPC* getNPC(const std::string& npcId);
    void changeRelationship(const std::string& npcId, int amount);
    void updateRelationshipType(const std::string& npcId);
    bool giveGift(const std::string& npcId, const std::string& itemId, GiftCategory category, int itemValue);
    void handleConversation(const std::string& npcId, const std::string& topic, bool isPositive);
    void advanceDay();
    void handleTaskCompletion(const std::string& npcId, int importance);
    void handleBetrayal(const std::string& npcId, int severity);
    std::string getRelationshipDescription(const std::string& npcId);
    std::vector<std::string> getNPCsAtLocation(const std::string& location, int day, int hour);
    bool changeRelationshipType(const std::string& npcId, RelationshipType newType, bool force = false);
    int getRelationshipValue(const std::string& npcId);
    RelationshipType getRelationshipType(const std::string& npcId);
    RelationshipState getRelationshipState(const std::string& npcId);
    bool saveRelationships(const std::string& filename);
    bool loadRelationships(const std::string& filename);

    // Getter for player relationships map
    const std::map<std::string, int>& getPlayerRelationships() const
    {
        return playerRelationships;
    }
};