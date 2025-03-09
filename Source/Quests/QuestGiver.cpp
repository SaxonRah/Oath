// QuestGiver.cpp
#include "QuestGiver.h"
#include "QuestManager.h"
#include "ProceduralGenerator.h"
#include "OathGameMode.h"
#include "Kismet/GameplayStatics.h"

AQuestGiver::AQuestGiver()
{
    PrimaryActorTick.bCanEverTick = false;
    
    bUseRandomQuests = true;
    NumRandomQuests = 3;
    MinDifficulty = 1;
    MaxDifficulty = 3;
    
    // Default to offering all quest types
    for (int32 i = 0; i <= static_cast<int32>(EQuestType::Mystery); ++i)
    {
        PreferredQuestTypes.Add(static_cast<EQuestType>(i));
    }
}

void AQuestGiver::BeginPlay()
{
    Super::BeginPlay();
    
    // Get references to the quest manager and procedural generator
    AOathGameMode* GameMode = Cast<AOathGameMode>(UGameplayStatics::GetGameMode(this));
    if (GameMode)
    {
        ProceduralGenerator = GameMode->ProceduralGenerator;
        
        // Get or create quest manager
        UQuestManager* FoundQuestManager = nullptr;
        for (const auto& Component : GameMode->GetComponents())
        {
            FoundQuestManager = Cast<UQuestManager>(Component);
            if (FoundQuestManager)
            {
                break;
            }
        }
        
        if (!FoundQuestManager)
        {
            // Create a quest manager if one doesn't exist
            QuestManager = NewObject<UQuestManager>(GameMode);
        }
        else
        {
            QuestManager = FoundQuestManager;
        }
    }
    
    // Generate quests if using random quests
    if (bUseRandomQuests)
    {
        GenerateQuests();
    }
    
    // Setup quest giver info for each offered quest
    for (UQuest* Quest : OfferedQuests)
    {
        if (Quest)
        {
            Quest->QuestGiver = GiverName;
            Quest->FactionName = FactionName;
        }
    }
}

TArray<UQuest*> AQuestGiver::GetAvailableQuests()
{
    TArray<UQuest*> AvailableQuests;
    
    for (UQuest* Quest : OfferedQuests)
    {
        if (Quest && Quest->Status == EQuestStatus::Available)
        {
            AvailableQuests.Add(Quest);
        }
    }
    
    return AvailableQuests;
}

void AQuestGiver::AcceptQuest(UQuest* Quest)
{
    if (QuestManager && Quest && Quest->Status == EQuestStatus::Available)
    {
        QuestManager->AcceptQuest(Quest);
    }
}

void AQuestGiver::TurnInQuest(UQuest* Quest)
{
    if (QuestManager && Quest && Quest->Status == EQuestStatus::Completed && Quest->QuestGiver == GiverName)
    {
        QuestManager->CompleteQuest(Quest);
    }
}

void AQuestGiver::GenerateQuests()
{
    if (ProceduralGenerator && bUseRandomQuests)
    {
        // Clear existing random quests
        OfferedQuests.Empty();
        
        // Generate new quests
        for (int32 i = 0; i < NumRandomQuests; ++i)
        {
            int32 Difficulty = FMath::RandRange(MinDifficulty, MaxDifficulty);
            
            // Choose a random quest type from preferred types
            EQuestType QuestType = PreferredQuestTypes[FMath::RandRange(0, PreferredQuestTypes.Num() - 1)];
            
            UQuest* NewQuest = ProceduralGenerator->GenerateQuest(Difficulty, FactionName, QuestType);
            if (NewQuest)
            {
                NewQuest->QuestGiver = GiverName;
                OfferedQuests.Add(NewQuest);
            }
        }
    }
}

bool AQuestGiver::HasQuestsAvailable()
{
    for (UQuest* Quest : OfferedQuests)
    {
        if (Quest && Quest->Status == EQuestStatus::Available)
        {
            return true;
        }
    }
    
    return false;
}

bool AQuestGiver::HasQuestsReadyToTurnIn()
{
    if (!QuestManager)
    {
        return false;
    }
    
    for (UQuest* Quest : QuestManager->ActiveQuests)
    {
        if (Quest && Quest->Status == EQuestStatus::Completed && Quest->QuestGiver == GiverName)
        {
            return true;
        }
    }
    
    return false;
}