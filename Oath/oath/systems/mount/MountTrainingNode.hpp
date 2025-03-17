#pragma once

#include "../../core/TANode.hpp"
#include <string>
#include <vector>

class Mount;
struct MountSystemConfig;
struct GameContext;
struct TAInput;
struct TAAction;

// Mount training node - for dedicated training sessions
class MountTrainingNode : public TANode {
public:
    Mount* trainingMount;
    std::vector<std::string> trainingTypes;
    MountSystemConfig* config;

    MountTrainingNode(const std::string& name, Mount* mount = nullptr, MountSystemConfig* cfg = nullptr);
    void setTrainingMount(Mount* mount);
    void setConfig(MountSystemConfig* cfg);
    void onEnter(GameContext* context) override;
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;
};