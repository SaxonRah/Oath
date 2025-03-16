// InventoryUIWidget.cpp
#include "InventoryUIWidget.h"
#include "Kismet/GameplayStatics.h"
#include "OathCharacter.h"

UInventoryUIWidget::UInventoryUIWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // Default filter settings
    CurrentCategoryFilter = "All";
    CurrentRarityFilter = EResourceRarity::Common;
    CurrentSortCriterion = "Name";
    bSortAscending = true;
}

void UInventoryUIWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // Find the player's inventory component
    FindInventoryComponent();
    
    // Initialize the UI with current inventory data
    RefreshInventoryDisplay();
}

void UInventoryUIWidget::FindInventoryComponent()
{
    AOathCharacter* PlayerCharacter = Cast<AOathCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
    if (PlayerCharacter)
    {
        InventoryComponent = PlayerCharacter->InventoryComponent;
        
        // Bind to inventory changes if component exists
        if (InventoryComponent)
        {
            InventoryComponent->OnInventoryChanged.AddDynamic(this, &UInventoryUIWidget::RefreshInventoryDisplay);
        }
    }
}

void UInventoryUIWidget::RefreshInventoryDisplay()
{
    if (!InventoryComponent)
    {
        return;
    }
    
    // Update all parts of the inventory display
    UpdateGoldDisplay(InventoryComponent->Gold);
    UpdateMaterialsList(InventoryComponent->Materials);
    
    // Apply filters and sorting to items
    TArray<FLootItem> FilteredItems = GetFilteredAndSortedItems();
    UpdateItemsList(FilteredItems);
}

void UInventoryUIWidget::UpdateGoldDisplay(int32 CurrentGold)
{
    // Call the blueprint event to update gold display
    OnGoldUpdated(CurrentGold);
}

void UInventoryUIWidget::UpdateMaterialsList(const TMap<FResourceData, int32>& Materials)
{
    // Call the blueprint event to update materials list
    OnMaterialsUpdated(Materials);
}

void UInventoryUIWidget::UpdateItemsList(const TArray<FLootItem>& Items)
{
    // Call the blueprint event to update items list
    OnItemsUpdated(Items);
}

void UInventoryUIWidget::SelectItem(const FLootItem& Item)
{
    SelectedItem = Item;
    
    // Call the blueprint event to update selection UI
    OnItemSelected(Item);
}

void UInventoryUIWidget::UseSelectedItem()
{
    if (SelectedItem.Name.IsEmpty() || !InventoryComponent)
    {
        return;
    }
    
    // Implement use logic based on item type
    // For example, consumables would be used, while equipment would be equipped
    
    // This is a simplified example - actual implementation would depend on item types
    
    // Check if this is a consumable
    // if (IsConsumable(SelectedItem))
    // {
    //     InventoryComponent->UseConsumable(SelectedItem);
    // }
    // else
    // {
    //     // For non-consumables, default to equip
    //     EquipSelectedItem();
    // }
    
    // Refresh display after using the item
    RefreshInventoryDisplay();
}

void UInventoryUIWidget::DropSelectedItem()
{
    if (SelectedItem.Name.IsEmpty() || !InventoryComponent)
    {
        return;
    }
    
    // Remove the item from inventory
    InventoryComponent->RemoveItem(SelectedItem);
    
    // Clear selection
    SelectedItem = FLootItem();
    
    // Refresh display
    RefreshInventoryDisplay();
}

void UInventoryUIWidget::EquipSelectedItem()
{
    if (SelectedItem.Name.IsEmpty() || !InventoryComponent)
    {
        return;
    }
    
    // Placeholder - actual implementation would need to call appropriate methods
    // on the player character to equip the item
    
    AOathCharacter* PlayerCharacter = Cast<AOathCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
    if (PlayerCharacter)
    {
        // PlayerCharacter->EquipItem(SelectedItem);
    }
    
    // Refresh display after equipping
    RefreshInventoryDisplay();
}

void UInventoryUIWidget::ViewItemDetails(const FLootItem& Item)
{
    // Call the blueprint event to show item details
    OnItemDetailsRequested(Item);
}

void UInventoryUIWidget::SetCategoryFilter(const FString& Category)
{
   CurrentCategoryFilter = Category;
   
   // Refresh display with new filter
   RefreshInventoryDisplay();
}

void UInventoryUIWidget::SetRarityFilter(EResourceRarity MinRarity)
{
   CurrentRarityFilter = MinRarity;
   
   // Refresh display with new filter
   RefreshInventoryDisplay();
}

void UInventoryUIWidget::SortInventoryBy(const FString& SortCriterion, bool bAscending)
{
   CurrentSortCriterion = SortCriterion;
   bSortAscending = bAscending;
   
   // Refresh display with new sorting
   RefreshInventoryDisplay();
}

TArray<FLootItem> UInventoryUIWidget::GetFilteredAndSortedItems() const
{
   TArray<FLootItem> FilteredItems;
   
   if (!InventoryComponent)
   {
       return FilteredItems;
   }
   
   // Apply filters
   for (const FLootItem& Item : InventoryComponent->Items)
   {
       // Apply category filter
       if (CurrentCategoryFilter != "All")
       {
           // This is a placeholder - actual implementation would check item type/category
           // if (GetItemCategory(Item) != CurrentCategoryFilter)
           // {
           //     continue;
           // }
       }
       
       // Apply rarity filter
       if (Item.Rarity < CurrentRarityFilter)
       {
           continue;
       }
       
       // Item passed all filters, add it to the result
       FilteredItems.Add(Item);
   }
   
   // Apply sorting
   if (CurrentSortCriterion == "Name")
   {
       FilteredItems.Sort([this](const FLootItem& A, const FLootItem& B) {
           if (bSortAscending)
           {
               return A.Name < B.Name;
           }
           else
           {
               return A.Name > B.Name;
           }
       });
   }
   else if (CurrentSortCriterion == "Value")
   {
       FilteredItems.Sort([this](const FLootItem& A, const FLootItem& B) {
           if (bSortAscending)
           {
               return A.Value < B.Value;
           }
           else
           {
               return A.Value > B.Value;
           }
       });
   }
   else if (CurrentSortCriterion == "Rarity")
   {
       FilteredItems.Sort([this](const FLootItem& A, const FLootItem& B) {
           if (bSortAscending)
           {
               return static_cast<int32>(A.Rarity) < static_cast<int32>(B.Rarity);
           }
           else
           {
               return static_cast<int32>(A.Rarity) > static_cast<int32>(B.Rarity);
           }
       });
   }
   
   return FilteredItems;
}