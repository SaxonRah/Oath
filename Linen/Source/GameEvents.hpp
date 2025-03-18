// GameEvents.hpp
#pragma once

#include <nlohmann/json.hpp>
#include <string>


namespace GameEvents {
// Crime events
const std::string CrimeCommitted = "event.crime.committed";
const std::string BountyPaid = "event.crime.bounty_paid";
const std::string ArrestOccurred = "event.crime.arrested";

// Health events
const std::string HealthChanged = "event.health.changed";
const std::string DiseaseContracted = "event.health.disease_contracted";
const std::string DiseaseRecovered = "event.health.disease_recovered";

// Economy events
const std::string TransactionCompleted = "event.economy.transaction";
const std::string MarketPriceChanged = "event.economy.price_changed";
const std::string PropertyPurchased = "event.economy.property_purchased";

// Faction events
const std::string ReputationChanged = "event.faction.reputation_changed";
const std::string FactionRelationChanged = "event.faction.relation_changed";
const std::string RankAdvanced = "event.faction.rank_advanced";

// NPC relationship events
const std::string RelationshipChanged = "event.npc.relationship_changed";
const std::string GiftGiven = "event.npc.gift_given";
const std::string DialogueCompleted = "event.npc.dialogue_completed";

// Create event data for a specific event
inline nlohmann::json createEventData(const std::string& eventType, const nlohmann::json& data)
{
    nlohmann::json eventData;
    eventData["type"] = eventType;
    eventData["data"] = data;
    eventData["timestamp"] = std::time(nullptr);
    return eventData;
}
}