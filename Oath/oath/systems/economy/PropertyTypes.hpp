// systems/economy/PropertyTypes.hpp
#pragma once

#include <string>

// Property types
enum class PropertyType {
    HOUSE,
    SHOP,
    FARM,
    WAREHOUSE,
    ESTATE
};

// Convert string to PropertyType
PropertyType stringToPropertyType(const std::string& typeStr);

// String conversion for PropertyType
const std::string propertyTypeToString(PropertyType type);
