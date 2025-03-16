// systems/economy/EconomicEvent.cpp

#include "EconomicEvent.hpp"

EconomicEvent EconomicEvent::fromJson(const json& j)
{
    EconomicEvent event;
    event.id = j["id"];
    event.name = j["name"];
    event.description = j["description"];
    event.duration = j["duration"];
    event.daysSinceStart = 0; // Start at 0

    // Load affected regions
    if (j.contains("affectedRegions") && j["affectedRegions"].is_array()) {
        for (const auto& region : j["affectedRegions"]) {
            event.affectedRegions.push_back(region);
        }
    }

    // Load commodity supply effects
    if (j.contains("commoditySupplyEffects") && j["commoditySupplyEffects"].is_object()) {
        for (auto it = j["commoditySupplyEffects"].begin(); it != j["commoditySupplyEffects"].end(); ++it) {
            event.commoditySupplyEffects[it.key()] = it.value();
        }
    }

    // Load commodity demand effects
    if (j.contains("commodityDemandEffects") && j["commodityDemandEffects"].is_object()) {
        for (auto it = j["commodityDemandEffects"].begin(); it != j["commodityDemandEffects"].end(); ++it) {
            event.commodityDemandEffects[it.key()] = it.value();
        }
    }

    // Load trade route disruptions
    if (j.contains("tradeRouteDisruptions") && j["tradeRouteDisruptions"].is_object()) {
        for (auto it = j["tradeRouteDisruptions"].begin(); it != j["tradeRouteDisruptions"].end(); ++it) {
            event.tradeRouteDisruptions[it.key()] = it.value();
        }
    }

    return event;
}

bool EconomicEvent::isActive() const
{
    return daysSinceStart < duration;
}

void EconomicEvent::advance()
{
    if (daysSinceStart < duration) {
        daysSinceStart++;
    }
}
