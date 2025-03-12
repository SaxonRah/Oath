// FTATransition.h
#pragma once

#include "CoreMinimal.h"
#include "TATypes.h"
#include "TACondition.h"
#include "TAContext.h"
#include "FTATransition.generated.h"

// Forward declaration
class FTANode;

/**
 * Transition between nodes with conditions
 */
struct TREEAUTOMATA_API FTATransition
{
    // Default constructor
    FTATransition();
    
    // Constructor with target
    FTATransition(TSharedPtr<FTANode> InTargetNode);
    
    // Target node to transition to
    TSharedPtr<FTANode> TargetNode;
    
    // Conditions that must be satisfied for this transition
    TArray<TSharedPtr<FTACondition>> Conditions;
    
    // Priority for resolving conflicts (higher wins)
    int32 Priority;
    
    // Unique identifier
    FGuid TransitionID;
    
    // Human-readable name
    FString TransitionName;
    
    // Evaluate if this transition should be taken given the context
    bool Evaluate(const FTAContext& Context) const;
    
    // Add a condition
    void AddCondition(TSharedPtr<FTACondition> Condition);
    
    // Remove a condition by index
    bool RemoveCondition(int32 Index);
    
    // Clone this transition
    FTATransition Clone() const;
    
    // Serialize the transition
    void Serialize(FArchive& Ar);
    
    // Get debug string representation
    FString ToString() const;
};