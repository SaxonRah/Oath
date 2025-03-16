// systems/economy/PropertyNode.hpp
#pragma once

#include "../../core/TAAction.hpp"
#include "../../core/TAInput.hpp"
#include "../../core/TANode.hpp"
#include "../../data/GameContext.hpp"
#include "Property.hpp"
#include <string>
#include <vector>

// Node for managing player properties
class PropertyNode : public TANode {
public:
    std::vector<Property> properties;
    int playerGold;
    int daysSinceLastUpkeep;
    std::vector<std::string> recentNotifications;

    PropertyNode(const std::string& name);

    void loadPropertiesFromJson();
    void onEnter(GameContext* context) override;
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;

    // Add a new property
    void addProperty(const Property& property);

    // Process a day passing for all properties
    void advanceDay();

private:
    void displayOwnedProperties();
    void displayAvailableProperties();
    void handlePropertyManagement();
    void handlePropertyPurchase();
};
