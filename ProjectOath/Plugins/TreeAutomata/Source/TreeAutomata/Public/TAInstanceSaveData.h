// TAInstanceSaveData.h
#pragma once

#include "CoreMinimal.h"
#include "TAInstanceSaveData.generated.h"

/**
 * Data structure for saving and loading Tree Automata instance state
 */
USTRUCT(BlueprintType)
struct TREEAUTOMATA_API FTAInstanceSaveData
{
    GENERATED_BODY()
    
    // Constructor
    FTAInstanceSaveData() {}
    
    // Template asset path
    UPROPERTY()
    FSoftObjectPath TemplatePath;
    
    // Active node ID
    UPROPERTY()
    FGuid CurrentNodeID;
    
    // Visit history node IDs
    UPROPERTY()
    TArray<FGuid> HistoryNodeIDs;
    
    // Instance variables
    UPROPERTY()
    TMap<FString, FString> Variables;
    
    // Variable type hints
    UPROPERTY()
    TMap<FString, FString> VariableTypes;
    
    // Completed transitions for one-time events
    UPROPERTY()
    TArray<FGuid> CompletedTransitions;
    
    // Serialized node hierarchy data
    UPROPERTY()
    TArray<uint8> SerializedNodes;
    
    // Version number for migration support
    UPROPERTY()
    int32 Version = 1;
};