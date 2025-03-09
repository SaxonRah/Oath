// ResourceData.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ResourceData.generated.h"

UENUM(BlueprintType)
enum class EResourceRarity : uint8
{
    Common UMETA(DisplayName = "Common"),
    Uncommon UMETA(DisplayName = "Uncommon"),
    Rare UMETA(DisplayName = "Rare"),
    Legendary UMETA(DisplayName = "Legendary"),
    Artifact UMETA(DisplayName = "Artifact")
};

USTRUCT(BlueprintType)
struct OATH_API FResourceData
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Name;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EResourceRarity Rarity;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* Icon;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 BaseValue;
    
    bool operator==(const FResourceData& Other) const
    {
        return Name == Other.Name;
    }
};

USTRUCT(BlueprintType)
struct OATH_API FLootItem
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Name;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EResourceRarity Rarity;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* Icon;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Value;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, float> Stats;
    
    bool operator==(const FLootItem& Other) const
    {
        return Name == Other.Name;
    }
};