// FollowerData.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "FollowerData.generated.h"

UENUM(BlueprintType)
enum class EFollowerProfession : uint8
{
    None UMETA(DisplayName = "None"),
    Blacksmith UMETA(DisplayName = "Blacksmith"),
    Farmer UMETA(DisplayName = "Farmer"),
    Miner UMETA(DisplayName = "Miner"),
    Lumberjack UMETA(DisplayName = "Lumberjack"),
    Hunter UMETA(DisplayName = "Hunter"),
    Alchemist UMETA(DisplayName = "Alchemist"),
    Guard UMETA(DisplayName = "Guard"),
    Merchant UMETA(DisplayName = "Merchant"),
    Scholar UMETA(DisplayName = "Scholar")
};

USTRUCT(BlueprintType)
struct OATH_API FFollowerData
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Name;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* Portrait;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EFollowerProfession Profession;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Level;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Happiness;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Productivity;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> Traits;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsHero;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, float> Relationships;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DailyGoldContribution;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, float> DailyResourceContribution;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> AvailableQuests;
};