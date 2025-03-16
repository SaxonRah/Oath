// Quest.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ResourceData.h"
#include "Quest.generated.h"

UENUM(BlueprintType)
enum class EQuestType : uint8
{
    Fetch UMETA(DisplayName = "Fetch"),
    Kill UMETA(DisplayName = "Kill"),
    Escort UMETA(DisplayName = "Escort"),
    Explore UMETA(DisplayName = "Explore"),
    Build UMETA(DisplayName = "Build"),
    Diplomatic UMETA(DisplayName = "Diplomatic"),
    Mystery UMETA(DisplayName = "Mystery")
};

UENUM(BlueprintType)
enum class EQuestStatus : uint8
{
    Available UMETA(DisplayName = "Available"),
    InProgress UMETA(DisplayName = "In Progress"),
    Completed UMETA(DisplayName = "Completed"),
    Failed UMETA(DisplayName = "Failed")
};

USTRUCT(BlueprintType)
struct OATH_API FQuestObjective
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentProgress;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RequiredProgress;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsCompleted;
};

UCLASS(BlueprintType, Blueprintable)
class OATH_API UQuest : public UObject
{
    GENERATED_BODY()

public:
    UQuest();
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestName;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuestType Type;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuestStatus Status;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 DifficultyLevel;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float QuestRenownReward;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 GoldReward;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TMap<FResourceData, int32> MaterialRewards;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FLootItem> ItemRewards;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuestObjective> Objectives;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestGiver;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString FactionName;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float FactionReputationReward;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bCanBeAssignedToFollower;
    
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void UpdateObjective(int32 ObjectiveIndex, int32 Progress);
    
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool AreAllObjectivesComplete();
    
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CompleteQuest();
    
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void FailQuest();
};