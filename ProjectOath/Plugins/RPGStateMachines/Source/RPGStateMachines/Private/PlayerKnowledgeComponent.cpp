#include "PlayerKnowledgeComponent.h"

UPlayerKnowledgeComponent::UPlayerKnowledgeComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerKnowledgeComponent::BeginPlay()
{
    Super::BeginPlay();
}

bool UPlayerKnowledgeComponent::HasKnowledge(FName KnowledgeID) const
{
    return AcquiredKnowledge.Contains(KnowledgeID);
}

void UPlayerKnowledgeComponent::AddKnowledge(FName KnowledgeID)
{
    if (!HasKnowledge(KnowledgeID))
    {
        AcquiredKnowledge.Add(KnowledgeID);
        UE_LOG(LogTemp, Log, TEXT("Player gained new knowledge: %s"), *KnowledgeID.ToString());
    }
}

void UPlayerKnowledgeComponent::RemoveKnowledge(FName KnowledgeID)
{
    if (HasKnowledge(KnowledgeID))
    {
        AcquiredKnowledge.Remove(KnowledgeID);
    }
}