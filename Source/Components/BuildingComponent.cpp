// BuildingComponent.cpp
#include "BuildingComponent.h"

UBuildingComponent::UBuildingComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Default values
    HealthPoints = 100;
    MaxHealthPoints = 100;
    Efficiency = 1.0f;
    bIsUnderConstruction = true;
    ConstructionProgress = 0.0f;
}

void UBuildingComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UBuildingComponent::SetBuildingData(const FBuildingData& InBuildingData)
{
    BuildingData = InBuildingData;
    
    // Set health based on building size/type
    MaxHealthPoints = 100 * BuildingData.Size;
    HealthPoints = MaxHealthPoints;
    
    // Buildings start under construction
    bIsUnderConstruction = true;
    ConstructionProgress = 0.0f;
    
    // Notify status change
    OnBuildingStatusChanged.Broadcast(bIsUnderConstruction, 1.0f);
}

void UBuildingComponent::TakeDamage(int32 DamageAmount)
{
    // Can't damage buildings under construction
    if (bIsUnderConstruction)
    {
        return;
    }
    
    // Apply damage
    HealthPoints = FMath::Max(0, HealthPoints - DamageAmount);
    
    // Update efficiency based on health
    UpdateEfficiency();
    
    // Notify status change
    float healthPercentage = static_cast<float>(HealthPoints) / static_cast<float>(MaxHealthPoints);
    OnBuildingStatusChanged.Broadcast(bIsUnderConstruction, healthPercentage);
    
    UE_LOG(LogTemp, Log, TEXT("Building %s took %d damage, health: %d/%d"), 
           *BuildingData.Name, DamageAmount, HealthPoints, MaxHealthPoints);
}

void UBuildingComponent::Repair(int32 RepairAmount)
{
    // Can't repair if under construction
    if (bIsUnderConstruction)
    {
        return;
    }
    
    // Apply repair
    HealthPoints = FMath::Min(MaxHealthPoints, HealthPoints + RepairAmount);
    
    // Update efficiency based on health
    UpdateEfficiency();
    
    // Notify status change
    float healthPercentage = static_cast<float>(HealthPoints) / static_cast<float>(MaxHealthPoints);
    OnBuildingStatusChanged.Broadcast(bIsUnderConstruction, healthPercentage);
    
    UE_LOG(LogTemp, Log, TEXT("Building %s repaired by %d points, health: %d/%d"), 
           *BuildingData.Name, RepairAmount, HealthPoints, MaxHealthPoints);
}

bool UBuildingComponent::AssignFollower(AActor* Follower)
{
    // Check if this follower is already assigned
    if (AssignedFollowers.Contains(Follower))
    {
        return false;
    }
    
    // Check if building is at capacity
    if (AssignedFollowers.Num() >= BuildingData.MaxOccupants)
    {
        return false;
    }
    
    // Add follower
    AssignedFollowers.Add(Follower);
    
    // Update efficiency
    UpdateEfficiency();
    
    UE_LOG(LogTemp, Log, TEXT("Follower assigned to building %s, total assigned: %d/%d"), 
           *BuildingData.Name, AssignedFollowers.Num(), BuildingData.MaxOccupants);
    
    return true;
}

bool UBuildingComponent::RemoveFollower(AActor* Follower)
{
    // Check if this follower is assigned
    int32 index = AssignedFollowers.Find(Follower);
    if (index == INDEX_NONE)
    {
        return false;
    }
    
    // Remove follower
    AssignedFollowers.RemoveAt(index);
    
    // Update efficiency
    UpdateEfficiency();
    
    UE_LOG(LogTemp, Log, TEXT("Follower removed from building %s, total assigned: %d/%d"), 
           *BuildingData.Name, AssignedFollowers.Num(), BuildingData.MaxOccupants);
    
    return true;
}

void UBuildingComponent::UpdateEfficiency()
{
    // Base efficiency on health
    float healthEfficiency = static_cast<float>(HealthPoints) / static_cast<float>(MaxHealthPoints);
    
    // Factor in assigned followers
    float staffingEfficiency = 1.0f;
    if (BuildingData.MaxOccupants > 0)
    {
        staffingEfficiency = static_cast<float>(AssignedFollowers.Num()) / static_cast<float>(BuildingData.MaxOccupants);
        
        // Buildings can function at reduced capacity
        staffingEfficiency = FMath::Max(0.25f, staffingEfficiency);
    }
    
    // Calculate final efficiency
    Efficiency = healthEfficiency * staffingEfficiency;
    
    UE_LOG(LogTemp, Verbose, TEXT("Building %s efficiency updated: %.2f (health: %.2f, staffing: %.2f)"), 
           *BuildingData.Name, Efficiency, healthEfficiency, staffingEfficiency);
}

float UBuildingComponent::GetResourceOutput(const FString& ResourceType) const
{
    // Check if building produces this resource
    if (BuildingData.DailyResourceContribution.Contains(ResourceType))
    {
        return BuildingData.DailyResourceContribution[ResourceType] * Efficiency;
    }
    
    return 0.0f;
}

float UBuildingComponent::GetGoldOutput() const
{
    return BuildingData.DailyGoldContribution * Efficiency;
}