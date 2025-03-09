// InventoryComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ResourceData.h"
#include "InventoryComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class OATH_API UInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UInventoryComponent();
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 Gold;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    TMap<FResourceData, int32> Materials;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    TArray<FLootItem> Items;
    
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void AddGold(int32 Amount, bool bNotify = true);
    
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool SpendGold(int32 Amount, bool bNotify = true);
    
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void AddMaterial(FResourceData Material, int32 Amount, bool bNotify = true);
    
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool SpendMaterial(FResourceData Material, int32 Amount, bool bNotify = true);
    
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void AddItem(FLootItem Item, bool bNotify = true);
    
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool RemoveItem(FLootItem Item, bool bNotify = true);
    
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void RegisterInventoryChangedDelegate(const FOnInventoryChangedDelegate& Delegate);

protected:
    virtual void BeginPlay() override;
    
private:
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnInventoryChangedDelegate, FString, ResourceType, UObject*, Resource, int32, NewAmount);
    
    UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
    FOnInventoryChangedDelegate OnInventoryChanged;
};