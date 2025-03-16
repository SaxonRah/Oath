#include "RegionNode.hpp"

#include <iostream>
#include <random>

RegionNode::RegionNode(const std::string& name, const std::string& region)
    : TANode(name)
    , regionName(region)
{
}

void RegionNode::onEnter(GameContext* context)
{
    std::cout << "Entered region: " << regionName << std::endl;
    std::cout << description << std::endl;

    if (!controllingFaction.empty()) {
        std::cout << "Controlled by: " << controllingFaction << std::endl;
    }

    // List locations
    if (!locations.empty()) {
        std::cout << "Locations in this region:" << std::endl;
        for (const auto& location : locations) {
            std::cout << "- " << location->locationName;
            if (context && !location->canAccess(*context)) {
                std::cout << " (Inaccessible)";
            }
            std::cout << std::endl;
        }
    }

    // List connected regions
    if (!connectedRegions.empty()) {
        std::cout << "Connected regions:" << std::endl;
        for (const auto& region : connectedRegions) {
            std::cout << "- " << region->regionName << std::endl;
        }
    }

    // Check for random events
    if (context) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0.0, 1.0);

        for (const auto& event : possibleEvents) {
            if (event.condition(*context) && dis(gen) < event.probability) {
                std::cout << "\nEvent: " << event.name << std::endl;
                std::cout << event.description << std::endl;
                event.effect(context);
                break;
            }
        }
    }
}

std::vector<TAAction> RegionNode::getAvailableActions()
{
    std::vector<TAAction> actions = TANode::getAvailableActions();

    // Add location travel actions
    for (size_t i = 0; i < locations.size(); i++) {
        actions.push_back({ "travel_to_location_" + std::to_string(i),
            "Travel to " + locations[i]->locationName,
            [this, i]() -> TAInput {
                return { "region_action",
                    { { "action", std::string("travel_location") },
                        { "location_index", static_cast<int>(i) } } };
            } });
    }

    // Add region travel actions
    for (size_t i = 0; i < connectedRegions.size(); i++) {
        actions.push_back({ "travel_to_region_" + std::to_string(i),
            "Travel to " + connectedRegions[i]->regionName,
            [this, i]() -> TAInput {
                return { "region_action",
                    { { "action", std::string("travel_region") },
                        { "region_index", static_cast<int>(i) } } };
            } });
    }

    return actions;
}

bool RegionNode::evaluateTransition(const TAInput& input, TANode*& outNextNode)
{
    if (input.type == "region_action") {
        std::string action = std::get<std::string>(input.parameters.at("action"));

        if (action == "travel_location") {
            int locationIndex = std::get<int>(input.parameters.at("location_index"));
            if (locationIndex >= 0 && locationIndex < static_cast<int>(locations.size())) {
                // Set the persistent ID for the location to include the region path
                locations[locationIndex]->nodeID.persistentID = "WorldSystem/" + locations[locationIndex]->nodeName;

                outNextNode = locations[locationIndex];
                return true;
            }
        } else if (action == "travel_region") {
            int regionIndex = std::get<int>(input.parameters.at("region_index"));
            if (regionIndex >= 0 && regionIndex < static_cast<int>(connectedRegions.size())) {
                outNextNode = connectedRegions[regionIndex];
                return true;
            }
        }
    }

    return TANode::evaluateTransition(input, outNextNode);
}