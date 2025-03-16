// systems/economy/PropertyNode.cpp

#include "PropertyNode.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>

PropertyNode::PropertyNode(const std::string& name)
    : TANode(name)
    , playerGold(1000)
    , daysSinceLastUpkeep(0)
{
    // Load properties from JSON
    loadPropertiesFromJson();
}

void PropertyNode::loadPropertiesFromJson()
{
    try {
        // Open and parse the JSON file
        std::ifstream file("resources/json/economy.json");
        if (!file.is_open()) {
            std::cerr << "Failed to open economy.json for properties" << std::endl;
            return;
        }

        json data;
        file >> data;
        file.close();

        // Load properties
        if (data.contains("properties") && data["properties"].is_array()) {
            for (const auto& propertyData : data["properties"]) {
                properties.push_back(Property::fromJson(propertyData));
            }

            std::cout << "Loaded " << properties.size() << " properties from JSON." << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error loading properties from JSON: " << e.what() << std::endl;
    }
}

void PropertyNode::onEnter(GameContext* context)
{
    std::cout << "Property Management" << std::endl;
    std::cout << "-----------------------------" << std::endl;
    std::cout << "Gold: " << playerGold << std::endl;

    displayOwnedProperties();

    // Show recent notifications
    if (!recentNotifications.empty()) {
        std::cout << "\nRecent Notifications:" << std::endl;
        for (const auto& notification : recentNotifications) {
            std::cout << "- " << notification << std::endl;
        }
        recentNotifications.clear();
    }
}

std::vector<TAAction> PropertyNode::getAvailableActions()
{
    std::vector<TAAction> actions;

    actions.push_back({ "view_owned", "View Owned Properties",
        [this]() -> TAInput {
            return { "property_action", { { "action", std::string("view_owned") } } };
        } });

    actions.push_back({ "view_available", "View Available Properties",
        [this]() -> TAInput {
            return { "property_action", { { "action", std::string("view_available") } } };
        } });

    actions.push_back({ "manage_property", "Manage a Property",
        [this]() -> TAInput {
            return { "property_action", { { "action", std::string("manage_property") } } };
        } });

    actions.push_back({ "buy_property", "Buy a Property",
        [this]() -> TAInput {
            return { "property_action", { { "action", std::string("buy_property") } } };
        } });

    actions.push_back({ "exit", "Exit Property Manager",
        [this]() -> TAInput {
            return { "property_action", { { "action", std::string("exit") } } };
        } });

    return actions;
}

bool PropertyNode::evaluateTransition(const TAInput& input, TANode*& outNextNode)
{
    if (input.type == "property_action") {
        std::string action = std::get<std::string>(input.parameters.at("action"));

        if (action == "view_owned") {
            displayOwnedProperties();
            outNextNode = this;
            return true;
        } else if (action == "view_available") {
            displayAvailableProperties();
            outNextNode = this;
            return true;
        } else if (action == "manage_property") {
            handlePropertyManagement();
            outNextNode = this;
            return true;
        } else if (action == "buy_property") {
            handlePropertyPurchase();
            outNextNode = this;
            return true;
        } else if (action == "exit") {
            // Find and use an exit transition
            for (const auto& rule : transitionRules) {
                if (rule.description == "Exit") {
                    outNextNode = rule.targetNode;
                    return true;
                }
            }
            // Fallback
            outNextNode = this;
            return true;
        }
    }

    return TANode::evaluateTransition(input, outNextNode);
}

void PropertyNode::addProperty(const Property& property)
{
    properties.push_back(property);
}

void PropertyNode::advanceDay()
{
    int totalIncome = 0;

    for (auto& property : properties) {
        property.advanceDay(totalIncome, recentNotifications);
    }

    if (totalIncome > 0) {
        playerGold += totalIncome;
        recentNotifications.push_back("Your properties generated " + std::to_string(totalIncome) + " gold today.");
    }

    // Process weekly upkeep
    daysSinceLastUpkeep++;
    if (daysSinceLastUpkeep >= 7) {
        int totalUpkeep = 0;

        for (const auto& property : properties) {
            if (property.isOwned) {
                totalUpkeep += property.weeklyUpkeep;
            }
        }

        if (totalUpkeep > 0) {
            playerGold -= totalUpkeep;
            recentNotifications.push_back("You paid " + std::to_string(totalUpkeep) + " gold for property upkeep.");

            // Check if player can't afford upkeep
            if (playerGold < 0) {
                recentNotifications.push_back("WARNING: You couldn't afford property upkeep! Your properties may deteriorate.");
                playerGold = 0; // Prevent negative gold in this example
            }
        }

        daysSinceLastUpkeep = 0;
    }
}

void PropertyNode::displayOwnedProperties()
{
    bool hasProperties = false;

    std::cout << "\nYour Properties:" << std::endl;
    std::cout << "-----------------------------------------------------------" << std::endl;
    std::cout << std::left << std::setw(25) << "Property"
              << std::setw(15) << "Type"
              << std::setw(15) << "Location"
              << std::setw(15) << "Weekly Income" << std::endl;
    std::cout << "-----------------------------------------------------------" << std::endl;

    for (const auto& property : properties) {
        if (property.isOwned) {
            hasProperties = true;
            std::cout << std::left << std::setw(25) << property.name
                      << std::setw(15) << propertyTypeToString(property.type)
                      << std::setw(15) << property.regionId
                      << std::setw(15) << property.calculateNetIncome() << std::endl;
        }
    }

    if (!hasProperties) {
        std::cout << "You don't own any properties yet." << std::endl;
    }
}

void PropertyNode::displayAvailableProperties()
{
    bool hasAvailable = false;

    std::cout << "\nAvailable Properties for Purchase:" << std::endl;
    std::cout << "-----------------------------------------------------------" << std::endl;
    std::cout << std::left << std::setw(25) << "Property"
              << std::setw(15) << "Type"
              << std::setw(15) << "Location"
              << std::setw(15) << "Price" << std::endl;
    std::cout << "-----------------------------------------------------------" << std::endl;

    for (const auto& property : properties) {
        if (!property.isOwned) {
            hasAvailable = true;
            std::cout << std::left << std::setw(25) << property.name
                      << std::setw(15) << propertyTypeToString(property.type)
                      << std::setw(15) << property.regionId
                      << std::setw(15) << property.purchasePrice << std::endl;
        }
    }

    if (!hasAvailable) {
        std::cout << "There are no properties available for purchase." << std::endl;
    }
}

void PropertyNode::handlePropertyManagement()
{
    // In a real implementation, this would allow selecting a property
    // and then managing upgrades, tenants, storage, etc.

    std::cout << "Which property would you like to manage? (Enter name)" << std::endl;

    // Example placeholder for property management interface
    std::cout << "\nProperty Management - Riverside Cottage" << std::endl;
    std::cout << "1. View/Manage Storage" << std::endl;
    std::cout << "2. View/Install Upgrades" << std::endl;
    std::cout << "3. View/Manage Tenants" << std::endl;
    std::cout << "4. Sell Property" << std::endl;
    std::cout << "5. Back" << std::endl;

    std::cout << "\nThis would be an interactive menu in a real implementation." << std::endl;
}

void PropertyNode::handlePropertyPurchase()
{
    // In a real implementation, this would allow selecting and purchasing a property

    std::cout << "Which property would you like to purchase? (Enter name)" << std::endl;

    // Example placeholder purchase
    if (playerGold >= 5000) {
        std::cout << "You have purchased Riverside Cottage for 5000 gold!" << std::endl;
        playerGold -= 5000;

        // Mark the first available property as owned for demonstration
        for (auto& property : properties) {
            if (!property.isOwned) {
                property.isOwned = true;
                break;
            }
        }
    } else {
        std::cout << "You don't have enough gold to purchase this property." << std::endl;
    }
}
