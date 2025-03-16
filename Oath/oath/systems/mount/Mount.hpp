#pragma once

#include <map>
#include <string>

enum class MountEquipmentSlot;
struct MountStats;
struct MountBreed;
struct MountEquipment;
struct MountSystemConfig;

// Complete mount class including stats, equipment, and state
class Mount {
public:
    std::string id;
    std::string name;
    MountBreed* breed;
    MountStats stats;
    int age;
    std::string color;

    // Current state
    bool isOwned;
    bool isStabled;
    bool isSummoned;
    bool isMounted;

    // Equipped items
    std::map<MountEquipmentSlot, MountEquipment*> equippedItems;

    Mount(const std::string& mountId, const std::string& mountName, MountBreed* mountBreed);
    static Mount* createFromTemplate(const nlohmann::json& templateJson, MountBreed* breed, MountSystemConfig& config);
    MountStats getEffectiveStats() const;
    bool equipItem(MountEquipment* equipment);
    MountEquipment* unequipItem(MountEquipmentSlot slot);
    void update(int minutes);
    float getTravelTimeModifier() const;
    bool useSpecialAbility(const std::string& ability, const MountSystemConfig& config);
    std::string getStateDescription() const;
};