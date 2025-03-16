#include "MountSystemController.hpp"
#include "../../core/TAAction.hpp"
#include "../../core/TAInput.hpp"
#include "../../data/GameContext.hpp"
#include "Mount.hpp"
#include "MountBreed.hpp"
#include "MountEquipment.hpp"
#include "MountStable.hpp"
#include "MountSystemConfig.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

MountSystemController::MountSystemController(const std::string& name, const std::string& jsonPath)
    : TANode(name)
    , activeMount(nullptr)
    , configPath(jsonPath)
{
    // Load configuration from JSON
    loadConfig();
}

void MountSystemController::loadConfig()
{
    try {
        // Check if file exists
        if (!std::filesystem::exists(configPath)) {
            std::cerr << "Config file not found: " << configPath << std::endl;
            // Initialize with basic defaults
            initializeBasicDefaults();
            return;
        }

        // Open and parse JSON file
        std::ifstream file(configPath);
        nlohmann::json mountConfig;
        file >> mountConfig;

        // Load breed types
        if (mountConfig.contains("breeds") && mountConfig["breeds"].is_object()) {
            for (auto& [id, breedJson] : mountConfig["breeds"].items()) {
                MountBreed* breed = MountBreed::createFromJson(breedJson);
                breedTypes[id] = breed;
            }
        }

        // Load equipment
        if (mountConfig.contains("equipment") && mountConfig["equipment"].is_object()) {
            for (auto& [id, equipJson] : mountConfig["equipment"].items()) {
                MountEquipment* equipment = MountEquipment::createFromJson(equipJson);
                knownEquipment.push_back(equipment);
            }
        }

        // Load stables
        if (mountConfig.contains("stables") && mountConfig["stables"].is_object()) {
            for (auto& [id, stableJson] : mountConfig["stables"].items()) {
                MountStable* stable = MountStable::createFromJson(stableJson, breedTypes, config);
                stables.push_back(stable);
            }
        }

        // Load special abilities
        if (mountConfig.contains("specialAbilities") && mountConfig["specialAbilities"].is_object()) {
            for (auto& [id, abilityJson] : mountConfig["specialAbilities"].items()) {
                SpecialAbilityInfo ability = SpecialAbilityInfo::fromJson(abilityJson);
                config.specialAbilities[id] = ability;
            }
        }

        // Load training types
        if (mountConfig.contains("trainingTypes") && mountConfig["trainingTypes"].is_array()) {
            for (const auto& trainingJson : mountConfig["trainingTypes"]) {
                std::string id = trainingJson["id"];
                std::string description = trainingJson["description"];
                config.trainingTypes.push_back({ id, description });
            }
        }

        // Load mount colors
        if (mountConfig.contains("colors") && mountConfig["colors"].is_array()) {
            for (const auto& color : mountConfig["colors"]) {
                config.colors.push_back(color);
            }
        }

        std::cout << "Mount system configuration loaded successfully from " << configPath << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error loading mount configuration: " << e.what() << std::endl;
        // Initialize with basic defaults
        initializeBasicDefaults();
    }
}

void MountSystemController::initializeBasicDefaults()
{
    std::cout << "Initializing mount system with default values" << std::endl;

    // Create default breeds
    MountBreed* standardHorse = new MountBreed("standard_horse", "Standard Horse");
    standardHorse->baseSpeed = 100;
    standardHorse->baseStamina = 100;
    standardHorse->baseCarryCapacity = 50;
    standardHorse->baseTrainability = 50;
    breedTypes["standard_horse"] = standardHorse;

    // Set default training types
    config.trainingTypes = {
        { "combat", "Fighting from mountback and defensive maneuvers" },
        { "endurance", "Long-distance travel and stamina management" },
        { "agility", "Jumping, balance, and difficult terrain navigation" },
        { "racing", "Burst speed and racing techniques" }
    };

    // Default colors
    config.colors = { "Bay", "Chestnut", "Black", "Gray", "White" };

    // Default special abilities
    SpecialAbilityInfo jump;
    jump.id = "jump";
    jump.name = "Jump";
    jump.description = "Jump over obstacles";
    jump.staminaCost = 25;
    jump.skillRequired = 30;
    jump.trainingType = "agility";
    jump.unlockThreshold = 50;
    config.specialAbilities["jump"] = jump;

    SpecialAbilityInfo swim;
    swim.id = "swim";
    swim.name = "Swim";
    swim.description = "Swim across water";
    swim.staminaCost = 15;
    swim.skillRequired = 30;
    swim.trainingType = "endurance";
    swim.unlockThreshold = 60;
    config.specialAbilities["swim"] = swim;
}

void MountSystemController::registerStable(MountStable* stable)
{
    if (stable) {
        stables.push_back(stable);
    }
}

Mount* MountSystemController::createMount(const std::string& name, const std::string& breedId)
{
    if (breedTypes.find(breedId) == breedTypes.end()) {
        return nullptr;
    }

    std::string mountId = breedId + "_" + name + "_" + std::to_string(ownedMounts.size() + 1);
    Mount* newMount = new Mount(mountId, name, breedTypes[breedId]);

    // Set a random color from the config
    newMount->color = config.getRandomColor();

    return newMount;
}

bool MountSystemController::addMount(Mount* mount)
{
    if (!mount)
        return false;

    mount->isOwned = true;
    ownedMounts.push_back(mount);
    return true;
}

bool MountSystemController::removeMount(const std::string& mountId)
{
    auto it = std::find_if(ownedMounts.begin(), ownedMounts.end(),
        [&mountId](Mount* m) { return m->id == mountId; });

    if (it != ownedMounts.end()) {
        if (activeMount == *it) {
            activeMount = nullptr;
        }

        Mount* mount = *it;
        ownedMounts.erase(it);
        delete mount; // Free the mount memory
        return true;
    }

    return false;
}

Mount* MountSystemController::findMount(const std::string& mountId)
{
    auto it = std::find_if(ownedMounts.begin(), ownedMounts.end(),
        [&mountId](Mount* m) { return m->id == mountId; });

    if (it != ownedMounts.end()) {
        return *it;
    }

    return nullptr;
}

void MountSystemController::setActiveMount(Mount* mount)
{
    // Dismount the current mount if any
    if (activeMount && activeMount->isMounted) {
        activeMount->isMounted = false;
    }

    // Set the new active mount
    activeMount = mount;

    if (activeMount) {
        // Summon the mount if it's not already
        activeMount->isSummoned = true;
        activeMount->isStabled = false;

        std::cout << activeMount->name << " is now your active mount." << std::endl;
    }
}

bool MountSystemController::mountActive()
{
    if (!activeMount) {
        std::cout << "You don't have an active mount." << std::endl;
        return false;
    }

    activeMount->isMounted = true;
    std::cout << "You mount " << activeMount->name << "." << std::endl;
    return true;
}

void MountSystemController::dismountActive()
{
    if (activeMount && activeMount->isMounted) {
        activeMount->isMounted = false;
        std::cout << "You dismount " << activeMount->name << "." << std::endl;
    }
}

MountStable* MountSystemController::findNearestStable(const std::string& playerLocation)
{
    for (MountStable* stable : stables) {
        if (stable->location == playerLocation) {
            return stable;
        }
    }

    // If no exact match, return first stable (would use distance in real implementation)
    return stables.empty() ? nullptr : stables[0];
}

void MountSystemController::onEnter(GameContext* context)
{
    std::cout << "=== Mount Management System ===" << std::endl;

    if (ownedMounts.empty()) {
        std::cout << "You don't own any mounts." << std::endl;
    } else {
        std::cout << "Your mounts:" << std::endl;
        for (size_t i = 0; i < ownedMounts.size(); i++) {
            Mount* mount = ownedMounts[i];
            std::cout << i + 1 << ". " << mount->getStateDescription();

            if (mount == activeMount) {
                std::cout << " (Active)";
            }

            std::cout << std::endl;
        }
    }

    std::cout << "\nNearby stables:" << std::endl;
    if (stables.empty()) {
        std::cout << "No stables in this region." << std::endl;
    } else {
        for (MountStable* stable : stables) {
            std::cout << "- " << stable->name << " in " << stable->location << std::endl;
        }
    }
}

std::vector<TAAction> MountSystemController::getAvailableActions()
{
    std::vector<TAAction> actions = TANode::getAvailableActions();

    // Actions for owned mounts
    if (!ownedMounts.empty()) {
        actions.push_back({ "view_mounts", "View your mounts",
            [this]() -> TAInput {
                return { "mount_system", { { "action", std::string("view") } } };
            } });

        actions.push_back({ "select_mount", "Select active mount",
            [this]() -> TAInput {
                return { "mount_system", { { "action", std::string("select") } } };
            } });
    }

    // Mount/dismount if there's an active mount
    if (activeMount) {
        if (activeMount->isMounted) {
            actions.push_back({ "dismount", "Dismount",
                [this]() -> TAInput {
                    return { "mount_system", { { "action", std::string("dismount") } } };
                } });
        } else {
            actions.push_back({ "mount", "Mount",
                [this]() -> TAInput {
                    return { "mount_system", { { "action", std::string("mount") } } };
                } });
        }

        // Interact with mount
        actions.push_back({ "interact_mount", "Interact with mount",
            [this]() -> TAInput {
                return { "mount_system", { { "action", std::string("interact") } } };
            } });
    }

    // Stable actions if there are stables
    if (!stables.empty()) {
        actions.push_back({ "visit_stable", "Visit a stable",
            [this]() -> TAInput {
                return { "mount_system", { { "action", std::string("stable") } } };
            } });
    }

    return actions;
}

bool MountSystemController::evaluateTransition(const TAInput& input, TANode*& outNextNode)
{
    if (input.type == "mount_system") {
        std::string action = std::get<std::string>(input.parameters.at("action"));

        // Most actions would be processed by the game logic
        // and would potentially set outNextNode to a different system node

        outNextNode = this; // Stay in the same node by default
        return true;
    }

    return TANode::evaluateTransition(input, outNextNode);
}

void MountSystemController::updateMounts(int minutes)
{
    for (Mount* mount : ownedMounts) {
        if (mount) {
            mount->update(minutes);
        }
    }
}

void MountSystemController::applyTravelEffects(float distance)
{
    if (!activeMount || !activeMount->isMounted)
        return;

    // Use stamina based on distance
    activeMount->stats.useStamina(distance * 5);

    // Increase hunger and fatigue
    activeMount->stats.hunger += distance * 2;
    if (activeMount->stats.hunger > 100)
        activeMount->stats.hunger = 100;

    activeMount->stats.fatigue += distance * 3;
    if (activeMount->stats.fatigue > 100)
        activeMount->stats.fatigue = 100;

    // Wear equipment
    for (auto& [slot, equipment] : activeMount->equippedItems) {
        if (equipment) {
            equipment->use(distance * 0.5f);
        }
    }
}

float MountSystemController::getMountedSpeedModifier()
{
    if (!activeMount || !activeMount->isMounted)
        return 1.0f;

    return activeMount->getTravelTimeModifier();
}

bool MountSystemController::canPerformSpecialMovement(const std::string& movementType)
{
    if (!activeMount || !activeMount->isMounted)
        return false;

    return config.canUseAbility(activeMount->stats, movementType);
}