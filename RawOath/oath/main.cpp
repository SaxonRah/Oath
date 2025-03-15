#include "core/TAController.hpp"
#include "utils/JSONLoader.hpp"

int main()
{
    // Create the automaton controller
    TAController controller;

    // Load game data
    loadGameData(controller);

    // Demo functionality
    // ...

    return 0;
}