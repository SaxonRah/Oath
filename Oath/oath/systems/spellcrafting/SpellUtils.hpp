#pragma once

#include "SpellEnums.hpp"
#include <string>


// Helper functions to convert between string IDs and enum values
SpellEffectType stringToEffectType(const std::string& typeStr);
SpellDeliveryMethod stringToDeliveryMethod(const std::string& methodStr);
SpellTargetType stringToTargetType(const std::string& targetStr);
std::string effectTypeToString(SpellEffectType type);
std::string deliveryMethodToString(SpellDeliveryMethod method);
std::string targetTypeToString(SpellTargetType type);