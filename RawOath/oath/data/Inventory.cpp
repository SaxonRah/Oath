#include <Inventory.hpp>

bool Inventory::hasItem(const std::string& itemId, int quantity) const
{
    for (const auto& item : items) {
        if (item.id == itemId && item.quantity >= quantity) {
            return true;
        }
    }
    return false;
}

bool Inventory::addItem(const Item& item)
{
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

bool Inventory::removeItem(const std::string& itemId, int quantity)
{
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