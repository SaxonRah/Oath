// systems/economy/MarketTypes.cpp

#include "MarketTypes.hpp"

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

    // Default to General Market
    return MarketType::GENERAL;
}

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