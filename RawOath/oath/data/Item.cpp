#include "Item.hpp"

Item::Item(const std::string& itemId, const std::string& itemName,
    const std::string& itemType, int itemValue, int itemQty)
    : id(itemId)
    , name(itemName)
    , type(itemType)
    , value(itemValue)
    , quantity(itemQty)
{
}