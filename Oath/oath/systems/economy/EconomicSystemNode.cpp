// systems/economy/EconomicSystemNode.cpp

#include "EconomicSystemNode.hpp"
#include "MarketNode.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>

EconomicSystemNode::EconomicSystemNode(const std::string& name)
    : TANode(name)
    , daysSinceLastEvent(0)
    , globalEconomicMultiplier(1.0f)
{
    // Load configuration from JSON file
    loadConfigFromJson();
}

EconomicSystemNode::~EconomicSystemNode()
{
    // Clean up owned markets
    for (auto market : markets) {
        delete market;
    }
}

void EconomicSystemNode::loadConfigFromJson()
{
    try {
        // Open and parse the JSON file
        std::ifstream file("resources/json/economy.json");
        if (!file.is_open()) {
            std::cerr << "Failed to open economy.json" << std::endl;
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

void EconomicSystemNode::onEnter(GameContext* context)
{
    std::cout << "Economic System Control Panel" << std::endl;
    std::cout << "-----------------------------" << std::endl;
    std::cout << "Markets: " << markets.size() << std::endl;
    std::cout << "Trade Routes: " << tradeRoutes.size() << std::endl;
    std::cout << "Active Economic Events: " << activeEvents.size() << std::endl;
    std::cout << "Global Economy Multiplier: " << globalEconomicMultiplier << std::endl;
}

std::vector<TAAction> EconomicSystemNode::getAvailableActions()
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

bool EconomicSystemNode::evaluateTransition(const TAInput& input, TANode*& outNextNode)
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

Market* EconomicSystemNode::createMarket(const std::string& id, const std::string& name, MarketType type)
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

void EconomicSystemNode::addTradeRoute(const std::string& id, const std::string& name,
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

void EconomicSystemNode::addPotentialEvent(const EconomicEvent& event)
{
    potentialEvents.push_back(event);
}

void EconomicSystemNode::processTradeRoutes()
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

// systems/economy/EconomicSystemNode.cpp (continued)

void EconomicSystemNode::processEconomicEvents()
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

void EconomicSystemNode::simulateEconomicDay()
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

void EconomicSystemNode::displayMarkets()
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

void EconomicSystemNode::displayTradeRoutes()
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

void EconomicSystemNode::displayEconomicEvents()
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

Market* EconomicSystemNode::findMarketById(const std::string& marketId)
{
    for (auto* market : markets) {
        if (market->id == marketId) {
            return market;
        }
    }
    return nullptr;
}
