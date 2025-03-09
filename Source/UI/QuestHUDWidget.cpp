// QuestHUDWidget.cpp
#include "QuestHUDWidget.h"
#include "QuestManager.h"
#include "OathGameMode.h"
#include "Kismet/GameplayStatics.h"

void UQuestHUDWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    FindQuestManager();
    
    // Register for quest events if the manager is found
    if (QuestManager)
    {
        QuestManager->OnQuestAccepted.AddDynamic(this, &UQuestHUDWidget::OnQuestStatusChanged);
        QuestManager->OnQuestCompleted.AddDynamic(this, &UQuestHUDWidget::OnQuestStatusChanged);
        QuestManager->OnQuestFailed.AddDynamic(this, &UQuestHUDWidget::OnQuestStatusChanged);
    }
}

void UQuestHUDWidget::FindQuestManager()
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

void UQuestHUDWidget::UpdateActiveQuests(const TArray<UQuest*>& Quests)
{
    ActiveQuests = Quests;
    
    // If we have active quests but no selected quest, select the first one
    if (ActiveQuests.Num() > 0 && !SelectedQuest)
    {
        SetSelectedQuest(ActiveQuests[0]);
    }
    // If the selected quest is no longer active, clear it
    else if (SelectedQuest && !ActiveQuests.Contains(SelectedQuest))
    {
        SelectedQuest = nullptr;
        OnSelectedQuestChanged();
    }
    
    OnActiveQuestsUpdated();
}

void UQuestHUDWidget::SetSelectedQuest(UQuest* Quest)
{
    if (Quest && (ActiveQuests.Contains(Quest) || Quest->Status == EQuestStatus::Available))
    {
        SelectedQuest = Quest;
        UpdateQuestObjectives(Quest);
        OnSelectedQuestChanged();
    }
}

void UQuestHUDWidget::UpdateQuestObjectives(UQuest* Quest)
{
    if (Quest)
    {
        OnQuestObjectivesUpdated();
    }
}

void UQuestHUDWidget::DisplayQuestNotification(const FString& QuestName, const FString& Message)
{
    OnQuestNotificationReceived(QuestName, Message);
}

void UQuestHUDWidget::DisplayQuestRewards(UQuest* Quest)
{
    if (Quest)
    {
        OnQuestRewardsDisplayed(Quest);
    }
}

void UQuestHUDWidget::OnQuestStatusChanged(UQuest* Quest)
{
    // If we have the quest manager, update our active quests list
    if (QuestManager)
    {
        UpdateActiveQuests(QuestManager->ActiveQuests);
    }
    
    // If this was the selected quest that changed status, update its objectives
    if (SelectedQuest == Quest)
    {
        UpdateQuestObjectives(Quest);
    }
    
    // Display appropriate notification
    if (Quest)
    {
        FString Message;
        
        switch (Quest->Status)
        {
            case EQuestStatus::InProgress:
                Message = "Quest accepted";
                break;
            case EQuestStatus::Completed:
                Message = "Quest objective completed";
                break;
            case EQuestStatus::Failed:
                Message = "Quest failed";
                break;
            default:
                break;
        }
        
        if (!Message.IsEmpty())
        {
            DisplayQuestNotification(Quest->QuestName, Message);
        }
    }
}