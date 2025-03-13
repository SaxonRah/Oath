#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerStatsComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RPGSTATEMACHINES_API UPlayerStatsComponent : public UActorComponent
{
    GENERATED_BODY()

public:    
    UPlayerStatsComponent();

    virtual void BeginPlay() override;
    
    UFUNCTION(BlueprintCallable, Category = "Stats")
    int32 GetCharacterLevel() const;
    
    UFUNCTION(BlueprintCallable, Category = "Stats")
    void SetCharacterLevel(int32 NewLevel);
    
    UFUNCTION(BlueprintCallable, Category = "Stats")
    float GetStatValue(FName StatName) const;
    
    UFUNCTION(BlueprintCallable, Category = "Stats")
    void SetStatValue(FName StatName, float Value);
    
    UFUNCTION(BlueprintCallable, Category = "Stats")
    void AddStatBonus(FName StatName, float BonusValue);
    
    UFUNCTION(BlueprintCallable, Category = "Stats")
    void AddStatPercentageBonus(FName StatName, float PercentBonus);
    
private:
    UPROPERTY(VisibleAnywhere, Category = "Stats")
    int32 CharacterLevel = 1;
    
    UPROPERTY(VisibleAnywhere, Category = "Stats")
    TMap<FName, float> BaseStats;
    
    UPROPERTY(VisibleAnywhere, Category = "Stats")
    TMap<FName, float> StatBonuses;
    
    UPROPERTY(VisibleAnywhere, Category = "Stats")
    TMap<FName, float> StatPercentageBonuses;
};