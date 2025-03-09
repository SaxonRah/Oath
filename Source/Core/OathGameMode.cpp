// OathGameMode.cpp
#include "OathGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "OathCharacter.h"
#include "QuestManager.h"
#include "QuestSaveGame.h"
#include "FollowerManager.h"
#include "ResourceManager.h"
#include "EngineUtils.h"
#include "ProceduralGenerator.h"

AOathGameMode::AOathGameMode()
{
    // Set default values
    TimeScale = 1.0f;
    DayLength = 600.0f; // 10 minutes in real time = 1 day
    DaysPerSeason = 30;
    
    CurrentDayTime = 0.0f;
    CurrentDay = 1;
    CurrentSeason = 0; // 0 = Spring, 1 = Summer, 2 = Fall, 3 = Winter
    
    // Set default player character class
    DefaultPawnClass = AOathCharacter::StaticClass();
    
    // Create procedural generator
    ProceduralGenerator = CreateDefaultSubobject<UProceduralGenerator>(TEXT("ProceduralGenerator"));
}

void AOathGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
    Super::InitGame(MapName, Options, ErrorMessage);
    
    // Create kingdom manager
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    
    KingdomManager = GetWorld()->SpawnActor<AKingdomManager>(AKingdomManager::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
    
    // Create additional managers as components
    UQuestManager* QuestManager = NewObject<UQuestManager>(this);
    if (QuestManager)
    {
        QuestManager->RegisterComponent();
    }
    
    UFollowerManager* FollowerManager = NewObject<UFollowerManager>(this);
    if (FollowerManager)
    {
        FollowerManager->RegisterComponent();
    }
    
    UResourceManager* ResourceManager = NewObject<UResourceManager>(this);
    if (ResourceManager)
    {
        ResourceManager->RegisterComponent();
    }
    
    // Check command line arguments for loading a saved game
    FString SaveGameName;
    if (FParse::Value(FCommandLine::Get(), TEXT("LoadGame="), SaveGameName))
    {
        LoadGame(SaveGameName);
    }
}

void AOathGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    // Register for key gameplay events
    if (KingdomManager)
    {
        // Set up event bindings here
    }
    
    // Seed the world with initial content
    InitializeWorld();
}

void AOathGameMode::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Process world time
    ProcessWorldTime(DeltaTime);
    
    // Update systems
    UpdateGameSystems(DeltaTime);
}

void AOathGameMode::ProcessWorldTime(float DeltaTime)
{
    // Apply time scale
    float ScaledDeltaTime = DeltaTime * TimeScale;
    
    // Update current time
    CurrentDayTime += ScaledDeltaTime;
    
    // Check if a day has passed
    if (CurrentDayTime >= DayLength)
    {
        // Reset day timer and increment day
        CurrentDayTime -= DayLength;
        CurrentDay++;
        
        // Process daily updates
        ProcessDayChange();
        
        // Check if a season has passed
        if (CurrentDay > DaysPerSeason)
        {
            CurrentDay = 1;
            CurrentSeason = (CurrentSeason + 1) % 4;
            
            // Process seasonal updates
            ProcessSeasonChange();
        }
    }
    
    // Update world lighting and atmosphere based on time of day
    UpdateWorldEnvironment();
}

void AOathGameMode::ProcessDayChange()
{
    // Update kingdom daily resources
    if (KingdomManager)
    {
        KingdomManager->ProcessDailyUpdate();
    }
    
    // Update quests
    UQuestManager* QuestManager = GetQuestManager();
    if (QuestManager)
    {
        QuestManager->UpdateAssignedQuests(DayLength);
        
        // Generate new quests occasionally
        if (CurrentDay % 3 == 0)
        {
            QuestManager->GenerateRandomQuests(FMath::RandRange(1, 3), 1, FMath::Max(1, GetPlayerLevel() / 5), ProceduralGenerator);
        }
    }
    
    // Update followers
    UFollowerManager* FollowerManager = GetFollowerManager();
    if (FollowerManager)
    {
        FollowerManager->UpdateFollowers();
    }
    
    // Check for random events
    if (FMath::RandRange(1, 10) <= 3) // 30% chance each day
    {
        TriggerRandomEvent();
    }
    
    // Trigger the day passed event for blueprints
    OnDayPassed();
}

void AOathGameMode::ProcessSeasonChange()
{
    // Update resource generation based on season
    UResourceManager* ResourceManager = GetResourceManager();
    if (ResourceManager)
    {
        ResourceManager->UpdateSeasonalResourceModifiers(CurrentSeason);
    }
    
    // Update kingdom for seasonal changes
    if (KingdomManager)
    {
        KingdomManager->ProcessSeasonalUpdate(CurrentSeason);
    }
    
    // Possibly trigger seasonal festival
    if (KingdomManager && KingdomManager->CurrentTier >= EKingdomTier::Village)
    {
        TriggerSeasonalFestival();
    }
    
    // Trigger the season changed event for blueprints
    OnSeasonChanged(CurrentSeason);
}

void AOathGameMode::UpdateWorldEnvironment()
{
    // Calculate time of day normalized from 0.0 to 1.0
    float TimeOfDay = CurrentDayTime / DayLength;
    
    // Update lighting and environmental effects based on time of day
    // This would typically be handled by a day/night cycle blueprint
    // Here we just expose the time value for blueprint usage
    
    // Example: Broadcast time of day to interested systems
    for (TActorIterator<AActor> It(GetWorld()); It; ++It)
    {
        // Check if actor implements a time of day interface
        // ITimeOfDayInterface* TODInterface = Cast<ITimeOfDayInterface>(*It);
        // if (TODInterface)
        // {
        //     TODInterface->OnTimeOfDayUpdated(TimeOfDay, CurrentSeason);
        // }
    }
}

void AOathGameMode::UpdateGameSystems(float DeltaTime)
{
    // Update AI and monster spawns
    UpdateMobSpawns();
    
    // Update resource nodes
    UpdateResourceNodes();
    
    // Process any queued game events
    ProcessEventQueue();
}

void AOathGameMode::UpdateMobSpawns()
{
    // Get player level and location
    AOathCharacter* PlayerCharacter = Cast<AOathCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
    if (!PlayerCharacter)
    {
        return;
    }
    
    int32 PlayerLevel = 1; // Placeholder - would get from player character
    FVector PlayerLocation = PlayerCharacter->GetActorLocation();
    
    // Check if we need to spawn new enemies
    // This would be based on the player's location, time of day, current
    // monster population in the area, etc.
    
    // Example: Spawn harder enemies at night
    float TimeOfDay = CurrentDayTime / DayLength;
    bool bIsNight = (TimeOfDay < 0.25f || TimeOfDay > 0.75f);
    
    // Increase spawn difficulty by player level and night modifier
    int32 SpawnLevel = PlayerLevel + (bIsNight ? 2 : 0);
    
    // Logic for determining if and what to spawn would go here
    // For demonstration purposes, we'll assume we're spawning
    
    // Spawn a notorious monster occasionally
    if (bIsNight && FMath::RandRange(1, 100) <= 5) // 5% chance
    {
        FString BiomeType = "Forest"; // Placeholder - would determine from player location
        
        // Spawn a notorious monster
        AActor* NotoriousMonster = ProceduralGenerator->GenerateNotoriousMonster(SpawnLevel, BiomeType);
        
        // Announce the spawn to the player
        // NotifyPlayer("A powerful creature stirs nearby...");
    }
}

void AOathGameMode::UpdateResourceNodes()
{
    // Periodically respawn harvested resource nodes
    
    // Get resource manager
    UResourceManager* ResourceManager = GetResourceManager();
    if (!ResourceManager)
    {
        return;
    }
    
    // Update resource node respawn timers
    ResourceManager->UpdateResourceNodes(GetWorld()->GetDeltaSeconds());
}

void AOathGameMode::ProcessEventQueue()
{
    // Process any queued game events
    // This could include things like delayed quest updates,
    // scheduled kingdom events, follower activities completing, etc.
    
    // For demonstration purposes, we'll leave this empty
}

void AOathGameMode::SaveGame(const FString& SaveSlot)
{
    // Create a new save game object
    UQuestSaveGame* SaveGameInstance = Cast<UQuestSaveGame>(UGameplayStatics::CreateSaveGameObject(UQuestSaveGame::StaticClass()));
    
    if (SaveGameInstance)
    {
        // Get quest manager and save quest data
        UQuestManager* QuestManager = GetQuestManager();
        if (QuestManager)
        {
            // Convert quests to save data
            for (UQuest* Quest : QuestManager->AvailableQuests)
            {
                SaveGameInstance->AvailableQuests.Add(UQuestSaveGame::ConvertQuestToSaveData(Quest));
            }
            
            for (UQuest* Quest : QuestManager->ActiveQuests)
            {
                SaveGameInstance->ActiveQuests.Add(UQuestSaveGame::ConvertQuestToSaveData(Quest));
            }
            
            for (UQuest* Quest : QuestManager->CompletedQuests)
            {
                SaveGameInstance->CompletedQuests.Add(UQuestSaveGame::ConvertQuestToSaveData(Quest));
            }
            
            for (UQuest* Quest : QuestManager->FailedQuests)
            {
                SaveGameInstance->FailedQuests.Add(UQuestSaveGame::ConvertQuestToSaveData(Quest));
            }
            
            // Save follower assigned quests
            // Simplified for demonstration
        }
        
        // Save kingdom data
        // This would involve serializing the kingdom manager state
        
        // Save follower data
        // This would involve serializing follower manager state
        
        // Save player data
        AOathCharacter* PlayerCharacter = Cast<AOathCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
        if (PlayerCharacter)
        {
            // Save player stats, inventory, etc.
        }
        
        // Save world time
        SaveGameInstance->WorldTime.CurrentDayTime = CurrentDayTime;
        SaveGameInstance->WorldTime.CurrentDay = CurrentDay;
        SaveGameInstance->WorldTime.CurrentSeason = CurrentSeason;
        
        // Actually save the game to a slot
        if (UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveSlot, 0))
        {
            UE_LOG(LogTemp, Display, TEXT("Game saved successfully to slot: %s"), *SaveSlot);
            
            // Notify player of successful save
            // NotifyPlayer("Game saved successfully.");
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to save game to slot: %s"), *SaveSlot);
            
            // Notify player of failed save
            // NotifyPlayer("Failed to save game.", true);
        }
    }
}

void AOathGameMode::LoadGame(const FString& SaveSlot)
{
    // Check if the save exists
    if (UGameplayStatics::DoesSaveGameExist(SaveSlot, 0))
    {
        // Load the save game
        UQuestSaveGame* SaveGameInstance = Cast<UQuestSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlot, 0));
        
        if (SaveGameInstance)
        {
            // Get quest manager and load quest data
            UQuestManager* QuestManager = GetQuestManager();
            if (QuestManager)
            {
                // Clear existing quests
                QuestManager->AvailableQuests.Empty();
                QuestManager->ActiveQuests.Empty();
                QuestManager->CompletedQuests.Empty();
                QuestManager->FailedQuests.Empty();
                
                // Convert save data to quests
                for (const FQuestSaveData& QuestData : SaveGameInstance->AvailableQuests)
                {
                    QuestManager->AvailableQuests.Add(UQuestSaveGame::ConvertSaveDataToQuest(QuestData));
                }
                
                for (const FQuestSaveData& QuestData : SaveGameInstance->ActiveQuests)
                {
                    QuestManager->ActiveQuests.Add(UQuestSaveGame::ConvertSaveDataToQuest(QuestData));
                }
                
                for (const FQuestSaveData& QuestData : SaveGameInstance->CompletedQuests)
                {
                    QuestManager->CompletedQuests.Add(UQuestSaveGame::ConvertSaveDataToQuest(QuestData));
                }
                
                for (const FQuestSaveData& QuestData : SaveGameInstance->FailedQuests)
                {
                    QuestManager->FailedQuests.Add(UQuestSaveGame::ConvertSaveDataToQuest(QuestData));
                }
                
                // Load follower assigned quests
                // Simplified for demonstration
            }
            
            // Load kingdom data
            // This would involve deserializing the kingdom manager state
            
            // Load follower data
            // This would involve deserializing follower manager state
            
            // Load player data
            AOathCharacter* PlayerCharacter = Cast<AOathCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
            if (PlayerCharacter)
            {
                // Load player stats, inventory, etc.
            }
            
            // Load world time
            CurrentDayTime = SaveGameInstance->WorldTime.CurrentDayTime;
            CurrentDay = SaveGameInstance->WorldTime.CurrentDay;
            CurrentSeason = SaveGameInstance->WorldTime.CurrentSeason;
            
            UE_LOG(LogTemp, Display, TEXT("Game loaded successfully from slot: %s"), *SaveSlot);
            
            // Notify player of successful load
            // NotifyPlayer("Game loaded successfully.");
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to load game from slot: %s"), *SaveSlot);
            
            // Notify player of failed load
            // NotifyPlayer("Failed to load game.", true);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No save game found in slot: %s"), *SaveSlot);
        
        // Notify player of missing save
        // NotifyPlayer("No save game found.", true);
    }
}

void AOathGameMode::InitializeWorld()
{
    // Generate initial world content when starting a new game
    
    // Generate initial quests
    UQuestManager* QuestManager = GetQuestManager();
    if (QuestManager && ProceduralGenerator)
    {
        QuestManager->GenerateRandomQuests(5, 1, 3, ProceduralGenerator);
    }
    
    // Initialize kingdom
    if (KingdomManager)
    {
        KingdomManager->InitializeKingdom();
    }
    
    // Spawn initial resource nodes
    UResourceManager* ResourceManager = GetResourceManager();
    if (ResourceManager)
    {
        ResourceManager->GenerateInitialResourceNodes(GetWorld());
    }
}

void AOathGameMode::TriggerRandomEvent()
{
    if (!KingdomManager || !ProceduralGenerator)
    {
        return;
    }
    
    // Generate a random event appropriate for the kingdom tier
    FKingdomEvent RandomEvent = ProceduralGenerator->GenerateRandomEvent(
        KingdomManager->CurrentTier, 
        KingdomManager->KingdomAlignment);
    
    // Process the event
    KingdomManager->ProcessKingdomEvent(RandomEvent);
    
    // Notify the player
    // NotifyPlayer("A kingdom event has occurred: " + RandomEvent.EventName);
}

void AOathGameMode::TriggerSeasonalFestival()
{
    if (!KingdomManager)
    {
        return;
    }
    
    // Each season has a different festival
    TArray<FString> FestivalNames = {
        "Spring Bloom Festival", "Summer Solstice Celebration", 
        "Autumn Harvest Feast", "Winter's Eve Ceremony"
    };
    
    FString FestivalName = FestivalNames[CurrentSeason];
    
    // Create a festival event
    FKingdomEvent FestivalEvent;
    FestivalEvent.EventName = FestivalName;
    FestivalEvent.EventDescription = "Your kingdom celebrates the change of seasons with a grand festival.";
    FestivalEvent.EventType = EKingdomEventType::Festival;
    FestivalEvent.EventOutcomes.Add(TEXT("KingdomHappiness"), 10.0f);
    FestivalEvent.EventOutcomes.Add(TEXT("FollowerAttraction"), 5.0f);
    FestivalEvent.EventOutcomes.Add(TEXT("Gold"), -100.0f * static_cast<int32>(KingdomManager->CurrentTier)); // Cost scales with kingdom size
    
    // Seasonal specific bonuses
    switch (CurrentSeason)
    {
        case 0: // Spring
            FestivalEvent.EventOutcomes.Add(TEXT("FoodProduction"), 15.0f);
            break;
        case 1: // Summer
            FestivalEvent.EventOutcomes.Add(TEXT("CombatRenown"), 15.0f);
            break;
        case 2: // Autumn
            FestivalEvent.EventOutcomes.Add(TEXT("ResourceProduction"), 15.0f);
            break;
        case 3: // Winter
            FestivalEvent.EventOutcomes.Add(TEXT("QuestRenown"), 15.0f);
            break;
    }
    
    // Process the festival event
    KingdomManager->ProcessKingdomEvent(FestivalEvent);
    
    // Notify the player
    // NotifyPlayer("A seasonal festival has begun: " + FestivalName);
}

int32 AOathGameMode::GetPlayerLevel() const
{
    AOathCharacter* PlayerCharacter = Cast<AOathCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
    if (PlayerCharacter)
    {
        // Assuming player character has a level property
        // return PlayerCharacter->Level;
        return 1; // Placeholder
    }
    return 1;
}

UQuestManager* AOathGameMode::GetQuestManager() const
{
    return Cast<UQuestManager>(GetComponentByClass(UQuestManager::StaticClass()));
}

UFollowerManager* AOathGameMode::GetFollowerManager() const
{
    return Cast<UFollowerManager>(GetComponentByClass(UFollowerManager::StaticClass()));
}

UResourceManager* AOathGameMode::GetResourceManager() const
{
    return Cast<UResourceManager>(GetComponentByClass(UResourceManager::StaticClass()));
}