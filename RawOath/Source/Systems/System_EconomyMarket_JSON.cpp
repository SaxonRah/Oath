// System_EconomyMarket_JSON.cpp

#include "System_EconomyMarket_JSON.hpp"

#include <algorithm>
#include <cmath>
#include <ctime>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <queue>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

// Forward declarations
class TANode;
class TAController;
class Inventory;
class Item;
class GameContext;
struct TAInput;
struct TAAction;
struct NodeID;
struct WorldState;

// ----------------------------------------
// ECONOMY/MARKET SYSTEM
// ----------------------------------------

// Market type enum
enum class MarketType {
    GENERAL,
    BLACKSMITH,
    ALCHEMIST,
    CLOTHIER,
    JEWELER,
    BOOKSTORE,
    MAGIC_SUPPLIES,
    FOOD,
    TAVERN
};

// Convert string to MarketType enum
MarketType stringToMarketType(const std::string& typeStr)
{
    if (typeStr == "GENERAL")
        return MarketType::GENERAL;
    if (typeStr == "BLACKSMITH")
        return MarketType::BLACKSMITH;
    if (typeStr == "ALCHEMIST")
        return MarketType::ALCHEMIST;
    if (typeStr == "CLOTHIER")
        return MarketType::CLOTHIER;
    if (typeStr == "JEWELER")
        return MarketType::JEWELER;
    if (typeStr == "BOOKSTORE")
        return MarketType::BOOKSTORE;
    if (typeStr == "MAGIC_SUPPLIES")
        return MarketType::MAGIC_SUPPLIES;
    if (typeStr == "FOOD")
        return MarketType::FOOD;
    if (typeStr == "TAVERN")
        return MarketType::TAVERN;
    return MarketType::GENERAL; // Default
}

// String conversion for MarketType
const std::string marketTypeToString(MarketType type)
{
    switch (type) {
    case MarketType::GENERAL:
        return "General Store";
    case MarketType::BLACKSMITH:
        return "Blacksmith";
    case MarketType::ALCHEMIST:
        return "Alchemist";
    case MarketType::CLOTHIER:
        return "Clothier";
    case MarketType::JEWELER:
        return "Jeweler";
    case MarketType::BOOKSTORE:
        return "Bookstore";
    case MarketType::MAGIC_SUPPLIES:
        return "Magic Supplies";
    case MarketType::FOOD:
        return "Food Vendor";
    case MarketType::TAVERN:
        return "Tavern";
    default:
        return "Unknown";
    }
}

// Trade good commodity that can be tracked for economic simulation
struct TradeCommodity {
    std::string id;
    std::string name;
    float basePrice;
    float currentPrice;
    int supply; // Local supply level
    int demand; // Local demand level
    int baseSupply; // Reference supply level
    int baseDemand; // Reference demand level
    std::string origin; // Primary region where this commodity is produced
    bool isLuxury; // Luxury goods have higher price elasticity
    float volatility; // How much prices fluctuate (0.0 - 1.0)

    // Load from JSON
    static TradeCommodity fromJson(const json& j)
    {
        TradeCommodity commodity;
        commodity.id = j["id"];
        commodity.name = j["name"];
        commodity.basePrice = j["basePrice"];
        commodity.currentPrice = j["basePrice"]; // Start at base price
        commodity.supply = j["supply"];
        commodity.demand = j["demand"];
        commodity.baseSupply = j["baseSupply"];
        commodity.baseDemand = j["baseDemand"];
        commodity.origin = j["origin"];
        commodity.isLuxury = j["isLuxury"];
        commodity.volatility = j["volatility"];

        return commodity;
    }

    // Calculate price based on supply/demand ratios
    void updatePrice()
    {
        float supplyRatio = (supply > 0) ? (float)baseSupply / supply : 2.0f;
        float demandRatio = (float)demand / baseDemand;

        // Price is affected by supply/demand dynamics
        float marketFactor = demandRatio * supplyRatio;

        // Luxury goods have higher price elasticity
        if (isLuxury) {
            marketFactor = std::pow(marketFactor, 1.5);
        }

        // Calculate new price with limited volatility
        float targetPrice = basePrice * marketFactor;
        float maxChange = basePrice * volatility;

        // Limit price change to volatility
        if (targetPrice > currentPrice + maxChange) {
            currentPrice += maxChange;
        } else if (targetPrice < currentPrice - maxChange) {
            currentPrice -= maxChange;
        } else {
            currentPrice = targetPrice;
        }

        // Ensure price never goes below 10% of base
        currentPrice = std::max(currentPrice, basePrice * 0.1f);
    }

    // Apply a market shock (war, natural disaster, etc.)
    void applyShock(float supplyShock, float demandShock)
    {
        supply = std::max(1, (int)(supply * supplyShock));
        demand = std::max(1, (int)(demand * demandShock));
        updatePrice();
    }
};

// Trade route connecting two markets with specific goods
struct TradeRoute {
    std::string id;
    std::string name;
    std::string sourceMarket;
    std::string destinationMarket;
    std::vector<std::string> tradedGoods;
    float distance; // Affects transport cost and time
    float dangerLevel; // Affects risk of disruption (0.0 - 1.0)
    bool isActive; // Can be disrupted by events
    int travelDays; // Days required for goods to travel

    // Load from JSON
    static TradeRoute fromJson(const json& j)
    {
        TradeRoute route;
        route.id = j["id"];
        route.name = j["name"];
        route.sourceMarket = j["sourceMarket"];
        route.destinationMarket = j["destinationMarket"];
        route.distance = j["distance"];
        route.dangerLevel = j["dangerLevel"];
        route.isActive = true; // Start active
        route.travelDays = std::max(1, (int)(route.distance / 20.0f)); // 20 distance units per day

        if (j.contains("tradedGoods") && j["tradedGoods"].is_array()) {
            for (const auto& good : j["tradedGoods"]) {
                route.tradedGoods.push_back(good);
            }
        }

        return route;
    }

    // Calculate transport cost multiplier
    float getTransportCostMultiplier() const
    {
        return 1.0f + (distance * 0.01f) + (dangerLevel * 0.2f);
    }

    // Check if route is disrupted by random event
    bool checkDisruption(float randomValue) const
    {
        return randomValue < dangerLevel * 0.1f;
    }
};

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
    static EconomicEvent fromJson(const json& j)
    {
        EconomicEvent event;
        event.id = j["id"];
        event.name = j["name"];
        event.description = j["description"];
        event.duration = j["duration"];
        event.daysSinceStart = 0; // Start at 0

        // Load affected regions
        if (j.contains("affectedRegions") && j["affectedRegions"].is_array()) {
            for (const auto& region : j["affectedRegions"]) {
                event.affectedRegions.push_back(region);
            }
        }

        // Load commodity supply effects
        if (j.contains("commoditySupplyEffects") && j["commoditySupplyEffects"].is_object()) {
            for (auto it = j["commoditySupplyEffects"].begin(); it != j["commoditySupplyEffects"].end(); ++it) {
                event.commoditySupplyEffects[it.key()] = it.value();
            }
        }

        // Load commodity demand effects
        if (j.contains("commodityDemandEffects") && j["commodityDemandEffects"].is_object()) {
            for (auto it = j["commodityDemandEffects"].begin(); it != j["commodityDemandEffects"].end(); ++it) {
                event.commodityDemandEffects[it.key()] = it.value();
            }
        }

        // Load trade route disruptions
        if (j.contains("tradeRouteDisruptions") && j["tradeRouteDisruptions"].is_object()) {
            for (auto it = j["tradeRouteDisruptions"].begin(); it != j["tradeRouteDisruptions"].end(); ++it) {
                event.tradeRouteDisruptions[it.key()] = it.value();
            }
        }

        return event;
    }

    bool isActive() const
    {
        return daysSinceStart < duration;
    }

    void advance()
    {
        if (daysSinceStart < duration) {
            daysSinceStart++;
        }
    }
};

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
    static Market* fromJson(const json& j, const json& commoditiesData)
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

    // Standard constructor
    Market(const std::string& marketId, const std::string& marketName, MarketType marketType)
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

    // Calculate buy price for an item
    int calculateBuyPrice(const Item& item, int playerBarterSkill) const
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

    // Calculate sell price (what merchant pays player)
    int calculateSellPrice(const Item& item, int playerBarterSkill) const
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

    // Restock inventory based on market type and wealth
    void restock()
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

    // Process a day passing
    void advanceDay()
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

    // Add a commodity to this market
    void addCommodity(const TradeCommodity& commodity)
    {
        commodities.push_back(commodity);
    }

    // Find a commodity by ID
    TradeCommodity* findCommodity(const std::string& commodityId)
    {
        for (auto& commodity : commodities) {
            if (commodity.id == commodityId) {
                return &commodity;
            }
        }
        return nullptr;
    }

    // Apply economic event effects
    void applyEconomicEvent(const EconomicEvent& event)
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

    // Record an item the player has sold for potential buyback
    void recordPlayerSoldItem(const std::string& itemId, int quantity)
    {
        playerSoldItems[itemId] = (playerSoldItems.count(itemId) ? playerSoldItems[itemId] : 0) + quantity;
    }

    // Get the player's reputation with this merchant
    float getRelationToPlayer() const
    {
        return relationToPlayer;
    }

    // Improve relation with player
    void improveRelation(float amount)
    {
        relationToPlayer = std::min(100.0f, relationToPlayer + amount);
    }

    // Worsen relation with player
    void worsenRelation(float amount)
    {
        relationToPlayer = std::max(-100.0f, relationToPlayer - amount);
    }

private:
    // Helper to check if an item is specialized for this market type
    bool isItemSpecializedForMarket(const Item& item) const
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

    // Get quantity of an item already in inventory
    int getExistingQuantity(const std::string& itemId) const
    {
        for (const auto& item : inventory.items) {
            if (item.id == itemId) {
                return item.quantity;
            }
        }
        return 0;
    }

    // Remove a portion of inventory randomly
    void removeRandomInventory(float portion)
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

    // Generate new stock based on market type
    void generateStock()
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
        default:
            marketTypeStr = "general";
            break;
        }

        // Load the item data from JSON
        std::ifstream file("EconomyMarket.JSON");
        if (!file.is_open()) {
            std::cerr << "Failed to open EconomyMarket.JSON" << std::endl;
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

    // Add an item to inventory with random properties based on quality
    void addItemToInventory(const std::string& id, const std::string& name, const std::string& type, int baseValue, int quantity)
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

    // Random number helpers
    int randomInt(int min, int max) const
    {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(min, max);
        return dist(gen);
    }

    float randomFloat(float min, float max) const
    {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dist(min, max);
        return dist(gen);
    }
};

// Node for representing a merchant market in the tree automata system
class MarketNode : public TANode {
public:
    Market* market;
    std::vector<std::string> availableDialogueOptions;

    MarketNode(const std::string& name, Market* linkedMarket)
        : TANode(name)
        , market(linkedMarket)
    {
        if (market) {
            availableDialogueOptions = {
                "Buy Items",
                "Sell Items",
                "Browse Commodities",
                "Haggle",
                "Exit"
            };
        }
    }

    void onEnter(GameContext* context) override
    {
        std::cout << "Entered " << market->name << " (" << marketTypeToString(market->type) << ")" << std::endl;
        std::cout << "Shopkeeper: " << market->ownerName << std::endl;

        // Different greetings based on relation
        float relation = market->getRelationToPlayer();
        if (relation > 50.0f) {
            std::cout << "\"Ah, my favorite customer! Welcome back! What can I get for you today?\"" << std::endl;
        } else if (relation > 0.0f) {
            std::cout << "\"Welcome to my shop. Feel free to browse my wares.\"" << std::endl;
        } else if (relation > -50.0f) {
            std::cout << "\"What do you want? I have goods to sell if you have coin.\"" << std::endl;
        } else {
            std::cout << "\"I'll do business with you, but don't expect any special treatment.\"" << std::endl;
        }

        // Show available options
        std::cout << "\nAvailable options:" << std::endl;
        for (size_t i = 0; i < availableDialogueOptions.size(); i++) {
            std::cout << (i + 1) << ". " << availableDialogueOptions[i] << std::endl;
        }
    }

    std::vector<TAAction> getAvailableActions() override
    {
        std::vector<TAAction> actions;

        for (size_t i = 0; i < availableDialogueOptions.size(); i++) {
            actions.push_back({ "market_option_" + std::to_string(i),
                availableDialogueOptions[i],
                [this, i]() -> TAInput {
                    return { "market_action", { { "action", std::string("option") }, { "index", static_cast<int>(i) } } };
                } });
        }

        return actions;
    }

    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override
    {
        if (input.type == "market_action") {
            std::string action = std::get<std::string>(input.parameters.at("action"));

            if (action == "option") {
                int index = std::get<int>(input.parameters.at("index"));

                if (index >= 0 && index < static_cast<int>(availableDialogueOptions.size())) {
                    handleMarketOption(index, outNextNode);
                    return true;
                }
            } else if (action == "buy") {
                // Handle buying specific item
                std::string itemId = std::get<std::string>(input.parameters.at("item_id"));
                int quantity = std::get<int>(input.parameters.at("quantity"));
                handleBuyItem(itemId, quantity);
                outNextNode = this; // Stay in market
                return true;
            } else if (action == "sell") {
                // Handle selling specific item
                std::string itemId = std::get<std::string>(input.parameters.at("item_id"));
                int quantity = std::get<int>(input.parameters.at("quantity"));
                handleSellItem(itemId, quantity);
                outNextNode = this; // Stay in market
                return true;
            } else if (action == "haggle") {
                // Handle haggling
                handleHaggle();
                outNextNode = this; // Stay in market
                return true;
            } else if (action == "exit") {
                // Find and use an exit transition if available
                for (const auto& rule : transitionRules) {
                    if (rule.description == "Exit") {
                        outNextNode = rule.targetNode;
                        return true;
                    }
                }
            }
        }

        return TANode::evaluateTransition(input, outNextNode);
    }

private:
    void handleMarketOption(int option, TANode*& outNextNode)
    {
        std::string optionName = availableDialogueOptions[option];

        if (optionName == "Buy Items") {
            displayBuyOptions();
            outNextNode = this; // Stay in shop
        } else if (optionName == "Sell Items") {
            displaySellOptions();
            outNextNode = this; // Stay in shop
        } else if (optionName == "Browse Commodities") {
            displayCommodities();
            outNextNode = this; // Stay in shop
        } else if (optionName == "Haggle") {
            handleHaggle();
            outNextNode = this; // Stay in shop
        } else if (optionName == "Exit") {
            std::cout << "Shopkeeper: \"Come back again!\"" << std::endl;
            // Find and use an exit transition
            for (const auto& rule : transitionRules) {
                if (rule.description == "Exit") {
                    outNextNode = rule.targetNode;
                    return;
                }
            }
            // Fallback if no exit transition found
            outNextNode = this;
        }
    }

    void displayBuyOptions()
    {
        std::cout << "Items for sale:" << std::endl;
        std::cout << "------------------------------------------" << std::endl;
        std::cout << std::left << std::setw(30) << "Item" << std::setw(10) << "Price" << std::setw(10) << "Quantity" << std::endl;
        std::cout << "------------------------------------------" << std::endl;

        // Show merchant inventory
        for (size_t i = 0; i < market->inventory.items.size(); i++) {
            const auto& item = market->inventory.items[i];
            std::cout << std::left << std::setw(30) << item.name
                      << std::setw(10) << market->calculateBuyPrice(item, 50) // Assuming barter skill of 50
                      << std::setw(10) << item.quantity << std::endl;
        }

        std::cout << "\nTo buy an item, use the 'buy [item_name] [quantity]' command." << std::endl;
    }

    void displaySellOptions()
    {
        // TODO: In a real implementation, this would access the player inventory from the game context
        std::cout << "Your inventory items that this merchant might buy:" << std::endl;
        std::cout << "------------------------------------------" << std::endl;
        std::cout << std::left << std::setw(30) << "Item" << std::setw(10) << "Value" << std::setw(10) << "Quantity" << std::endl;
        std::cout << "------------------------------------------" << std::endl;

        // Placeholder for player inventory display
        std::cout << std::left << std::setw(30) << "Iron Dagger"
                  << std::setw(10) << "15"
                  << std::setw(10) << "1" << std::endl;

        std::cout << std::left << std::setw(30) << "Leather Strips"
                  << std::setw(10) << "2"
                  << std::setw(10) << "5" << std::endl;

        std::cout << "\nTo sell an item, use the 'sell [item_name] [quantity]' command." << std::endl;
    }

    void displayCommodities()
    {
        std::cout << "Trade commodities at " << market->name << ":" << std::endl;
        std::cout << "------------------------------------------" << std::endl;
        std::cout << std::left << std::setw(20) << "Commodity"
                  << std::setw(10) << "Price"
                  << std::setw(10) << "Supply"
                  << std::setw(10) << "Demand" << std::endl;
        std::cout << "------------------------------------------" << std::endl;

        for (const auto& commodity : market->commodities) {
            std::cout << std::left << std::setw(20) << commodity.name
                      << std::setw(10) << std::fixed << std::setprecision(2) << commodity.currentPrice
                      << std::setw(10) << commodity.supply
                      << std::setw(10) << commodity.demand << std::endl;
        }

        std::cout << "\nCommodity prices change based on supply and demand, and can be affected by trade routes and economic events." << std::endl;
    }

    void handleHaggle()
    {
        // TODO: In a real implementation, this would check player skills from the game context
        int persuasionSkill = 50; // Placeholder value
        float successChance = (float)persuasionSkill / (persuasionSkill + market->haggleSkillLevel);

        std::cout << "You attempt to haggle with " << market->ownerName << "..." << std::endl;

        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);

        if (dist(gen) < successChance) {
            // Success
            std::cout << "Your haggling was successful! Prices are temporarily improved." << std::endl;
            market->improveRelation(2.0f);
            // This would set a temporary price modifier in the actual implementation
        } else {
            // Failure
            std::cout << "The merchant is not impressed with your haggling attempt." << std::endl;
            market->worsenRelation(1.0f);
        }
    }

    void handleBuyItem(const std::string& itemId, int quantity)
    {
        // This is a simplified version - a real implementation would:
        // 1. Find the item in the merchant's inventory
        // 2. Calculate total cost based on player's barter skill
        // 3. Check if player has enough gold
        // 4. Transfer the item from merchant to player
        // 5. Transfer gold from player to merchant

        std::cout << "You bought " << quantity << "x " << itemId << "." << std::endl;
        std::cout << "This would normally transfer the item and deduct gold." << std::endl;
    }

    void handleSellItem(const std::string& itemId, int quantity)
    {
        // This is a simplified version - a real implementation would:
        // 1. Find the item in the player's inventory
        // 2. Calculate total value based on player's barter skill
        // 3. Transfer the item from player to merchant
        // 4. Transfer gold from merchant to player
        // 5. Record the item as player-sold for potential buyback

        std::cout << "You sold " << quantity << "x " << itemId << "." << std::endl;
        std::cout << "This would normally transfer the item and add gold." << std::endl;

        // Record for buyback
        market->recordPlayerSoldItem(itemId, quantity);
    }
};

// Economic system manager node that controls all markets and trade
class EconomicSystemNode : public TANode {
public:
    std::vector<Market*> markets;
    std::vector<TradeRoute> tradeRoutes;
    std::vector<EconomicEvent> activeEvents;
    std::vector<EconomicEvent> potentialEvents;
    int daysSinceLastEvent;
    float globalEconomicMultiplier;
    json configData; // Store the loaded JSON configuration

    EconomicSystemNode(const std::string& name)
        : TANode(name)
        , daysSinceLastEvent(0)
        , globalEconomicMultiplier(1.0f)
    {
        // Load configuration from JSON file
        loadConfigFromJson();
    }

    ~EconomicSystemNode()
    {
        // Clean up owned markets
        for (auto market : markets) {
            delete market;
        }
    }

    void loadConfigFromJson()
    {
        try {
            // Open and parse the JSON file
            std::ifstream file("EconomyMarket.JSON");
            if (!file.is_open()) {
                std::cerr << "Failed to open EconomyMarket.JSON" << std::endl;
                return;
            }

            file >> configData;
            file.close();

            // Initialize markets
            if (configData.contains("markets") && configData["markets"].is_array()) {
                for (const auto& marketData : configData["markets"]) {
                    Market* market = Market::fromJson(marketData, configData["commodities"]);
                    markets.push_back(market);

                    // Create a corresponding market node
                    MarketNode* marketNode = new MarketNode("Market_" + market->id, market);

                    // Add exit transition back to economic system
                    marketNode->addTransition(
                        [](const TAInput& input) {
                            return input.type == "market_action" && std::get<std::string>(input.parameters.at("action")) == "exit";
                        },
                        this, "Exit");

                    // Add as child node
                    addChild(marketNode);
                }
            }

            // Initialize trade routes
            if (configData.contains("tradeRoutes") && configData["tradeRoutes"].is_array()) {
                for (const auto& routeData : configData["tradeRoutes"]) {
                    tradeRoutes.push_back(TradeRoute::fromJson(routeData));
                }
            }

            // Initialize potential economic events
            if (configData.contains("economicEvents") && configData["economicEvents"].is_array()) {
                for (const auto& eventData : configData["economicEvents"]) {
                    potentialEvents.push_back(EconomicEvent::fromJson(eventData));
                }
            }

            std::cout << "Loaded " << markets.size() << " markets, "
                      << tradeRoutes.size() << " trade routes, and "
                      << potentialEvents.size() << " potential economic events from JSON." << std::endl;

        } catch (const std::exception& e) {
            std::cerr << "Error loading configuration from JSON: " << e.what() << std::endl;
        }
    }

    void onEnter(GameContext* context) override
    {
        std::cout << "Economic System Control Panel" << std::endl;
        std::cout << "-----------------------------" << std::endl;
        std::cout << "Markets: " << markets.size() << std::endl;
        std::cout << "Trade Routes: " << tradeRoutes.size() << std::endl;
        std::cout << "Active Economic Events: " << activeEvents.size() << std::endl;
        std::cout << "Global Economy Multiplier: " << globalEconomicMultiplier << std::endl;
    }

    std::vector<TAAction> getAvailableActions() override
    {
        std::vector<TAAction> actions = TANode::getAvailableActions();

        actions.push_back({ "view_markets", "View Markets",
            [this]() -> TAInput {
                return { "economy_action", { { "action", std::string("view_markets") } } };
            } });

        actions.push_back({ "view_trade_routes", "View Trade Routes",
            [this]() -> TAInput {
                return { "economy_action", { { "action", std::string("view_trade_routes") } } };
            } });

        actions.push_back({ "view_events", "View Economic Events",
            [this]() -> TAInput {
                return { "economy_action", { { "action", std::string("view_events") } } };
            } });

        actions.push_back({ "simulate_day", "Simulate Economic Day",
            [this]() -> TAInput {
                return { "economy_action", { { "action", std::string("simulate_day") } } };
            } });

        return actions;
    }

    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override
    {
        if (input.type == "economy_action") {
            std::string action = std::get<std::string>(input.parameters.at("action"));

            if (action == "view_markets") {
                displayMarkets();
                outNextNode = this;
                return true;
            } else if (action == "view_trade_routes") {
                displayTradeRoutes();
                outNextNode = this;
                return true;
            } else if (action == "view_events") {
                displayEconomicEvents();
                outNextNode = this;
                return true;
            } else if (action == "simulate_day") {
                simulateEconomicDay();
                outNextNode = this;
                return true;
            } else if (action == "visit_market") {
                int marketIndex = std::get<int>(input.parameters.at("market_index"));
                if (marketIndex >= 0 && marketIndex < static_cast<int>(childNodes.size())) {
                    outNextNode = childNodes[marketIndex];
                    return true;
                }
            }
        }

        return TANode::evaluateTransition(input, outNextNode);
    }

    // Create and register a new market
    Market* createMarket(const std::string& id, const std::string& name, MarketType type)
    {
        Market* market = new Market(id, name, type);
        markets.push_back(market);

        // Create a corresponding market node
        MarketNode* marketNode = new MarketNode("Market_" + id, market);

        // Add exit transition back to economic system
        marketNode->addTransition(
            [](const TAInput& input) {
                return input.type == "market_action" && std::get<std::string>(input.parameters.at("action")) == "exit";
            },
            this, "Exit");

        // Add as child node
        addChild(marketNode);

        return market;
    }

    // Add a trade route between markets
    void addTradeRoute(const std::string& id, const std::string& name,
        const std::string& sourceMarketId, const std::string& destMarketId,
        float distance, float danger)
    {
        TradeRoute route;
        route.id = id;
        route.name = name;
        route.sourceMarket = sourceMarketId;
        route.destinationMarket = destMarketId;
        route.distance = distance;
        route.dangerLevel = danger;
        route.isActive = true;
        route.travelDays = std::max(1, (int)(distance / 20.0f)); // 20 distance units per day

        tradeRoutes.push_back(route);
    }

    // Add a potential economic event
    void addPotentialEvent(const EconomicEvent& event)
    {
        potentialEvents.push_back(event);
    }

    // Process market supplies based on trade routes
    void processTradeRoutes()
    {
        for (auto& route : tradeRoutes) {
            if (!route.isActive)
                continue;

            // Find source and destination markets
            Market* sourceMarket = findMarketById(route.sourceMarket);
            Market* destMarket = findMarketById(route.destinationMarket);

            if (!sourceMarket || !destMarket)
                continue;

            // Check for disruption
            static std::random_device rd;
            static std::mt19937 gen(rd());
            std::uniform_real_distribution<float> dist(0.0f, 1.0f);

            if (route.checkDisruption(dist(gen))) {
                std::cout << "Trade route " << route.name << " has been disrupted!" << std::endl;
                route.isActive = false;
                continue;
            }

            // For each traded good, adjust supply and demand
            for (const auto& goodId : route.tradedGoods) {
                TradeCommodity* sourceCommodity = sourceMarket->findCommodity(goodId);
                TradeCommodity* destCommodity = destMarket->findCommodity(goodId);

                if (sourceCommodity && destCommodity) {
                    // Source has surplus, destination has demand
                    if (sourceCommodity->supply > sourceCommodity->baseSupply && destCommodity->supply < destCommodity->baseSupply) {

                        // Calculate amount to trade (limited by route capacity)
                        int tradeAmount = std::min(
                            sourceCommodity->supply - sourceCommodity->baseSupply,
                            destCommodity->baseSupply - destCommodity->supply);

                        tradeAmount = std::min(tradeAmount, 10); // Limit per day

                        if (tradeAmount > 0) {
                            // Transfer goods
                            sourceCommodity->supply -= tradeAmount;
                            destCommodity->supply += tradeAmount;

                            // Update prices
                            sourceCommodity->updatePrice();
                            destCommodity->updatePrice();

                            std::cout << tradeAmount << "x " << sourceCommodity->name
                                      << " traded from " << sourceMarket->name
                                      << " to " << destMarket->name << std::endl;
                        }
                    }
                }
            }
        }
    }
    // Process economic events
    void processEconomicEvents()
    {
        // Process existing events
        for (auto it = activeEvents.begin(); it != activeEvents.end();) {
            it->advance();

            if (!it->isActive()) {
                std::cout << "Economic event '" << it->name << "' has ended." << std::endl;
                it = activeEvents.erase(it);
            } else {
                // Apply ongoing effects
                for (auto* market : markets) {
                    market->applyEconomicEvent(*it);
                }
                ++it;
            }
        }

        // Check for new events
        daysSinceLastEvent++;

        if (!potentialEvents.empty() && daysSinceLastEvent >= 10) { // Check every 10 days
            static std::random_device rd;
            static std::mt19937 gen(rd());
            std::uniform_int_distribution<> dist(0, potentialEvents.size() - 1);
            std::uniform_real_distribution<float> chanceDist(0.0f, 1.0f);

            if (chanceDist(gen) < 0.2f) { // 20% chance of new event
                int eventIndex = dist(gen);
                activeEvents.push_back(potentialEvents[eventIndex]);

                std::cout << "New economic event: " << potentialEvents[eventIndex].name << std::endl;
                std::cout << potentialEvents[eventIndex].description << std::endl;

                // Apply initial effects
                for (auto* market : markets) {
                    market->applyEconomicEvent(potentialEvents[eventIndex]);
                }

                daysSinceLastEvent = 0;
            }
        }
    }

    // Simulate one economic day
    void simulateEconomicDay()
    {
        // Process trade routes
        processTradeRoutes();

        // Process economic events
        processEconomicEvents();

        // Update all markets
        for (auto* market : markets) {
            market->advanceDay();
        }

        std::cout << "Simulated one economic day. Markets updated, trade processed, events checked." << std::endl;
    }

private:
    void displayMarkets()
    {
        std::cout << "Available Markets:" << std::endl;
        std::cout << "-----------------------------------------------------------" << std::endl;
        std::cout << std::left << std::setw(20) << "Name"
                  << std::setw(15) << "Type"
                  << std::setw(15) << "Region"
                  << std::setw(10) << "Wealth" << std::endl;
        std::cout << "-----------------------------------------------------------" << std::endl;

        for (size_t i = 0; i < markets.size(); i++) {
            const auto* market = markets[i];
            std::cout << std::left << std::setw(20) << market->name
                      << std::setw(15) << marketTypeToString(market->type)
                      << std::setw(15) << market->region
                      << std::setw(10) << std::fixed << std::setprecision(2) << market->wealthLevel << std::endl;
        }

        std::cout << "\nTo visit a market, use the 'visit [market_name]' command." << std::endl;
    }

    void displayTradeRoutes()
    {
        std::cout << "Active Trade Routes:" << std::endl;
        std::cout << "-----------------------------------------------------------" << std::endl;
        std::cout << std::left << std::setw(20) << "Name"
                  << std::setw(15) << "From"
                  << std::setw(15) << "To"
                  << std::setw(10) << "Distance"
                  << std::setw(10) << "Danger" << std::endl;
        std::cout << "-----------------------------------------------------------" << std::endl;

        for (const auto& route : tradeRoutes) {
            if (route.isActive) {
                std::cout << std::left << std::setw(20) << route.name
                          << std::setw(15) << route.sourceMarket
                          << std::setw(15) << route.destinationMarket
                          << std::setw(10) << route.distance
                          << std::setw(10) << route.dangerLevel << std::endl;
            }
        }

        if (!tradeRoutes.empty()) {
            std::cout << "\nDisrupted Trade Routes:" << std::endl;
            std::cout << "-----------------------------------------------------------" << std::endl;

            bool hasDisrupted = false;
            for (const auto& route : tradeRoutes) {
                if (!route.isActive) {
                    std::cout << std::left << std::setw(20) << route.name
                              << std::setw(15) << route.sourceMarket
                              << std::setw(15) << route.destinationMarket
                              << std::setw(10) << route.distance
                              << std::setw(10) << route.dangerLevel << std::endl;
                    hasDisrupted = true;
                }
            }

            if (!hasDisrupted) {
                std::cout << "No disrupted trade routes currently." << std::endl;
            }
        }
    }

    void displayEconomicEvents()
    {
        std::cout << "Active Economic Events:" << std::endl;
        std::cout << "-----------------------------------------------------------" << std::endl;

        if (activeEvents.empty()) {
            std::cout << "No active economic events currently." << std::endl;
        } else {
            for (const auto& event : activeEvents) {
                std::cout << "Event: " << event.name << std::endl;
                std::cout << "Description: " << event.description << std::endl;
                std::cout << "Affected Regions: ";
                for (const auto& region : event.affectedRegions) {
                    std::cout << region << " ";
                }
                std::cout << std::endl;
                std::cout << "Duration: " << event.daysSinceStart << "/" << event.duration << " days" << std::endl;
                std::cout << "-----------------------------------------------------------" << std::endl;
            }
        }

        std::cout << "\nPotential Future Events: " << potentialEvents.size() << std::endl;
    }

    Market* findMarketById(const std::string& marketId)
    {
        for (auto* market : markets) {
            if (market->id == marketId) {
                return market;
            }
        }
        return nullptr;
    }
};

//----------------------------------------
// MARKETPLACE INVESTMENT SYSTEM
//----------------------------------------

// Investment opportunity in a business
struct BusinessInvestment {
    std::string id;
    std::string name;
    std::string description;
    std::string marketId; // Market where this business is located
    int initialCost; // Cost to invest
    float riskLevel; // 0.0 to 1.0 (higher = riskier)
    float returnRate; // Base return rate (higher for riskier investments)
    float marketSensitivity; // How much market conditions affect this business (0.0 to 2.0)
    int playerInvestment; // Amount player has invested
    int daysSinceLastPayout; // Days since last profit payout
    int payoutInterval; // Days between profit payouts
    bool isActive; // If the player has invested

    BusinessInvestment()
        : initialCost(100)
        , riskLevel(0.5f)
        , returnRate(0.05f)
        , marketSensitivity(1.0f)
        , playerInvestment(0)
        , daysSinceLastPayout(0)
        , payoutInterval(7)
        , isActive(false)
    {
    }

    // Load from JSON
    static BusinessInvestment fromJson(const json& j)
    {
        BusinessInvestment investment;
        investment.id = j["id"];
        investment.name = j["name"];
        investment.description = j["description"];
        investment.marketId = j["marketId"];
        investment.initialCost = j["initialCost"];
        investment.riskLevel = j["riskLevel"];
        investment.returnRate = j["returnRate"];
        investment.marketSensitivity = j["marketSensitivity"];
        investment.payoutInterval = j["payoutInterval"];

        return investment;
    }

    // Calculate expected profit
    int calculateExpectedProfit(float marketMultiplier) const
    {
        float effectiveRate = returnRate * (1.0f + (marketMultiplier - 1.0f) * marketSensitivity);
        return (int)(playerInvestment * effectiveRate);
    }

    // Calculate actual profit (with random variation)
    int calculateActualProfit(float marketMultiplier) const
    {
        int expectedProfit = calculateExpectedProfit(marketMultiplier);

        // Add random variation based on risk
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::normal_distribution<float> variation(1.0f, riskLevel * 0.5f);

        float randomFactor = variation(gen);
        randomFactor = std::max(0.1f, randomFactor); // Never lose more than 90%

        return (int)(expectedProfit * randomFactor);
    }

    // Process a day passing
    bool advanceDay(float marketMultiplier, int& profit)
    {
        if (!isActive)
            return false;

        daysSinceLastPayout++;

        if (daysSinceLastPayout >= payoutInterval) {
            profit = calculateActualProfit(marketMultiplier);
            daysSinceLastPayout = 0;
            return true;
        }

        return false;
    }
};

// Node for managing business investments
class InvestmentNode : public TANode {
public:
    std::vector<BusinessInvestment> investments;
    EconomicSystemNode* economicSystem;

    InvestmentNode(const std::string& name, EconomicSystemNode* econSystem)
        : TANode(name)
        , economicSystem(econSystem)
    {
        // Load investments from JSON if available
        loadInvestmentsFromJson();
    }

    void loadInvestmentsFromJson()
    {
        try {
            // Open and parse the JSON file
            std::ifstream file("EconomyMarket.JSON");
            if (!file.is_open()) {
                std::cerr << "Failed to open EconomyMarket.JSON for investments" << std::endl;
                return;
            }

            json data;
            file >> data;
            file.close();

            // Load investments
            if (data.contains("investments") && data["investments"].is_array()) {
                for (const auto& investmentData : data["investments"]) {
                    investments.push_back(BusinessInvestment::fromJson(investmentData));
                }

                std::cout << "Loaded " << investments.size() << " investment opportunities from JSON." << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "Error loading investments from JSON: " << e.what() << std::endl;
        }
    }

    void onEnter(GameContext* context) override
    {
        std::cout << "Business Investment Management" << std::endl;
        std::cout << "-----------------------------" << std::endl;
        std::cout << "You can invest in businesses to generate passive income." << std::endl;
        std::cout << "The returns depend on the business type, risk level, and market conditions." << std::endl;

        displayInvestments();
    }

    std::vector<TAAction> getAvailableActions() override
    {
        std::vector<TAAction> actions;

        actions.push_back({ "view_investments", "View Your Investments",
            [this]() -> TAInput {
                return { "investment_action", { { "action", std::string("view_investments") } } };
            } });

        actions.push_back({ "view_opportunities", "View Investment Opportunities",
            [this]() -> TAInput {
                return { "investment_action", { { "action", std::string("view_opportunities") } } };
            } });

        actions.push_back({ "invest", "Invest in a Business",
            [this]() -> TAInput {
                return { "investment_action", { { "action", std::string("invest") } } };
            } });

        actions.push_back({ "collect_profits", "Collect Investment Profits",
            [this]() -> TAInput {
                return { "investment_action", { { "action", std::string("collect_profits") } } };
            } });

        actions.push_back({ "exit", "Exit Investment Manager",
            [this]() -> TAInput {
                return { "investment_action", { { "action", std::string("exit") } } };
            } });

        return actions;
    }

    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override
    {
        if (input.type == "investment_action") {
            std::string action = std::get<std::string>(input.parameters.at("action"));

            if (action == "view_investments") {
                displayInvestments();
                outNextNode = this;
                return true;
            } else if (action == "view_opportunities") {
                displayOpportunities();
                outNextNode = this;
                return true;
            } else if (action == "invest") {
                handleInvesting();
                outNextNode = this;
                return true;
            } else if (action == "collect_profits") {
                collectProfits();
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

    // Add a new investment opportunity
    void addInvestmentOpportunity(const BusinessInvestment& investment)
    {
        investments.push_back(investment);
    }

    // Process a day passing for all investments
    void advanceDay(GameContext* context)
    {
        if (!context)
            return;

        int totalProfit = 0;

        for (auto& investment : investments) {
            int profit = 0;

            if (investment.advanceDay(economicSystem->globalEconomicMultiplier, profit)) {
                std::cout << "Your investment in " << investment.name << " has generated "
                          << profit << " gold in profits!" << std::endl;
                totalProfit += profit;
            }
        }

        if (totalProfit > 0) {
            // Add gold to player inventory in a real implementation
            std::cout << "Total profit from all investments: " << totalProfit << " gold" << std::endl;
        }
    }

private:
    void displayInvestments()
    {
        bool hasInvestments = false;

        std::cout << "Your Active Investments:" << std::endl;
        std::cout << "-----------------------------------------------------------" << std::endl;
        std::cout << std::left << std::setw(25) << "Business"
                  << std::setw(15) << "Investment"
                  << std::setw(15) << "Risk Level"
                  << std::setw(15) << "Next Payout" << std::endl;
        std::cout << "-----------------------------------------------------------" << std::endl;

        for (const auto& investment : investments) {
            if (investment.isActive) {
                hasInvestments = true;
                std::cout << std::left << std::setw(25) << investment.name
                          << std::setw(15) << investment.playerInvestment
                          << std::setw(15) << (investment.riskLevel < 0.3f ? "Low" : (investment.riskLevel < 0.7f ? "Medium" : "High"))
                          << std::setw(15) << (investment.payoutInterval - investment.daysSinceLastPayout)
                          << " days" << std::endl;
            }
        }

        if (!hasInvestments) {
            std::cout << "You don't have any active investments." << std::endl;
        }
    }

    void displayOpportunities()
    {
        std::cout << "Available Investment Opportunities:" << std::endl;
        std::cout << "-----------------------------------------------------------" << std::endl;
        std::cout << std::left << std::setw(25) << "Business"
                  << std::setw(15) << "Initial Cost"
                  << std::setw(15) << "Risk Level"
                  << std::setw(15) << "Return Rate" << std::endl;
        std::cout << "-----------------------------------------------------------" << std::endl;

        for (size_t i = 0; i < investments.size(); i++) {
            const auto& investment = investments[i];
            if (!investment.isActive) {
                std::cout << std::left << std::setw(25) << investment.name
                          << std::setw(15) << investment.initialCost
                          << std::setw(15) << (investment.riskLevel < 0.3f ? "Low" : (investment.riskLevel < 0.7f ? "Medium" : "High"))
                          << std::setw(15) << std::fixed << std::setprecision(1)
                          << (investment.returnRate * 100.0f) << "%" << std::endl;
            }
        }
    }

    void handleInvesting()
    {
        // In a real implementation, this would select from available investments
        // and process the gold transaction

        std::cout << "Which business would you like to invest in? (Enter index or name)" << std::endl;

        // Example placeholder response
        std::cout << "You've invested in The Golden Goblet Tavern for 500 gold." << std::endl;
        std::cout << "You'll receive returns every 7 days." << std::endl;

        // For demonstration, activate the first inactive investment
        for (auto& investment : investments) {
            if (!investment.isActive) {
                investment.isActive = true;
                investment.playerInvestment = investment.initialCost;
                break;
            }
        }
    }

    void collectProfits()
    {
        std::cout << "Your profits have been automatically added to your gold." << std::endl;
        std::cout << "The next payout will occur when the investment interval completes." << std::endl;
    }
};

//----------------------------------------
// PROPERTY OWNERSHIP SYSTEM
//----------------------------------------

// Property types
enum class PropertyType {
    HOUSE,
    SHOP,
    FARM,
    WAREHOUSE,
    ESTATE
};

// Convert string to PropertyType
PropertyType stringToPropertyType(const std::string& typeStr)
{
    if (typeStr == "HOUSE")
        return PropertyType::HOUSE;
    if (typeStr == "SHOP")
        return PropertyType::SHOP;
    if (typeStr == "FARM")
        return PropertyType::FARM;
    if (typeStr == "WAREHOUSE")
        return PropertyType::WAREHOUSE;
    if (typeStr == "ESTATE")
        return PropertyType::ESTATE;
    return PropertyType::HOUSE; // Default
}

// String conversion for PropertyType
const std::string propertyTypeToString(PropertyType type)
{
    switch (type) {
    case PropertyType::HOUSE:
        return "House";
    case PropertyType::SHOP:
        return "Shop";
    case PropertyType::FARM:
        return "Farm";
    case PropertyType::WAREHOUSE:
        return "Warehouse";
    case PropertyType::ESTATE:
        return "Estate";
    default:
        return "Unknown";
    }
}

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
        static PropertyUpgrade fromJson(const json& j)
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

        void applyEffect(Property& property)
        {
            property.weeklyUpkeep += upkeepIncrease;
        }
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
        static Tenant fromJson(const json& j)
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
    };
    std::vector<Tenant> tenants;

    Property()
        : type(PropertyType::HOUSE)
        , purchasePrice(5000)
        , weeklyIncome(0)
        , weeklyUpkeep(50)
        , isOwned(false)
        , storageCapacity(100)
    {
    }

    // Load from JSON
    static Property fromJson(const json& j)
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

    // Calculate net income including tenant rent
    int calculateNetIncome() const
    {
        int totalIncome = weeklyIncome;

        // Add tenant income
        for (const auto& tenant : tenants) {
            totalIncome += tenant.rentAmount;
        }

        return totalIncome - weeklyUpkeep;
    }

    // Process a day passing
    void advanceDay(int& income, std::vector<std::string>& notifications)
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

    // Add an upgrade
    void addUpgrade(const PropertyUpgrade& upgrade)
    {
        availableUpgrades.push_back(upgrade);
    }

    // Install an upgrade
    bool installUpgrade(const std::string& upgradeId, int& cost)
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

    // Add a tenant
    void addTenant(const Tenant& tenant)
    {
        tenants.push_back(tenant);
    }

    // Evict a tenant
    bool evictTenant(const std::string& tenantId)
    {
        for (auto it = tenants.begin(); it != tenants.end(); ++it) {
            if (it->id == tenantId) {
                tenants.erase(it);
                return true;
            }
        }
        return false;
    }

    // Add an item to storage
    bool addToStorage(const Item& item)
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

    // Remove an item from storage
    bool removeFromStorage(const std::string& itemId, int quantity)
    {
        return storage.removeItem(itemId, quantity);
    }
};

// Node for managing player properties
class PropertyNode : public TANode {
public:
    std::vector<Property> properties;
    int playerGold;
    int daysSinceLastUpkeep;
    std::vector<std::string> recentNotifications;

    PropertyNode(const std::string& name)
        : TANode(name)
        , playerGold(1000)
        , daysSinceLastUpkeep(0)
    {
        // Load properties from JSON
        loadPropertiesFromJson();
    }

    void loadPropertiesFromJson()
    {
        try {
            // Open and parse the JSON file
            std::ifstream file("EconomyMarket.JSON");
            if (!file.is_open()) {
                std::cerr << "Failed to open EconomyMarket.JSON for properties" << std::endl;
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

    void onEnter(GameContext* context) override
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

    std::vector<TAAction> getAvailableActions() override
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

    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override
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

    // Add a new property
    void addProperty(const Property& property)
    {
        properties.push_back(property);
    }

    // Process a day passing for all properties
    void advanceDay()
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

private:
    void displayOwnedProperties()
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

    void displayAvailableProperties()
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

    void handlePropertyManagement()
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

    void handlePropertyPurchase()
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
};

//----------------------------------------
// TEST MAIN FUNCTION
//----------------------------------------

int main()
{
    std::cout << "=== ECONOMIC AND PROPERTY SYSTEM DEMO ===" << std::endl;

    // Load configuration before creating systems
    std::cout << "Loading configuration from EconomyMarket.JSON..." << std::endl;

    // Create controller
    TAController controller;

    // Create economic system
    EconomicSystemNode* economicSystem = dynamic_cast<EconomicSystemNode*>(
        controller.createNode<EconomicSystemNode>("EconomicSystem"));

    // Create investment system
    InvestmentNode* investmentSystem = dynamic_cast<InvestmentNode*>(
        controller.createNode<InvestmentNode>("InvestmentSystem", economicSystem));

    // Create property system
    PropertyNode* propertySystem = dynamic_cast<PropertyNode*>(
        controller.createNode<PropertyNode>("PropertySystem"));

    // Connect the systems
    controller.setSystemRoot("EconomySystem", economicSystem);

    // Add transitions between systems
    economicSystem->addTransition(
        [](const TAInput& input) {
            return input.type == "economy_action" && std::get<std::string>(input.parameters.at("action")) == "to_investments";
        },
        investmentSystem, "Go to Investments");

    economicSystem->addTransition(
        [](const TAInput& input) {
            return input.type == "economy_action" && std::get<std::string>(input.parameters.at("action")) == "to_properties";
        },
        propertySystem, "Go to Properties");

    investmentSystem->addTransition(
        [](const TAInput& input) {
            return input.type == "investment_action" && std::get<std::string>(input.parameters.at("action")) == "exit";
        },
        economicSystem, "Exit");

    propertySystem->addTransition(
        [](const TAInput& input) {
            return input.type == "property_action" && std::get<std::string>(input.parameters.at("action")) == "exit";
        },
        economicSystem, "Exit");

    // Initialize inventory
    controller.gameContext.playerInventory.addItem(Item("gold_coin", "Gold", "currency", 1, 1000));
    controller.gameContext.playerInventory.addItem(Item("iron_sword", "Iron Sword", "weapon", 100, 1));
    controller.gameContext.playerInventory.addItem(Item("leather_strips", "Leather Strips", "material", 5, 10));
    controller.gameContext.playerInventory.addItem(Item("red_herb", "Red Herb", "herb", 8, 5));

    // Demo
    std::cout << "\n=== ECONOMY SYSTEM DEMO ===\n"
              << std::endl;

    // Initialize the system
    controller.processInput("EconomySystem", {});

    // View markets
    TAInput viewMarketsInput = { "economy_action", { { "action", std::string("view_markets") } } };
    controller.processInput("EconomySystem", viewMarketsInput);

    // Simulate an economic day
    TAInput simulateDayInput = { "economy_action", { { "action", std::string("simulate_day") } } };
    controller.processInput("EconomySystem", simulateDayInput);

    // View the effects on commodities
    TAInput viewTradeRoutesInput = { "economy_action", { { "action", std::string("view_trade_routes") } } };
    controller.processInput("EconomySystem", viewTradeRoutesInput);

    // Visit a market
    TAInput visitMarketInput = { "economy_action",
        { { "action", std::string("visit_market") },
            { "market_index", 0 } } }; // First market
    controller.processInput("EconomySystem", visitMarketInput);

    // View market items
    TAInput buyItemsInput = { "market_action", { { "action", std::string("option") }, { "index", 0 } } }; // Buy Items
    controller.processInput("EconomySystem", buyItemsInput);

    // Exit market
    TAInput exitMarketInput = { "market_action", { { "action", std::string("exit") } } };
    controller.processInput("EconomySystem", exitMarketInput);

    // Go to investment system
    TAInput toInvestmentsInput = { "economy_action", { { "action", std::string("to_investments") } } };
    controller.processInput("EconomySystem", toInvestmentsInput);

    // View investment opportunities
    TAInput viewOpportunitiesInput = { "investment_action", { { "action", std::string("view_opportunities") } } };
    controller.processInput("InvestmentSystem", viewOpportunitiesInput);

    // Invest in a business
    TAInput investInput = { "investment_action", { { "action", std::string("invest") } } };
    controller.processInput("InvestmentSystem", investInput);

    // Back to economy system
    TAInput exitInvestmentsInput = { "investment_action", { { "action", std::string("exit") } } };
    controller.processInput("InvestmentSystem", exitInvestmentsInput);

    // Go to property system
    TAInput toPropertiesInput = { "economy_action", { { "action", std::string("to_properties") } } };
    controller.processInput("EconomySystem", toPropertiesInput);

    // View available properties
    TAInput viewAvailableInput = { "property_action", { { "action", std::string("view_available") } } };
    controller.processInput("PropertySystem", viewAvailableInput);

    // Buy a property
    TAInput buyPropertyInput = { "property_action", { { "action", std::string("buy_property") } } };
    controller.processInput("PropertySystem", buyPropertyInput);

    // Manage property
    TAInput managePropertyInput = { "property_action", { { "action", std::string("manage_property") } } };
    controller.processInput("PropertySystem", managePropertyInput);

    // Simulate multiple days - Economy, Investment, and Property systems
    std::cout << "\n=== SIMULATING MULTIPLE DAYS ===\n"
              << std::endl;

    for (int i = 0; i < 10; i++) {
        std::cout << "\nDay " << (i + 1) << ":" << std::endl;

        // Simulate economic day
        economicSystem->simulateEconomicDay();

        // Process investments
        investmentSystem->advanceDay(&controller.gameContext);

        // Process properties
        propertySystem->advanceDay();
    }

    // Final status
    std::cout << "\n=== FINAL STATUS ===\n"
              << std::endl;

    // Economy status
    controller.processInput("EconomySystem", viewMarketsInput);

    // Investment status
    controller.processInput("EconomySystem", toInvestmentsInput);
    controller.processInput("InvestmentSystem", { "investment_action", { { "action", std::string("view_investments") } } });

    // Property status
    controller.processInput("InvestmentSystem", exitInvestmentsInput);
    controller.processInput("EconomySystem", toPropertiesInput);
    controller.processInput("PropertySystem", { "property_action", { { "action", std::string("view_owned") } } });

    return 0;
}