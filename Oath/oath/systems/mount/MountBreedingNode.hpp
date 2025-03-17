#pragma once

#include "../../core/TANode.hpp"
#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

class Mount;
struct MountBreed;
struct MountSystemConfig;
struct GameContext;
struct TAInput;
struct TAAction;

// Mount breeding center node
class MountBreedingNode : public TANode {
public:
    std::string centerName;
    std::vector<Mount*> availableForBreeding;
    int breedingFee;
    MountSystemConfig* config;

    MountBreedingNode(const std::string& name, const std::string& center, int fee = 200, MountSystemConfig* cfg = nullptr);
    void setConfig(MountSystemConfig* cfg);
    static MountBreedingNode* createFromJson(const std::string& name, const nlohmann::json& j,
        std::map<std::string, MountBreed*>& breedTypes, MountSystemConfig& config);
    void onEnter(GameContext* context) override;
    Mount* breedMounts(Mount* playerMount, Mount* centerMount, const std::string& foalName);
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;
};