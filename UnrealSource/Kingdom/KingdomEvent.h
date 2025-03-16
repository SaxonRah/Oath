// KingdomEvent.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "KingdomManager.h"
#include "KingdomEvent.generated.h"

USTRUCT(BlueprintType)
struct OATH_API FKingdomEvent
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Name;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EKingdomTier MinimumTier;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EKingdomTier MaximumTier;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool RequiresPositiveAlignment;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool RequiresNegativeAlignment;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AlignmentEffect;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 GoldEffect;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, int32> ResourceEffects;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CombatRenownEffect;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float QuestRenownEffect;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float KingdomReputationEffect;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FollowerHappinessEffect;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ChanceToDestroyBuilding;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> PossibleRewards;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> PossibleFollowerTypes;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool RequiresPlayerChoice;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> PlayerChoiceOptions;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> PlayerChoiceResults;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EventWeight;
    
    FKingdomEvent()
    {
        Name = TEXT("Generic Event");
        Description = TEXT("A random event has occurred in your kingdom.");
        MinimumTier = EKingdomTier::Camp;
        MaximumTier = EKingdomTier::Kingdom;
        RequiresPositiveAlignment = false;
        RequiresNegativeAlignment = false;
        AlignmentEffect = 0.0f;
        GoldEffect = 0;
        CombatRenownEffect = 0.0f;
        QuestRenownEffect = 0.0f;
        KingdomReputationEffect = 0.0f;
        FollowerHappinessEffect = 0.0f;
        ChanceToDestroyBuilding = 0.0f;
        RequiresPlayerChoice = false;
        EventWeight = 1.0f;
    }
};