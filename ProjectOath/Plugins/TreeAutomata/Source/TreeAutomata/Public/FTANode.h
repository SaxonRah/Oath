// FTANode.h
#pragma once

#include "CoreMinimal.h"
#include "TATypes.h"
#include "TAAction.h"
#include "TATransition.h"
#include "TAContext.h"
#include "FTANode.generated.h"

/**
 * Base node class that represents a state in our tree automaton
 */
class TREEAUTOMATA_API FTANode
{
public:
    // Constructor
    FTANode();
    
    // Copy constructor
    FTANode(const FTANode& Other);
    
    // Destructor
    virtual ~FTANode();

    // Unique identifier
    FGuid NodeID;
    
    // Human-readable name for debugging and editor
    FString NodeName;
    
    // Node type for categorization (quest, dialogue, skill, etc.)
    FString NodeType;
    
    // Custom state data - flexible container for any system-specific data
    TMap<FString, FVariant> StateData;
    
    // Condition-based transitions to other nodes
    TArray<FTATransition> Transitions;
    
    // Child nodes (for hierarchical structures)
    TArray<TSharedPtr<FTANode>> Children;
    
    // Parent node (for upward traversal)
    TWeakPtr<FTANode> Parent;
    
    // Is this a terminal/accepting state?
    bool bIsAcceptingState;
    
    // Actions to execute when entering this node
    TArray<TSharedPtr<FTAAction>> EntryActions;
    
    // Actions to execute when exiting this node
    TArray<TSharedPtr<FTAAction>> ExitActions;
    
    // Evaluate if a transition should be taken based on input
    virtual bool EvaluateTransitions(const FTAContext& Context, TSharedPtr<FTANode>& OutNextNode);
    
    // Execute entry actions with the given context
    virtual void ExecuteEntryActions(const FTAContext& Context);
    
    // Execute exit actions with the given context
    virtual void ExecuteExitActions(const FTAContext& Context);
    
    // Find a specific child node by ID
    TSharedPtr<FTANode> FindChildByID(const FGuid& ChildID);
    
    // Find a specific child node by name
    TSharedPtr<FTANode> FindChildByName(const FString& ChildName);
    
    // Add a child node
    void AddChild(TSharedPtr<FTANode> Child);
    
    // Remove a child node
    bool RemoveChild(const FGuid& ChildID);
    
    // Add a transition
    void AddTransition(const FTATransition& Transition);
    
    // Remove a transition
    bool RemoveTransition(int32 Index);
    
    // Clone this node and all its children
    virtual TSharedPtr<FTANode> Clone() const;
    
    // Serialize the node for saving
    virtual void Serialize(FArchive& Ar);
    
    // Called after loading all nodes to establish the proper transition connections between nodes.
    void ResolveNodeReferences(TMap<FGuid, TSharedPtr<FTANode>>& NodeMap);

    // Get debug string representation
    virtual FString ToString() const;
};