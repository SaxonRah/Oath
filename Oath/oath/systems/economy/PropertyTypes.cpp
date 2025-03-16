// systems/economy/PropertyTypes.cpp

#include "PropertyTypes.hpp"

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