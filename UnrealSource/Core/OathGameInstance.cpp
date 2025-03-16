// OathGameInstance.cpp
#include "OathGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "OathSaveGame.h"
#include "OathPlayerController.h"

UOathGameInstance::UOathGameInstance()
{
    // Create procedural generator
    ProceduralGenerator = CreateDefaultSubobject<UProceduralGenerator>(TEXT("ProceduralGenerator"));
}

void UOathGameInstance::Init()
{
    Super::Init();
    
    // Initialize factions
    InitializeFactions();
}

TArray<FString> UOathGameInstance::GetAllFactions() const
{
    return AvailableFactions;
}

void UOathGameInstance::UpdateFactionAvailability(const FString& FactionName, float Reputation)
{
    // Find any faction-specific events that should trigger at this reputation level
    
    // Example: Unlock faction-specific quests or areas
    bool bUnlockingNewContent = false;
    FString UnlockMessage;
    
    if (Reputation >= 50.0f && !FactionUnlocks.Contains(FactionName + "_Ally"))
    {
        // Unlock ally-level content
        FactionUnlocks.Add(FactionName + "_Ally");
        UnlockMessage = FString::Printf(TEXT("You are now considered an ally of the %s. New quests and areas are now available."), *FactionName);
        bUnlockingNewContent = true;
    }
    else if (Reputation >= 25.0f && !FactionUnlocks.Contains(FactionName + "_Friend"))
    {
        // Unlock friend-level content
        FactionUnlocks.Add(FactionName + "_Friend");
        UnlockMessage = FString::Printf(TEXT("You are now considered a friend of the %s. New quests and merchants are now available."), *FactionName);
        bUnlockingNewContent = true;
    }
    else if (Reputation <= -50.0f && !FactionUnlocks.Contains(FactionName + "_Enemy"))
    {
        // Unlock enemy-level content
        FactionUnlocks.Add(FactionName + "_Enemy");
        UnlockMessage = FString::Printf(TEXT("You are now considered an enemy of the %s. Be careful in their territory."), *FactionName);
        bUnlockingNewContent = true;
    }
    
    // Show notification if needed
    if (bUnlockingNewContent)
    {
        APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (PC)
        {
            AOathPlayerController* OathPC = Cast<AOathPlayerController>(PC);
            if (OathPC)
            {
                OathPC->ShowNotification(FString::Printf(TEXT("%s Relationship Changed"), *FactionName), UnlockMessage, 10.0f);
            }
        }
    }
}

void UOathGameInstance::OnCombatRenownMilestoneReached(float Milestone)
{
    // Handle game-wide events for combat renown milestones
    
    // Get player controller to show notifications
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC)
    {
        return;
    }
    
    AOathPlayerController* OathPC = Cast<AOathPlayerController>(PC);
    if (!OathPC)
    {
        return;
    }
    
    // Different events based on milestone level
    if (Milestone >= 5000.0f)
    {
        OathPC->ShowMajorEvent(
            "Legendary Fighter",
            "Your combat prowess is legendary across the land. The greatest warriors now seek to challenge you, and your name strikes fear into the hearts of monsters.",
            LoadObject<UTexture2D>(nullptr, TEXT("/Game/UI/Icons/legendary_combat.legendary_combat"))
        );
        
        // Maybe spawn a special legendary challenger somewhere in the world
        if (ProceduralGenerator)
        {
            FVector SpawnLocation = GetRandomLocationNearPlayer(1000.0f);
            AActor* LegendaryChallenger = ProceduralGenerator->GenerateNotoriousMonster(5, "Any");
            if (LegendaryChallenger)
            {
                // Set up the challenger with special AI and loot
            }
        }
    }
    else if (Milestone >= 1000.0f)
    {
        OathPC->ShowMajorEvent(
            "Renowned Fighter",
            "Your combat skills are renowned throughout the region. Warriors speak your name with respect, and monsters grow wary of your presence.",
            LoadObject<UTexture2D>(nullptr, TEXT("/Game/UI/Icons/renowned_combat.renowned_combat"))
        );
        
        // Unlock special combat quests
        if (ProceduralGenerator)
        {
            UQuest* SpecialQuest = ProceduralGenerator->GenerateQuest(3, "Warriors Guild", EQuestType::Kill);
            if (SpecialQuest)
            {
                OathPC->UpdateQuestLog(SpecialQuest, true, false);
            }
        }
    }
    else if (Milestone >= 100.0f)
    {
        OathPC->ShowNotification(
            "Combat Milestone",
            "Your combat skills are beginning to be recognized. Local warriors take notice of your growing abilities.",
            8.0f
        );
    }
}

void UOathGameInstance::OnQuestRenownMilestoneReached(float Milestone)
{
    // Handle game-wide events for quest renown milestones
    
    // Get player controller to show notifications
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC)
    {
        return;
    }
    
    AOathPlayerController* OathPC = Cast<AOathPlayerController>(PC);
    if (!OathPC)
    {
        return;
    }
    
    // Different events based on milestone level
    if (Milestone >= 5000.0f)
    {
        OathPC->ShowMajorEvent(
            "Legendary Questor",
            "Your deeds are the stuff of legend. Bards sing of your accomplishments, and people from far and wide seek your aid with their most difficult problems.",
            LoadObject<UTexture2D>(nullptr, TEXT("/Game/UI/Icons/legendary_quest.legendary_quest"))
        );
        
        // Special follower seeks to join you
        if (ProceduralGenerator)
        {
            FFollowerData LegendaryFollower = ProceduralGenerator->GenerateFollower(10, 10, true);
            // Offer this follower to the player
        }
    }
    else if (Milestone >= 1000.0f)
    {
        OathPC->ShowMajorEvent(
            "Renowned Problem Solver",
            "Your quest accomplishments have spread far and wide. People recognize you as someone who gets things done, and your reputation draws those seeking help.",
            LoadObject<UTexture2D>(nullptr, TEXT("/Game/UI/Icons/renowned_quest.renowned_quest"))
        );
        
        // Unlock special diplomatic opportunities
        if (KingdomManager)
        {
            KingdomManager->UnlockDiplomaticOptions("Advanced");
        }
    }
    else if (Milestone >= 100.0f)
    {
        OathPC->ShowNotification(
            "Quest Milestone",
            "Your helpfulness is becoming known. People are starting to remember your name when they need assistance.",
            8.0f
        );
    }
}

bool UOathGameInstance::SaveGameState()
{
    // Create save game object
    UOathSaveGame* SaveGameInstance = Cast<UOathSaveGame>(UGameplayStatics::CreateSaveGameObject(UOathSaveGame::StaticClass()));
    if (!SaveGameInstance)
    {
        return false;
    }
    
    // Populate save data
    
    // Save player data
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PC)
    {
        APawn* PlayerPawn = PC->GetPawn();
        if (PlayerPawn)
        {
            // Save location
            SaveGameInstance->PlayerLocation = PlayerPawn->GetActorLocation();
            SaveGameInstance->PlayerRotation = PlayerPawn->GetActorRotation();
            
            // Save player stats
            AOathCharacter* OathCharacter = Cast<AOathCharacter>(PlayerPawn);
            if (OathCharacter)
            {
                SaveGameInstance->CharacterClass = OathCharacter->CharacterClass;
                SaveGameInstance->KingdomVision = OathCharacter->KingdomVision;
                
                // Save reputation
                if (OathCharacter->ReputationComponent)
                {
                    SaveGameInstance->CombatRenown = OathCharacter->ReputationComponent->CombatRenown;
                    SaveGameInstance->QuestRenown = OathCharacter->ReputationComponent->QuestRenown;
                    SaveGameInstance->KingdomReputation = OathCharacter->ReputationComponent->KingdomReputation;
                    SaveGameInstance->FactionReputation = OathCharacter->ReputationComponent->FactionReputation;
                }
                
                // Save inventory
                if (OathCharacter->InventoryComponent)
                {
                    SaveGameInstance->Gold = OathCharacter->InventoryComponent->Gold;
                    SaveGameInstance->Materials = OathCharacter->InventoryComponent->Materials;
                    SaveGameInstance->Items = OathCharacter->InventoryComponent->Items;
                }
            }
        }
    }
    
    // Save kingdom data
    if (KingdomManager)
    {
        SaveGameInstance->KingdomName = KingdomManager->KingdomName;
        SaveGameInstance->CurrentTier = KingdomManager->CurrentTier;
        SaveGameInstance->KingdomAlignment = KingdomManager->KingdomAlignment;
        SaveGameInstance->Followers = KingdomManager->Followers;
        SaveGameInstance->Buildings = KingdomManager->Buildings;
        SaveGameInstance->TaxRate = KingdomManager->TaxRate;
        SaveGameInstance->DailyIncome = KingdomManager->DailyIncome;
    }
    
    // Save faction data
    SaveGameInstance->AvailableFactions = AvailableFactions;
    SaveGameInstance->FactionUnlocks = FactionUnlocks;
    
    // Save world data
    SaveGameInstance->CurrentDayTime = GetWorld()->GetTimeSeconds();
    SaveGameInstance->CurrentDay = CurrentDay;
    SaveGameInstance->CurrentSeason = CurrentSeason;
    
    // Save game to slot
    return UGameplayStatics::SaveGameToSlot(SaveGameInstance, "OathSaveSlot", 0);
}

bool UOathGameInstance::LoadGameState()
{
    // Check if save exists
    if (!UGameplayStatics::DoesSaveGameExist("OathSaveSlot", 0))
    {
        return false;
    }
    
    // Load save game
    UOathSaveGame* SaveGameInstance = Cast<UOathSaveGame>(UGameplayStatics::LoadGameFromSlot("OathSaveSlot", 0));
    if (!SaveGameInstance)
    {
        return false;
    }
    
    // Restore player data
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PC)
    {
        APawn* PlayerPawn = PC->GetPawn();
        if (PlayerPawn)
        {
            // Restore location
            PlayerPawn->SetActorLocation(SaveGameInstance->PlayerLocation);
            PlayerPawn->SetActorRotation(SaveGameInstance->PlayerRotation);
            
            // Restore player stats
            AOathCharacter* OathCharacter = Cast<AOathCharacter>(PlayerPawn);
            if (OathCharacter)
            {
                OathCharacter->CharacterClass = SaveGameInstance->CharacterClass;
                OathCharacter->KingdomVision = SaveGameInstance->KingdomVision;
                
                // Restore reputation
                if (OathCharacter->ReputationComponent)
                {
                    OathCharacter->ReputationComponent->CombatRenown = SaveGameInstance->CombatRenown;
                    OathCharacter->ReputationComponent->QuestRenown = SaveGameInstance->QuestRenown;
                    OathCharacter->ReputationComponent->KingdomReputation = SaveGameInstance->KingdomReputation;
                    OathCharacter->ReputationComponent->FactionReputation = SaveGameInstance->FactionReputation;
                }
                
                // Restore inventory
                if (OathCharacter->InventoryComponent)
                {
                    OathCharacter->InventoryComponent->Gold = SaveGameInstance->Gold;
                    OathCharacter->InventoryComponent->Materials = SaveGameInstance->Materials;
                    OathCharacter->InventoryComponent->Items = SaveGameInstance->Items;
                }
            }
        }
    }
    
    // Restore kingdom data
    if (KingdomManager)
    {
        KingdomManager->KingdomName = SaveGameInstance->KingdomName;
        KingdomManager->CurrentTier = SaveGameInstance->CurrentTier;
        KingdomManager->KingdomAlignment = SaveGameInstance->KingdomAlignment;
        KingdomManager->Followers = SaveGameInstance->Followers;
        KingdomManager->Buildings = SaveGameInstance->Buildings;
        KingdomManager->TaxRate = SaveGameInstance->TaxRate;
        KingdomManager->DailyIncome = SaveGameInstance->DailyIncome;
    }
    
    // Restore faction data
    AvailableFactions = SaveGameInstance->AvailableFactions;
    FactionUnlocks = SaveGameInstance->FactionUnlocks;
    
    // Restore world data
    CurrentDay = SaveGameInstance->CurrentDay;
    CurrentSeason = SaveGameInstance->CurrentSeason;
    
    return true;
}

void UOathGameInstance::InitializeFactions()
{
    // Initialize basic factions
    AvailableFactions.Add("Hunters Guild");
    AvailableFactions.Add("Mages Circle");
    AvailableFactions.Add("Merchants Union");
    AvailableFactions.Add("Royal Army");
    AvailableFactions.Add("Forest Tribes");
    AvailableFactions.Add("Mountain Clans");
    AvailableFactions.Add("Desert Nomads");
    
    // Initialize faction reputation thresholds
    FactionReputationThresholds.Add("Hostile", -50.0f);
    FactionReputationThresholds.Add("Unfriendly", -25.0f);
    FactionReputationThresholds.Add("Neutral", 0.0f);
    FactionReputationThresholds.Add("Friendly", 25.0f);
    FactionReputationThresholds.Add("Honored", 50.0f);
    FactionReputationThresholds.Add("Exalted", 75.0f);
}