// FTAAction.h
#pragma once

#include "CoreMinimal.h"
#include "TAContext.h"
#include "TALogging.h"
#include "FTAAction.generated.h"

// Forward declaration of EActorSpawnLocation if needed
// Make this a regular enum, not an enum class
UENUM(BlueprintType)
enum ETAActorSpawnLocation
{
    PlayerLocation UMETA(DisplayName = "Player Location"),
    PlayerViewpoint UMETA(DisplayName = "Player Viewpoint"),
    CustomLocation UMETA(DisplayName = "Custom Location")
};

/**
 * Abstract base class for actions
 */
USTRUCT(BlueprintType)
struct TREEAUTOMATA_API FFTAAction
{
    GENERATED_BODY()

public:
    // Constructor
    FFTAAction();

    // Destructor
    virtual ~FFTAAction();

    // Execute the action given the context
    virtual void Execute(const FTAContext& Context) const;

    // Description for editing and debugging
    virtual FString GetDescription() const;

    // Clone this action
    virtual TSharedPtr<FFTAAction> Clone() const;

    // Unique identifier
    UPROPERTY(BlueprintReadWrite, Category = "Tree Automata")
    FGuid ActionID;

    // Action name
    UPROPERTY(BlueprintReadWrite, Category = "Tree Automata")
    FString ActionName;

    // Serialize the action
    virtual void Serialize(FArchive& Ar);
};

/**
 * Set variable action
 */
USTRUCT(BlueprintType)
struct TREEAUTOMATA_API FSetVariableAction : public FFTAAction
{
    GENERATED_BODY()

public:
    // Constructor
    FSetVariableAction();

    // Variable name
    UPROPERTY(BlueprintReadWrite, Category = "Tree Automata")
    FString VariableName;

    // New value
    UPROPERTY(BlueprintReadWrite, Category = "Tree Automata")
    FTAVariant Value;

    // Set in global state
    UPROPERTY(BlueprintReadWrite, Category = "Tree Automata")
    bool bUseGlobalState;

    // Implementation
    virtual void Execute(const FTAContext& Context) const override;
    virtual FString GetDescription() const override;
    virtual TSharedPtr<FFTAAction> Clone() const override;
    virtual void Serialize(FArchive& Ar) override;
};

/**
 * Log message action for debugging
 */
USTRUCT(BlueprintType)
struct TREEAUTOMATA_API FLogMessageAction : public FFTAAction
{
    GENERATED_BODY()

public:
    // Constructor
    FLogMessageAction();

    // Message to log
    UPROPERTY(BlueprintReadWrite, Category = "Tree Automata")
    FString Message;

    // Log verbosity - using uint8 for Blueprint compatibility
    UPROPERTY(BlueprintReadWrite, Category = "Tree Automata")
    uint8 Verbosity;

    // Implementation
    virtual void Execute(const FTAContext& Context) const override;
    virtual FString GetDescription() const override;
    virtual TSharedPtr<FFTAAction> Clone() const override;
    virtual void Serialize(FArchive& Ar) override;
};

/**
 * Spawn actor action
 */
USTRUCT(BlueprintType)
struct TREEAUTOMATA_API FSpawnActorAction : public FFTAAction
{
    GENERATED_BODY()

public:
    // Constructor
    FSpawnActorAction();

    // Actor class to spawn
    UPROPERTY(BlueprintReadWrite, Category = "Tree Automata")
    UClass* ActorClass;

    // Spawn location
    UPROPERTY(BlueprintReadWrite, Category = "Tree Automata")
    TEnumAsByte<ETAActorSpawnLocation> SpawnLocationType;

    // Offset from spawn location
    UPROPERTY(BlueprintReadWrite, Category = "Tree Automata")
    FVector SpawnOffset;

    // Actor tag for identification
    UPROPERTY(BlueprintReadWrite, Category = "Tree Automata")
    FName ActorTag;

    // Implementation
    virtual void Execute(const FTAContext& Context) const override;
    virtual FString GetDescription() const override;
    virtual TSharedPtr<FFTAAction> Clone() const override;
    virtual void Serialize(FArchive& Ar) override;
};