// OathPlayerController.cpp
#include "OathPlayerController.h"
#include "OathCharacter.h"
#include "OathGameMode.h"
#include "KingdomManager.h"
#include "QuestManager.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

AOathPlayerController::AOathPlayerController()
{
    // Set default values
    bShowMouseCursor = true;
    DefaultMouseCursor = EMouseCursor::Default;
    
    // Enable mouse input for camera movement
    bEnableClickEvents = true;
    bEnableMouseOverEvents = true;
}

void AOathPlayerController::BeginPlay()
{
    Super::BeginPlay();
    
    // Bind to character events
    AOathCharacter* PlayerCharacter = Cast<AOathCharacter>(GetPawn());
    if (PlayerCharacter)
    {
        // Bind to reputation changes
        if (PlayerCharacter->ReputationComponent)
        {
            PlayerCharacter->ReputationComponent->OnReputationChanged.AddDynamic(this, &AOathPlayerController::HandleReputationChanged);
        }
        
        // Bind to inventory changes
        if (PlayerCharacter->InventoryComponent)
        {
            PlayerCharacter->InventoryComponent->OnInventoryChanged.AddDynamic(this, &AOathPlayerController::HandleInventoryChanged);
        }
    }
    
    // Bind to game mode events
    AOathGameMode* GameMode = Cast<AOathGameMode>(UGameplayStatics::GetGameMode(this));
    if (GameMode)
    {
        // Subscribe to day/night cycle changes
        GameMode->OnDayPassed.AddDynamic(this, &AOathPlayerController::HandleDayPassed);
        GameMode->OnSeasonChanged.AddDynamic(this, &AOathPlayerController::HandleSeasonChanged);
        
        // Subscribe to quest changes
        UQuestManager* QuestManager = GameMode->GetQuestManager();
        if (QuestManager)
        {
            QuestManager->OnQuestAccepted.AddDynamic(this, &AOathPlayerController::HandleQuestStatusChanged);
            QuestManager->OnQuestCompleted.AddDynamic(this, &AOathPlayerController::HandleQuestStatusChanged);
            QuestManager->OnQuestFailed.AddDynamic(this, &AOathPlayerController::HandleQuestStatusChanged);
        }
        
        // Subscribe to kingdom events
        if (GameMode->KingdomManager)
        {
            GameMode->KingdomManager->OnKingdomEventOccurred.AddDynamic(this, &AOathPlayerController::HandleKingdomEvent);
            GameMode->KingdomManager->OnKingdomTierChanged.AddDynamic(this, &AOathPlayerController::HandleKingdomTierChanged);
            GameMode->KingdomManager->OnFollowerJoined.AddDynamic(this, &AOathPlayerController::HandleFollowerJoined);
            GameMode->KingdomManager->OnBuildingConstructed.AddDynamic(this, &AOathPlayerController::HandleBuildingConstructed);
        }
    }
    
    // Initialize UI
    SetupHUD();
}

void AOathPlayerController::SetupHUD()
{
    // Create main HUD widget
    // Note: Actual widget creation would typically be done in Blueprints
    // This is just a placeholder for what would happen in the C++ side
    
    // MainHUDWidget = CreateWidget<UUserWidget>(this, MainHUDWidgetClass);
    // if (MainHUDWidget)
    // {
    //     MainHUDWidget->AddToViewport();
    // }
    
    // Initialize minimap
    UpdateMinimap();
    
    // Initialize quest tracker
    UpdateQuestTracker();
    
    // Initialize kingdom status
    UpdateKingdomStatus();
}

void AOathPlayerController::UpdateMinimap()
{
    // Update minimap based on explored areas and current location
    // This would typically involve getting data about explored chunks
    // and player position, then updating a minimap widget
    
    // Example:
    // Get player location
    AOathCharacter* PlayerCharacter = Cast<AOathCharacter>(GetPawn());
    if (PlayerCharacter)
    {
        FVector PlayerLocation = PlayerCharacter->GetActorLocation();
        // MinimapWidget->UpdatePlayerPosition(PlayerLocation);
    }
    
    // Get explored areas from a world exploration manager
    // ExplorableAreasManager->GetExploredChunks();
    // MinimapWidget->UpdateExploredAreas(ExploredChunks);
    
    // Get points of interest
    // MinimapWidget->UpdatePointsOfInterest(PointsOfInterest);
}

void AOathPlayerController::UpdateQuestTracker()
{
    // Update quest tracker widget with active quests
    AOathGameMode* GameMode = Cast<AOathGameMode>(UGameplayStatics::GetGameMode(this));
    if (GameMode)
    {
        UQuestManager* QuestManager = GameMode->GetQuestManager();
        if (QuestManager)
        {
            // Send active quests to the quest tracker widget
            // QuestTrackerWidget->UpdateActiveQuests(QuestManager->ActiveQuests);
        }
    }
}

void AOathPlayerController::UpdateKingdomStatus()
{
    // Update kingdom status widget
    AOathGameMode* GameMode = Cast<AOathGameMode>(UGameplayStatics::GetGameMode(this));
    if (GameMode && GameMode->KingdomManager)
    {
        // Get kingdom data
        // KingdomStatusWidget->UpdateKingdomInfo(
        //     GameMode->KingdomManager->KingdomName,
        //     GameMode->KingdomManager->CurrentTier,
        //     GameMode->KingdomManager->Followers.Num(),
        //     GameMode->KingdomManager->Buildings.Num(),
        //     GameMode->KingdomManager->DailyIncome
        // );
    }
}

void AOathPlayerController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update UI elements that need constant updates
    UpdateMinimap();
    
    // Check for interactive objects under the cursor
    UpdateCursorHoverInfo();
    
    // Handle any pending notifications
    ProcessNotificationQueue();
}

void AOathPlayerController::UpdateCursorHoverInfo()
{
    // Perform a trace under the cursor
    FHitResult HitResult;
    GetHitResultUnderCursor(ECC_Visibility, false, HitResult);
    
    if (HitResult.bBlockingHit)
    {
        // Check if we hit something interactive
        IInteractableInterface* Interactable = Cast<IInteractableInterface>(HitResult.GetActor());
        if (Interactable)
        {
            // Update cursor and tooltip
            // SetMouseCursorStyle(Interactable->GetCursorType());
            // TooltipWidget->UpdateTooltip(Interactable->GetTooltipText(), Interactable->GetTooltipIcon());
            // TooltipWidget->SetVisibility(ESlateVisibility::Visible);
            return;
        }
    }
    
    // If we didn't hit anything interactive, reset cursor and hide tooltip
    // SetMouseCursorStyle(EMouseCursor::Default);
    // TooltipWidget->SetVisibility(ESlateVisibility::Hidden);
}

void AOathPlayerController::ProcessNotificationQueue()
{
    // Process any queued notifications
    // This allows for stacking notifications rather than
    // showing them all at once or overwriting each other
    
    // Example implementation:
    // if (!NotificationQueue.IsEmpty() && !IsNotificationActive())
    // {
    //     FNotificationInfo NextNotification;
    //     NotificationQueue.Dequeue(NextNotification);
    //     ShowNotification(NextNotification.Title, NextNotification.Message, NextNotification.Duration);
    // }
}

// Event handlers

void AOathPlayerController::HandleReputationChanged(FString ReputationType, float OldValue, float NewValue)
{
    // Call the blueprint implementable event
    OnReputationChanged(ReputationType, OldValue, NewValue);
    
    // Show a notification if the change is significant
    float Change = NewValue - OldValue;
    if (FMath::Abs(Change) >= 5.0f)
    {
        FString Title = ReputationType + " Reputation";
        FString Message;
        
        if (Change > 0)
        {
            Message = FString::Printf(TEXT("Your %s reputation has increased by %.0f points."), *ReputationType, Change);
        }
        else
        {
            Message = FString::Printf(TEXT("Your %s reputation has decreased by %.0f points."), *ReputationType, -Change);
        }
        
        ShowNotification(Title, Message);
    }
    
    // Update any UI elements that show reputation
    // ReputationWidget->UpdateReputation(ReputationType, NewValue);
}

void AOathPlayerController::HandleInventoryChanged(FString ResourceType, UObject* Resource, int32 NewAmount)
{
    // Show notification for significant resource changes
    if (ResourceType == "Gold" && FMath::Abs(NewAmount) >= 100)
    {
        FString Title = "Gold Changed";
        FString Message = FString::Printf(TEXT("Gold: %d"), NewAmount);
        ShowNotification(Title, Message, 3.0f);
    }
    
    // Update inventory UI
    // InventoryWidget->UpdateInventory();
}

void AOathPlayerController::HandleDayPassed()
{
    // Update day/night cycle UI
    // TimeWidget->UpdateDay();
    
    // Show daily summary if in kingdom
    AOathGameMode* GameMode = Cast<AOathGameMode>(UGameplayStatics::GetGameMode(this));
    if (GameMode && GameMode->KingdomManager)
    {
        // Check if player is in kingdom boundaries
        AOathCharacter* PlayerCharacter = Cast<AOathCharacter>(GetPawn());
        if (PlayerCharacter && GameMode->KingdomManager->IsInKingdomBoundaries(PlayerCharacter->GetActorLocation()))
        {
            // Show daily kingdom report
            // DailySummaryWidget->ShowDailySummary(
            //     GameMode->KingdomManager->DailyIncome,
            //     GameMode->KingdomManager->GetDailyResourceProduction(),
            //     GameMode->KingdomManager->GetDailyEvents()
            // );
        }
    }
}

void AOathPlayerController::HandleSeasonChanged(int32 NewSeason)
{
    // Show season change notification
    TArray<FString> SeasonNames = {"Spring", "Summer", "Autumn", "Winter"};
    
    if (NewSeason >= 0 && NewSeason < SeasonNames.Num())
    {
        FString Title = "Season Changed";
        FString Message = "The season has changed to " + SeasonNames[NewSeason] + ".";
        ShowNotification(Title, Message);
    }
    
    // Update seasonal UI elements
    // SeasonWidget->UpdateSeason(NewSeason);
}

void AOathPlayerController::HandleQuestStatusChanged(UQuest* Quest)
{
    if (!Quest)
    {
        return;
    }
    
    switch (Quest->Status)
    {
        case EQuestStatus::InProgress:
            ShowNotification("Quest Accepted", "You have accepted the quest: " + Quest->QuestName);
            UpdateQuestLog(Quest, true, false);
            break;
            
        case EQuestStatus::Completed:
            ShowNotification("Quest Completed", "You have completed the quest: " + Quest->QuestName);
            UpdateQuestLog(Quest, false, true);
            break;
            
        case EQuestStatus::Failed:
            ShowNotification("Quest Failed", "You have failed the quest: " + Quest->QuestName);
            UpdateQuestLog(Quest, false, false);
            break;
            
        default:
            break;
    }
    
    // Update quest tracker UI
    UpdateQuestTracker();
}

void AOathPlayerController::HandleKingdomEvent(FKingdomEvent Event)
{
    // Show kingdom event notification or major event popup based on importance
    if (Event.bIsMajorEvent)
    {
        ShowMajorEvent(Event.EventName, Event.EventDescription, Event.EventIcon);
    }
    else
    {
        ShowNotification("Kingdom Event", Event.EventName);
    }
    
    // Update kingdom status UI
    UpdateKingdomStatus();
}

void AOathPlayerController::HandleKingdomTierChanged(EKingdomTier OldTier, EKingdomTier NewTier)
{
    // Show major event for kingdom tier upgrade
    FString TierName;
    switch (NewTier)
    {
        case EKingdomTier::Camp:
            TierName = "Camp";
            break;
        case EKingdomTier::Village:
            TierName = "Village";
            break;
        case EKingdomTier::Town:
            TierName = "Town";
            break;
        case EKingdomTier::City:
            TierName = "City";
            break;
        case EKingdomTier::Kingdom:
            TierName = "Kingdom";
            break;
        default:
            TierName = "Unknown";
            break;
    }
    
    FString Title = "Kingdom Upgraded";
    FString Message = "Your settlement has grown into a " + TierName + "!";
    
    // Note: EventIcon would be set appropriately in the actual implementation
    ShowMajorEvent(Title, Message, nullptr);
    
    // Update kingdom status UI
    UpdateKingdomStatus();
}

void AOathPlayerController::HandleFollowerJoined(FFollowerData Follower)
{
    // Show notification for new follower
    FString Title = "New Follower";
    FString Message = Follower.Name + " has joined your kingdom as a " + GetProfessionName(Follower.Profession) + ".";
    ShowNotification(Title, Message);
    
    // Update kingdom status UI
    UpdateKingdomStatus();
    
    // Show follower details if they're a hero
    if (Follower.bIsHero)
    {
        // FollowerDetailsWidget->ShowFollowerDetails(Follower);
    }
}

FString AOathPlayerController::GetProfessionName(EFollowerProfession Profession)
{
    switch (Profession)
    {
        case EFollowerProfession::Blacksmith:
            return "Blacksmith";
        case EFollowerProfession::Farmer:
            return "Farmer";
        case EFollowerProfession::Miner:
            return "Miner";
        case EFollowerProfession::Lumberjack:
            return "Lumberjack";
        case EFollowerProfession::Hunter:
            return "Hunter";
        case EFollowerProfession::Alchemist:
            return "Alchemist";
        case EFollowerProfession::Guard:
            return "Guard";
        case EFollowerProfession::Merchant:
            return "Merchant";
        case EFollowerProfession::Scholar:
            return "Scholar";
        default:
            return "Settler";
    }
}

void AOathPlayerController::HandleBuildingConstructed(FBuildingData Building, FVector Location)
{
    // Show notification for new building
    FString Title = "Building Constructed";
    FString Message = "A new " + Building.Name + " has been built in your kingdom.";
    ShowNotification(Title, Message);
    
    // Update kingdom status UI
    UpdateKingdomStatus();
    
    // Optionally move camera to the new building
    // if (Building.Type == EBuildingType::Monument || Building.Type == EBuildingType::Cultural)
    // {
    //     FViewTargetTransitionParams TransitionParams;
    //     TransitionParams.BlendTime = 1.0f;
    //     SetViewTarget(Building.GetActorRef(), TransitionParams);
    // }
}

// Input handling

void AOathPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    
    // Action mappings
    InputComponent->BindAction("Interact", IE_Pressed, this, &AOathPlayerController::HandleInteraction);
    InputComponent->BindAction("ToggleInventory", IE_Pressed, this, &AOathPlayerController::ToggleInventory);
    InputComponent->BindAction("ToggleQuestLog", IE_Pressed, this, &AOathPlayerController::ToggleQuestLog);
    InputComponent->BindAction("ToggleMap", IE_Pressed, this, &AOathPlayerController::ToggleMap);
    InputComponent->BindAction("ToggleKingdomMenu", IE_Pressed, this, &AOathPlayerController::ToggleKingdomMenu);
    
    // Axis mappings
    InputComponent->BindAxis("MoveForward", this, &AOathPlayerController::MoveForward);
    InputComponent->BindAxis("MoveRight", this, &AOathPlayerController::MoveRight);
    InputComponent->BindAxis("Turn", this, &AOathPlayerController::Turn);
    InputComponent->BindAxis("LookUp", this, &AOathPlayerController::LookUp);
}

void AOathPlayerController::HandleInteraction()
{
    // Perform a trace to find interactable objects
    FHitResult HitResult;
    GetHitResultUnderCursor(ECC_Visibility, false, HitResult);
    
    if (HitResult.bBlockingHit)
    {
        // Check if we hit something interactive
        IInteractableInterface* Interactable = Cast<IInteractableInterface>(HitResult.GetActor());
        if (Interactable)
        {
            // Interact with the object
            Interactable->Interact(this);
        }
    }
}

void AOathPlayerController::ToggleInventory()
{
    // Toggle inventory UI visibility
    // if (InventoryWidget->IsVisible())
    // {
    //     InventoryWidget->SetVisibility(ESlateVisibility::Hidden);
    // }
    // else
    // {
    //     InventoryWidget->SetVisibility(ESlateVisibility::Visible);
    //     InventoryWidget->UpdateInventory();
    // }
}

void AOathPlayerController::ToggleQuestLog()
{
    // Toggle quest log UI visibility
    // if (QuestLogWidget->IsVisible())
    // {
    //     QuestLogWidget->SetVisibility(ESlateVisibility::Hidden);
    // }
    // else
    // {
    //     QuestLogWidget->SetVisibility(ESlateVisibility::Visible);
    //     QuestLogWidget->UpdateQuestList();
    // }
}

void AOathPlayerController::ToggleMap()
{
    // Toggle map UI visibility
    // if (MapWidget->IsVisible())
    // {
    //     MapWidget->SetVisibility(ESlateVisibility::Hidden);
    // }
    // else
    // {
    //     MapWidget->SetVisibility(ESlateVisibility::Visible);
    //     MapWidget->UpdateMap();
    // }
}

void AOathPlayerController::ToggleKingdomMenu()
{
    // Toggle kingdom management UI visibility
    // if (KingdomMenuWidget->IsVisible())
    // {
    //     KingdomMenuWidget->SetVisibility(ESlateVisibility::Hidden);
    // }
    // else
    // {
    //     // Only show kingdom menu if player has a kingdom
    //     AOathGameMode* GameMode = Cast<AOathGameMode>(UGameplayStatics::GetGameMode(this));
    //     if (GameMode && GameMode->KingdomManager)
    //     {
    //         KingdomMenuWidget->SetVisibility(ESlateVisibility::Visible);
    //         KingdomMenuWidget->UpdateKingdomData();
    //     }
    //     else
    //     {
    //         ShowNotification("Kingdom Required", "You must establish a kingdom first.");
    //     }
    // }
}

void AOathPlayerController::MoveForward(float Value)
{
    if (Value != 0.0f && GetPawn())
    {
        // Find out which way is forward
        const FRotator Rotation = GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);
        
        // Get forward vector
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        GetPawn()->AddMovementInput(Direction, Value);
    }
}

void AOathPlayerController::MoveRight(float Value)
{
    if (Value != 0.0f && GetPawn())
    {
        // Find out which way is right
        const FRotator Rotation = GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);
        
        // Get right vector
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        GetPawn()->AddMovementInput(Direction, Value);
    }
}

void AOathPlayerController::Turn(float Value)
{
    AddYawInput(Value);
}

void AOathPlayerController::LookUp(float Value)
{
    AddPitchInput(Value);
}