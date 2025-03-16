// systems/economy/MarketTypes.hpp
#pragma once

#include <string>

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
MarketType stringToMarketType(const std::string& typeStr);

// String conversion for MarketType
const std::string marketTypeToString(MarketType type);
