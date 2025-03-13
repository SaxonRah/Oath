// Tree_Automata.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameFramework/Actor.h"
#include "Json.h"
#include "JsonObjectConverter.h"
#include "Serialization/JsonSerializer.h"

#include "Tree_Automata.generated.h"

/**
 * Represents a node in the automaton tree
 */
USTRUCT(BlueprintType)
struct TREEAUTOMATA_API FAutomatonNode
{
    GENERATED_BODY()

    // Unique identifier for this node
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid NodeId;

    // Human-readable name
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Name;

    // Optional contextual data
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, FString> Metadata;

    // Parent node ID (empty for root)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGuid ParentId;

    // Child node IDs
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FGuid> ChildrenIds;

    // Current state of this node
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString State;

    FAutomatonNode() : NodeId(FGuid::NewGuid()), State("Inactive") {}

    FAutomatonNode(const FString& InName, const FGuid& InParentId = FGuid())
        : NodeId(FGuid::NewGuid()), Name(InName), ParentId(InParentId), State("Inactive") {
    }
};

/**
 * Represents a transition rule between states
 */
USTRUCT(BlueprintType)
struct TREEAUTOMATA_API FStateTransition
{
    GENERATED_BODY()

    // Current state required for this transition
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString FromState;

    // State to transition to
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ToState;

    // Name of the event that triggers this transition
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString TriggerEvent;

    // Additional conditions that must be true for transition
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> Conditions;

    // Actions to perform during transition
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> Actions;

    FStateTransition() {}

    FStateTransition(const FString& From, const FString& To, const FString& Trigger)
        : FromState(From), ToState(To), TriggerEvent(Trigger) {
    }
};

/**
 * Base class for rule evaluators that determine if a transition is valid
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class TREEAUTOMATA_API UConditionEvaluator : public UObject
{
    GENERATED_BODY()

public:
    // Evaluate if a condition is met
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    bool EvaluateCondition(const FString& Condition, UObject* Context);
    virtual bool EvaluateCondition_Implementation(const FString& Condition, UObject* Context);
};

/**
 * Base class for action performers during transitions
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class TREEAUTOMATA_API UActionPerformer : public UObject
{
    GENERATED_BODY()

public:
    // Perform an action during transition
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void PerformAction(const FString& Action, UObject* Context);
    virtual void PerformAction_Implementation(const FString& Action, UObject* Context);
};

/**
 * Main Tree Automata class
 */
UCLASS(Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TREEAUTOMATA_API UTreeAutomata : public UActorComponent
{
    GENERATED_BODY()

public:
    UTreeAutomata();

    // Initialize the automaton with a root node
    UFUNCTION(BlueprintCallable, Category = "Tree Automata")
    FGuid InitializeRoot(const FString& RootName);

    // Add a node to the tree
    UFUNCTION(BlueprintCallable, Category = "Tree Automata")
    FGuid AddNode(const FString& NodeName, const FGuid& ParentId);

    // Get a node by ID
    UFUNCTION(BlueprintCallable, Category = "Tree Automata")
    FAutomatonNode GetNode(const FGuid& NodeId) const;

    // Define a state transition rule
    UFUNCTION(BlueprintCallable, Category = "Tree Automata")
    void AddTransition(const FStateTransition& Transition);

    // Trigger an event on a specific node
    UFUNCTION(BlueprintCallable, Category = "Tree Automata")
    bool TriggerEvent(const FGuid& NodeId, const FString& EventName, UObject* Context = nullptr);

    // Set the condition evaluator
    UFUNCTION(BlueprintCallable, Category = "Tree Automata")
    void SetConditionEvaluator(UConditionEvaluator* Evaluator);

    // Set the action performer
    UFUNCTION(BlueprintCallable, Category = "Tree Automata")
    void SetActionPerformer(UActionPerformer* Performer);

    // Get all children of a node
    UFUNCTION(BlueprintCallable, Category = "Tree Automata")
    TArray<FAutomatonNode> GetChildren(const FGuid& NodeId) const;

    // Get the parent of a node
    UFUNCTION(BlueprintCallable, Category = "Tree Automata")
    FAutomatonNode GetParent(const FGuid& NodeId) const;

    // Find nodes by state
    UFUNCTION(BlueprintCallable, Category = "Tree Automata")
    TArray<FAutomatonNode> FindNodesByState(const FString& State) const;

    // Check if a path exists between two nodes
    UFUNCTION(BlueprintCallable, Category = "Tree Automata")
    bool PathExists(const FGuid& FromNodeId, const FGuid& ToNodeId) const;

    // Serialize the entire automaton to JSON
    UFUNCTION(BlueprintCallable, Category = "Tree Automata")
    FString SerializeToJson() const;

    // Deserialize from JSON
    UFUNCTION(BlueprintCallable, Category = "Tree Automata")
    bool DeserializeFromJson(const FString& JsonString);

    // Get all nodes in the automaton
    UFUNCTION(BlueprintCallable, Category = "Tree Automata")
    const TMap<FGuid, FAutomatonNode>& GetNodes() const { return Nodes; }

    // Get mutable reference to a node
    UFUNCTION(BlueprintCallable, Category = "Tree Automata")
    FAutomatonNode& GetNodeMutable(const FGuid& NodeId) { return Nodes[NodeId]; }

    // Get the root node ID
    UFUNCTION(BlueprintCallable, Category = "Tree Automata")
    FGuid GetRootNodeId() const { return RootNodeId; }

    // Get the condition evaluator
    UFUNCTION(BlueprintCallable, Category = "Tree Automata")
    UConditionEvaluator* GetConditionEvaluator() const { return ConditionEvaluator; }

protected:
    // Storage for all nodes in the automaton
    UPROPERTY(VisibleAnywhere, Category = "Tree Automata")
    TMap<FGuid, FAutomatonNode> Nodes;

    // State transition rules
    UPROPERTY(VisibleAnywhere, Category = "Tree Automata")
    TArray<FStateTransition> TransitionRules;

    // Root node ID
    UPROPERTY(VisibleAnywhere, Category = "Tree Automata")
    FGuid RootNodeId;

    // Condition evaluator
    UPROPERTY(VisibleAnywhere, Category = "Tree Automata")
    UConditionEvaluator* ConditionEvaluator;

    // Action performer
    UPROPERTY(VisibleAnywhere, Category = "Tree Automata")
    UActionPerformer* ActionPerformer;

    // Find applicable transitions for a given event and node state
    TArray<FStateTransition> FindApplicableTransitions(const FString& CurrentState, const FString& EventName) const;
};