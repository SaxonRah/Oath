// System_EconomyMarket_JSON.hpp
#ifndef SYSTEM_ECONOMY_MARKET_JSON_HPP
#define SYSTEM_ECONOMY_MARKET_JSON_HPP

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

// Market type conversion functions
MarketType stringToMarketType(const std::string& typeStr);
const std::string marketTypeToString(MarketType type);

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
    static TradeCommodity fromJson(const json& j);

    // Calculate price based on supply/demand ratios
    void updatePrice();

    // Apply a market shock (war, natural disaster, etc.)
    void applyShock(float supplyShock, float demandShock);
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
    static TradeRoute fromJson(const json& j);

    // Calculate transport cost multiplier
    float getTransportCostMultiplier() const;

    // Check if route is disrupted by random event
    bool checkDisruption(float randomValue) const;
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
    static EconomicEvent fromJson(const json& j);

    bool isActive() const;
    void advance();
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

// Node for representing a merchant market in the tree automata system
class MarketNode : public TANode {
public:
    Market* market;
    std::vector<std::string> availableDialogueOptions;

    MarketNode(const std::string& name, Market* linkedMarket);

    void onEnter(GameContext* context) override;
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;

private:
    void handleMarketOption(int option, TANode*& outNextNode);
    void displayBuyOptions();
    void displaySellOptions();
    void displayCommodities();
    void handleHaggle();
    void handleBuyItem(const std::string& itemId, int quantity);
    void handleSellItem(const std::string& itemId, int quantity);
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

    EconomicSystemNode(const std::string& name);
    ~EconomicSystemNode();

    void loadConfigFromJson();
    void onEnter(GameContext* context) override;
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;

    // Create and register a new market
    Market* createMarket(const std::string& id, const std::string& name, MarketType type);

    // Add a trade route between markets
    void addTradeRoute(const std::string& id, const std::string& name,
        const std::string& sourceMarketId, const std::string& destMarketId,
        float distance, float danger);

    // Add a potential economic event
    void addPotentialEvent(const EconomicEvent& event);

    // Process market supplies based on trade routes
    void processTradeRoutes();

    // Process economic events
    void processEconomicEvents();

    // Simulate one economic day
    void simulateEconomicDay();

private:
    void displayMarkets();
    void displayTradeRoutes();
    void displayEconomicEvents();
    Market* findMarketById(const std::string& marketId);
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

    BusinessInvestment();

    // Load from JSON
    static BusinessInvestment fromJson(const json& j);

    // Calculate expected profit
    int calculateExpectedProfit(float marketMultiplier) const;

    // Calculate actual profit (with random variation)
    int calculateActualProfit(float marketMultiplier) const;

    // Process a day passing
    bool advanceDay(float marketMultiplier, int& profit);
};

// Node for managing business investments
class InvestmentNode : public TANode {
public:
    std::vector<BusinessInvestment> investments;
    EconomicSystemNode* economicSystem;

    InvestmentNode(const std::string& name, EconomicSystemNode* econSystem);

    void loadInvestmentsFromJson();
    void onEnter(GameContext* context) override;
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;

    // Add a new investment opportunity
    void addInvestmentOpportunity(const BusinessInvestment& investment);

    // Process a day passing for all investments
    void advanceDay(GameContext* context);

private:
    void displayInvestments();
    void displayOpportunities();
    void handleInvesting();
    void collectProfits();
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

// Property type conversion functions
PropertyType stringToPropertyType(const std::string& typeStr);
const std::string propertyTypeToString(PropertyType type);

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

#endif // SYSTEM_ECONOMY_MARKET_JSON_HPP