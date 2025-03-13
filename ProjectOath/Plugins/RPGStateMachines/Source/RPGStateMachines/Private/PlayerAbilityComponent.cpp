#include "PlayerAbilityComponent.h"

UPlayerAbilityComponent::UPlayerAbilityComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerAbilityComponent::BeginPlay()
{
    Super::BeginPlay();
}

bool UPlayerAbilityComponent::HasAbility(FName AbilityID) const
{
    return UnlockedAbilities.Contains(AbilityID);
}

void UPlayerAbilityComponent::UnlockAbility(FName AbilityID)
{
    if (!HasAbility(AbilityID))
    {
        UnlockedAbilities.Add(AbilityID);
        UE_LOG(LogTemp, Log, TEXT("Player unlocked new ability: %s"), *AbilityID.ToString());
    }
}