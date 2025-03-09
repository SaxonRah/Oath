// QuestManager.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Quest.h"
#include "ProceduralGenerator.h"
#include "QuestManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestStatusChanged, UQuest*, Quest);

UCLASS(BlueprintType, Blueprintable)
class OATH_API UQuestManager : public UObject
{
    GENERATED_BODY()

public:
    UQuestManager();
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quests")
    TArray<UQuest*> AvailableQuests;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quests")
    TArray<UQuest*> ActiveQuests;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quests")
    TArray<UQuest*> CompletedQuests;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quests")
    TArray<UQuest*> FailedQuests;
    
    UPROPERTY(BlueprintAssignable, Category = "Quests")
    FOnQuestStatusChanged OnQuestAccepted;
    
    UPROPERTY(BlueprintAssignable, Category = "Quests")
    FOnQuestStatusChanged OnQuestCompleted;
    
    UPROPERTY(BlueprintAssignable, Category = "Quests")
    FOnQuestStatusChanged OnQuestFailed;
    
    UFUNCTION(BlueprintCallable, Category = "Quests")
    void AddAvailableQuest(UQuest* Quest);
    
    UFUNCTION(BlueprintCallable, Category = "Quests")
    bool AcceptQuest(UQuest* Quest);
    
    UFUNCTION(BlueprintCallable, Category = "Quests")
    void CompleteQuest(UQuest* Quest);
    
    UFUNCTION(BlueprintCallable, Category = "Quests")
    void FailQuest(UQuest* Quest);
    
    UFUNCTION(BlueprintCallable, Category = "Quests")
    void GenerateRandomQuests(int32 Count, int32 MinDifficulty, int32 MaxDifficulty, UProceduralGenerator* Generator);
    
    UFUNCTION(BlueprintCallable, Category = "Quests")
    TArray<UQuest*> GetQuestsByFaction(FString FactionName);
    
    UFUNCTION(BlueprintCallable, Category = "Quests")
    TArray<UQuest*> GetQuestsByType(EQuestType QuestType);
    
    UFUNCTION(BlueprintCallable, Category = "Quests")
    void AssignQuestToFollower(UQuest* Quest, FString FollowerName);
    
    UFUNCTION(BlueprintCallable, Category = "Quests")
    void UpdateAssignedQuests(float DeltaTime);
    
private:
    UPROPERTY()
    TMap<FString, UQuest*> FollowerAssignedQuests;
    
    UPROPERTY()
    TMap<FString, float> FollowerQuestProgress;
    
    void MoveQuestBetweenLists(UQuest* Quest, TArray<UQuest*>& SourceList, TArray<UQuest*>& DestinationList);
};