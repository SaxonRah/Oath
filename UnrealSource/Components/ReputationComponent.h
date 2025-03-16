// ReputationComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ReputationComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class OATH_API UReputationComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UReputationComponent();
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reputation")
    float CombatRenown;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reputation")
    float QuestRenown;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reputation")
    float KingdomReputation;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reputation")
    TMap<FString, float> FactionReputation;
    
    UFUNCTION(BlueprintCallable, Category = "Reputation")
    void GainCombatRenown(float Amount, bool bNotify = true);
    
    UFUNCTION(BlueprintCallable, Category = "Reputation")
    void GainQuestRenown(float Amount, bool bNotify = true);
    
    UFUNCTION(BlueprintCallable, Category = "Reputation")
    void GainKingdomReputation(float Amount, bool bNotify = true);
    
    UFUNCTION(BlueprintCallable, Category = "Reputation")
    void ModifyFactionReputation(FString FactionName, float Amount, bool bNotify = true);
    
    UFUNCTION(BlueprintPure, Category = "Reputation")
    int32 GetKingdomTier() const;
    
    UFUNCTION(BlueprintCallable, Category = "Reputation")
    void RegisterReputationChangedDelegate(const FOnReputationChangedDelegate& Delegate);


    UFUNCTION(BlueprintPure, Category = "Reputation")
    float GetFactionReputation(const FString& FactionName) const
    {
        return FactionReputation.Contains(FactionName) ? FactionReputation[FactionName] : 0.0f;
    }
    
    UFUNCTION(BlueprintPure, Category = "Reputation")
    FString GetFactionStandingName(const FString& FactionName) const
    {
        float Rep = GetFactionReputation(FactionName);
        
        if (Rep >= 75.0f)
            return TEXT("Exalted");
        else if (Rep >= 50.0f)
            return TEXT("Honored");
        else if (Rep >= 25.0f)
            return TEXT("Friendly");
        else if (Rep >= 0.0f)
            return TEXT("Neutral");
        else if (Rep >= -25.0f)
            return TEXT("Unfriendly");
        else if (Rep >= -50.0f)
            return TEXT("Hostile");
        else
            return TEXT("Hated");
    }
    
    UFUNCTION(BlueprintPure, Category = "Reputation")
    FString GetKingdomTierName() const
    {
        switch (GetKingdomTier())
        {
            case 0: return TEXT("Camp");
            case 1: return TEXT("Village");
            case 2: return TEXT("Town");
            case 3: return TEXT("City");
            case 4: return TEXT("Kingdom");
            default: return TEXT("Unknown");
        }
    }
    
    UFUNCTION(BlueprintPure, Category = "Reputation")
    int32 GetMaxFollowersForTier() const
    {
        switch (GetKingdomTier())
        {
            case 0: return 5;    // Camp
            case 1: return 15;   // Village
            case 2: return 30;   // Town
            case 3: return 50;   // City
            case 4: return 100;  // Kingdom
            default: return 0;
        }
    }
    
    UFUNCTION(BlueprintPure, Category = "Reputation")
    int32 GetMaxBuildingsForTier() const
    {
        switch (GetKingdomTier())
        {
            case 0: return 3;    // Camp
            case 1: return 10;   // Village
            case 2: return 25;   // Town
            case 3: return 50;   // City
            case 4: return 100;  // Kingdom
            default: return 0;
        }
    }
    
    UFUNCTION(BlueprintCallable, Category = "Reputation")
    float GetReputationNeededForNextTier() const
    {
        int32 CurrentTier = GetKingdomTier();
        
        switch (CurrentTier)
        {
            case 0: return 100.0f - KingdomReputation;  // Camp to Village
            case 1: return 500.0f - KingdomReputation;  // Village to Town
            case 2: return 1500.0f - KingdomReputation; // Town to City
            case 3: return 5000.0f - KingdomReputation; // City to Kingdom
            case 4: return 0.0f; // Already at max tier
            default: return 0.0f;
        }
    }

protected:
    virtual void BeginPlay() override;
    
private:
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnReputationChangedDelegate, FString, ReputationType, float, OldValue, float, NewValue);
    
    UPROPERTY(BlueprintAssignable, Category = "Reputation|Events")
    FOnReputationChangedDelegate OnReputationChanged;

    // Track previous values for milestone checks
    float PreviousCombatRenown;
    float PreviousQuestRenown;
    TMap<FString, float> PreviousFactionReputations;
    
    // Milestone event delegates
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCombatRenownMilestoneReachedDelegate, float, Milestone);
    UPROPERTY(BlueprintAssignable, Category = "Reputation|Events")
    FOnCombatRenownMilestoneReachedDelegate OnCombatRenownMilestoneReached;
    
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestRenownMilestoneReachedDelegate, float, Milestone);
    UPROPERTY(BlueprintAssignable, Category = "Reputation|Events")
    FOnQuestRenownMilestoneReachedDelegate OnQuestRenownMilestoneReached;
    
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnFactionReputationThresholdCrossedDelegate, FString, FactionName, float, Threshold, bool, IsIncrease);
    UPROPERTY(BlueprintAssignable, Category = "Reputation|Events")
    FOnFactionReputationThresholdCrossedDelegate OnFactionReputationThresholdCrossed;
    
    // Helper functions for milestone checks
    void CheckCombatRenownMilestones();
    void CheckQuestRenownMilestones();
    void CheckFactionReputationThresholds(const FString& FactionName);
    void GenerateMilestoneContent(const FString& ReputationType, float Milestone);

};