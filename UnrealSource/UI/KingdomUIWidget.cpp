// KingdomUIWidget.cpp
#include "KingdomUIWidget.h"
#include "Kismet/GameplayStatics.h"
#include "OathGameMode.h"
#include "OathCharacter.h"

UKingdomUIWidget::UKingdomUIWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void UKingdomUIWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // Find the kingdom manager
    FindKingdomManager();
    
    // Initialize UI with current data
    if (KingdomManager)
    {
        UpdateKingdomInfo(
            KingdomManager->KingdomName,
            KingdomManager->CurrentTier,
            KingdomManager->Followers.Num(),
            KingdomManager->Buildings.Num(),
            KingdomManager->DailyIncome
        );
        
        UpdateBuildingsList(KingdomManager->Buildings);
        UpdateFollowersList(KingdomManager->Followers);
        
        // For available buildings, we'd need to filter based on current tier, etc.
        TArray<FBuildingData> AvailableBuildings;
        // Logic to populate available buildings would go here
        UpdateAvailableBuildings(AvailableBuildings);
        
        // Update resource info
        TMap<FString, float> Production;
        TMap<FString, float> Consumption;
        // Logic to populate production/consumption would go here
        UpdateResourceProduction(Production);
        UpdateResourceConsumption(Consumption);
    }
}

void UKingdomUIWidget::FindKingdomManager()
{
    AOathGameMode* GameMode = Cast<AOathGameMode>(UGameplayStatics::GetGameMode(this));
    if (GameMode)
    {
        KingdomManager = GameMode->KingdomManager;
    }
}

void UKingdomUIWidget::UpdateKingdomInfo(const FString& KingdomName, EKingdomTier KingdomTier, 
                                        int32 FollowerCount, int32 BuildingCount, float DailyIncome)
{
    // Call the blueprint event to update UI
    OnKingdomInfoUpdated(KingdomName, KingdomTier, FollowerCount, BuildingCount, DailyIncome);
}

void UKingdomUIWidget::UpdateBuildingsList(const TArray<FBuildingData>& Buildings)
{
    // Call the blueprint event to update buildings list
    OnBuildingsListUpdated(Buildings);
}

void UKingdomUIWidget::UpdateAvailableBuildings(const TArray<FBuildingData>& AvailableBuildings)
{
    // Call the blueprint event to update available buildings list
    OnAvailableBuildingsUpdated(AvailableBuildings);
}

void UKingdomUIWidget::SelectBuilding(const FBuildingData& BuildingData)
{
    SelectedBuilding = BuildingData;
    OnBuildingSelected(BuildingData);
}

bool UKingdomUIWidget::CanBuildSelected() const
{
    // Check if we have a valid building selected
    if (SelectedBuilding.Name.IsEmpty())
    {
        return false;
    }
    
    // Check if the player has sufficient resources
    return HasSufficientResources(SelectedBuilding);
}

void UKingdomUIWidget::ConstructSelectedBuilding()
{
    if (!CanBuildSelected() || !KingdomManager)
    {
        return;
    }
    
    // In a real implementation, we'd need to get a position to place the building
    // This could come from a placement preview mode in the game
    FVector BuildingLocation = FVector::ZeroVector; // Placeholder
    
    // Attempt to construct the building
    if (KingdomManager->ConstructBuilding(SelectedBuilding, BuildingLocation))
    {
        // Update UI after successful construction
        UpdateKingdomInfo(
            KingdomManager->KingdomName,
            KingdomManager->CurrentTier,
            KingdomManager->Followers.Num(),
            KingdomManager->Buildings.Num(),
            KingdomManager->DailyIncome
        );
        
        UpdateBuildingsList(KingdomManager->Buildings);
        
        // Clear selection
        SelectedBuilding = FBuildingData();
        OnBuildingSelected(SelectedBuilding);
    }
}

void UKingdomUIWidget::UpdateFollowersList(const TArray<FFollowerData>& Followers)
{
    // Call the blueprint event to update followers list
    OnFollowersListUpdated(Followers);
}

void UKingdomUIWidget::SelectFollower(const FFollowerData& FollowerData)
{
    SelectedFollower = FollowerData;
    OnFollowerSelected(FollowerData);
}

void UKingdomUIWidget::AssignFollowerToBuilding(const FFollowerData& Follower, const FBuildingData& Building)
{
    if (!KingdomManager)
    {
        return;
    }
    
    // Placeholder - actual implementation would need to call KingdomManager method
    // KingdomManager->AssignFollowerToBuilding(Follower.Name, Building.Name);
    
    // Update UI after assignment
    UpdateFollowersList(KingdomManager->Followers);
}

void UKingdomUIWidget::AssignFollowerToQuest(const FFollowerData& Follower, UQuest* Quest)
{
    if (!KingdomManager || !Quest)
    {
        return;
    }
    
    // Placeholder - actual implementation would need to call KingdomManager method
    // KingdomManager->AssignFollowerToQuest(Follower.Name, Quest);
    
    // Update UI after assignment
    UpdateFollowersList(KingdomManager->Followers);
}

void UKingdomUIWidget::UpdateResourceProduction(const TMap<FString, float>& DailyProduction)
{
    // Call the blueprint event to update production stats
    OnResourceProductionUpdated(DailyProduction);
}

void UKingdomUIWidget::UpdateResourceConsumption(const TMap<FString, float>& DailyConsumption)
{
    // Call the blueprint event to update consumption stats
    OnResourceConsumptionUpdated(DailyConsumption);
}

void UKingdomUIWidget::AddEventToLog(const FKingdomEvent& Event)
{
    // Call the blueprint event to add an event to the log
    OnEventAdded(Event);
}

void UKingdomUIWidget::ClearEventLog()
{
    // This would be implemented in blueprints
    // The C++ side just needs to provide the interface
}

bool UKingdomUIWidget::HasSufficientResources(const FBuildingData& Building) const
{
    // Get the player character to check resources
    AOathCharacter* PlayerCharacter = Cast<AOathCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
    
    if (!PlayerCharacter || !PlayerCharacter->InventoryComponent)
    {
        return false;
    }
    
    // Check gold cost
    if (PlayerCharacter->InventoryComponent->Gold < Building.GoldConstructionCost)
    {
        return false;
    }
    
    // Check material costs
    for (const TPair<FResourceData, int32>& ResourceCost : Building.ConstructionCost)
    {
        // Find this resource in the inventory
        if (!PlayerCharacter->InventoryComponent->Materials.Contains(ResourceCost.Key) ||
            PlayerCharacter->InventoryComponent->Materials[ResourceCost.Key] < ResourceCost.Value)
        {
            return false;
        }
    }
    
    return true;
}