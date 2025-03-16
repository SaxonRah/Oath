// systems/economy/EconomicEvent.hpp
#pragma once

#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>


using json = nlohmann::json;

// Economic event that affects markets and trade
struct EconomicEvent {
    std::string id;
    std::string name;
    std::string description;
    std::map<std::string, float> commoditySupplyEffects; // id -> multiplier
    std::map<std::string, float> commodityDemandEffects; // id -> multiplier
    std::map<std::string, bool> tradeRouteDisruptions; // id -> isDisrupted
    std::vector<std::string> affectedRegions;
    int duration; // In game days
    int daysSinceStart; // Track progress of the event

    // Load from JSON
    static EconomicEvent fromJson(const json& j);

    bool isActive() const;
    void advance();
};