// OathGameInstance.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "KingdomManager.h"
#include "ProceduralGenerator.h"
#include "OathGameInstance.generated.h"

UCLASS()
class OATH_API UOathGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    UOathGameInstance();
    
    virtual void Init() override;
    
    UFUNCTION(BlueprintCallable, Category = "Kingdom")
    AKingdomManager* GetKingdomManager() const { return KingdomManager; }
    
    UFUNCTION(BlueprintCallable, Category = "Procedural")
    UProceduralGenerator* GetProceduralGenerator() const { return ProceduralGenerator; }
    
    UFUNCTION(BlueprintCallable, Category = "Factions")
    TArray<FString> GetAllFactions() const;
    
    UFUNCTION(BlueprintCallable, Category = "Factions")
    void UpdateFactionAvailability(const FString& FactionName, float Reputation);
    
    UFUNCTION(BlueprintCallable, Category = "Events")
    void OnCombatRenownMilestoneReached(float Milestone);
    
    UFUNCTION(BlueprintCallable, Category = "Events")
    void OnQuestRenownMilestoneReached(float Milestone);
    
    UFUNCTION(BlueprintCallable, Category = "Save/Load")
    bool SaveGameState();
    
    UFUNCTION(BlueprintCallable, Category = "Save/Load")
    bool LoadGameState();

private:
    UPROPERTY()
    AKingdomManager* KingdomManager;
    
    UPROPERTY()
    UProceduralGenerator* ProceduralGenerator;
    
    UPROPERTY()
    TArray<FString> AvailableFactions;
    
    UPROPERTY()
    TMap<FString, float> FactionReputationThresholds;
    
    void InitializeFactions();
};