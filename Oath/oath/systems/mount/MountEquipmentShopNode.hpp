#pragma once

#include "../../core/TANode.hpp"
#include <string>
#include <vector>

struct MountEquipment;
struct GameContext;
struct TAInput;
struct TAAction;

// Mount equipment shop node
class MountEquipmentShopNode : public TANode {
public:
    std::string shopName;
    std::vector<MountEquipment*> availableEquipment;

    MountEquipmentShopNode(const std::string& name, const std::string& shop);
    void onEnter(GameContext* context) override;
    std::vector<TAAction> getAvailableActions() override;
    bool evaluateTransition(const TAInput& input, TANode*& outNextNode) override;
};