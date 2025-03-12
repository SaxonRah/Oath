// UTAInstance.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "FTANode.h"
#include "TATypes.h"
#include "TAContext.h"
#include "TAInstanceSaveData.h"
#include "UTAInstance.generated.h"

/**
 * Instance of a tree automaton
 */
UCLASS(BlueprintType)
class TREEAUTOMATA_API UTAInstance : public UObject
{
    GENERATED_BODY()
    
public:
    UTAInstance();
    
    // Initialize the instance
    void Initialize(const FString& InName, UObject* InTemplate, AActor* InOwner);
    
    // Template this instance was created from
    UPROPERTY()
    UObject* Template;
    
    // Instance name
    UPROPERTY(BlueprintReadOnly, Category = "Tree Automata")
    FString InstanceName;
    
    // Owner actor
    UPROPERTY(BlueprintReadOnly, Category = "Tree Automata")
    AActor* OwnerActor;
    
    // Root node of this automaton
    TSharedPtr<FTANode> RootNode;
    
    // Current active node
    TSharedPtr<FTANode> CurrentNode;
    
    // History of visited nodes
    TArray<TSharedPtr<FTANode>> History;
    
    // Global state data for this automaton instance
    TMap<FString, FVariant> LocalState;
    
    // Process an input and potentially transition
    bool ProcessInput(const FTAContext& Context);
    
    // Get all available actions from current state
    TArray<FTAActionInfo> GetAvailableActions() const;
    
    // Check if a node is reachable from current state
    bool CanReachNode(const FGuid& TargetNodeID) const;
    
    // Find paths to a target node
    TArray<FTAPath> FindPathsToNode(const FGuid& TargetNodeID, int32 MaxPaths = 3) const;
    
    // Force transition to a specific node
    bool ForceTransitionToNode(const FGuid& TargetNodeID);
    
    // Reset to initial state
    void Reset();
    
    // Save instance state
    void SaveState(FTAInstanceSaveData& OutSaveData) const;
    
    // Load instance state
    void LoadState(const FTAInstanceSaveData& SaveData);
    
    // Helper method to build a map of node IDs to node pointers
    void BuildNodeMap(TSharedPtr<FTANode> Node, TMap<FGuid, TSharedPtr<FTANode>>& NodeMap);

    // Helper method to resolve node references in the hierarchy
    void ResolveNodeReferences(TSharedPtr<FTANode> Node, TMap<FGuid, TSharedPtr<FTANode>>& NodeMap);
    
    // Helper method to create nodes from a template asset
    void CreateNodesFromTemplate(UTreeAutomatonAsset* TemplateAsset);

    // Get current node ID
    FGuid GetCurrentNodeID() const;
    
    // Get current node name
    FString GetCurrentNodeName() const;
    
    // Set root node
    void SetRootNode(TSharedPtr<FTANode> InRootNode);
    
    // Get root node
    TSharedPtr<FTANode> GetRootNode() const;
    
    // Get a local variable
    FVariant GetLocalVariable(const FString& Name, const FVariant& DefaultValue = FVariant()) const;
    
    // Set a local variable
    void SetLocalVariable(const FString& Name, const FVariant& Value);
    
    // Serialize instance state
    void Serialize(FArchive& Ar);
    
private:
    // Find a node by ID recursively
    TSharedPtr<FTANode> FindNodeByID(TSharedPtr<FTANode> StartNode, const FGuid& TargetID) const;
    
    // Helper for path finding
    void FindPathsRecursive(TSharedPtr<FTANode> CurrentNode, const FGuid& TargetID, 
        TArray<FTAPathNode>& CurrentPath, TArray<FTAPath>& Results, int32 MaxPaths, int32 MaxDepth) const;
};