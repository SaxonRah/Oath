#pragma once

#include "CoreMinimal.h"

#include "StateTreeTypes.h"
#include "StateTreeNodeBase.h"
#include "StateTreeExecutionContext.h"

#include "WorldStateTreeNode.generated.h"

UENUM(BlueprintType)
enum class EWorldEventType : uint8
{
    TownDevelopment,
    FactionControl,
    SeasonChange,
    EconomicEvent,
    NaturalDisaster
};

USTRUCT(BlueprintType)
struct FWorldEventCondition
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName VariableName;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MinValue = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxValue = 100.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bInverseCondition = false;
};

USTRUCT(BlueprintType)
struct FWorldEventEffect
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName VariableName;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ChangeValue = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsMultiplier = false;
};

USTRUCT()
struct RPGSTATEMACHINES_API FWorldStateTreeNode : public FStateTreeNodeBase
{
    GENERATED_BODY()
    
public:
    FWorldStateTreeNode();
    
    virtual bool Link(FStateTreeLinker& Linker) override;
    //virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
    //virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
    
    UPROPERTY(EditAnywhere, Category = "World State")
    FName WorldEventID;
    
    UPROPERTY(EditAnywhere, Category = "World State")
    FText EventName;
    
    UPROPERTY(EditAnywhere, Category = "World State")
    FText EventDescription;
    
    UPROPERTY(EditAnywhere, Category = "World State")
    EWorldEventType EventType;
    
    UPROPERTY(EditAnywhere, Category = "World State")
    TArray<FWorldEventCondition> RequiredConditions;
    
    UPROPERTY(EditAnywhere, Category = "World State")
    TArray<FWorldEventEffect> EventEffects;
    
    UPROPERTY(EditAnywhere, Category = "World State")
    float TimeBetweenChecks = 30.0f;
    
    UPROPERTY(EditAnywhere, Category = "World State")
    float EventDuration = 0.0f; // 0 means permanent until changed
    
    UPROPERTY(EditAnywhere, Category = "World State")
    TArray<FName> QuestsTriggered;
    
    UPROPERTY(EditAnywhere, Category = "World State")
    TArray<FName> NextPossibleEvents;
    
    UPROPERTY(EditAnywhere, Category = "World State")
    bool bCyclical = false;
    
private:
    TStateTreeExternalDataHandle<class UWorldStateComponent> WorldStateHandle;
    TStateTreeExternalDataHandle<class UQuestDataComponent> QuestDataHandle;
    
    float CheckTimer = 0.0f;
    float EventTimer = 0.0f;
    bool bEventActive = false;
};