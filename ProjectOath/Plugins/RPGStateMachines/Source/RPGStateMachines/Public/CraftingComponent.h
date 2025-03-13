#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CraftingComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RPGSTATEMACHINES_API UCraftingComponent : public UActorComponent
{
    GENERATED_BODY()

public:    
    UCraftingComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void SetActiveRecipe(FName RecipeID);
    
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    FName GetActiveRecipe() const;
    
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool IsRecipeKnown(FName RecipeID) const;
    
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void UnlockRecipe(FName RecipeID);
    
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void StartCrafting(float CraftingTime);
    
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    bool IsCraftingComplete() const;
    
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void NotifyCraftingComplete(bool bSuccess);
    
    UFUNCTION(BlueprintCallable, Category = "Crafting")
    void NotifyRecipeDiscovered(FName RecipeID);
    
private:
    UPROPERTY(VisibleAnywhere, Category = "Crafting")
    TSet<FName> KnownRecipes;
    
    UPROPERTY(VisibleAnywhere, Category = "Crafting")
    FName ActiveRecipeID;
    
    UPROPERTY(VisibleAnywhere, Category = "Crafting")
    float CraftingTimer;
    
    UPROPERTY(VisibleAnywhere, Category = "Crafting")
    float CraftingDuration;
    
    UPROPERTY(VisibleAnywhere, Category = "Crafting")
    bool bIsCrafting;
};