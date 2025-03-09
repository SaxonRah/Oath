// InventoryComponent.cpp
#include "InventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

UInventoryComponent::UInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize default values
    Gold = 0;
    bReplicates = true;
}

void UInventoryComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(UInventoryComponent, Gold);
    DOREPLIFETIME(UInventoryComponent, Materials);
    DOREPLIFETIME(UInventoryComponent, Items);
}

void UInventoryComponent::AddGold(int32 Amount, bool bNotify)
{
    if (Amount <= 0)
    {
        return;
    }
    
    int32 OldGold = Gold;
    Gold += Amount;
    
    if (bNotify)
    {
        FString ResourceType = TEXT("Gold");
        OnInventoryChanged.Broadcast(ResourceType, nullptr, Gold);
        
        // Notify local UI if owned by player
        if (GetOwnerRole() == ROLE_Authority)
        {
            APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
            if (PC && PC->GetPawn() == GetOwner())
            {
                ClientNotifyGoldChanged(OldGold, Gold);
            }
        }
    }
}

bool UInventoryComponent::SpendGold(int32 Amount, bool bNotify)
{
    if (Amount <= 0 || Gold < Amount)
    {
        return false;
    }
    
    int32 OldGold = Gold;
    Gold -= Amount;
    
    if (bNotify)
    {
        FString ResourceType = TEXT("Gold");
        OnInventoryChanged.Broadcast(ResourceType, nullptr, Gold);
        
        // Notify local UI if owned by player
        if (GetOwnerRole() == ROLE_Authority)
        {
            APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
            if (PC && PC->GetPawn() == GetOwner())
            {
                ClientNotifyGoldChanged(OldGold, Gold);
            }
        }
    }
    
    return true;
}

void UInventoryComponent::AddMaterial(FResourceData Material, int32 Amount, bool bNotify)
{
    if (Amount <= 0)
    {
        return;
    }
    
    int32 OldAmount = 0;
    if (Materials.Contains(Material))
    {
        OldAmount = Materials[Material];
        Materials[Material] += Amount;
    }
    else
    {
        Materials.Add(Material, Amount);
    }
    
    if (bNotify)
    {
        FString ResourceType = TEXT("Material");
        UResourceDataAsset* ResourceDataAsset = NewObject<UResourceDataAsset>();
        ResourceDataAsset->ResourceData = Material;
        OnInventoryChanged.Broadcast(ResourceType, ResourceDataAsset, Materials[Material]);
        
        // Notify local UI if owned by player
        if (GetOwnerRole() == ROLE_Authority)
        {
            APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
            if (PC && PC->GetPawn() == GetOwner())
            {
                ClientNotifyMaterialChanged(Material, OldAmount, Materials[Material]);
            }
        }
    }
}

bool UInventoryComponent::SpendMaterial(FResourceData Material, int32 Amount, bool bNotify)
{
    if (Amount <= 0 || !Materials.Contains(Material) || Materials[Material] < Amount)
    {
        return false;
    }
    
    int32 OldAmount = Materials[Material];
    Materials[Material] -= Amount;
    
    // Remove entry if amount becomes zero
    if (Materials[Material] <= 0)
    {
        Materials.Remove(Material);
    }
    
    if (bNotify)
    {
        FString ResourceType = TEXT("Material");
        UResourceDataAsset* ResourceDataAsset = NewObject<UResourceDataAsset>();
        ResourceDataAsset->ResourceData = Material;
        
        int32 NewAmount = Materials.Contains(Material) ? Materials[Material] : 0;
        OnInventoryChanged.Broadcast(ResourceType, ResourceDataAsset, NewAmount);
        
        // Notify local UI if owned by player
        if (GetOwnerRole() == ROLE_Authority)
        {
            APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
            if (PC && PC->GetPawn() == GetOwner())
            {
                ClientNotifyMaterialChanged(Material, OldAmount, NewAmount);
            }
        }
    }
    
    return true;
}

void UInventoryComponent::AddItem(FLootItem Item, bool bNotify)
{
    // Check if item already exists (by name or other unique identifier)
    int32 ExistingIndex = -1;
    for (int32 i = 0; i < Items.Num(); i++)
    {
        if (Items[i].Name == Item.Name)
        {
            ExistingIndex = i;
            break;
        }
    }
    
    if (ExistingIndex != -1)
    {
        // If the item is stackable, you might increase count here
        // For this example, we'll just replace it
        Items[ExistingIndex] = Item;
    }
    else
    {
        Items.Add(Item);
    }
    
    if (bNotify)
    {
        FString ResourceType = TEXT("Item");
        ULootItemDataAsset* ItemDataAsset = NewObject<ULootItemDataAsset>();
        ItemDataAsset->LootItem = Item;
        OnInventoryChanged.Broadcast(ResourceType, ItemDataAsset, 1);
        
        // Notify local UI if owned by player
        if (GetOwnerRole() == ROLE_Authority)
        {
            APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
            if (PC && PC->GetPawn() == GetOwner())
            {
                ClientNotifyItemAdded(Item);
            }
        }
    }
}

bool UInventoryComponent::RemoveItem(FLootItem Item, bool bNotify)
{
    int32 ExistingIndex = -1;
    for (int32 i = 0; i < Items.Num(); i++)
    {
        if (Items[i].Name == Item.Name)
        {
            ExistingIndex = i;
            break;
        }
    }
    
    if (ExistingIndex == -1)
    {
        return false;
    }
    
    FLootItem RemovedItem = Items[ExistingIndex];
    Items.RemoveAt(ExistingIndex);
    
    if (bNotify)
    {
        FString ResourceType = TEXT("Item");
        ULootItemDataAsset* ItemDataAsset = NewObject<ULootItemDataAsset>();
        ItemDataAsset->LootItem = RemovedItem;
        OnInventoryChanged.Broadcast(ResourceType, ItemDataAsset, 0);
        
        // Notify local UI if owned by player
        if (GetOwnerRole() == ROLE_Authority)
        {
            APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
            if (PC && PC->GetPawn() == GetOwner())
            {
                ClientNotifyItemRemoved(RemovedItem);
            }
        }
    }
    
    return true;
}

void UInventoryComponent::RegisterInventoryChangedDelegate(const FOnInventoryChangedDelegate& Delegate)
{
    OnInventoryChanged.Add(Delegate);
}

void UInventoryComponent::ClientNotifyGoldChanged_Implementation(int32 OldValue, int32 NewValue)
{
    // Update UI or trigger effects on client
    UE_LOG(LogTemp, Display, TEXT("Gold changed from %d to %d"), OldValue, NewValue);
    
    // Here you would call your UI update functions
    // If you have a HUD class, you could get it and call a function on it
    // APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    // if (PC)
    // {
    //     AOathHUD* HUD = Cast<AOathHUD>(PC->GetHUD());
    //     if (HUD)
    //     {
    //         HUD->UpdateGoldDisplay(NewValue);
    //     }
    // }
}

void UInventoryComponent::ClientNotifyMaterialChanged_Implementation(FResourceData Material, int32 OldValue, int32 NewValue)
{
    UE_LOG(LogTemp, Display, TEXT("Material %s changed from %d to %d"), *Material.Name, OldValue, NewValue);
    
    // Update UI or trigger effects on client
}

void UInventoryComponent::ClientNotifyItemAdded_Implementation(FLootItem Item)
{
    UE_LOG(LogTemp, Display, TEXT("Item added: %s"), *Item.Name);
    
    // Update UI or trigger effects on client
}

void UInventoryComponent::ClientNotifyItemRemoved_Implementation(FLootItem Item)
{
    UE_LOG(LogTemp, Display, TEXT("Item removed: %s"), *Item.Name);
    
    // Update UI or trigger effects on client
}