// OathPlayerController.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "OathPlayerController.generated.h"

UCLASS()
class OATH_API AOathPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AOathPlayerController();
    
    // Called when reputation changes to update UI
    UFUNCTION(BlueprintImplementableEvent, Category = "Reputation")
    void OnReputationChanged(const FString& ReputationType, float OldValue, float NewValue);
    
    // Called when a notification needs to be displayed to the player
    UFUNCTION(BlueprintImplementableEvent, Category = "UI")
    void ShowNotification(const FString& Title, const FString& Message, float Duration = 5.0f);
    
    // Called when a major event happens that should interrupt the player
    UFUNCTION(BlueprintImplementableEvent, Category = "UI")
    void ShowMajorEvent(const FString& EventTitle, const FString& EventDescription, UTexture2D* EventIcon);
    
    // Called to update the quest log
    UFUNCTION(BlueprintImplementableEvent, Category = "Quests")
    void UpdateQuestLog(UQuest* Quest, bool bAdded, bool bCompleted);
};