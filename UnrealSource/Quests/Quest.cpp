// Quest.cpp
#include "Quest.h"
#include "OathCharacter.h"
#include "KingdomManager.h"

UQuest::UQuest()
{
    Status = EQuestStatus::Available;
    DifficultyLevel = 1;
    QuestRenownReward = 10.0f;
    GoldReward = 50;
    bCanBeAssignedToFollower = false;
}

void UQuest::UpdateObjective(int32 ObjectiveIndex, int32 Progress)
{
    if (ObjectiveIndex >= 0 && ObjectiveIndex < Objectives.Num())
    {
        FQuestObjective& Objective = Objectives[ObjectiveIndex];
        Objective.CurrentProgress += Progress;
        
        // Clamp progress to required amount
        Objective.CurrentProgress = FMath::Min(Objective.CurrentProgress, Objective.RequiredProgress);
        
        // Check if objective is completed
        if (Objective.CurrentProgress >= Objective.RequiredProgress)
        {
            Objective.bIsCompleted = true;
            Objective.CurrentProgress = Objective.RequiredProgress;
            
            // Check if all objectives are complete
            if (AreAllObjectivesComplete())
            {
                // Don't auto-complete the quest, let the player turn it in
                Status = EQuestStatus::Completed;
            }
        }
    }
}

bool UQuest::AreAllObjectivesComplete()
{
    for (const FQuestObjective& Objective : Objectives)
    {
        if (!Objective.bIsCompleted)
        {
            return false;
        }
    }
    return true;
}

void UQuest::CompleteQuest()
{
    if (Status == EQuestStatus::InProgress || Status == EQuestStatus::Completed)
    {
        Status = EQuestStatus::Completed;
        
        // Award rewards to player
        AOathCharacter* Player = Cast<AOathCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn());
        if (Player)
        {
            // Award quest renown
            if (Player->ReputationComponent)
            {
                Player->ReputationComponent->GainQuestRenown(QuestRenownReward * DifficultyLevel);
                
                // Modify faction reputation if applicable
                if (!FactionName.IsEmpty())
                {
                    Player->ReputationComponent->ModifyFactionReputation(FactionName, FactionReputationReward);
                }
            }
            
            // Award gold
            if (Player->InventoryComponent)
            {
                Player->InventoryComponent->AddGold(GoldReward);
                
                // Award materials
                for (const TPair<FResourceData, int32>& MaterialReward : MaterialRewards)
                {
                    Player->InventoryComponent->AddMaterial(MaterialReward.Key, MaterialReward.Value);
                }
                
                // Award items
                for (const FLootItem& ItemReward : ItemRewards)
                {
                    Player->InventoryComponent->AddItem(ItemReward);
                }
            }
        }
        
        // Notify quest giver if applicable
        // This would likely be implemented through a quest manager or event system
    }
}

void UQuest::FailQuest()
{
    if (Status == EQuestStatus::InProgress)
    {
        Status = EQuestStatus::Failed;
        
        // Apply any consequences for failing the quest
        // This could include negative faction reputation, etc.
        AOathCharacter* Player = Cast<AOathCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn());
        if (Player && Player->ReputationComponent && !FactionName.IsEmpty())
        {
            // Apply small negative reputation penalty for failing
            Player->ReputationComponent->ModifyFactionReputation(FactionName, -FactionReputationReward * 0.5f);
        }
    }
}