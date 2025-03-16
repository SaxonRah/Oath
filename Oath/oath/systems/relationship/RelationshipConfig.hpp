#pragma once

#include "RelationshipTypes.hpp"
#include <map>
#include <nlohmann/json.hpp>


class RelationshipConfig {
private:
    nlohmann::json configData;
    static RelationshipConfig* instance;

    // Private constructor for singleton
    RelationshipConfig();

public:
    // Prevent copy and assignment
    RelationshipConfig(const RelationshipConfig&) = delete;
    RelationshipConfig& operator=(const RelationshipConfig&) = delete;

    // Singleton access
    static RelationshipConfig& getInstance();
    bool loadConfig(const std::string& filename);

    // Getter methods for constants
    int getMinRelationship() const;
    int getMaxRelationship() const;
    int getHatredThreshold() const;
    int getDislikeThreshold() const;
    int getNeutralThreshold() const;
    int getFriendlyThreshold() const;
    int getCloseThreshold() const;
    int getIntimateThreshold() const;
    float getSharedTraitBonus() const;
    float getOpposingTraitPenalty() const;
    float getFavoriteGiftMultiplier() const;
    float getLikedGiftMultiplier() const;
    float getDislikedGiftMultiplier() const;
    float getHatedGiftMultiplier() const;
    int getDailyDecayAmount() const;
    int getMinDaysBetweenGifts() const;

    // Get all opposing traits pairs
    std::map<PersonalityTrait, PersonalityTrait> getOpposingTraits() const;

    // Get all NPCs from config
    nlohmann::json getNPCs() const;

    // Get default relationships
    nlohmann::json getDefaultRelationships() const;

    // Helper methods for enum conversions
    PersonalityTrait getPersonalityTraitFromString(const std::string& traitName) const;
    std::string getPersonalityTraitString(PersonalityTrait trait) const;
    RelationshipType getRelationshipTypeFromString(const std::string& typeName) const;
    std::string getRelationshipTypeString(RelationshipType type) const;
    RelationshipState getRelationshipStateFromString(const std::string& stateName) const;
    std::string getRelationshipStateString(RelationshipState state) const;
    GiftCategory getGiftCategoryFromString(const std::string& categoryName) const;
    std::string getGiftCategoryString(GiftCategory category) const;
};