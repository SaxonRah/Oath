#include "MountEquipment.hpp"
#include "MountStats.hpp"
#include <iostream>
#include <nlohmann/json.hpp>


MountEquipmentSlot stringToSlot(const std::string& slotStr)
{
    if (slotStr == "Saddle")
        return MountEquipmentSlot::Saddle;
    if (slotStr == "Armor")
        return MountEquipmentSlot::Armor;
    if (slotStr == "Bags")
        return MountEquipmentSlot::Bags;
    if (slotStr == "Shoes")
        return MountEquipmentSlot::Shoes;
    if (slotStr == "Accessory")
        return MountEquipmentSlot::Accessory;

    // Default
    std::cerr << "Unknown equipment slot: " << slotStr << std::endl;
    return MountEquipmentSlot::Saddle;
}

std::string slotToString(MountEquipmentSlot slot)
{
    switch (slot) {
    case MountEquipmentSlot::Saddle:
        return "Saddle";
    case MountEquipmentSlot::Armor:
        return "Armor";
    case MountEquipmentSlot::Bags:
        return "Bags";
    case MountEquipmentSlot::Shoes:
        return "Shoes";
    case MountEquipmentSlot::Accessory:
        return "Accessory";
    default:
        return "Unknown";
    }
}

MountEquipment::MountEquipment(const std::string& itemId, const std::string& itemName, MountEquipmentSlot itemSlot)
    : id(itemId)
    , name(itemName)
    , slot(itemSlot)
    , quality(50)
    , durability(100)
    , maxDurability(100)
    , price(100)
{
}

MountEquipment* MountEquipment::createFromJson(const nlohmann::json& j)
{
    std::string id = j["id"];
    std::string name = j["name"];
    MountEquipmentSlot slot = stringToSlot(j["slot"]);

    MountEquipment* equipment = new MountEquipment(id, name, slot);
    equipment->description = j.value("description", "");
    equipment->quality = j.value("quality", 50);
    equipment->durability = j.value("durability", 100);
    equipment->maxDurability = j.value("maxDurability", 100);
    equipment->price = j.value("price", 100);

    // Load stat modifiers
    if (j.contains("statModifiers") && j["statModifiers"].is_object()) {
        for (auto& [stat, modifier] : j["statModifiers"].items()) {
            equipment->statModifiers[stat] = modifier;
        }
    }

    return equipment;
}

bool MountEquipment::isWorn() const
{
    return durability < maxDurability / 5;
}

void MountEquipment::applyModifiers(MountStats& stats) const
{
    for (const auto& [stat, modifier] : statModifiers) {
        if (stat == "speed")
            stats.speed += modifier;
        else if (stat == "stamina")
            stats.maxStamina += modifier;
        else if (stat == "carryCapacity")
            stats.carryCapacity += modifier;
        else if (stat == "staminaRegen")
            stats.staminaRegen += modifier;
    }
}

void MountEquipment::use(int intensity)
{
    durability -= intensity;
    if (durability < 0)
        durability = 0;
}

void MountEquipment::repair(int amount)
{
    durability += amount;
    if (durability > maxDurability)
        durability = maxDurability;
}