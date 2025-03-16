// QuestGiver.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Quest.h"
#include "QuestGiver.generated.h"

UCLASS()
class OATH_API AQuestGiver : public AActor
{
    GENERATED_BODY()

public:
    AQuestGiver();
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quests|Giver")
    FString GiverName;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quests|Giver")
    UTexture2D* Portrait;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quests|Giver")
    FString FactionName;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quests|Giver")
    TArray<UQuest*> OfferedQuests;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QuestGiver")
    bool bUseRandomQuests;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quests|Giver", meta = (EditCondition = "bUseRandomQuests"))
    int32 NumRandomQuests;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quests|Giver", meta = (EditCondition = "bUseRandomQuests"))
    int32 MinDifficulty;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quests|Giver", meta = (EditCondition = "bUseRandomQuests"))
    int32 MaxDifficulty;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quests|Giver", meta = (EditCondition = "bUseRandomQuests"))
    TArray<EQuestType> PreferredQuestTypes;
    
    UFUNCTION(BlueprintCallable, Category = "Quests|Giver")
    TArray<UQuest*> GetAvailableQuests();
    
    UFUNCTION(BlueprintCallable, Category = "Quests|Giver")
    void AcceptQuest(UQuest* Quest);
    
    UFUNCTION(BlueprintCallable, Category = "Quests|Giver")
    void TurnInQuest(UQuest* Quest);
    
    UFUNCTION(BlueprintCallable, Category = "Quests|Giver")
    void GenerateQuests();
    
    UFUNCTION(BlueprintCallable, Category = "Quests|Giver")
    bool HasQuestsAvailable();
    
    UFUNCTION(BlueprintCallable, Category = "Quests|Giver")
    bool HasQuestsReadyToTurnIn();
    
    virtual void BeginPlay() override;
    
private:
    UPROPERTY()
    class UQuestManager* QuestManager;
    
    UPROPERTY()
    class UProceduralGenerator* ProceduralGenerator;
};