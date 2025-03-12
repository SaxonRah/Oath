// TAContext.h
#pragma once

#include "CoreMinimal.h"
#include "TAVariant.h"
#include "TAContext.generated.h"

/**
 * Context object passed during evaluation and execution
 */
struct TREEAUTOMATA_API FTAContext
{
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
    TMap<FString, FVariant> InputParams;
    
    // Global state data that persists across all nodes
    TMap<FString, FVariant> GlobalState;
    
    // Debug trace enabled
    bool bDebugTraceEnabled;
    
    // Add an input parameter
    void AddParam(const FString& Name, const FVariant& Value);
    
    // Get a parameter value
    FVariant GetParam(const FString& Name, const FVariant& DefaultValue = FVariant()) const;
    
    // Add a global state variable
    void SetGlobal(const FString& Name, const FVariant& Value);
    
    // Get a global state variable
    FVariant GetGlobal(const FString& Name, const FVariant& DefaultValue = FVariant()) const;
    
    // Debug trace
    void DebugTrace(const FString& Message) const;
    
    // Create a copy of this context
    FTAContext Clone() const;
};