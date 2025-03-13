
#include "StateTreeLinker.h"

#include "CraftingTreeNode.h"
#include "PlayerSkillComponent.h"
#include "InventoryComponent.h"
#include "CraftingComponent.h"

FCraftingTreeNode::FCraftingTreeNode()
{
    // Set default values
}

bool FCraftingTreeNode::Link(FStateTreeLinker& Linker)
{
    Linker.LinkExternalData(SkillsHandle);
    Linker.LinkExternalData(InventoryHandle);
    Linker.LinkExternalData(CraftingHandle);
    return true;
}

//EStateTreeRunStatus FCraftingTreeNode::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
//{
//    UCraftingComponent* Crafting = Context.GetExternalData(CraftingHandle);
//    
//    if (!Crafting)
//    {
//        return EStateTreeRunStatus::Failed;
//    }
//    
//    // Set active recipe
//    Crafting->SetActiveRecipe(RecipeID);
//    
//    // Check if recipe is known
//    if (!Crafting->IsRecipeKnown(RecipeID))
//    {
//        return EStateTreeRunStatus::Failed;
//    }
//    
//    // Check if player has required skill level
//    const UPlayerSkillComponent* Skills = Context.GetExternalData(SkillsHandle);
//    if (Skills)
//    {
//        int32 PlayerSkillLevel = Skills->GetSkillLevel(CraftingSkillRequired);
//        if (PlayerSkillLevel < SkillLevelRequired)
//        {
//            return EStateTreeRunStatus::Failed;
//        }
//    }
//    
//    // Check if player has required ingredients
//    const UInventoryComponent* Inventory = Context.GetExternalData(InventoryHandle);
//    if (Inventory)
//    {
//        for (const FCraftingIngredient& Ingredient : Ingredients)
//        {
//            int32 PlayerQuantity = Inventory->GetItemQuantity(Ingredient.ItemID);
//            if (PlayerQuantity < Ingredient.Quantity)
//            {
//                return EStateTreeRunStatus::Failed;
//            }
//        }
//    }
//    
//    // Reset crafting timer
//    Crafting->StartCrafting(CraftingTime);
//    
//    return EStateTreeRunStatus::Running;
//}

//EStateTreeRunStatus FCraftingTreeNode::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
//{
//    UCraftingComponent* Crafting = Context.GetExternalData(CraftingHandle);
//    UInventoryComponent* Inventory = Context.GetExternalData(InventoryHandle);
//    UPlayerSkillComponent* Skills = Context.GetExternalData(SkillsHandle);
//    
//    if (!Crafting || !Inventory)
//    {
//        return EStateTreeRunStatus::Failed;
//    }
//    
//    // Check if crafting is complete
//    if (Crafting->IsCraftingComplete())
//    {
//        // Consume ingredients
//        for (const FCraftingIngredient& Ingredient : Ingredients)
//        {
//            if (Ingredient.bConsumed)
//            {
//                Inventory->RemoveItem(Ingredient.ItemID, Ingredient.Quantity);
//            }
//        }
//        
//        // Determine success and quality
//        float SuccessChance = GetSuccessChance(Context);
//        bool bCraftingSuccessful = FMath::FRand() <= SuccessChance;
//        
//        if (bCraftingSuccessful)
//        {
//            // Add primary result to inventory
//            int32 Quality = CalculateResultQuality(Context);
//            Inventory->AddItem(PrimaryResult.ItemID, PrimaryResult.Quantity, Quality);
//            
//            // Chance for bonus results
//            for (const FCraftingResult& Bonus : BonusResults)
//            {
//                if (FMath::FRand() <= Bonus.BaseSuccessChance)
//                {
//                    Inventory->AddItem(Bonus.ItemID, Bonus.Quantity);
//                }
//            }
//            
//            // Unlock new recipes
//            for (const FName& UnlockedRecipe : UnlockedRecipes)
//            {
//                Crafting->UnlockRecipe(UnlockedRecipe);
//            }
//            
//            // Experimental crafting can discover new recipes
//            if (bExperimental && FMath::FRand() <= 0.2f)
//            {
//                int32 RandomIndex = FMath::RandRange(0, PotentialDiscoveries.Num() - 1);
//                if (PotentialDiscoveries.IsValidIndex(RandomIndex))
//                {
//                    Crafting->UnlockRecipe(PotentialDiscoveries[RandomIndex]);
//                    Crafting->NotifyRecipeDiscovered(PotentialDiscoveries[RandomIndex]);
//                }
//            }
//            
//            // Grant crafting experience
//            if (Skills)
//            {
//                float ExpGain = 10.0f * SkillLevelRequired;
//                Skills->AddExperience(CraftingSkillRequired, ExpGain);
//            }
//        }
//        else
//        {
//            // Failed crafting - potential partial ingredient return
//            for (const FCraftingIngredient& Ingredient : Ingredients)
//            {
//                if (FMath::FRand() <= 0.5f)
//                {
//                    int32 ReturnAmount = FMath::RandRange(1, Ingredient.Quantity / 2);
//                    if (ReturnAmount > 0)
//                    {
//                        Inventory->AddItem(Ingredient.ItemID, ReturnAmount);
//                    }
//                }
//            }
//            
//            // Still gain some experience from failure
//            if (Skills)
//            {
//                float ExpGain = 3.0f * SkillLevelRequired;
//                Skills->AddExperience(CraftingSkillRequired, ExpGain);
//            }
//        }
//        
//        Crafting->NotifyCraftingComplete(bCraftingSuccessful);
//        return EStateTreeRunStatus::Succeeded;
//    }
//    
//    return EStateTreeRunStatus::Running;
//}

float FCraftingTreeNode::GetSuccessChance(const FStateTreeExecutionContext& Context) const
{
    const UPlayerSkillComponent Skills = Context.GetExternalData(SkillsHandle);
    if (!IsValid(&Skills))
    {
        return PrimaryResult.BaseSuccessChance;
    }
    
    int32 PlayerSkillLevel = Skills.GetSkillLevel(CraftingSkillRequired);
    int32 SkillDifference = PlayerSkillLevel - SkillLevelRequired;
    
    // Base success chance adjusted by skill differential
    float SuccessChance = PrimaryResult.BaseSuccessChance + (SkillDifference * 0.05f);
    
    // Clamp between 0.05 (5%) and 0.95 (95%)
    return FMath::Clamp(SuccessChance, 0.05f, 0.95f);
}

int32 FCraftingTreeNode::CalculateResultQuality(const FStateTreeExecutionContext& Context) const
{
    const UPlayerSkillComponent Skills = Context.GetExternalData(SkillsHandle);
    if (!Skills || PrimaryResult.QualityDeterminant.IsNone())
    {
        return 1; // Normal quality
    }
    
    int32 RelevantSkill = Skills.GetSkillLevel(PrimaryResult.QualityDeterminant);

	// Calculate quality based on relevant skill
	if (RelevantSkill >= SkillLevelRequired + 20)
	{
		return 5; // Legendary quality
	}
	else if (RelevantSkill >= SkillLevelRequired + 15)
	{
		return 4; // Epic quality
	}
	else if (RelevantSkill >= SkillLevelRequired + 10)
	{
		return 3; // Rare quality
	}
	else if (RelevantSkill >= SkillLevelRequired + 5)
	{
		return 2; // Uncommon quality
	}
	else
	{
		return 1; // Common quality
	}
}