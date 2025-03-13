#pragma once

#include "CoreMinimal.h"

#include "StateTreeTypes.h"
#include "StateTreeNodeBase.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeConditionBase.h"
#include "CraftingTreeNode.generated.h"

USTRUCT(BlueprintType)
struct FCraftingIngredient
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ItemID;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Quantity = 1;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bConsumed = true;
};

USTRUCT(BlueprintType)
struct FCraftingResult
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ItemID;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Quantity = 1;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseSuccessChance = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName QualityDeterminant;
};

USTRUCT()
struct RPGSTATEMACHINES_API FCraftingTreeNode : public FStateTreeNodeBase
{
    GENERATED_BODY()
    
public:
    FCraftingTreeNode();
    
    virtual bool Link(FStateTreeLinker& Linker) override;
    //virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
    //virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
    
    UPROPERTY(EditAnywhere, Category = "Crafting")
    FName RecipeID;
    
    UPROPERTY(EditAnywhere, Category = "Crafting")
    FText RecipeName;
    
    UPROPERTY(EditAnywhere, Category = "Crafting")
    FName CraftingSkillRequired;
    
    UPROPERTY(EditAnywhere, Category = "Crafting")
    int32 SkillLevelRequired = 1;
    
    UPROPERTY(EditAnywhere, Category = "Crafting")
    TArray<FCraftingIngredient> Ingredients;
    
    UPROPERTY(EditAnywhere, Category = "Crafting")
    FCraftingResult PrimaryResult;
    
    UPROPERTY(EditAnywhere, Category = "Crafting")
    TArray<FCraftingResult> BonusResults;
    
    UPROPERTY(EditAnywhere, Category = "Crafting")
    float CraftingTime = 2.0f;
    
    UPROPERTY(EditAnywhere, Category = "Crafting")
    TArray<FName> UnlockedRecipes;
    
    UPROPERTY(EditAnywhere, Category = "Crafting")
    bool bExperimental = false;
    
    UPROPERTY(EditAnywhere, Category = "Crafting")
    TArray<FName> PotentialDiscoveries;
    
private:
    TStateTreeExternalDataHandle<class UPlayerSkillComponent> SkillsHandle;
    TStateTreeExternalDataHandle<class UInventoryComponent> InventoryHandle;
    TStateTreeExternalDataHandle<class UCraftingComponent> CraftingHandle;
    
    float GetSuccessChance(const FStateTreeExecutionContext& Context) const;
    int32 CalculateResultQuality(const FStateTreeExecutionContext& Context) const;
};