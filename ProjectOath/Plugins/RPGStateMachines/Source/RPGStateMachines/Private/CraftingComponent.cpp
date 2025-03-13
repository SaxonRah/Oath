#include "CraftingComponent.h"

UCraftingComponent::UCraftingComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    bIsCrafting = false;
    CraftingTimer = 0.0f;
    CraftingDuration = 0.0f;
}

void UCraftingComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UCraftingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsCrafting)
    {
        CraftingTimer += DeltaTime;
    }
}

void UCraftingComponent::SetActiveRecipe(FName RecipeID)
{
    ActiveRecipeID = RecipeID;
}

FName UCraftingComponent::GetActiveRecipe() const
{
    return ActiveRecipeID;
}

bool UCraftingComponent::IsRecipeKnown(FName RecipeID) const
{
    return KnownRecipes.Contains(RecipeID);
}

void UCraftingComponent::UnlockRecipe(FName RecipeID)
{
    if (!IsRecipeKnown(RecipeID))
    {
        KnownRecipes.Add(RecipeID);
    }
}

void UCraftingComponent::StartCrafting(float CraftingTime)
{
    CraftingTimer = 0.0f;
    CraftingDuration = CraftingTime;
    bIsCrafting = true;
}

bool UCraftingComponent::IsCraftingComplete() const
{
    return bIsCrafting && CraftingTimer >= CraftingDuration;
}

void UCraftingComponent::NotifyCraftingComplete(bool bSuccess)
{
    bIsCrafting = false;
    
    if (bSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("Crafting successful for recipe: %s"), *ActiveRecipeID.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Crafting failed for recipe: %s"), *ActiveRecipeID.ToString());
    }
}

void UCraftingComponent::NotifyRecipeDiscovered(FName RecipeID)
{
    UE_LOG(LogTemp, Log, TEXT("New recipe discovered: %s"), *RecipeID.ToString());
}