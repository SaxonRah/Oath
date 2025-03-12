// UTANodeWrapper.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "FTANode.h"
#include "Templates/SharedPointer.h"
#include "UTANodeWrapper.generated.h"

/**
 * Wrapper class that makes TSharedPtr<FTANode> accessible from Blueprints
 */
UCLASS(BlueprintType)
class TREEAUTOMATA_API UTANodeWrapper : public UObject
{
    GENERATED_BODY()
    
public:
    UTANodeWrapper();
    
    // The internal node this wrapper represents
    TSharedPtr<FTANode> InternalNode;
    
    // Create a new node wrapper
    UFUNCTION(BlueprintCallable, Category = "Tree Automata|Nodes")
    static UTANodeWrapper* CreateNodeWrapper(UObject* Outer);
    
    // Create a new quest node wrapper
    UFUNCTION(BlueprintCallable, Category = "Tree Automata|Nodes")
    static UTANodeWrapper* CreateQuestNode(UObject* Outer, const FString& NodeName);
    
    // Create a new dialogue node wrapper
    UFUNCTION(BlueprintCallable, Category = "Tree Automata|Nodes")
    static UTANodeWrapper* CreateDialogueNode(UObject* Outer, const FString& NodeName);
    
    // Get node ID
    UFUNCTION(BlueprintCallable, Category = "Tree Automata|Nodes")
    FGuid GetNodeID() const;
    
    // Get node name
    UFUNCTION(BlueprintCallable, Category = "Tree Automata|Nodes")
    FString GetNodeName() const;
    
    // Set node name
    UFUNCTION(BlueprintCallable, Category = "Tree Automata|Nodes")
    void SetNodeName(const FString& NewName);
    
    // Get node type
    UFUNCTION(BlueprintCallable, Category = "Tree Automata|Nodes")
    FString GetNodeType() const;
    
    // Add a child node
    UFUNCTION(BlueprintCallable, Category = "Tree Automata|Nodes")
    void AddChild(UTANodeWrapper* ChildWrapper);
    
    // Create a transition to another node
    UFUNCTION(BlueprintCallable, Category = "Tree Automata|Nodes")
    void AddTransition(UTANodeWrapper* TargetNode, const FString& TransitionName, int32 Priority = 0);
    
    // Check if this is a quest node
    UFUNCTION(BlueprintCallable, Category = "Tree Automata|Nodes")
    bool IsQuestNode() const;
    
    // Check if this is a dialogue node
    UFUNCTION(BlueprintCallable, Category = "Tree Automata|Nodes")
    bool IsDialogueNode() const;
    
    // Set a state variable
    UFUNCTION(BlueprintCallable, Category = "Tree Automata|Nodes")
    void SetStateVariable(const FString& Name, const FTAVariant& Value);
    
    // Get a state variable
    UFUNCTION(BlueprintCallable, Category = "Tree Automata|Nodes")
    FTAVariant GetStateVariable(const FString& Name, const FTAVariant& DefaultValue) const;
    
    // Set accepting state flag
    UFUNCTION(BlueprintCallable, Category = "Tree Automata|Nodes")
    void SetAcceptingState(bool bIsAccepting);
    
    // Get accepting state flag
    UFUNCTION(BlueprintCallable, Category = "Tree Automata|Nodes")
    bool IsAcceptingState() const;
    
    // Get parent node
    UFUNCTION(BlueprintCallable, Category = "Tree Automata|Nodes")
    UTANodeWrapper* GetParentNode() const;
    
    // Create a node wrapper from an existing internal node
    static UTANodeWrapper* CreateFromInternalNode(UObject* Outer, TSharedPtr<FTANode> Node);
};