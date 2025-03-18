// Inventory.hpp
#pragma once

#include <Include/nlohmann/json.hpp>
#include <map>
#include <string>
#include <variant>
#include <vector>

class Item {
public:
    std::string id;
    std::string name;
    std::string type;
    int value;
    int quantity;
    std::map<std::string, std::variant<int, float, std::string, bool>> properties;

    Item(const std::string& itemId = "", const std::string& itemName = "",
        const std::string& itemType = "", int itemValue = 0, int itemQty = 1)
        : id(itemId)
        , name(itemName)
        , type(itemType)
        , value(itemValue)
        , quantity(itemQty)
    {
    }

    // Serialize to JSON
    nlohmann::json toJson() const
    {
        nlohmann::json j;
        j["id"] = id;
        j["name"] = name;
        j["type"] = type;
        j["value"] = value;
        j["quantity"] = quantity;

        // Serialize properties map
        nlohmann::json props = nlohmann::json::object();
        for (const auto& [key, val] : properties) {
            if (std::holds_alternative<int>(val)) {
                props[key] = std::get<int>(val);
            } else if (std::holds_alternative<float>(val)) {
                props[key] = std::get<float>(val);
            } else if (std::holds_alternative<std::string>(val)) {
                props[key] = std::get<std::string>(val);
            } else if (std::holds_alternative<bool>(val)) {
                props[key] = std::get<bool>(val);
            }
        }
        j["properties"] = props;

        return j;
    }

    // Deserialize from JSON
    static Item fromJson(const nlohmann::json& j)
    {
        Item item(
            j["id"],
            j["name"],
            j["type"],
            j["value"],
            j["quantity"]);

        // Load properties if present
        if (j.contains("properties") && j["properties"].is_object()) {
            for (auto& [key, value] : j["properties"].items()) {
                if (value.is_number_integer()) {
                    item.properties[key] = value.get<int>();
                } else if (value.is_number_float()) {
                    item.properties[key] = value.get<float>();
                } else if (value.is_string()) {
                    item.properties[key] = value.get<std::string>();
                } else if (value.is_boolean()) {
                    item.properties[key] = value.get<bool>();
                }
            }
        }

        return item;
    }
};

class Inventory {
public:
    std::vector<Item> items;
    int maxWeight = 100;
    int currentWeight = 0;

    Inventory() = default;

    // Check if inventory has an item
    bool hasItem(const std::string& itemId, int quantity = 1) const
    {
        for (const auto& item : items) {
            if (item.id == itemId && item.quantity >= quantity) {
                return true;
            }
        }
        return false;
    }

    // Add an item to inventory
    bool addItem(const Item& item)
    {
        if (item.quantity <= 0)
            return false;

        // Check if item already exists
        for (auto& existingItem : items) {
            if (existingItem.id == item.id) {
                existingItem.quantity += item.quantity;
                return true;
            }
        }

        // Add new item
        items.push_back(item);
        return true;
    }

    // Remove an item from inventory
    bool removeItem(const std::string& itemId, int quantity = 1)
    {
        if (quantity <= 0)
            return false;

        for (auto it = items.begin(); it != items.end(); ++it) {
            if (it->id == itemId) {
                if (it->quantity > quantity) {
                    it->quantity -= quantity;
                    return true;
                } else if (it->quantity == quantity) {
                    items.erase(it);
                    return true;
                } else {
                    return false; // Not enough quantity
                }
            }
        }
        return false; // Item not found
    }

    // Find an item by id
    Item* findItem(const std::string& itemId)
    {
        for (auto& item : items) {
            if (item.id == itemId) {
                return &item;
            }
        }
        return nullptr;
    }

    // Get total value of all items
    int getTotalValue() const
    {
        int total = 0;
        for (const auto& item : items) {
            total += item.value * item.quantity;
        }
        return total;
    }

    // Count total items (counting each stack as one)
    int getUniqueItemCount() const
    {
        return items.size();
    }

    // Count total individual items
    int getTotalItemCount() const
    {
        int count = 0;
        for (const auto& item : items) {
            count += item.quantity;
        }
        return count;
    }

    // Serialize to JSON
    nlohmann::json toJson() const
    {
        nlohmann::json j;
        j["maxWeight"] = maxWeight;
        j["currentWeight"] = currentWeight;

        nlohmann::json itemsJson = nlohmann::json::array();
        for (const auto& item : items) {
            itemsJson.push_back(item.toJson());
        }
        j["items"] = itemsJson;

        return j;
    }

    // Deserialize from JSON
    static Inventory fromJson(const nlohmann::json& j)
    {
        Inventory inventory;

        if (j.contains("maxWeight")) {
            inventory.maxWeight = j["maxWeight"];
        }

        if (j.contains("currentWeight")) {
            inventory.currentWeight = j["currentWeight"];
        }

        if (j.contains("items") && j["items"].is_array()) {
            for (const auto& itemJson : j["items"]) {
                inventory.items.push_back(Item::fromJson(itemJson));
            }
        }

        return inventory;
    }
};