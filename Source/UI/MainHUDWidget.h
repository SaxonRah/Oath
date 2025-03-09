// MainHUDWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Quest.h"
#include "KingdomEvent.h"
#include "MainHUDWidget.generated.h"

UCLASS()
class OATH_API UMainHUDWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UMainHUDWidget(const FObjectInitializer& ObjectInitializer);
    
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
    
    // HUD elements update methods
    UFUNCTION(BlueprintCallable, Category = "UI|HUD")
    void UpdateHealthDisplay(float CurrentHealth, float MaxHealth);
    
    UFUNCTION(BlueprintCallable, Category = "UI|HUD")
    void UpdateManaDisplay(float CurrentMana, float MaxMana);
    
    UFUNCTION(BlueprintCallable, Category = "UI|HUD")
    void UpdateStaminaDisplay(float CurrentStamina, float MaxStamina);
    
    UFUNCTION(BlueprintCallable, Category = "UI|HUD")
    void UpdateExperienceDisplay(float CurrentXP, float NextLevelXP, int32 CurrentLevel);
    
    UFUNCTION(BlueprintCallable, Category = "UI|HUD")
    void UpdateCompassDisplay(float Heading);
    
    UFUNCTION(BlueprintCallable, Category = "UI|HUD")
    void UpdateMinimap(TArray<FVector2D> ExploredLocations, FVector2D PlayerPosition);
    
    // Quest tracker methods
    UFUNCTION(BlueprintCallable, Category = "UI|HUD")
    void UpdateQuestTracker(const TArray<UQuest*>& ActiveQuests);
    
    UFUNCTION(BlueprintCallable, Category = "UI|HUD")
    void HighlightQuestObjective(UQuest* Quest, int32 ObjectiveIndex);
    
    // Notification methods
    UFUNCTION(BlueprintCallable, Category = "UI|HUD")
    void AddNotification(const FString& Title, const FString& Message, float Duration = 5.0f);
    
    UFUNCTION(BlueprintCallable, Category = "UI|HUD")
    void ShowMajorEventNotification(const FString& EventTitle, const FString& EventDescription, UTexture2D* EventIcon);
    
    // Combat UI methods
    UFUNCTION(BlueprintCallable, Category = "UI|HUD")
    void ShowEnemyHealthBar(const FString& EnemyName, float CurrentHealth, float MaxHealth);
    
    UFUNCTION(BlueprintCallable, Category = "UI|HUD")
    void HideEnemyHealthBar();
    
    UFUNCTION(BlueprintCallable, Category = "UI|HUD")
    void ShowCombatText(const FString& Text, FVector WorldLocation, FLinearColor Color);
    
    // Time/Weather display
    UFUNCTION(BlueprintCallable, Category = "UI|HUD")
    void UpdateTimeDisplay(float TimeOfDay, int32 CurrentDay, int32 CurrentSeason);
    
    UFUNCTION(BlueprintCallable, Category = "UI|HUD")
    void UpdateWeatherDisplay(const FString& WeatherType, float Intensity);
    
    // Interaction prompt
    UFUNCTION(BlueprintCallable, Category = "UI|HUD")
    void ShowInteractionPrompt(const FString& PromptText, const FString& InputKey);
    
    UFUNCTION(BlueprintCallable, Category = "UI|HUD")
    void HideInteractionPrompt();
    
    // Menu toggling
    UFUNCTION(BlueprintCallable, Category = "UI|HUD")
    bool ToggleInventoryMenu();
    
    UFUNCTION(BlueprintCallable, Category = "UI|HUD")
    bool ToggleQuestLog();
    
    UFUNCTION(BlueprintCallable, Category = "UI|HUD")
    bool ToggleKingdomMenu();
    
    UFUNCTION(BlueprintCallable, Category = "UI|HUD")
    bool ToggleMap();
    
    UFUNCTION(BlueprintCallable, Category = "UI|HUD")
    bool ToggleCharacterMenu();
    
    // Blueprint implementable events
    UFUNCTION(BlueprintImplementableEvent, Category = "UI|HUD")
    void OnHealthUpdated(float CurrentHealth, float MaxHealth);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "UI|HUD")
    void OnManaUpdated(float CurrentMana, float MaxMana);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "UI|HUD")
    void OnStaminaUpdated(float CurrentStamina, float MaxStamina);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "UI|HUD")
    void OnExperienceUpdated(float CurrentXP, float NextLevelXP, int32 CurrentLevel);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "UI|HUD")
    void OnCompassUpdated(float Heading);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "UI|HUD")
    void OnMinimapUpdated(TArray<FVector2D> ExploredLocations, FVector2D PlayerPosition);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "UI|HUD")
    void OnQuestTrackerUpdated(const TArray<UQuest*>& ActiveQuests);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "UI|HUD")
    void OnQuestObjectiveHighlighted(UQuest* Quest, int32 ObjectiveIndex);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "UI|HUD")
    void OnNotificationAdded(const FString& Title, const FString& Message);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "UI|HUD")
    void OnMajorEventShown(const FString& EventTitle, const FString& EventDescription, UTexture2D* EventIcon);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "UI|HUD")
    void OnEnemyHealthBarShown(const FString& EnemyName, float CurrentHealth, float MaxHealth);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "UI|HUD")
    void OnEnemyHealthBarHidden();
    
    UFUNCTION(BlueprintImplementableEvent, Category = "UI|HUD")
    void OnCombatTextShown(const FString& Text, FVector2D ScreenLocation, FLinearColor Color);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "UI|HUD")
    void OnTimeDisplayUpdated(float TimeOfDay, int32 CurrentDay, int32 CurrentSeason);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "UI|HUD")
    void OnWeatherDisplayUpdated(const FString& WeatherType, float Intensity);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "UI|HUD")
    void OnInteractionPromptShown(const FString& PromptText, const FString& InputKey);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "UI|HUD")
    void OnInteractionPromptHidden();
    
    UFUNCTION(BlueprintImplementableEvent, Category = "UI|HUD")
    void OnInventoryMenuToggled(bool bIsVisible);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "UI|HUD")
    void OnQuestLogToggled(bool bIsVisible);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "UI|HUD")
    void OnKingdomMenuToggled(bool bIsVisible);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "UI|HUD")
    void OnMapToggled(bool bIsVisible);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "UI|HUD")
    void OnCharacterMenuToggled(bool bIsVisible);
    
private:
    // References to sub-widgets
    UPROPERTY()
    UUserWidget* InventoryWidget;
    
    UPROPERTY()
    UUserWidget* QuestLogWidget;
    
    UPROPERTY()
    UUserWidget* KingdomMenuWidget;
    
    UPROPERTY()
    UUserWidget* MapWidget;
    
    UPROPERTY()
    UUserWidget* CharacterMenuWidget;
    
    // Current menu states
    UPROPERTY()
    bool bInventoryVisible;
    
    UPROPERTY()
    bool bQuestLogVisible;
    
    UPROPERTY()
    bool bKingdomMenuVisible;
    
    UPROPERTY()
    bool bMapVisible;
    
    UPROPERTY()
    bool bCharacterMenuVisible;
    
    // Notification queue
    struct FNotificationInfo
    {
        FString Title;
        FString Message;
        float Duration;
        float RemainingTime;
    };
    
    UPROPERTY()
    TArray<FNotificationInfo> NotificationQueue;
    
    // Helper methods
    void ProcessNotificationQueue(float DeltaTime);
    FVector2D WorldToMinimapPosition(const FVector& WorldLocation);
    FVector2D WorldToScreenPosition(const FVector& WorldLocation);
};