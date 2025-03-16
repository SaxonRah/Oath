// CrimeLawSystem.cpp
#include "CrimeLawSystem.hpp"

#include <fstream>
#include <iostream>
#include <stdexcept>

CrimeLawSystem::CrimeLawSystem(TAController* controller)
    : controller(controller)
{
    // Make sure config is loaded
    try {
        loadCrimeLawConfig();
    } catch (const std::exception& e) {
        std::cerr << "Failed to load crime system config: " << e.what() << std::endl;
        throw;
    }

    setupNodes();
    setupTransitions();
    registerSystem();
}

void CrimeLawSystem::setupNodes()
{
    // Create all nodes
    criminalStatusNode = dynamic_cast<CrimeSystemNode*>(
        controller->createNode<CrimeSystemNode>("CriminalStatus"));

    theftNode = dynamic_cast<TheftNode*>(
        controller->createNode<TheftNode>("TheftPlanning"));

    guardNode = dynamic_cast<GuardEncounterNode*>(
        controller->createNode<GuardEncounterNode>("GuardEncounter"));

    jailNode = dynamic_cast<JailNode*>(
        controller->createNode<JailNode>("Jail"));

    bountyNode = dynamic_cast<BountyPaymentNode*>(
        controller->createNode<BountyPaymentNode>("BountyPayment"));

    pickpocketNode = dynamic_cast<PickpocketNode*>(
        controller->createNode<PickpocketNode>("Pickpocket"));

    // Create theft execution nodes from config
    for (const auto& target : crimeLawConfig["theftTargets"]) {
        std::string id = target["id"];
        std::string name = "Theft_" + id;

        TheftExecutionNode* node = dynamic_cast<TheftExecutionNode*>(
            controller->createNode<TheftExecutionNode>(name, id));

        theftExecutionNodes[id] = node;
    }
}

void CrimeLawSystem::setupTransitions()
{
    // From criminal status to various crime options
    criminalStatusNode->addTransition(
        [](const TAInput& input) {
            return input.type == "crime_choice" && std::get<std::string>(input.parameters.at("choice")) == "theft";
        },
        theftNode, "Plan theft");

    criminalStatusNode->addTransition(
        [](const TAInput& input) {
            return input.type == "crime_choice" && std::get<std::string>(input.parameters.at("choice")) == "pickpocket";
        },
        pickpocketNode, "Attempt pickpocketing");

    criminalStatusNode->addTransition(
        [](const TAInput& input) {
            return input.type == "crime_choice" && std::get<std::string>(input.parameters.at("choice")) == "pay_bounty";
        },
        bountyNode, "Pay bounty");

    // From theft planning to execution
    for (const auto& [id, node] : theftExecutionNodes) {
        theftNode->addTransition(
            [id](const TAInput& input) {
                return input.type == "theft_action" && std::get<std::string>(input.parameters.at("target")) == id;
            },
            node, "Steal " + id);
    }

    theftNode->addTransition(
        [](const TAInput& input) {
            return input.type == "theft_action" && std::get<std::string>(input.parameters.at("target")) == "cancel";
        },
        criminalStatusNode, "Cancel theft");

    // From theft execution back to status
    for (const auto& [id, node] : theftExecutionNodes) {
        node->addTransition(
            [](const TAInput& input) { return true; },
            criminalStatusNode, "Return after theft");
    }

    // Guard encounters
    guardNode->addTransition(
        [](const TAInput& input) {
            return input.type == "guard_response" && std::get<std::string>(input.parameters.at("action")) == "pay";
        },
        bountyNode, "Pay bounty");

    guardNode->addTransition(
        [](const TAInput& input) {
            return input.type == "guard_response" && std::get<std::string>(input.parameters.at("action")) == "surrender";
        },
        jailNode, "Go to jail");

    guardNode->addTransition(
        [](const TAInput& input) {
            return input.type == "guard_response" && std::get<std::string>(input.parameters.at("action")) == "resist";
        },
        criminalStatusNode, "Combat with guards"); // Would link to combat system

    guardNode->addTransition(
        [](const TAInput& input) {
            return input.type == "guard_response" && std::get<std::string>(input.parameters.at("action")) == "flee";
        },
        criminalStatusNode, "Attempt to flee"); // Would have chance to escape

    // Jail transitions
    jailNode->addTransition(
        [](const TAInput& input) {
            return input.type == "jail_action" && std::get<std::string>(input.parameters.at("action")) == "serve";
        },
        criminalStatusNode, "Serve jail time");

    jailNode->addTransition(
        [](const TAInput& input) {
            return input.type == "jail_action" && std::get<std::string>(input.parameters.at("action")) == "escape";
        },
        criminalStatusNode, "escape_success");

    jailNode->addTransition(
        [](const TAInput& input) {
            return input.type == "jail_action" && std::get<std::string>(input.parameters.at("action")) == "escape";
        },
        jailNode, "escape_failure");

    // Bounty office transitions
    bountyNode->addTransition(
        [](const TAInput& input) {
            return input.type == "bounty_action" && std::get<std::string>(input.parameters.at("action")) == "pay_full";
        },
        criminalStatusNode, "payment_success");

    bountyNode->addTransition(
        [](const TAInput& input) {
            return input.type == "bounty_action" && std::get<std::string>(input.parameters.at("action")) == "pay_full";
        },
        bountyNode, "payment_failure");

    bountyNode->addTransition(
        [](const TAInput& input) {
            return input.type == "bounty_action" && std::get<std::string>(input.parameters.at("action")) == "negotiate";
        },
        criminalStatusNode, "negotiate_success");

    bountyNode->addTransition(
        [](const TAInput& input) {
            return input.type == "bounty_action" && std::get<std::string>(input.parameters.at("action")) == "negotiate";
        },
        bountyNode, "negotiate_failure");

    bountyNode->addTransition(
        [](const TAInput& input) {
            return input.type == "bounty_action" && std::get<std::string>(input.parameters.at("action")) == "leave";
        },
        criminalStatusNode, "leave");

    // Pickpocket transitions
    pickpocketNode->addTransition(
        [](const TAInput& input) {
            return input.type == "pickpocket_action" && std::get<std::string>(input.parameters.at("target")) == "cancel";
        },
        criminalStatusNode, "cancel");
}

void CrimeLawSystem::registerSystem()
{
    // Register the crime system with the controller
    controller->setSystemRoot("CrimeLawSystem", criminalStatusNode);

    // Set up guard encounter triggering
    setupGuardEncounters();
}

void CrimeLawSystem::setupGuardEncounters()
{
    // This would integrate with your world system to trigger guard encounters
    // when the player enters certain locations and has a wanted status

    // For example, you might add a condition to location transitions that
    // checks if the player is wanted in that region, and if so, redirects
    // to the guard encounter node
}

void CrimeLawSystem::commitCrime(GameContext* context, const std::string& crimeType, int severity, const std::string& region, const std::string& location)
{
    // Get the law context
    CrimeLawContext* lawContext = getLawContextFromController();

    // Determine if witnessed based on location and other factors
    bool witnessed = (rand() % 100) < 50; // 50% chance for demonstration

    // Create and record the crime
    CrimeRecord crime(crimeType, region, location, witnessed, severity);
    lawContext->criminalRecord.addCrime(crime);

    std::cout << "Crime committed: " << crime.getDescription() << std::endl;

    if (witnessed) {
        lawContext->guardAlerted[region] = true;
    }
}

CrimeLawContext* CrimeLawSystem::getLawContextFromController()
{
    // In a real implementation, this would get the context from the controller
    // Here we're simplifying by creating a static instance
    static CrimeLawContext lawContext;
    return &lawContext;
}