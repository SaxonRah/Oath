// systems/economy/Market.cpp

#include "Market.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>

Market* Market::fromJson(const json& j, const json& commoditiesData)
{
    std::string id = j["id"];
    std::string name = j["name"];
    MarketType type = stringToMarketType(j["type"]);

    Market* market = new Market(id, name, type);

    if (j.contains("region"))
        market->region = j["region"];
    if (j.contains("wealthLevel"))
        market->wealthLevel = j["wealthLevel"];
    if (j.contains("ownerName"))
        market->ownerName = j["ownerName"];

    // Add commodities
    if (j.contains("commodities") && j["commodities"].is_array()) {
        for (const auto& commodityId : j["commodities"]) {
            // Find commodity data in commodities array
            for (const auto& commodityData : commoditiesData) {
                if (commodityData["id"] == commodityId) {
                    market->addCommodity(TradeCommodity::fromJson(commodityData));
                    break;
                }
            }
        }
    }

    return market;
}

Market::Market(const std::string& marketId, const std::string& marketName, MarketType marketType)
    : id(marketId)
    , name(marketName)
    , type(marketType)
    , wealthLevel(1.0f)
    , taxRate(0.05f)
    , isPrimaryMarket(false)
    , restockDays(7)
    , daysSinceRestock(0)
    , relationToPlayer(0.0f)
    , haggleSkillLevel(50.0f)
{
}

int Market::calculateBuyPrice(const Item& item, int playerBarterSkill) const
{
    float basePriceMultiplier = 1.0f + taxRate;

    // Relationship affects price (better relations = better prices)
    float relationFactor = 1.0f - (relationToPlayer / 200.0f); // 0.5 at max relation, 1.5 at min

    // Player haggling skill vs merchant skill
    float haggleFactor = 1.0f - (playerBarterSkill / (playerBarterSkill + haggleSkillLevel));

    // Discounts for selling multiple of the same item
    float quantityDiscount = 1.0f - std::min(0.15f, (float)(item.quantity - 1) * 0.01f);

    // Final price calculation
    float finalMultiplier = basePriceMultiplier * relationFactor * haggleFactor * quantityDiscount;

    // Apply market type specialization
    if (isItemSpecializedForMarket(item)) {
        finalMultiplier *= 0.85f; // 15% discount at specialized shops
    }

    int price = std::max(1, (int)(item.value * finalMultiplier));
    return price;
}

int Market::calculateSellPrice(const Item& item, int playerBarterSkill) const
{
    // Base price is lower when selling to merchants
    float basePriceMultiplier = 0.4f; // Merchants buy at 40% of value generally

    // Relationship affects price
    float relationFactor = 1.0f + (relationToPlayer / 300.0f); // +33% at max relation

    // Player haggling skill
    float haggleFactor = 1.0f + (playerBarterSkill / (playerBarterSkill + haggleSkillLevel + 50.0f));

    // Merchants pay less when they have many of an item
    int existingQuantity = getExistingQuantity(item.id);
    float supplyFactor = 1.0f - std::min(0.5f, (float)existingQuantity * 0.05f);

    // Apply market specialization - they pay more for items they specialize in
    float specializationFactor = isItemSpecializedForMarket(item) ? 1.2f : 1.0f;

    // Final price
    float finalMultiplier = basePriceMultiplier * relationFactor * haggleFactor * supplyFactor * specializationFactor;

    int price = std::max(1, (int)(item.value * finalMultiplier));
    return price;
}

void Market::restock()
{
    daysSinceRestock = 0;

    // Remove a portion of existing inventory to simulate sales
    removeRandomInventory(0.3f); // Remove 30% of inventory

    // Generate new stock based on market type
    generateStock();

    // Update commodity prices
    for (auto& commodity : commodities) {
        // Random supply/demand fluctuations
        commodity.supply += randomInt(-5, 5);
        commodity.demand += randomInt(-3, 3);

        // Ensure minimums
        commodity.supply = std::max(1, commodity.supply);
        commodity.demand = std::max(1, commodity.demand);

        // Update prices
        commodity.updatePrice();
    }

    std::cout << "Market " << name << " has been restocked." << std::endl;
}

void Market::advanceDay()
{
    daysSinceRestock++;

    // Check if restock is needed
    if (daysSinceRestock >= restockDays) {
        restock();
    }

    // Small daily commodity fluctuations
    for (auto& commodity : commodities) {
        // Small random changes
        if (randomInt(1, 100) <= 20) { // 20% chance of change
            commodity.supply += randomInt(-2, 2);
            commodity.demand += randomInt(-1, 1);

            // Ensure minimums
            commodity.supply = std::max(1, commodity.supply);
            commodity.demand = std::max(1, commodity.demand);

            // Update price
            commodity.updatePrice();
        }
    }
}

void Market::addCommodity(const TradeCommodity& commodity)
{
    commodities.push_back(commodity);
}

TradeCommodity* Market::findCommodity(const std::string& commodityId)
{
    for (auto& commodity : commodities) {
        if (commodity.id == commodityId) {
            return &commodity;
        }
    }
    return nullptr;
}

void Market::applyEconomicEvent(const EconomicEvent& event)
{
    // Check if this market's region is affected
    bool isAffected = false;
    for (const auto& region : event.affectedRegions) {
        if (region == this->region) {
            isAffected = true;
            break;
        }
    }

    if (!isAffected)
        return;

    // Apply commodity effects
    for (auto& commodity : commodities) {
        // Apply supply effects
        auto supplyIt = event.commoditySupplyEffects.find(commodity.id);
        if (supplyIt != event.commoditySupplyEffects.end()) {
            commodity.supply = std::max(1, (int)(commodity.supply * supplyIt->second));
        }

        // Apply demand effects
        auto demandIt = event.commodityDemandEffects.find(commodity.id);
        if (demandIt != event.commodityDemandEffects.end()) {
            commodity.demand = std::max(1, (int)(commodity.demand * demandIt->second));
        }

        // Update price after effects
        commodity.updatePrice();
    }

    std::cout << "Economic event '" << event.name << "' has affected market " << name << "." << std::endl;
}

void Market::recordPlayerSoldItem(const std::string& itemId, int quantity)
{
    playerSoldItems[itemId] = (playerSoldItems.count(itemId) ? playerSoldItems[itemId] : 0) + quantity;
}

float Market::getRelationToPlayer() const
{
    return relationToPlayer;
}

void Market::improveRelation(float amount)
{
    relationToPlayer = std::min(100.0f, relationToPlayer + amount);
}

void Market::worsenRelation(float amount)
{
    relationToPlayer = std::max(-100.0f, relationToPlayer - amount);
}

bool Market::isItemSpecializedForMarket(const Item& item) const
{
    switch (type) {
    case MarketType::BLACKSMITH:
        return item.type == "weapon" || item.type == "armor" || item.type == "metal";
    case MarketType::ALCHEMIST:
        return item.type == "potion" || item.type == "herb" || item.type == "ingredient";
    case MarketType::CLOTHIER:
        return item.type == "clothing" || item.type == "fabric";
    case MarketType::JEWELER:
        return item.type == "jewelry" || item.type == "gem";
    case MarketType::BOOKSTORE:
        return item.type == "book" || item.type == "scroll";
    case MarketType::MAGIC_SUPPLIES:
        return item.type == "magic" || item.type == "soul_gem" || item.type == "staff";
    case MarketType::FOOD:
        return item.type == "food" || item.type == "ingredient";
    case MarketType::TAVERN:
        return item.type == "food" || item.type == "drink";
    case MarketType::GENERAL:
        return true; // General stores buy/sell everything but at standard rates
    default:
        return false;
    }
}

int Market::getExistingQuantity(const std::string& itemId) const
{
    for (const auto& item : inventory.items) {
        if (item.id == itemId) {
            return item.quantity;
        }
    }
    return 0;
}

void Market::removeRandomInventory(float portion)
{
    std::vector<Item> remainingItems;

    for (const auto& item : inventory.items) {
        // Random chance to keep the item
        if (randomFloat(0, 1) > portion) {
            remainingItems.push_back(item);
        }
    }

    inventory.items = remainingItems;
}

void Market::generateStock()
{
    // Get market type string for JSON access
    std::string marketTypeStr;

    switch (type) {
    case MarketType::BLACKSMITH:
        marketTypeStr = "blacksmith";
        break;
    case MarketType::ALCHEMIST:
        marketTypeStr = "alchemist";
        break;
    case MarketType::CLOTHIER:
        marketTypeStr = "clothier";
        break;
    case MarketType::JEWELER:
        marketTypeStr = "jeweler";
        break;
    case MarketType::BOOKSTORE:
        marketTypeStr = "bookstore";
        break;
    case MarketType::MAGIC_SUPPLIES:
        marketTypeStr = "magic_supplies";
        break;
    case MarketType::FOOD:
        marketTypeStr = "food";
        break;
    case MarketType::TAVERN:
        marketTypeStr = "tavern";
        break;
    case MarketType::GENERAL:
        marketTypeStr = "general";
        break;
    }

    // Load the item data from JSON
    std::ifstream file("resources/json/economy.json");
    if (!file.is_open()) {
        std::cerr << "Failed to open economy.json" << std::endl;
        return;
    }

    json marketData;
    try {
        file >> marketData;
    } catch (const std::exception& e) {
        std::cerr << "Error parsing JSON: " << e.what() << std::endl;
        return;
    }

    // Inventory size based on wealth and primary status
    int baseInventorySize = isPrimaryMarket ? 25 : 15;
    int inventorySize = (int)(baseInventorySize * wealthLevel);

    // Generate items based on market type
    if (marketData.contains("items") && marketData["items"].contains(marketTypeStr)) {
        const auto& items = marketData["items"][marketTypeStr];

        for (const auto& item : items) {
            // Extract item data
            std::string id = item["id"];
            std::string name = item["name"];
            std::string type = item["type"];
            int value = item["value"];

            // Quantity range
            int minQuantity = item["quantityRange"][0];
            int maxQuantity = item["quantityRange"][1];
            int quantity = randomInt(minQuantity, maxQuantity);

            if (quantity > 0) {
                addItemToInventory(id, name, type, value, quantity);
            }
        }
    }
}

void Market::addItemToInventory(const std::string& id, const std::string& name, const std::string& type, int baseValue, int quantity)
{
    if (quantity <= 0)
        return;

    // Check if we already have this item
    for (auto& item : inventory.items) {
        if (item.id == id) {
            item.quantity += quantity;
            return;
        }
    }

    // Adjust value based on market wealth
    int adjustedValue = (int)(baseValue * (0.8f + (wealthLevel * 0.4f)));

    // Create and add the item
    Item item(id, name, type, adjustedValue, quantity);

    // Add some random properties based on item type
    if (type == "weapon") {
        item.properties["damage"] = 10 + randomInt(0, 5);
        if (randomInt(1, 100) <= 10) { // 10% chance for special property
            item.properties["enchanted"] = true;
            item.properties["enchantment"] = "fire";
            item.properties["enchantment_power"] = 5;
            item.value = (int)(item.value * 2.5f);
        }
    } else if (type == "armor") {
        item.properties["defense"] = 5 + randomInt(0, 3);
        if (randomInt(1, 100) <= 10) { // 10% chance for special property
            // systems/economy/Market.cpp (continued)

            item.properties["enchanted"] = true;
            item.properties["enchantment"] = "protection";
            item.properties["enchantment_power"] = 5;
            item.value = (int)(item.value * 2.5f);
        }
    } else if (type == "potion") {
        item.properties["potency"] = 25 + randomInt(0, 15);
        item.properties["duration"] = 30 + randomInt(0, 30);
    }

    inventory.addItem(item);
}

int Market::randomInt(int min, int max) const
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(min, max);
    return dist(gen);
}

float Market::randomFloat(float min, float max) const
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(min, max);
    return dist(gen);
}