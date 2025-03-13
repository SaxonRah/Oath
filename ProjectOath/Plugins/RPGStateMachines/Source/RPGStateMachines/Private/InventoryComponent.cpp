#include "InventoryComponent.h"

UInventoryComponent::UInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::BeginPlay()
{
    Super::BeginPlay();
}

int32 UInventoryComponent::GetItemQuantity(FName ItemID) const
{
    if (const int32* Quantity = Items.Find(ItemID))
    {
        return *Quantity;
    }
    return 0;
}

void UInventoryComponent::AddItem(FName ItemID, int32 Quantity, int32 Quality)
{
    if (Quantity <= 0)
    {
        return;
    }
    
    int32 CurrentQuantity = GetItemQuantity(ItemID);
    Items.Add(ItemID, CurrentQuantity + Quantity);
    
    // Store the highest quality version of the item
    int32 CurrentQuality = 1;
    if (const int32* ExistingQuality = ItemQualities.Find(ItemID))
    {
        CurrentQuality = *ExistingQuality;
    }
    
    ItemQualities.Add(ItemID, FMath::Max(CurrentQuality, Quality));
}

bool UInventoryComponent::RemoveItem(FName ItemID, int32 Quantity)
{
    if (Quantity <= 0)
    {
        return true;
    }
    
    int32 CurrentQuantity = GetItemQuantity(ItemID);
    
    if (CurrentQuantity < Quantity)
    {
        return false;
    }
    
    Items.Add(ItemID, CurrentQuantity - Quantity);
    
    if (CurrentQuantity - Quantity <= 0)
    {
        Items.Remove(ItemID);
        ItemQualities.Remove(ItemID);
    }
    
    return true;
}