#pragma once

#include "../../core/TANode.hpp"
#include <map>
#include <string>
#include <vector>


class Mount;
class MountStable;
struct MountBreed;
struct MountEquipment;
struct MountSystemConfig;
class GameContext;
struct TAInput;
struct TAAction;

// Mount system main controller
class MountSystemController : public TANode {
public:
    // Player's owned mounts
    std::vector<Mount*> ownedMounts;

    // Currently active mount
    Mount* activeMount;

    // Registered stables
    std::vector<MountStable*> stables;

    // Registered breed types
    std::map<std::string, MountBreed*> breedTypes;

    // Available mount equipment
    std::vector<MountEquipment*> knownEquipment;

    // Global configuration
    MountSystemConfig config;

    // Path to the config file
    std::string configPath;

    MountSystemController(const std::string& name, const std::string& jsonPath = "resources/json/mount.json");
    void loadConfig();
    void initializeBasicDefaults();
    void registerStable(MountStable* stable);
    Mount* createMount(const std::string& name, const std::string& breedId);
    bool addMount(Mount* mount);
    bool removeMount(const std::string& mountId);
    Mount* findMount(const std::string& mountId);
    void setActiveMount(Mount* mount);
    bool mountActive();
    void dismountActive();
    MountStable* findNearestStable(const std::string& playerLocation);
    void onEnter(GameContext* context) override;
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;
    void updateMounts(int minutes);
    void applyTravelEffects(float distance);
    float getMountedSpeedModifier();
    bool canPerformSpecialMovement(const std::string& movementType);
};