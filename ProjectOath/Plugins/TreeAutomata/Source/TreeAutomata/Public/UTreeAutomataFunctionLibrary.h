// UTreeAutomataFunctionLibrary.h
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UTAController.h"
#include "TATypes.h"
#include "UTreeAutomataFunctionLibrary.generated.h"

/**
 * Blueprint function library for easy access from Blueprints
 */
UCLASS()
class TREEAUTOMATA_API UTreeAutomataFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
    
public:
    // Get the tree automata controller for an actor
    UFUNCTION(BlueprintPure, Category = "Tree Automata")
    static UTAController* GetTreeAutomataController(AActor* Actor);
    
    // Create a tree automata controller for an actor
    UFUNCTION(BlueprintCallable, Category = "Tree Automata")
    static UTAController* CreateTreeAutomataController(AActor* Actor);
    
    // Process player input for a specific automaton
    UFUNCTION(BlueprintCallable, Category = "Tree Automata")
    static bool ProcessAutomatonInput(AActor* Actor, const FString& AutomatonName, const FString& InputID, const TMap<FString, FTAVariant>& Params);
    
    // Get all available actions for a player
    UFUNCTION(BlueprintCallable, Category = "Tree Automata")
    static TArray<FTAActionInfo> GetAvailablePlayerActions(AActor* Actor, const FString& AutomatonName);
    
    // Set a global variable
    UFUNCTION(BlueprintCallable, Category = "Tree Automata")
    static void SetGlobalVariable(AActor* Actor, const FString& VariableName, const FTAVariant& Value);
    
    // Get a global variable
    UFUNCTION(BlueprintCallable, Category = "Tree Automata")
    static FTAVariant GetGlobalVariable(AActor* Actor, const FString& VariableName, const FTAVariant& DefaultValue);
    
    // Convert blueprint map to variant map
    UFUNCTION(BlueprintCallable, Category = "Tree Automata")
    static TMap<FString, FTAVariant> ConvertBlueprintMapToVariantMap(const TMap<FString, FString>& BlueprintMap);
    
    // Create a new automaton from a template
    UFUNCTION(BlueprintCallable, Category = "Tree Automata")
    static UTAInstance* CreateAutomaton(AActor* Actor, UObject* Template, const FString& InstanceName);
    
    // Force transition to a node by name
    UFUNCTION(BlueprintCallable, Category = "Tree Automata")
    static bool ForceTransitionToNode(AActor* Actor, const FString& AutomatonName, const FString& NodeName);
};