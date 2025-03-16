# Economy TODO

```cpp
// main.cpp (partial example)

#include "core/TAController.hpp"
#include "systems/economy/EconomicSystemNode.hpp"
#include "systems/economy/InvestmentNode.hpp"
#include "systems/economy/PropertyNode.hpp"

using namespace oath;
using namespace oath::economy;

int main() {
    // Initialize controller
    TAController controller;
    
    // Create economy system nodes
    EconomicSystemNode* economicSystem = new EconomicSystemNode("EconomicSystem");
    InvestmentNode* investmentSystem = new InvestmentNode("InvestmentSystem", economicSystem);
    PropertyNode* propertySystem = new PropertyNode("PropertySystem");
    
    // Register nodes with controller
    controller.registerNode(economicSystem);
    controller.registerNode(investmentSystem);
    controller.registerNode(propertySystem);
    
    // Connect the systems with transitions
    economicSystem->addTransition(
        [](const TAInput& input) {
            return input.type == "economy_action" && std::get<std::string>(input.parameters.at("action")) == "to_investments";
        },
        investmentSystem, "Go to Investments");

    economicSystem->addTransition(
        [](const TAInput& input) {
            return input.type == "economy_action" && std::get<std::string>(input.parameters.at("action")) == "to_properties";
        },
        propertySystem, "Go to Properties");

    investmentSystem->addTransition(
        [](const TAInput& input) {
            return input.type == "investment_action" && std::get<std::string>(input.parameters.at("action")) == "exit";
        },
        economicSystem, "Exit");

    propertySystem->addTransition(
        [](const TAInput& input) {
            return input.type == "property_action" && std::get<std::string>(input.parameters.at("action")) == "exit";
        },
        economicSystem, "Exit");
    
    // Set economy system as active
    controller.setActiveNode(economicSystem);
    
    // Main game loop
    while (true) {
        // Process user input, update game state, etc.
        controller.update();
    }
    
    return 0;
}
```