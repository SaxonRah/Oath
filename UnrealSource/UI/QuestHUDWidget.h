// QuestHUDWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Quest.h"
#include "QuestHUDWidget.generated.h"

UCLASS()
class OATH_API UQuestHUDWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "UI|Quests|HUD")
    void UpdateActiveQuests(const TArray<UQuest*>& Quests);
    
    UFUNCTION(BlueprintCallable, Category = "UI|Quests|HUD")
    void SetSelectedQuest(UQuest* Quest);
    
    UFUNCTION(BlueprintCallable, Category = "UI|Quests|HUD")
    void UpdateQuestObjectives(UQuest* Quest);
    
    UFUNCTION(BlueprintCallable, Category = "UI|Quests|HUD")
    void DisplayQuestNotification(const FString& QuestName, const FString& Message);
    
    UFUNCTION(BlueprintCallable, Category = "UI|Quests|HUD")
    void DisplayQuestRewards(UQuest* Quest);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "UI|Quests|HUD")
    void OnActiveQuestsUpdated();
    
    UFUNCTION(BlueprintImplementableEvent, Category = "UI|Quests|HUD")
    void OnSelectedQuestChanged();

    UFUNCTION(BlueprintImplementableEvent, Category = "UI|Quests|HUD")
   void OnQuestObjectivesUpdated();
   
   UFUNCTION(BlueprintImplementableEvent, Category = "UI|Quests|HUD")
   void OnQuestNotificationReceived(const FString& QuestName, const FString& Message);
   
   UFUNCTION(BlueprintImplementableEvent, Category = "UI|Quests|HUD")
   void OnQuestRewardsDisplayed(UQuest* Quest);
   
protected:
   virtual void NativeConstruct() override;
   
   UPROPERTY(BlueprintReadOnly, Category = "UI|Quests|HUD")
   TArray<UQuest*> ActiveQuests;
   
   UPROPERTY(BlueprintReadOnly, Category = "UI|Quests|HUD")
   UQuest* SelectedQuest;
   
private:
   UPROPERTY()
   class UQuestManager* QuestManager;
   
   void FindQuestManager();
   
   UFUNCTION()
   void OnQuestStatusChanged(UQuest* Quest);
};