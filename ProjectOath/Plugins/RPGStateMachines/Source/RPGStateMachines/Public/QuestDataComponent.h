#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "QuestDataComponent.generated.h"

UENUM(BlueprintType)
enum class EQuestStatus : uint8
{
    Locked,
    Available,
    InProgress,
    Completed,
    Failed
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RPGSTATEMACHINES_API UQuestDataComponent : public UActorComponent
{
    GENERATED_BODY()

public:    
    UQuestDataComponent();

    virtual void BeginPlay() override;
    
    UFUNCTION(BlueprintCallable, Category = "Quest")
    EQuestStatus GetQuestStatus(FName QuestID) const;
    
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void SetQuestStatus(FName QuestID, EQuestStatus Status);
    
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void GiveQuestRewards(const TArray<FName>& RewardIDs);
    
private:
    UPROPERTY(VisibleAnywhere, Category = "Quest")
    TMap<FName, EQuestStatus> QuestStatuses;
};