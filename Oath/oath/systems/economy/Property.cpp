// systems/economy/Property.cpp

#include "Property.hpp"
#include <random>

Property::PropertyUpgrade Property::PropertyUpgrade::fromJson(const json& j)
{
    PropertyUpgrade upgrade;
    upgrade.id = j["id"];
    upgrade.name = j["name"];
    upgrade.description = j["description"];
    upgrade.cost = j["cost"];
    upgrade.upkeepIncrease = j.value("upkeepIncrease", 0);
    upgrade.installed = false;

    return upgrade;
}

void Property::PropertyUpgrade::applyEffect(Property& property)
{
    property.weeklyUpkeep += upkeepIncrease;
}

Property::Tenant Property::Tenant::fromJson(const json& j)
{
    Tenant tenant;
    tenant.id = j["id"];
    tenant.name = j["name"];
    tenant.rentAmount = j["rentAmount"];
    tenant.daysSinceLastPayment = 0;
    tenant.paymentInterval = j["paymentInterval"];
    tenant.reliability = j["reliability"];

    return tenant;
}

Property::Property()
    : type(PropertyType::HOUSE)
    , purchasePrice(5000)
    , weeklyIncome(0)
    , weeklyUpkeep(50)
    , isOwned(false)
    , storageCapacity(100)
{
}

Property Property::fromJson(const json& j)
{
    Property property;
    property.id = j["id"];
    property.name = j["name"];
    property.description = j["description"];
    property.regionId = j["regionId"];
    property.type = stringToPropertyType(j["type"]);
    property.purchasePrice = j["purchasePrice"];
    property.weeklyIncome = j["weeklyIncome"];
    property.weeklyUpkeep = j["weeklyUpkeep"];
    property.storageCapacity = j["storageCapacity"];
    property.isOwned = false;

    // Load upgrades
    if (j.contains("upgrades") && j["upgrades"].is_array()) {
        for (const auto& upgradeData : j["upgrades"]) {
            property.availableUpgrades.push_back(PropertyUpgrade::fromJson(upgradeData));
        }
    }

    // Load tenants
    if (j.contains("tenants") && j["tenants"].is_array()) {
        for (const auto& tenantData : j["tenants"]) {
            property.tenants.push_back(Tenant::fromJson(tenantData));
        }
    }

    return property;
}

int Property::calculateNetIncome() const
{
    int totalIncome = weeklyIncome;

    // Add tenant income
    for (const auto& tenant : tenants) {
        totalIncome += tenant.rentAmount;
    }

    return totalIncome - weeklyUpkeep;
}

void Property::advanceDay(int& income, std::vector<std::string>& notifications)
{
    if (!isOwned)
        return;

    // Process daily income
    int dailyIncome = weeklyIncome / 7;
    income += dailyIncome;

    // Process tenants
    for (auto& tenant : tenants) {
        tenant.daysSinceLastPayment++;

        if (tenant.daysSinceLastPayment >= tenant.paymentInterval) {
            static std::random_device rd;
            static std::mt19937 gen(rd());
            std::uniform_real_distribution<float> dist(0.0f, 1.0f);

            if (dist(gen) <= tenant.reliability) {
                // Tenant pays rent
                income += tenant.rentAmount;
                notifications.push_back("Tenant " + tenant.name + " paid " + std::to_string(tenant.rentAmount) + " gold in rent.");
            } else {
                // Tenant missed payment
                notifications.push_back("Tenant " + tenant.name + " missed their rent payment!");
            }

            tenant.daysSinceLastPayment = 0;
        }
    }
}

void Property::addUpgrade(const PropertyUpgrade& upgrade)
{
    availableUpgrades.push_back(upgrade);
}

bool Property::installUpgrade(const std::string& upgradeId, int& cost)
{
    for (auto& upgrade : availableUpgrades) {
        if (upgrade.id == upgradeId && !upgrade.installed) {
            cost = upgrade.cost;
            upgrade.installed = true;

            // Apply upgrade effects
            upgrade.applyEffect(*this);

            return true;
        }
    }
    return false;
}

void Property::addTenant(const Tenant& tenant)
{
    tenants.push_back(tenant);
}

bool Property::evictTenant(const std::string& tenantId)
{
    for (auto it = tenants.begin(); it != tenants.end(); ++it) {
        if (it->id == tenantId) {
            tenants.erase(it);
            return true;
        }
    }
    return false;
}

bool Property::addToStorage(const Item& item)
{
    // Calculate current storage usage
    int currentUsage = 0;
    for (const auto& storageItem : storage.items) {
        currentUsage += storageItem.quantity;
    }

    // Check if there's enough space
    if (currentUsage + item.quantity > storageCapacity) {
        return false;
    }

    // Add to storage
    storage.addItem(item);
    return true;
}

bool Property::removeFromStorage(const std::string& itemId, int quantity)
{
    return storage.removeItem(itemId, quantity);
}