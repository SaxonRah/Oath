#pragma once

#include "../../core/TANode.hpp"
#include <string>

class Mount;
struct MountSystemConfig;
class GameContext;
struct TAInput;
struct TAAction;

// Mount interaction node - for when actively using a mount
class MountInteractionNode : public TANode {
public:
    Mount* activeMount;
    MountSystemConfig* config;

    MountInteractionNode(const std::string& name, Mount* mount = nullptr, MountSystemConfig* cfg = nullptr);
    void setActiveMount(Mount* mount);
    void setConfig(MountSystemConfig* cfg);
    void onEnter(GameContext* context) override;
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;
};