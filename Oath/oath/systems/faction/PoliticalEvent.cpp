// systems/faction/PoliticalEvent.cpp
#include "PoliticalEvent.hpp"
#include <iostream>

namespace oath {

PoliticalEvent::PoliticalEvent(const std::string& eventName, const std::string& eventDesc)
    : name(eventName)
    , description(eventDesc)
    , hasOccurred(false)
    , daysTillNextCheck(0)
{
    // Default condition always returns true
    condition = [](const GameContext&) { return true; };
}

PoliticalEvent PoliticalEvent::fromJson(const json& j)
{
    PoliticalEvent event(
        j.value("name", "Unnamed Event"),
        j.value("description", "No description"));

    // Load faction power shifts
    if (j.contains("factionPowerShifts") && j["factionPowerShifts"].is_object()) {
        for (auto& [factionId, shift] : j["factionPowerShifts"].items()) {
            event.factionPowerShifts[factionId] = shift;
        }
    }

    // Load relation shifts
    if (j.contains("relationShifts") && j["relationShifts"].is_array()) {
        for (const auto& shift : j["relationShifts"]) {
            if (shift.contains("factionA") && shift.contains("factionB") && shift.contains("amount")) {
                std::string factionA = shift["factionA"];
                std::string factionB = shift["factionB"];
                int amount = shift["amount"];
                event.relationShifts.push_back(std::make_tuple(factionA, factionB, amount));
            }
        }
    }

    return event;
}

bool PoliticalEvent::checkAndExecute(GameContext* context, FactionSystemNode* factionSystem)
{
    if (hasOccurred || !factionSystem || !context) {
        return false;
    }

    // Check if event conditions are met
    if (condition(*context)) {
        std::cout << "=== Political Shift: " << name << " ===" << std::endl;
        std::cout << description << std::endl;

        // Apply power shifts to affected factions
        for (const auto& [factionId, powerChange] : factionPowerShifts) {
            auto it = factionSystem->factions.find(factionId);
            if (it != factionSystem->factions.end()) {
                Faction& faction = it->second;

                // Decide which power types are affected
                // This implementation assumes even distribution, but you could specify which
                int economicChange = powerChange / 3;
                int militaryChange = powerChange / 3;
                int politicalChange = powerChange - economicChange - militaryChange;

                faction.economicPower = std::max(0, std::min(100, faction.economicPower + economicChange));
                faction.militaryPower = std::max(0, std::min(100, faction.militaryPower + militaryChange));
                faction.politicalInfluence = std::max(0, std::min(100, faction.politicalInfluence + politicalChange));

                // Update faction state based on new power levels
                updateFactionState(faction);

                std::cout << faction.name << " has been "
                          << (powerChange > 0 ? "strengthened" : "weakened")
                          << " by this event." << std::endl;
            }
        }

        // Apply relation shifts
        for (const auto& [factionA, factionB, relationChange] : relationShifts) {
            if (factionSystem->adjustFactionRelation(factionA, factionB, relationChange)) {
                std::string relationState = factionSystem->getFactionRelationState(factionA, factionB);

                std::cout << "Relations between " << factionSystem->factions[factionA].name
                          << " and " << factionSystem->factions[factionB].name
                          << " have " << (relationChange > 0 ? "improved" : "deteriorated")
                          << " to " << relationState << "." << std::endl;

                // If relations reach war or alliance levels, this is significant
                if (relationState == "war") {
                    std::cout << "WAR has broken out between "
                              << factionSystem->factions[factionA].name << " and "
                              << factionSystem->factions[factionB].name << "!" << std::endl;

                    // Update faction states to reflect war
                    auto itA = factionSystem->factions.find(factionA);
                    auto itB = factionSystem->factions.find(factionB);

                    if (itA != factionSystem->factions.end()) {
                        itA->second.currentState = "at_war";
                    }

                    if (itB != factionSystem->factions.end()) {
                        itB->second.currentState = "at_war";
                    }
                } else if (relationState == "allied") {
                    std::cout << "A formal alliance has formed between "
                              << factionSystem->factions[factionA].name << " and "
                              << factionSystem->factions[factionB].name << "!" << std::endl;
                }
            }
        }

        hasOccurred = true;

        // Save the updated faction system
        factionSystem->saveToJson();

        return true;
    }

    return false;
}

void PoliticalEvent::updateFactionState(Faction& faction)
{
    // Average the three power indicators
    int avgPower = (faction.economicPower + faction.militaryPower + faction.politicalInfluence) / 3;

    // Determine state based on average power
    if (faction.currentState == "at_war") {
        // War state persists unless explicitly changed
        return;
    } else if (avgPower < 20) {
        faction.currentState = "in_crisis";
    } else if (avgPower < 40) {
        faction.currentState = "declining";
    } else if (avgPower < 60) {
        faction.currentState = "stable";
    } else if (avgPower < 80) {
        faction.currentState = "growing";
    } else {
        faction.currentState = "flourishing";
    }
}

} // namespace oath