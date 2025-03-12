// FTAAction.h
#pragma once

#include "CoreMinimal.h"
#include "TAContext.h"
#include "FTAAction.generated.h"

/**
 * Abstract base class for actions
 */
class TREEAUTOMATA_API FTAAction
{
public:
    // Constructor
    FTAAction();
    
    // Destructor
    virtual ~FTAAction();
    
    // Execute the action given the context
    virtual void Execute(const FTAContext& Context) const = 0;
    
    // Description for editing and debugging
    virtual FString GetDescription() const = 0;
    
    // Clone this action
    virtual TSharedPtr<FTAAction> Clone() const = 0;
    
    // Unique identifier
    FGuid ActionID;
    
    // Action name
    FString ActionName;
    
    // Serialize the action
    virtual void Serialize(FArchive& Ar);
};

/**
 * Set variable action
 */
class TREEAUTOMATA_API FSetVariableAction : public FTAAction
{
public:
    // Constructor
    FSetVariableAction();
    
    // Variable name
    FString VariableName;
    
    // New value
    FTAVariant Value;
    
    // Set in global state
    bool bUseGlobalState;
    
    // Implementation
    virtual void Execute(const FTAContext& Context) const override;
    virtual FString GetDescription() const override;
    virtual TSharedPtr<FTAAction> Clone() const override;
    virtual void Serialize(FArchive& Ar) override;
};

/**
 * Log message action for debugging
 */
class TREEAUTOMATA_API FLogMessageAction : public FTAAction
{
public:
    // Constructor
    FLogMessageAction(const FString& InMessage = TEXT(""));
    
    // Message to log
    FString Message;
    
    // Log verbosity
    ELogVerbosity::Type Verbosity;
    
    // Implementation
    virtual void Execute(const FTAContext& Context) const override;
    virtual FString GetDescription() const override;
    virtual TSharedPtr<FTAAction> Clone() const override;
    virtual void Serialize(FArchive& Ar) override;
};

/**
 * Spawn actor action
 */
class TREEAUTOMATA_API FSpawnActorAction : public FTAAction
{
public:
    // Constructor
    FSpawnActorAction();
    
    // Actor class to spawn
    UClass* ActorClass;
    
    // Spawn location
    EActorSpawnLocation SpawnLocationType;
    
    // Offset from spawn location
    FVector SpawnOffset;
    
    // Actor tag for identification
    FName ActorTag;
    
    // Implementation
    virtual void Execute(const FTAContext& Context) const override;
    virtual FString GetDescription() const override;
    virtual TSharedPtr<FTAAction> Clone() const override;
    virtual void Serialize(FArchive& Ar) override;
};