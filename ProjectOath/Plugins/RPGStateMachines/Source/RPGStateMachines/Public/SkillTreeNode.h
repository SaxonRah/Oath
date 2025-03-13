#pragma once

#include "CoreMinimal.h"

#include "StateTreeTypes.h"
#include "StateTreeNodeBase.h"
#include "StateTreeExecutionContext.h"

#include "SkillTreeNode.generated.h"

USTRUCT(BlueprintType)
struct FSkillPrerequisite
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName PrerequisiteSkill;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RequiredLevel = 1;
};

USTRUCT(BlueprintType)
struct FSkillEffect
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName StatModified;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ValuePerLevel = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsPercentage = false;
};

USTRUCT()
struct RPGSTATEMACHINES_API FSkillTreeNode : public FStateTreeNodeBase
{
    GENERATED_BODY()
    
public:
    FSkillTreeNode();
    
    virtual bool Link(FStateTreeLinker& Linker) override;
    //virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
    //virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
    
    UPROPERTY(EditAnywhere, Category = "Skill")
    FName SkillID;
    
    UPROPERTY(EditAnywhere, Category = "Skill")
    FText SkillName;
    
    UPROPERTY(EditAnywhere, Category = "Skill", meta = (MultiLine = true))
    FText SkillDescription;
    
    UPROPERTY(EditAnywhere, Category = "Skill")
    int32 MaxLevel = 5;
    
    UPROPERTY(EditAnywhere, Category = "Skill")
    TArray<FSkillPrerequisite> Prerequisites;
    
    UPROPERTY(EditAnywhere, Category = "Skill")
    TArray<FSkillEffect> SkillEffects;
    
    UPROPERTY(EditAnywhere, Category = "Skill")
    TArray<FName> UnlockedAbilities;
    
    UPROPERTY(EditAnywhere, Category = "Skill")
    TArray<FName> ChildSkills;
    
    UPROPERTY(EditAnywhere, Category = "Skill")
    int32 SkillPointCost = 1;
    
    UPROPERTY(EditAnywhere, Category = "Skill")
    int32 CharacterLevelRequired = 1;
    
private:
    TStateTreeExternalDataHandle<class UPlayerSkillComponent> SkillsHandle;
    TStateTreeExternalDataHandle<class UPlayerStatsComponent> StatsHandle;
    TStateTreeExternalDataHandle<class UPlayerAbilityComponent> AbilitiesHandle;
};