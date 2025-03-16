#pragma once

#include <map>
#include <string>

#include <nlohmann/json.hpp>

struct MountStats;

// Mount equipment slot types
enum class MountEquipmentSlot {
    Saddle,
    Armor,
    Bags,
    Shoes,
    Accessory
};

// Helper function to convert string to MountEquipmentSlot
MountEquipmentSlot stringToSlot(const std::string& slotStr);

// Helper function to convert MountEquipmentSlot to string
std::string slotToString(MountEquipmentSlot slot);

// Mount equipment item
struct MountEquipment {
    std::string id;
    std::string name;
    std::string description;
    MountEquipmentSlot slot;
    int quality;
    int durability;
    int maxDurability;
    int price;
    std::map<std::string, int> statModifiers;

    MountEquipment(const std::string& itemId, const std::string& itemName, MountEquipmentSlot itemSlot);
    static MountEquipment* createFromJson(const nlohmann::json& j);
    bool isWorn() const;
    void applyModifiers(MountStats& stats) const;
    void use(int intensity = 1);
    void repair(int amount);
};