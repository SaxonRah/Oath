// TAContext.h
#pragma once

#include "CoreMinimal.h"
#include "TATypes.h"
#include "TAContext.generated.h"

/**
 * Context object passed during evaluation and execution
 */
struct TREEAUTOMATA_API FTAContext
{
    GENERATED_BODY()

    // Constructor
    FTAContext();
    
    // Constructor with world and player
    FTAContext(UWorld* InWorld, AActor* InPlayerActor);
    
    // World context
    UWorld* World;
    
    // Player character or controller
    AActor* PlayerActor;
    
    // Current input triggering evaluation
    FString InputID;
    
    // Input parameters
    TMap<FString, FTAVariant> InputParams;
    
    // Global state data that persists across all nodes
    TMap<FString, FTAVariant> GlobalState;
    
    // Debug trace enabled
    bool bDebugTraceEnabled;
    
    // Add an input parameter
    void AddParam(const FString& Name, const FTAVariant& Value);
    
    // Get a parameter value
    FTAVariant GetParam(const FString& Name, const FTAVariant& DefaultValue = FTAVariant()) const;
    
    // Add a global state variable
    void SetGlobal(const FString& Name, const FTAVariant& Value);
    
    // Get a global state variable
    FTAVariant GetGlobal(const FString& Name, const FTAVariant& DefaultValue = FTAVariant()) const;
    
    // Debug trace
    void DebugTrace(const FString& Message) const;
    
    // Create a copy of this context
    FTAContext Clone() const;
};