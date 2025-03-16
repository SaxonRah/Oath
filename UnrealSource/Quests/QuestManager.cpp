// QuestManager.cpp
#include "QuestManager.h"
#include "KingdomManager.h"

UQuestManager::UQuestManager()
{
}

void UQuestManager::AddAvailableQuest(UQuest* Quest)
{
    if (Quest && Quest->Status == EQuestStatus::Available)
    {
        AvailableQuests.Add(Quest);
    }
}

bool UQuestManager::AcceptQuest(UQuest* Quest)
{
    if (Quest && Quest->Status == EQuestStatus::Available)
    {
        Quest->Status = EQuestStatus::InProgress;
        MoveQuestBetweenLists(Quest, AvailableQuests, ActiveQuests);
        OnQuestAccepted.Broadcast(Quest);
        return true;
    }
    return false;
}

void UQuestManager::CompleteQuest(UQuest* Quest)
{
    if (Quest && (Quest->Status == EQuestStatus::InProgress || Quest->Status == EQuestStatus::Completed))
    {
        // Call the quest's complete method to award rewards
        Quest->CompleteQuest();
        
        // Move from active to completed list
        MoveQuestBetweenLists(Quest, ActiveQuests, CompletedQuests);
        
        // Remove from follower assignments if assigned
        for (auto It = FollowerAssignedQuests.CreateIterator(); It; ++It)
        {
            if (It.Value() == Quest)
            {
                FollowerQuestProgress.Remove(It.Key());
                It.RemoveCurrent();
                break;
            }
        }
        
        OnQuestCompleted.Broadcast(Quest);
    }
}

void UQuestManager::FailQuest(UQuest* Quest)
{
    if (Quest && Quest->Status == EQuestStatus::InProgress)
    {
        // Call the quest's fail method to apply consequences
        Quest->FailQuest();
        
        // Move from active to failed list
        MoveQuestBetweenLists(Quest, ActiveQuests, FailedQuests);
        
        // Remove from follower assignments if assigned
        for (auto It = FollowerAssignedQuests.CreateIterator(); It; ++It)
        {
            if (It.Value() == Quest)
            {
                FollowerQuestProgress.Remove(It.Key());
                It.RemoveCurrent();
                break;
            }
        }
        
        OnQuestFailed.Broadcast(Quest);
    }
}

void UQuestManager::GenerateRandomQuests(int32 Count, int32 MinDifficulty, int32 MaxDifficulty, UProceduralGenerator* Generator)
{
    if (Generator)
    {
        for (int32 i = 0; i < Count; ++i)
        {
            int32 Difficulty = FMath::RandRange(MinDifficulty, MaxDifficulty);
            FString FactionName = ""; // Could randomize this
            EQuestType Type = static_cast<EQuestType>(FMath::RandRange(0, static_cast<int32>(EQuestType::Mystery)));
            
            UQuest* NewQuest = Generator->GenerateQuest(Difficulty, FactionName, Type);
            if (NewQuest)
            {
                AddAvailableQuest(NewQuest);
            }
        }
    }
}

TArray<UQuest*> UQuestManager::GetQuestsByFaction(FString FactionName)
{
    TArray<UQuest*> Result;
    
    // Check available quests
    for (UQuest* Quest : AvailableQuests)
    {
        if (Quest && Quest->FactionName == FactionName)
        {
            Result.Add(Quest);
        }
    }
    
    // Check active quests
    for (UQuest* Quest : ActiveQuests)
    {
        if (Quest && Quest->FactionName == FactionName)
        {
            Result.Add(Quest);
        }
    }
    
    return Result;
}

TArray<UQuest*> UQuestManager::GetQuestsByType(EQuestType QuestType)
{
    TArray<UQuest*> Result;
    
    // Check available quests
    for (UQuest* Quest : AvailableQuests)
    {
        if (Quest && Quest->Type == QuestType)
        {
            Result.Add(Quest);
        }
    }
    
    // Check active quests
    for (UQuest* Quest : ActiveQuests)
    {
        if (Quest && Quest->Type == QuestType)
        {
            Result.Add(Quest);
        }
    }
    
    return Result;
}

void UQuestManager::AssignQuestToFollower(UQuest* Quest, FString FollowerName)
{
    if (Quest && !FollowerName.IsEmpty() && Quest->bCanBeAssignedToFollower && 
        Quest->Status == EQuestStatus::InProgress && !FollowerAssignedQuests.Contains(FollowerName))
    {
        FollowerAssignedQuests.Add(FollowerName, Quest);
        FollowerQuestProgress.Add(FollowerName, 0.0f);
    }
}

void UQuestManager::UpdateAssignedQuests(float DeltaTime)
{
    // Process follower progress on assigned quests
    for (auto It = FollowerAssignedQuests.CreateIterator(); It; ++It)
    {
        FString FollowerName = It.Key();
        UQuest* Quest = It.Value();
        
        if (Quest && Quest->Status == EQuestStatus::InProgress)
        {
            // Update progress
            float& Progress = FollowerQuestProgress[FollowerName];
            
            // Calculate progress speed based on follower's stats and quest difficulty
            // This is just a placeholder - you'd likely get these values from actual follower data
            float ProgressSpeed = 0.1f / Quest->DifficultyLevel;
            
            Progress += ProgressSpeed * DeltaTime;
            
            // Check if quest is complete
            if (Progress >= 1.0f)
            {
                // Complete all objectives
                for (int32 i = 0; i < Quest->Objectives.Num(); ++i)
                {
                    FQuestObjective& Objective = Quest->Objectives[i];
                    Objective.CurrentProgress = Objective.RequiredProgress;
                    Objective.bIsCompleted = true;
                }
                
                Quest->Status = EQuestStatus::Completed;
                CompleteQuest(Quest);
            }
        }
    }
}

void UQuestManager::MoveQuestBetweenLists(UQuest* Quest, TArray<UQuest*>& SourceList, TArray<UQuest*>& DestinationList)
{
    if (Quest)
    {
        SourceList.Remove(Quest);
        DestinationList.Add(Quest);
    }
}