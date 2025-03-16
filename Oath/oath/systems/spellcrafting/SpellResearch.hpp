#pragma once

#include "SpellComponent.hpp"
#include "SpellModifier.hpp"

// Spell research result - what happens when experimenting
struct SpellResearchResult {
    enum ResultType {
        Success,
        PartialSuccess,
        Failure,
        Disaster
    } type;

    std::string message;
    SpellComponent* discoveredComponent;
    SpellModifier* discoveredModifier;
    float skillProgress;
};