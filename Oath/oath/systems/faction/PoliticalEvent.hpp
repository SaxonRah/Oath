// systems/faction/PoliticalEvent.hpp
#ifndef OATH_POLITICAL_EVENT_HPP
#define OATH_POLITICAL_EVENT_HPP

#include "FactionSystemNode.hpp"
#include "data/GameContext.hpp"


#include <functional>
#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <tuple>
#include <vector>


using json = nlohmann::json;

namespace oath {

// Political shift event that changes faction relations
class PoliticalEvent {
public:
    std::string name;
    std::string description;
    std::map<std::string, int> factionPowerShifts; // Changes to economic/military/political power
    std::vector<std::tuple<std::string, std::string, int>> relationShifts; // Changes to relations (factionA, factionB, amount)
    std::function<bool(const GameContext&)> condition;
    bool hasOccurred;
    int daysTillNextCheck;

    PoliticalEvent(const std::string& eventName, const std::string& eventDesc);

    // Create from JSON
    static PoliticalEvent fromJson(const json& j);

    bool checkAndExecute(GameContext* context, FactionSystemNode* factionSystem);

private:
    void updateFactionState(Faction& faction);
};

} // namespace oath

#endif // OATH_POLITICAL_EVENT_HPP