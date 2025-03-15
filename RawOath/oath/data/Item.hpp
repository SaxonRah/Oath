#pragma once

#include <map>
#include <string>
#include <variant>

// Item system for crafting
struct Item {
    std::string id;
    std::string name;
    std::string type;
    int value;
    int quantity;
    std::map<std::string, std::variant<int, float, std::string, bool>> properties;

    Item(const std::string& itemId, const std::string& itemName,
        const std::string& itemType, int itemValue = 1, int itemQty = 1);
};