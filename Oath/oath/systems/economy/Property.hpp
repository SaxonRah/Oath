// systems/economy/Property.hpp
#pragma once

#include "../../data/Inventory.hpp"
#include "../../data/Item.hpp"
#include "PropertyTypes.hpp"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>


using json = nlohmann::json;

// Property that the player can own
struct Property {
    std::string id;
    std::string name;
    std::string description;
    std::string regionId;
    PropertyType type;
    int purchasePrice;
    int weeklyIncome;
    int weeklyUpkeep;
    bool isOwned;
    int storageCapacity;
    Inventory storage;

    // Upgrades
    struct PropertyUpgrade {
        std::string id;
        std::string name;
        std::string description;
        int cost;
        int upkeepIncrease;
        bool installed;

        // Load from JSON
        static PropertyUpgrade fromJson(const json& j);

        void applyEffect(Property& property);
    };
    std::vector<PropertyUpgrade> availableUpgrades;

    // Tenants for rental properties
    struct Tenant {
        std::string id;
        std::string name;
        int rentAmount;
        int daysSinceLastPayment;
        int paymentInterval;
        float reliability; // 0.0 to 1.0, chance they pay on time

        // Load from JSON
        static Tenant fromJson(const json& j);
    };
    std::vector<Tenant> tenants;

    Property();

    // Load from JSON
    static Property fromJson(const json& j);

    // Calculate net income including tenant rent
    int calculateNetIncome() const;

    // Process a day passing
    void advanceDay(int& income, std::vector<std::string>& notifications);

    // Add an upgrade
    void addUpgrade(const PropertyUpgrade& upgrade);

    // Install an upgrade
    bool installUpgrade(const std::string& upgradeId, int& cost);

    // Add a tenant
    void addTenant(const Tenant& tenant);

    // Evict a tenant
    bool evictTenant(const std::string& tenantId);

    // Add an item to storage
    bool addToStorage(const Item& item);

    // Remove an item from storage
    bool removeFromStorage(const std::string& itemId, int quantity);
};