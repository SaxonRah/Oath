#pragma once

#include "CoreMinimal.h"

#include "StateTreeTypes.h"
#include "StateTreeNodeBase.h"
#include "StateTreeExecutionContext.h"

#include "QuestTreeNode.generated.h"

UENUM(BlueprintType)
enum class EQuestStatus : uint8
{
    Locked,
    Available,
    InProgress,
    Completed,
    Failed
};

USTRUCT(BlueprintType)
struct FQuestRequirement
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName RequiredSkill;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RequiredLevel = 0;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> RequiredQuestsCompleted;
};


USTRUCT()
struct RPGSTATEMACHINES_API FQuestTreeNode : public FStateTreeNodeBase
{
    GENERATED_BODY()
    
public:
    FQuestTreeNode();
    
    virtual bool Link(FStateTreeLinker& Linker) override;
    //virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
    //virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
    
    UPROPERTY(EditAnywhere, Category = "Quest")
    FName QuestID;
    
    UPROPERTY(EditAnywhere, Category = "Quest")
    FText QuestTitle;
    
    UPROPERTY(EditAnywhere, Category = "Quest", meta = (MultiLine = true))
    FText QuestDescription;
    
    UPROPERTY(EditAnywhere, Category = "Quest")
    FQuestRequirement Requirements;
    
    UPROPERTY(EditAnywhere, Category = "Quest")
    TArray<FName> SubQuestIDs;
    
    UPROPERTY(EditAnywhere, Category = "Quest")
    TArray<FName> RewardIDs;
    
private:
    TStateTreeExternalDataHandle<class UPlayerSkillComponent> SkillsHandle;
    TStateTreeExternalDataHandle<class UQuestDataComponent> QuestDataHandle;
};