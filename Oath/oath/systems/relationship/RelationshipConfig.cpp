#include "RelationshipConfig.hpp"
#include <fstream>
#include <iostream>

// Initialize the static instance pointer
RelationshipConfig* RelationshipConfig::instance = nullptr;

RelationshipConfig::RelationshipConfig()
{
    loadConfig("resources/config/NPCRelationships.json");
}

RelationshipConfig& RelationshipConfig::getInstance()
{
    if (instance == nullptr) {
        instance = new RelationshipConfig();
    }
    return *instance;
}

bool RelationshipConfig::loadConfig(const std::string& filename)
{
    try {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open config file: " << filename << std::endl;
            return false;
        }
        file >> configData;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading config: " << e.what() << std::endl;
        return false;
    }
}

int RelationshipConfig::getMinRelationship() const
{
    return configData["relationshipConstants"]["minRelationship"];
}

int RelationshipConfig::getMaxRelationship() const
{
    return configData["relationshipConstants"]["maxRelationship"];
}

int RelationshipConfig::getHatredThreshold() const
{
    return configData["relationshipConstants"]["thresholds"]["hatred"];
}

int RelationshipConfig::getDislikeThreshold() const
{
    return configData["relationshipConstants"]["thresholds"]["dislike"];
}

int RelationshipConfig::getNeutralThreshold() const
{
    return configData["relationshipConstants"]["thresholds"]["neutral"];
}

int RelationshipConfig::getFriendlyThreshold() const
{
    return configData["relationshipConstants"]["thresholds"]["friendly"];
}

int RelationshipConfig::getCloseThreshold() const
{
    return configData["relationshipConstants"]["thresholds"]["close"];
}

int RelationshipConfig::getIntimateThreshold() const
{
    return configData["relationshipConstants"]["thresholds"]["intimate"];
}

float RelationshipConfig::getSharedTraitBonus() const
{
    return configData["relationshipConstants"]["traitInfluence"]["sharedTraitBonus"];
}

float RelationshipConfig::getOpposingTraitPenalty() const
{
    return configData["relationshipConstants"]["traitInfluence"]["opposingTraitPenalty"];
}

float RelationshipConfig::getFavoriteGiftMultiplier() const
{
    return configData["relationshipConstants"]["giftMultipliers"]["favorite"];
}

float RelationshipConfig::getLikedGiftMultiplier() const
{
    return configData["relationshipConstants"]["giftMultipliers"]["liked"];
}

float RelationshipConfig::getDislikedGiftMultiplier() const
{
    return configData["relationshipConstants"]["giftMultipliers"]["disliked"];
}

float RelationshipConfig::getHatedGiftMultiplier() const
{
    return configData["relationshipConstants"]["giftMultipliers"]["hated"];
}

int RelationshipConfig::getDailyDecayAmount() const
{
    return configData["relationshipConstants"]["timeConstants"]["dailyDecayAmount"];
}

int RelationshipConfig::getMinDaysBetweenGifts() const
{
    return configData["relationshipConstants"]["timeConstants"]["minDaysBetweenGifts"];
}

std::map<PersonalityTrait, PersonalityTrait> RelationshipConfig::getOpposingTraits() const
{
    std::map<PersonalityTrait, PersonalityTrait> result;

    for (const auto& pair : configData["opposingTraits"]) {
        std::string trait1 = pair["trait1"];
        std::string trait2 = pair["trait2"];

        result[getPersonalityTraitFromString(trait1)] = getPersonalityTraitFromString(trait2);
    }

    return result;
}

nlohmann::json RelationshipConfig::getNPCs() const
{
    return configData["npcs"];
}

nlohmann::json RelationshipConfig::getDefaultRelationships() const
{
    return configData["defaultRelationships"];
}

PersonalityTrait RelationshipConfig::getPersonalityTraitFromString(const std::string& traitName) const
{
    for (size_t i = 0; i < configData["personalityTraits"].size(); i++) {
        if (configData["personalityTraits"][i] == traitName) {
            return static_cast<PersonalityTrait>(i);
        }
    }
    // Default if not found
    return PersonalityTrait::Agreeable;
}

std::string RelationshipConfig::getPersonalityTraitString(PersonalityTrait trait) const
{
    int index = static_cast<int>(trait);
    if (index >= 0 && index < static_cast<int>(configData["personalityTraits"].size())) {
        return configData["personalityTraits"][index];
    }
    return "Unknown";
}

RelationshipType RelationshipConfig::getRelationshipTypeFromString(const std::string& typeName) const
{
    for (size_t i = 0; i < configData["relationshipTypes"].size(); i++) {
        if (configData["relationshipTypes"][i] == typeName) {
            return static_cast<RelationshipType>(i);
        }
    }
    return RelationshipType::None;
}

std::string RelationshipConfig::getRelationshipTypeString(RelationshipType type) const
{
    int index = static_cast<int>(type);
    if (index >= 0 && index < static_cast<int>(configData["relationshipTypes"].size())) {
        return configData["relationshipTypes"][index];
    }
    return "Unknown";
}

RelationshipState RelationshipConfig::getRelationshipStateFromString(const std::string& stateName) const
{
    for (size_t i = 0; i < configData["relationshipStates"].size(); i++) {
        if (configData["relationshipStates"][i] == stateName) {
            return static_cast<RelationshipState>(i);
        }
    }
    return RelationshipState::Neutral;
}

std::string RelationshipConfig::getRelationshipStateString(RelationshipState state) const
{
    int index = static_cast<int>(state);
    if (index >= 0 && index < static_cast<int>(configData["relationshipStates"].size())) {
        return configData["relationshipStates"][index];
    }
    return "Unknown";
}

GiftCategory RelationshipConfig::getGiftCategoryFromString(const std::string& categoryName) const
{
    for (size_t i = 0; i < configData["giftCategories"].size(); i++) {
        if (configData["giftCategories"][i] == categoryName) {
            return static_cast<GiftCategory>(i);
        }
    }
    return GiftCategory::Weapon; // Default
}

std::string RelationshipConfig::getGiftCategoryString(GiftCategory category) const
{
    int index = static_cast<int>(category);
    if (index >= 0 && index < static_cast<int>(configData["giftCategories"].size())) {
        return configData["giftCategories"][index];
    }
    return "Unknown";
}