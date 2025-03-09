// QuestObjectiveComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Quest.h"
#include "QuestObjectiveComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class OATH_API UQuestObjectiveComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuestObjectiveComponent();
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quests|Objective")
    TMap<FString, int32> QuestObjectiveIDs;
    
    UFUNCTION(BlueprintCallable, Category = "Quests|Objective")
    void RegisterWithQuestSystem();
    
    UFUNCTION(BlueprintCallable, Category = "Quests|Objective")
    void TriggerObjectiveProgress(FString QuestName, int32 ObjectiveIndex, int32 Progress = 1);
    
    UFUNCTION(BlueprintCallable, Category = "Quests|Objective")
    void TriggerObjectiveCompletion(FString QuestName, int32 ObjectiveIndex);
    
    UFUNCTION(BlueprintPure, Category = "Quests|Objective")
    bool IsRelevantForQuest(UQuest* Quest) const;
    
protected:
    virtual void BeginPlay() override;
    
private:
    UPROPERTY()
    class UQuestManager* QuestManager;
    
    UPROPERTY()
    TMap<FString, TArray<UQuest*>> AssociatedQuests;
    
    void FindQuestManager();
};