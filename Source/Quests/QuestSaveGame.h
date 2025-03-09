// QuestSaveGame.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Quest.h"
#include "QuestSaveGame.generated.h"

USTRUCT(BlueprintType)
struct FQuestObjectiveSaveData
{
    GENERATED_BODY()
    
    UPROPERTY(SaveGame)
    FString Description;
    
    UPROPERTY(SaveGame)
    int32 CurrentProgress;
    
    UPROPERTY(SaveGame)
    int32 RequiredProgress;
    
    UPROPERTY(SaveGame)
    bool bIsCompleted;
};

USTRUCT(BlueprintType)
struct FQuestSaveData
{
    GENERATED_BODY()
    
    UPROPERTY(SaveGame)
    FString QuestName;
    
    UPROPERTY(SaveGame)
    FString Description;
    
    UPROPERTY(SaveGame)
    EQuestType Type;
    
    UPROPERTY(SaveGame)
    EQuestStatus Status;
    
    UPROPERTY(SaveGame)
    int32 DifficultyLevel;
    
    UPROPERTY(SaveGame)
    float QuestRenownReward;
    
    UPROPERTY(SaveGame)
    int32 GoldReward;
    
    UPROPERTY(SaveGame)
    TMap<FString, int32> MaterialRewards;
    
    UPROPERTY(SaveGame)
    TArray<FQuestObjectiveSaveData> Objectives;
    
    UPROPERTY(SaveGame)
    FString QuestGiver;
    
    UPROPERTY(SaveGame)
    FString FactionName;
    
    UPROPERTY(SaveGame)
    float FactionReputationReward;
    
    UPROPERTY(SaveGame)
    bool bCanBeAssignedToFollower;
};

UCLASS()
class OATH_API UQuestSaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    UQuestSaveGame();
    
    UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Quests|Save")
    TArray<FQuestSaveData> AvailableQuests;
    
    UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Quests|Save")
    TArray<FQuestSaveData> ActiveQuests;
    
    UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Quests|Save")
    TArray<FQuestSaveData> CompletedQuests;
    
    UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Quests|Save")
    TArray<FQuestSaveData> FailedQuests;
    
    UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Quests|Save")
    TMap<FString, FString> FollowerAssignedQuests;
    
    UPROPERTY(SaveGame, BlueprintReadWrite, Category = "Quests|Save")
    TMap<FString, float> FollowerQuestProgress;
    
    // Helper functions
    static FQuestObjectiveSaveData ConvertObjectiveToSaveData(const FQuestObjective& Objective);
    static FQuestObjective ConvertSaveDataToObjective(const FQuestObjectiveSaveData& SaveData);
    
    static FQuestSaveData ConvertQuestToSaveData(UQuest* Quest);
    static UQuest* ConvertSaveDataToQuest(const FQuestSaveData& SaveData);
};