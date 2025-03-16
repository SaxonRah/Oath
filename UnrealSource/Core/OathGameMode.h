// OathGameMode.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "KingdomManager.h"
#include "ProceduralGenerator.h"
#include "OathGameMode.generated.h"

UCLASS()
class OATH_API AOathGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AOathGameMode();
    
    virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
    virtual void BeginPlay() override;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kingdom")
    AKingdomManager* KingdomManager;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural")
    UProceduralGenerator* ProceduralGenerator;
    
    UFUNCTION(BlueprintCallable, Category = "Game")
    void SaveGame();
    
    UFUNCTION(BlueprintCallable, Category = "Game")
    void LoadGame();
    
    UFUNCTION(BlueprintCallable, Category = "Game")
    void ProcessWorldTime(float DeltaTime);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Game")
    void OnDayPassed();
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Game")
    void OnSeasonChanged(int32 NewSeason);

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Game")
    float TimeScale;
    
    UPROPERTY(EditDefaultsOnly, Category = "Game")
    float DayLength; // In seconds
    
    UPROPERTY(EditDefaultsOnly, Category = "Game")
    int32 DaysPerSeason;
    
private:
    float CurrentDayTime;
    int32 CurrentDay;
    int32 CurrentSeason;
    
    void UpdateWorldTime(float DeltaTime);
    void SpawnRandomEncounters();
};