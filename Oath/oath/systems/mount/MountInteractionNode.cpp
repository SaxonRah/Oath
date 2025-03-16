#include "MountInteractionNode.hpp"
#include "../../core/TAAction.hpp"
#include "../../core/TAInput.hpp"
#include "../../data/GameContext.hpp"
#include "Mount.hpp"
#include "MountEquipment.hpp"
#include "MountStats.hpp"
#include "MountSystemConfig.hpp"
#include <iostream>


MountInteractionNode::MountInteractionNode(const std::string& name, Mount* mount, MountSystemConfig* cfg)
    : TANode(name)
    , activeMount(mount)
    , config(cfg)
{
}

void MountInteractionNode::setActiveMount(Mount* mount)
{
    activeMount = mount;
}

void MountInteractionNode::setConfig(MountSystemConfig* cfg)
{
    config = cfg;
}

void MountInteractionNode::onEnter(GameContext* context)
{
    if (!activeMount) {
        std::cout << "You don't currently have an active mount." << std::endl;
        return;
    }

    std::cout << "Interacting with " << activeMount->name << std::endl;
    std::cout << activeMount->getStateDescription() << std::endl;

    // Show mount stats
    MountStats effectiveStats = activeMount->getEffectiveStats();
    std::cout << "\nStats:" << std::endl;
    std::cout << "Health: " << effectiveStats.health << "/" << effectiveStats.maxHealth << std::endl;
    std::cout << "Stamina: " << effectiveStats.stamina << "/" << effectiveStats.maxStamina << std::endl;
    std::cout << "Speed: " << effectiveStats.speed << " (" << effectiveStats.getEffectiveSpeed() << " effective)" << std::endl;
    std::cout << "Hunger: " << effectiveStats.hunger << "/100" << std::endl;
    std::cout << "Fatigue: " << effectiveStats.fatigue << "/100" << std::endl;
    std::cout << "Carry Capacity: " << effectiveStats.carryCapacity << " additional units" << std::endl;

    // Show training levels
    std::cout << "\nTraining:" << std::endl;
    for (const auto& [skill, level] : effectiveStats.specialTraining) {
        std::cout << skill << ": " << level << "/100" << std::endl;
    }

    // Show special abilities
    std::cout << "\nSpecial Abilities:" << std::endl;
    if (effectiveStats.canJump)
        std::cout << "- Can jump over obstacles" << std::endl;
    if (effectiveStats.canSwim)
        std::cout << "- Can swim across water" << std::endl;
    if (effectiveStats.canClimb)
        std::cout << "- Can climb steep slopes" << std::endl;

    // Show equipped items
    std::cout << "\nEquipment:" << std::endl;
    if (activeMount->equippedItems.empty()) {
        std::cout << "No equipment" << std::endl;
    } else {
        for (const auto& [slot, equipment] : activeMount->equippedItems) {
            if (equipment) {
                std::cout << slotToString(slot) << ": " << equipment->name;
                if (equipment->isWorn())
                    std::cout << " (Worn)";
                std::cout << " - " << equipment->durability << "/" << equipment->maxDurability << std::endl;
            }
        }
    }
}

std::vector<TAAction> MountInteractionNode::getAvailableActions()
{
    std::vector<TAAction> actions = TANode::getAvailableActions();

    if (!activeMount) {
        actions.push_back({ "summon_mount", "Summon one of your mounts",
            [this]() -> TAInput {
                return { "mount_action", { { "action", std::string("summon") } } };
            } });
        return actions;
    }

    // Mount/dismount
    if (activeMount->isMounted) {
        actions.push_back({ "dismount", "Dismount",
            [this]() -> TAInput {
                return { "mount_action", { { "action", std::string("dismount") } } };
            } });
    } else {
        actions.push_back({ "mount", "Mount",
            [this]() -> TAInput {
                return { "mount_action", { { "action", std::string("mount") } } };
            } });
    }

    // Feed mount
    actions.push_back({ "feed_mount", "Feed your mount",
        [this]() -> TAInput {
            return { "mount_action", { { "action", std::string("feed") } } };
        } });

    // Rest mount
    actions.push_back({ "rest_mount", "Allow your mount to rest",
        [this]() -> TAInput {
            return { "mount_action", { { "action", std::string("rest") } } };
        } });

    // Groom mount (improves relationship)
    actions.push_back({ "groom_mount", "Groom your mount",
        [this]() -> TAInput {
            return { "mount_action", { { "action", std::string("groom") } } };
        } });

    // Manage equipment
    actions.push_back({ "manage_equipment", "Manage mount equipment",
        [this]() -> TAInput {
            return { "mount_action", { { "action", std::string("equipment") } } };
        } });

    // Special abilities (if mounted)
    if (activeMount->isMounted && config) {
        MountStats effectiveStats = activeMount->getEffectiveStats();

        // Add actions for abilities the mount can use
        for (const auto& [abilityId, abilityInfo] : config->specialAbilities) {
            bool canUse = config->canUseAbility(effectiveStats, abilityId);

            if (canUse) {
                actions.push_back({ abilityId, abilityInfo.name,
                    [this, abilityId]() -> TAInput {
                        return { "mount_action", { { "action", std::string("ability") }, { "ability", abilityId } } };
                    } });
            }
        }
    }

    // Dismiss mount (send back to stable or camp)
    if (activeMount->isSummoned) {
        actions.push_back({ "dismiss_mount", "Dismiss your mount",
            [this]() -> TAInput {
                return { "mount_action", { { "action", std::string("dismiss") } } };
            } });
    }

    // Exit interaction
    actions.push_back({ "exit_interaction", "Stop interacting with mount",
        [this]() -> TAInput {
            return { "mount_action", { { "action", std::string("exit") } } };
        } });

    return actions;
}

bool MountInteractionNode::evaluateTransition(const TAInput& input, TANode*& outNextNode)
{
    if (input.type == "mount_action") {
        std::string action = std::get<std::string>(input.parameters.at("action"));

        // Handle different mount actions
        if (action == "exit") {
            // Find the exit transition
            for (const auto& rule : transitionRules) {
                if (rule.description == "Exit") {
                    outNextNode = rule.targetNode;
                    return true;
                }
            }
        } else if (action == "ability" && activeMount && config) {
            // Handle special abilities
            std::string ability = std::get<std::string>(input.parameters.at("ability"));

            bool success = activeMount->useSpecialAbility(ability, *config);

            if (success) {
                std::cout << activeMount->name << " successfully performs "
                          << config->specialAbilities[ability].name << "!" << std::endl;
            } else {
                std::cout << activeMount->name << " is unable to perform "
                          << config->specialAbilities[ability].name << " right now." << std::endl;
            }

            outNextNode = this;
            return true;
        }

        // Most actions would be processed in the game logic
        // and would likely stay in the same node
        outNextNode = this;
        return true;
    }

    return TANode::evaluateTransition(input, outNextNode);
}