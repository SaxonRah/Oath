#include "MountStable.hpp"
#include "Mount.hpp"
#include "MountBreed.hpp"
#include "MountSystemConfig.hpp"
#include <algorithm>
#include <iostream>
#include <nlohmann/json.hpp>


MountStable::MountStable(const std::string& stableId, const std::string& stableName, const std::string& stableLocation, int stableCapacity)
    : id(stableId)
    , name(stableName)
    , location(stableLocation)
    , capacity(stableCapacity)
    , dailyFeedCost(5)
    , dailyCareCost(3)
{
}

MountStable* MountStable::createFromJson(const nlohmann::json& j, std::map<std::string, MountBreed*>& breedTypes, MountSystemConfig& config)
{
    std::string id = j["id"];
    std::string name = j["name"];
    std::string location = j["location"];
    int capacity = j.value("capacity", 5);

    MountStable* stable = new MountStable(id, name, location, capacity);
    stable->dailyFeedCost = j.value("dailyFeedCost", 5);
    stable->dailyCareCost = j.value("dailyCareCost", 3);

    // Create mounts available for purchase
    if (j.contains("availableMounts") && j["availableMounts"].is_array()) {
        for (const auto& mountTemplate : j["availableMounts"]) {
            std::string breedId = mountTemplate["templateId"];

            // Find the breed
            if (breedTypes.find(breedId) != breedTypes.end()) {
                MountBreed* breed = breedTypes[breedId];
                Mount* mount = Mount::createFromTemplate(mountTemplate, breed, config);

                if (mount) {
                    stable->availableForPurchase.push_back(mount);
                }
            }
        }
    }

    return stable;
}

bool MountStable::hasSpace() const
{
    return stabledMounts.size() < capacity;
}

bool MountStable::stableMount(Mount* mount)
{
    if (!mount || !hasSpace()) {
        return false;
    }

    // Update mount state
    mount->isStabled = true;
    mount->isSummoned = false;
    mount->isMounted = false;

    // Add to stabled mounts
    stabledMounts.push_back(mount);
    return true;
}

Mount* MountStable::retrieveMount(const std::string& mountId)
{
    auto it = std::find_if(stabledMounts.begin(), stabledMounts.end(),
        [&mountId](Mount* m) { return m->id == mountId; });

    if (it != stabledMounts.end()) {
        Mount* mount = *it;
        stabledMounts.erase(it);
        mount->isStabled = false;
        return mount;
    }

    return nullptr;
}

int MountStable::getDailyCost() const
{
    return stabledMounts.size() * (dailyFeedCost + dailyCareCost);
}

void MountStable::provideDailyCare()
{
    for (Mount* mount : stabledMounts) {
        if (mount) {
            // Feed and care for mount
            mount->stats.feed(50); // Good daily feeding
            mount->stats.rest(480); // 8 hours of good rest

            // Slight health recovery from stable care
            mount->stats.health += 5;
            if (mount->stats.health > mount->stats.maxHealth) {
                mount->stats.health = mount->stats.maxHealth;
            }

            // Repair equipment slightly
            for (auto& [slot, equipment] : mount->equippedItems) {
                if (equipment) {
                    equipment->repair(1);
                }
            }
        }
    }
}