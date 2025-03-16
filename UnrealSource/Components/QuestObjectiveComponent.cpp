// QuestObjectiveComponent.cpp
#include "QuestObjectiveComponent.h"
#include "QuestManager.h"
#include "OathGameMode.h"
#include "Kismet/GameplayStatics.h"

UQuestObjectiveComponent::UQuestObjectiveComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UQuestObjectiveComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Find the quest manager
    FindQuestManager();
    
    // Register with quest system
    RegisterWithQuestSystem();
}

void UQuestObjectiveComponent::FindQuestManager()
{
    AOathGameMode* GameMode = Cast<AOathGameMode>(UGameplayStatics::GetGameMode(this));
    if (GameMode)
    {
        // Find quest manager component
        for (const auto& Component : GameMode->GetComponents())
        {
            QuestManager = Cast<UQuestManager>(Component);
            if (QuestManager)
            {
                break;
            }
        }
    }
}

void UQuestObjectiveComponent::RegisterWithQuestSystem()
{
    if (!QuestManager)
    {
        return;
    }
    
    // Clear previous associations
    AssociatedQuests.Empty();
    
    // Check active quests to see if we're relevant
    for (UQuest* Quest : QuestManager->ActiveQuests)
    {
        if (IsRelevantForQuest(Quest))
        {
            FString QuestKey = Quest->QuestName;
            if (!AssociatedQuests.Contains(QuestKey))
            {
                AssociatedQuests.Add(QuestKey, TArray<UQuest*>());
            }
            AssociatedQuests[QuestKey].Add(Quest);
        }
    }
    
    // Subscribe to quest accepted events to update our associations
    if (!QuestManager->OnQuestAccepted.IsAlreadyBound(this, &UQuestObjectiveComponent::RegisterWithQuestSystem))
    {
        QuestManager->OnQuestAccepted.AddDynamic(this, &UQuestObjectiveComponent::RegisterWithQuestSystem);
    }
}

void UQuestObjectiveComponent::TriggerObjectiveProgress(FString QuestName, int32 ObjectiveIndex, int32 Progress)
{
    if (!QuestManager || !AssociatedQuests.Contains(QuestName))
    {
        return;
    }
    
    // Update all associated quests with this name
    for (UQuest* Quest : AssociatedQuests[QuestName])
    {
        if (Quest && Quest->Status == EQuestStatus::InProgress)
        {
            Quest->UpdateObjective(ObjectiveIndex, Progress);
        }
    }
}

void UQuestObjectiveComponent::TriggerObjectiveCompletion(FString QuestName, int32 ObjectiveIndex)
{
    if (!QuestManager || !AssociatedQuests.Contains(QuestName))
    {
        return;
    }
    
    // Complete objective for all associated quests with this name
    for (UQuest* Quest : AssociatedQuests[QuestName])
    {
        if (Quest && Quest->Status == EQuestStatus::InProgress && 
            ObjectiveIndex >= 0 && ObjectiveIndex < Quest->Objectives.Num())
        {
            FQuestObjective& Objective = Quest->Objectives[ObjectiveIndex];
            int32 RemainingProgress = Objective.RequiredProgress - Objective.CurrentProgress;
            
            if (RemainingProgress > 0)
            {
                Quest->UpdateObjective(ObjectiveIndex, RemainingProgress);
            }
        }
    }
}

bool UQuestObjectiveComponent::IsRelevantForQuest(UQuest* Quest) const
{
    if (!Quest)
    {
        return false;
    }
    
    // Check if we have an objective ID that matches this quest
    return QuestObjectiveIDs.Contains(Quest->QuestName);
}