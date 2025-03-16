#pragma once

#include <map>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

class Mount;
struct MountBreed;
struct MountSystemConfig;

// Mount stable location for managing multiple mounts
class MountStable {
public:
    std::string id;
    std::string name;
    std::string location;
    int capacity;
    int dailyFeedCost;
    int dailyCareCost;

    std::vector<Mount*> stabledMounts;
    std::vector<Mount*> availableForPurchase;

    MountStable(const std::string& stableId, const std::string& stableName, const std::string& stableLocation, int stableCapacity = 5);
    static MountStable* createFromJson(const nlohmann::json& j, std::map<std::string, MountBreed*>& breedTypes, MountSystemConfig& config);
    bool hasSpace() const;
    bool stableMount(Mount* mount);
    Mount* retrieveMount(const std::string& mountId);
    int getDailyCost() const;
    void provideDailyCare();
};