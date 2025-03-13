#pragma once

#include "CoreMinimal.h"

#include "StateTreeTypes.h"
#include "StateTreeNodeBase.h"
#include "StateTreeExecutionContext.h"

#include "DialogueTreeNode.generated.h"

USTRUCT(BlueprintType)
struct FDialogueOption
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText OptionText;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName NextNodeID;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName RequiredSkill;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RequiredSkillLevel = 0;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName RequiredFaction;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RequiredReputation = 0;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> RequiredKnowledge;
};

USTRUCT()
struct RPGSTATEMACHINES_API FDialogueTreeNode : public FStateTreeNodeBase
{
    GENERATED_BODY()
    
public:
    FDialogueTreeNode();
    
    virtual bool Link(FStateTreeLinker& Linker) override;
    //virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
    //virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
    
    UPROPERTY(EditAnywhere, Category = "Dialogue")
    FName DialogueNodeID;
    
    UPROPERTY(EditAnywhere, Category = "Dialogue")
    FText NPCText;
    
    UPROPERTY(EditAnywhere, Category = "Dialogue")
    TArray<FDialogueOption> DialogueOptions;
    
    UPROPERTY(EditAnywhere, Category = "Dialogue")
    TArray<FName> KnowledgeGained;
    
    UPROPERTY(EditAnywhere, Category = "Dialogue")
    FName QuestTriggered;
    
    UPROPERTY(EditAnywhere, Category = "Dialogue")
    int32 ReputationChangeOnSelect = 0;
    
private:
    TStateTreeExternalDataHandle<class UPlayerSkillComponent> SkillsHandle;
    TStateTreeExternalDataHandle<class UPlayerKnowledgeComponent> KnowledgeHandle;
    TStateTreeExternalDataHandle<class UFactionComponent> FactionHandle;
    TStateTreeExternalDataHandle<class UDialogueManagerComponent> DialogueManagerHandle;
};