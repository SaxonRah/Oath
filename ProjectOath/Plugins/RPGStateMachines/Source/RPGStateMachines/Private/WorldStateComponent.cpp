#include "WorldStateComponent.h"

UWorldStateComponent::UWorldStateComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UWorldStateComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UWorldStateComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

float UWorldStateComponent::GetWorldVariable(FName VariableName) const
{
    float BaseValue = 0.0f;
    if (const float* FoundValue = WorldVariables.Find(VariableName))
    {
        BaseValue = *FoundValue;
    }
    
    // Apply multiplier if exists
    if (const float* FoundMultiplier = WorldVariableMultipliers.Find(VariableName))
    {
        BaseValue *= *FoundMultiplier;
    }
    
    return BaseValue;
}

void UWorldStateComponent::SetWorldVariable(FName VariableName, float Value)
{
    WorldVariables.Add(VariableName, Value);
    OnWorldVariableChanged.Broadcast(VariableName, GetWorldVariable(VariableName));
}

void UWorldStateComponent::ChangeWorldVariable(FName VariableName, float Delta)
{
    float CurrentValue = 0.0f;
    if (const float* FoundValue = WorldVariables.Find(VariableName))
    {
        CurrentValue = *FoundValue;
    }
    
    WorldVariables.Add(VariableName, CurrentValue + Delta);
    OnWorldVariableChanged.Broadcast(VariableName, GetWorldVariable(VariableName));
}

void UWorldStateComponent::SetWorldVariableMultiplier(FName VariableName, float Multiplier)
{
    WorldVariableMultipliers.Add(VariableName, Multiplier);
    OnWorldVariableChanged.Broadcast(VariableName, GetWorldVariable(VariableName));
}

void UWorldStateComponent::RegisterPossibleEvent(FName EventID)
{
    if (!PossibleEvents.Contains(EventID))
    {
        PossibleEvents.Add(EventID);
    }
}

void UWorldStateComponent::NotifyWorldEventStarted(FName EventID, FText EventName, FText EventDescription, EWorldEventType EventType)
{
    if (!ActiveEvents.Contains(EventID))
    {
        ActiveEvents.Add(EventID);
    }
    
    OnWorldEventStarted.Broadcast(EventID, EventName, EventDescription, EventType);
}

void UWorldStateComponent::NotifyWorldEventEnded(FName EventID)
{
    ActiveEvents.Remove(EventID);
    OnWorldEventEnded.Broadcast(EventID);
}