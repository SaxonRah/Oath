// UTAController.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TATypes.h"
#include "UTAInstance.h"
#include "UTAController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAutomatonTransition, const FString&, InstanceName, const FGuid&, NewNodeID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAutomatonCreated, const FString&, InstanceName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAutomatonDestroyed, const FString&, InstanceName);

/**
 * Main controller that manages tree automata instances
 */
UCLASS(BlueprintType, Blueprintable)
class TREEAUTOMATA_API UTAController : public UObject
{
    GENERATED_BODY()
    
public:
    UTAController();
    
    // Initialize the controller
    UFUNCTION(BlueprintCallable, Category = "Tree Automata")
    virtual void Initialize(AActor* InOwner);
    
    // Active automata instances by system type
    UPROPERTY(BlueprintReadOnly, Category = "Tree Automata")
    TMap<FString, UTAInstance*> ActiveInstances;
    
    // Owner actor
    UPROPERTY(BlueprintReadOnly, Category = "Tree Automata")
    AActor* OwnerActor;
    
    // World reference
    UPROPERTY(BlueprintReadOnly, Category = "Tree Automata")
    UWorld* World;
    
    // Debug mode
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tree Automata|Debug")
    bool bDebugMode;
    
    // Auto-save on transitions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tree Automata|Persistence")
    bool bAutoSaveOnTransition;
    
    // Create a new automaton instance from a template
    UFUNCTION(BlueprintCallable, Category = "Tree Automata")
    UTAInstance* CreateInstance(UObject* Template, const FString& InstanceName);
    
    // Create a new automaton instance from a node hierarchy
    UFUNCTION(BlueprintCallable, Category = "Tree Automata")
    UTAInstance* CreateInstanceFromNodes(TSharedPtr<FTANode> RootNode, const FString& InstanceName);
    
    // Process input on a specific automaton instance
    UFUNCTION(BlueprintCallable, Category = "Tree Automata")
    bool ProcessInput(const FString& InstanceName, const FString& InputID, const TMap<FString, FVariant>& Params);
    
    // Get all available actions from current state
    UFUNCTION(BlueprintCallable, Category = "Tree Automata")
    TArray<FTAActionInfo> GetAvailableActions(const FString& InstanceName);
    
    // Check if a state is accessible from current state
    UFUNCTION(BlueprintCallable, Category = "Tree Automata")
    bool CanReachState(const FString& InstanceName, const FGuid& TargetNodeID);
    
    // Find paths to a target state
    UFUNCTION(BlueprintCallable, Category = "Tree Automata")
    TArray<FTAPath> FindPathsToState(const FString& InstanceName, const FGuid& TargetNodeID, int32 MaxPaths = 3);
    
    // Save all automata to save game
    UFUNCTION(BlueprintCallable, Category = "Tree Automata|Persistence")
    bool SaveAutomataState(const FString& SaveSlot);
    
    // Load all automata from save game
    UFUNCTION(BlueprintCallable, Category = "Tree Automata|Persistence")
    bool LoadAutomataState(const FString& SaveSlot);
    
    // Reset all automata to initial state
    UFUNCTION(BlueprintCallable, Category = "Tree Automata")
    void ResetAllAutomata();
    
    // Reset specific automaton to initial state
    UFUNCTION(BlueprintCallable, Category = "Tree Automata")
    bool ResetAutomaton(const FString& InstanceName);
    
    // Destroy specific automaton instance
    UFUNCTION(BlueprintCallable, Category = "Tree Automata")
    bool DestroyInstance(const FString& InstanceName);
    
    // Get current node ID of an automaton
    UFUNCTION(BlueprintCallable, Category = "Tree Automata")
    FGuid GetCurrentNodeID(const FString& InstanceName);
    
    // Get current node name of an automaton
    UFUNCTION(BlueprintCallable, Category = "Tree Automata")
    FString GetCurrentNodeName(const FString& InstanceName);
    
    // Force transition to a specific node
    UFUNCTION(BlueprintCallable, Category = "Tree Automata|Debug")
    bool ForceTransitionToNode(const FString& InstanceName, const FGuid& TargetNodeID);
    
    // Force transition to a node by name
    UFUNCTION(BlueprintCallable, Category = "Tree Automata|Debug")
    bool ForceTransitionToNodeByName(const FString& InstanceName, const FString& NodeName);
    
    // Set a global variable
    UFUNCTION(BlueprintCallable, Category = "Tree Automata")
    void SetGlobalVariable(const FString& VariableName, const FVariant& Value);
    
    // Get a global variable
    UFUNCTION(BlueprintCallable, Category = "Tree Automata")
    FVariant GetGlobalVariable(const FString& VariableName, const FVariant& DefaultValue);
    
    // Delegates
    UPROPERTY(BlueprintAssignable, Category = "Tree Automata|Events")
    FOnAutomatonTransition OnAutomatonTransition;
    
    UPROPERTY(BlueprintAssignable, Category = "Tree Automata|Events")
    FOnAutomatonCreated OnAutomatonCreated;
    
    UPROPERTY(BlueprintAssignable, Category = "Tree Automata|Events")
    FOnAutomatonDestroyed OnAutomatonDestroyed;
    
    // Serialize all automata states (for save games)
    void Serialize(FArchive& Ar);
    
protected:
    // Global state data shared across all automata
    TMap<FString, FVariant> GlobalState;
    
    // Recursively find node by ID
    TSharedPtr<FTANode> FindNodeByID(TSharedPtr<FTANode> StartNode, const FGuid& TargetID);
    
    // Recursively find node by name
    TSharedPtr<FTANode> FindNodeByName(TSharedPtr<FTANode> StartNode, const FString& TargetName);
};