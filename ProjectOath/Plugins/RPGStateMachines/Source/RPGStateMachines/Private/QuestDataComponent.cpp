#include "QuestDataComponent.h"

UQuestDataComponent::UQuestDataComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UQuestDataComponent::BeginPlay()
{
    Super::BeginPlay();
}

EQuestStatus UQuestDataComponent::GetQuestStatus(FName QuestID) const
{
    if (const EQuestStatus* Status = QuestStatuses.Find(QuestID))
    {
        return *Status;
    }
    return EQuestStatus::Locked;
}

void UQuestDataComponent::SetQuestStatus(FName QuestID, EQuestStatus Status)
{
    QuestStatuses.Add(QuestID, Status);
}

void UQuestDataComponent::GiveQuestRewards(const TArray<FName>& RewardIDs)
{
    // In a full implementation, this would distribute rewards to the player
    // For now, we'll just log the rewards
    for (const FName& RewardID : RewardIDs)
    {
        UE_LOG(LogTemp, Log, TEXT("Giving quest reward: %s"), *RewardID.ToString());
    }
}