// systems/economy/Market.hpp
#pragma once

#include <map>
#include <nlohmann/json.hpp>
#include <random>
#include <string>
#include <vector>

#include "../../data/Inventory.hpp"
#include "../../data/Item.hpp"
#include "EconomicEvent.hpp"
#include "MarketTypes.hpp"
#include "TradeCommodity.hpp"

using json = nlohmann::json;

// Market representing a shop or trading post
class Market {
public:
    std::string id;
    std::string name;
    MarketType type;
    std::string region; // Region where this market is located
    float wealthLevel; // Affects available inventory and prices (0.0 - 2.0)
    float taxRate; // Local tax rate (0.0 - 0.3)
    bool isPrimaryMarket; // Main market in a region has more goods
    int restockDays; // Days between inventory restocks
    int daysSinceRestock; // Days since last restock
    Inventory inventory; // Items for sale
    std::vector<TradeCommodity> commodities; // Tracked commodities
    std::map<std::string, int> playerSoldItems; // Track items sold by player for buyback

    // NPC owner details
    std::string ownerName;
    float relationToPlayer; // -100 to 100, affects prices
    float haggleSkillLevel; // Resistance to player's persuasion (0-100)

    // Constructor from JSON
    static Market* fromJson(const json& j, const json& commoditiesData);

    // Standard constructor
    Market(const std::string& marketId, const std::string& marketName, MarketType marketType);

    // Calculate buy price for an item
    int calculateBuyPrice(const Item& item, int playerBarterSkill) const;

    // Calculate sell price (what merchant pays player)
    int calculateSellPrice(const Item& item, int playerBarterSkill) const;

    // Restock inventory based on market type and wealth
    void restock();

    // Process a day passing
    void advanceDay();

    // Add a commodity to this market
    void addCommodity(const TradeCommodity& commodity);

    // Find a commodity by ID
    TradeCommodity* findCommodity(const std::string& commodityId);

    // Apply economic event effects
    void applyEconomicEvent(const EconomicEvent& event);

    // Record an item the player has sold for potential buyback
    void recordPlayerSoldItem(const std::string& itemId, int quantity);

    // Get the player's reputation with this merchant
    float getRelationToPlayer() const;

    // Improve relation with player
    void improveRelation(float amount);

    // Worsen relation with player
    void worsenRelation(float amount);

private:
    // Helper to check if an item is specialized for this market type
    bool isItemSpecializedForMarket(const Item& item) const;

    // Get quantity of an item already in inventory
    int getExistingQuantity(const std::string& itemId) const;

    // Remove a portion of inventory randomly
    void removeRandomInventory(float portion);

    // Generate new stock based on market type
    void generateStock();

    // Add an item to inventory with random properties based on quality
    void addItemToInventory(const std::string& id, const std::string& name, const std::string& type, int baseValue, int quantity);

    // Random number helpers
    int randomInt(int min, int max) const;
    float randomFloat(float min, float max) const;
};
