// InventoryUIWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ResourceData.h"
#include "InventoryComponent.h"
#include "InventoryUIWidget.generated.h"

UCLASS()
class OATH_API UInventoryUIWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UInventoryUIWidget(const FObjectInitializer& ObjectInitializer);
    
    virtual void NativeConstruct() override;
    
    // Inventory update methods
    UFUNCTION(BlueprintCallable, Category = "UI|Inventory")
    void UpdateGoldDisplay(int32 CurrentGold);
    
    UFUNCTION(BlueprintCallable, Category = "UI|Inventory")
    void UpdateMaterialsList(const TMap<FResourceData, int32>& Materials);
    
    UFUNCTION(BlueprintCallable, Category = "UI|Inventory")
    void UpdateItemsList(const TArray<FLootItem>& Items);
    
    // Item interaction methods
    UFUNCTION(BlueprintCallable, Category = "UI|Inventory")
    void SelectItem(const FLootItem& Item);
    
    UFUNCTION(BlueprintCallable, Category = "UI|Inventory")
    void UseSelectedItem();
    
    UFUNCTION(BlueprintCallable, Category = "UI|Inventory")
    void DropSelectedItem();
    
    UFUNCTION(BlueprintCallable, Category = "UI|Inventory")
    void EquipSelectedItem();
    
    UFUNCTION(BlueprintCallable, Category = "UI|Inventory")
    void ViewItemDetails(const FLootItem& Item);
    
    // Filter methods
    UFUNCTION(BlueprintCallable, Category = "UI|Inventory")
    void SetCategoryFilter(const FString& Category);
    
    UFUNCTION(BlueprintCallable, Category = "UI|Inventory")
    void SetRarityFilter(EResourceRarity MinRarity);
    
    UFUNCTION(BlueprintCallable, Category = "UI|Inventory")
    void SortInventoryBy(const FString& SortCriterion, bool bAscending);
    
    // Blueprint implementable events
    UFUNCTION(BlueprintImplementableEvent, Category = "UI|Inventory")
    void OnGoldUpdated(int32 CurrentGold);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "UI|Inventory")
    void OnMaterialsUpdated(const TMap<FResourceData, int32>& Materials);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "UI|Inventory")
    void OnItemsUpdated(const TArray<FLootItem>& Items);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "UI|Inventory")
    void OnItemSelected(const FLootItem& Item);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "UI|Inventory")
    void OnItemDetailsRequested(const FLootItem& Item);
    
private:
    // Reference to the player's inventory component
    UPROPERTY()
    UInventoryComponent* InventoryComponent;
    
    // Current selected item
    UPROPERTY()
    FLootItem SelectedItem;
    
    // Filter settings
    UPROPERTY()
    FString CurrentCategoryFilter;
    
    UPROPERTY()
    EResourceRarity CurrentRarityFilter;
    
    UPROPERTY()
    FString CurrentSortCriterion;
    
    UPROPERTY()
    bool bSortAscending;
    
    // Helper methods
    void FindInventoryComponent();
    void RefreshInventoryDisplay();
    TArray<FLootItem> GetFilteredAndSortedItems() const;
};