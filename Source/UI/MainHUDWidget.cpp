// MainHUDWidget.cpp
#include "MainHUDWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Engine/World.h"
#include "OathPlayerController.h"
#include "OathCharacter.h"
#include "InventoryUIWidget.h"
#include "QuestHUDWidget.h"
#include "KingdomUIWidget.h"

UMainHUDWidget::UMainHUDWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // Initialize menu states
    bInventoryVisible = false;
    bQuestLogVisible = false;
    bKingdomMenuVisible = false;
    bMapVisible = false;
    bCharacterMenuVisible = false;
}

void UMainHUDWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // Initialize the HUD with current player data
    AOathCharacter* PlayerCharacter = Cast<AOathCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
    if (PlayerCharacter)
    {
        // Update basic stats displays (placeholder values)
        UpdateHealthDisplay(100.0f, 100.0f);
        UpdateManaDisplay(100.0f, 100.0f);
        UpdateStaminaDisplay(100.0f, 100.0f);
        UpdateExperienceDisplay(0.0f, 1000.0f, 1);
    }
    
    // Initialize compass display
    UpdateCompassDisplay(0.0f);
    
    // Initialize time display (placeholder values)
    UpdateTimeDisplay(0.5f, 1, 0);
    
    // Initialize weather display (placeholder values)
    UpdateWeatherDisplay("Clear", 0.0f);
}

void UMainHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    
    // Process notification queue
    ProcessNotificationQueue(InDeltaTime);
    
    // Update dynamic HUD elements
    AOathCharacter* PlayerCharacter = Cast<AOathCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
    if (PlayerCharacter)
    {
        // Update compass based on player rotation
        float Heading = PlayerCharacter->GetActorRotation().Yaw;
        UpdateCompassDisplay(Heading);
    }
}

void UMainHUDWidget::UpdateHealthDisplay(float CurrentHealth, float MaxHealth)
{
    // Call the blueprint event to update health display
    OnHealthUpdated(CurrentHealth, MaxHealth);
}

void UMainHUDWidget::UpdateManaDisplay(float CurrentMana, float MaxMana)
{
    // Call the blueprint event to update mana display
    OnManaUpdated(CurrentMana, MaxMana);
}

void UMainHUDWidget::UpdateStaminaDisplay(float CurrentStamina, float MaxStamina)
{
    // Call the blueprint event to update stamina display
    OnStaminaUpdated(CurrentStamina, MaxStamina);
}

void UMainHUDWidget::UpdateExperienceDisplay(float CurrentXP, float NextLevelXP, int32 CurrentLevel)
{
    // Call the blueprint event to update experience display
    OnExperienceUpdated(CurrentXP, NextLevelXP, CurrentLevel);
}

void UMainHUDWidget::UpdateCompassDisplay(float Heading)
{
    // Call the blueprint event to update compass display
    OnCompassUpdated(Heading);
}

void UMainHUDWidget::UpdateMinimap(TArray<FVector2D> ExploredLocations, FVector2D PlayerPosition)
{
    // Call the blueprint event to update minimap
    OnMinimapUpdated(ExploredLocations, PlayerPosition);
}

void UMainHUDWidget::UpdateQuestTracker(const TArray<UQuest*>& ActiveQuests)
{
    // Call the blueprint event to update quest tracker
    OnQuestTrackerUpdated(ActiveQuests);
}

void UMainHUDWidget::HighlightQuestObjective(UQuest* Quest, int32 ObjectiveIndex)
{
    // Call the blueprint event to highlight a quest objective
    OnQuestObjectiveHighlighted(Quest, ObjectiveIndex);
}

void UMainHUDWidget::AddNotification(const FString& Title, const FString& Message, float Duration)
{
    // Add to notification queue
    FNotificationInfo NewNotification;
    NewNotification.Title = Title;
    NewNotification.Message = Message;
    NewNotification.Duration = Duration;
    NewNotification.RemainingTime = Duration;
    
    NotificationQueue.Add(NewNotification);
    
    // Immediately show the notification if it's the only one
    if (NotificationQueue.Num() == 1)
    {
        OnNotificationAdded(Title, Message);
    }
}

void UMainHUDWidget::ShowMajorEventNotification(const FString& EventTitle, const FString& EventDescription, UTexture2D* EventIcon)
{
    // Call the blueprint event to show a major event notification
    OnMajorEventShown(EventTitle, EventDescription, EventIcon);
}

void UMainHUDWidget::ShowEnemyHealthBar(const FString& EnemyName, float CurrentHealth, float MaxHealth)
{
    // Call the blueprint event to show enemy health bar
    OnEnemyHealthBarShown(EnemyName, CurrentHealth, MaxHealth);
}

void UMainHUDWidget::HideEnemyHealthBar()
{
    // Call the blueprint event to hide enemy health bar
    OnEnemyHealthBarHidden();
}

void UMainHUDWidget::ShowCombatText(const FString& Text, FVector WorldLocation, FLinearColor Color)
{
    // Convert world location to screen position
    FVector2D ScreenPosition = WorldToScreenPosition(WorldLocation);
    
    // Call the blueprint event to show combat text
    OnCombatTextShown(Text, ScreenPosition, Color);
}

void UMainHUDWidget::UpdateTimeDisplay(float TimeOfDay, int32 CurrentDay, int32 CurrentSeason)
{
    // Call the blueprint event to update time display
    OnTimeDisplayUpdated(TimeOfDay, CurrentDay, CurrentSeason);
}

void UMainHUDWidget::UpdateWeatherDisplay(const FString& WeatherType, float Intensity)
{
    // Call the blueprint event to update weather display
    OnWeatherDisplayUpdated(WeatherType, Intensity);
}

void UMainHUDWidget::ShowInteractionPrompt(const FString& PromptText, const FString& InputKey)
{
    // Call the blueprint event to show interaction prompt
    OnInteractionPromptShown(PromptText, InputKey);
}

void UMainHUDWidget::HideInteractionPrompt()
{
    // Call the blueprint event to hide interaction prompt
    OnInteractionPromptHidden();
}

bool UMainHUDWidget::ToggleInventoryMenu()
{
    bInventoryVisible = !bInventoryVisible;
    
    // Close other menus if opening this one
    if (bInventoryVisible)
    {
        bQuestLogVisible = false;
        bKingdomMenuVisible = false;
        bMapVisible = false;
        bCharacterMenuVisible = false;
        
        OnQuestLogToggled(false);
        OnKingdomMenuToggled(false);
        OnMapToggled(false);
        OnCharacterMenuToggled(false);
    }
    
    // Call the blueprint event to toggle inventory menu
    OnInventoryMenuToggled(bInventoryVisible);
    
    return bInventoryVisible;
}

bool UMainHUDWidget::ToggleQuestLog()
{
    bQuestLogVisible = !bQuestLogVisible;
    
    // Close other menus if opening this one
    if (bQuestLogVisible)
    {
        bInventoryVisible = false;
        bKingdomMenuVisible = false;
        bMapVisible = false;
        bCharacterMenuVisible = false;
        
        OnInventoryMenuToggled(false);
        OnKingdomMenuToggled(false);
        OnMapToggled(false);
        OnCharacterMenuToggled(false);
    }
    
    // Call the blueprint event to toggle quest log
    OnQuestLogToggled(bQuestLogVisible);
    
    return bQuestLogVisible;
}

bool UMainHUDWidget::ToggleKingdomMenu()
{
    bKingdomMenuVisible = !bKingdomMenuVisible;
    
    // Close other menus if opening this one
    if (bKingdomMenuVisible)
    {
        bInventoryVisible = false;
        bQuestLogVisible = false;
        bMapVisible = false;
        bCharacterMenuVisible = false;
        
        OnInventoryMenuToggled(false);
        OnQuestLogToggled(false);
        OnMapToggled(false);
        OnCharacterMenuToggled(false);
    }
    
    // Call the blueprint event to toggle kingdom menu
    OnKingdomMenuToggled(bKingdomMenuVisible);
    
    return bKingdomMenuVisible;
}

bool UMainHUDWidget::ToggleMap()
{
    bMapVisible = !bMapVisible;
    
    // Close other menus if opening this one
    if (bMapVisible)
    {
        bInventoryVisible = false;
        bQuestLogVisible = false;
        bKingdomMenuVisible = false;
        bCharacterMenuVisible = false;
        
        OnInventoryMenuToggled(false);
        OnQuestLogToggled(false);
        OnKingdomMenuToggled(false);
        OnCharacterMenuToggled(false);
    }
    
    // Call the blueprint event to toggle map
    OnMapToggled(bMapVisible);
    
    return bMapVisible;
}

bool UMainHUDWidget::ToggleCharacterMenu()
{
    bCharacterMenuVisible = !bCharacterMenuVisible;
    
    // Close other menus if opening this one
    if (bCharacterMenuVisible)
    {
        bInventoryVisible = false;
        bQuestLogVisible = false;
        bKingdomMenuVisible = false;
        bMapVisible = false;
        
        OnInventoryMenuToggled(false);
        OnQuestLogToggled(false);
        OnKingdomMenuToggled(false);
        OnMapToggled(false);
    }
    
    // Call the blueprint event to toggle character menu
    OnCharacterMenuToggled(bCharacterMenuVisible);
    
    return bCharacterMenuVisible;
}

void UMainHUDWidget::ProcessNotificationQueue(float DeltaTime)
{
    if (NotificationQueue.Num() == 0)
    {
        return;
    }
    
    // Update the current notification's remaining time
    NotificationQueue[0].RemainingTime -= DeltaTime;
    
    // If the current notification has expired, remove it and show the next one
    if (NotificationQueue[0].RemainingTime <= 0.0f)
    {
        NotificationQueue.RemoveAt(0);
        
        // Show the next notification if there is one
        if (NotificationQueue.Num() > 0)
        {
            OnNotificationAdded(NotificationQueue[0].Title, NotificationQueue[0].Message);
        }
    }
}

FVector2D UMainHUDWidget::WorldToMinimapPosition(const FVector& WorldLocation)
{
    // This is a placeholder implementation - would depend on your game's coordinate system
    // Scale and shift world coordinates to fit the minimap UI
    
    // Get player location as a reference point
    AOathCharacter* PlayerCharacter = Cast<AOathCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
    if (!PlayerCharacter)
    {
        return FVector2D::ZeroVector;
    }
    
    FVector PlayerLocation = PlayerCharacter->GetActorLocation();
    
    // Calculate relative position (centered on player)
    float RelativeX = (WorldLocation.X - PlayerLocation.X) / 1000.0f; // Scale factor
    float RelativeY = (WorldLocation.Y - PlayerLocation.Y) / 1000.0f; // Scale factor
    
    // Convert to 2D position on minimap (assuming 100x100 minimap centered at 0,0)
    // Would need to be adjusted based on actual minimap size and position
    FVector2D MinimapCenter = FVector2D(50.0f, 50.0f);
    FVector2D MinimapPosition = MinimapCenter + FVector2D(RelativeX, -RelativeY); // Y is inverted in UI space
    
    return MinimapPosition;
}

FVector2D UMainHUDWidget::WorldToScreenPosition(const FVector& WorldLocation)
{
    // Convert world space position to screen space position
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
    if (!PlayerController)
    {
        return FVector2D::ZeroVector;
    }
    
    FVector2D ScreenPosition;
    PlayerController->ProjectWorldLocationToScreen(WorldLocation, ScreenPosition);
    
    return ScreenPosition;
}