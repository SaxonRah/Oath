// BuildingData.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ResourceData.h"
#include "BuildingData.generated.h"

UENUM(BlueprintType)
enum class EBuildingType : uint8
{
    Residential UMETA(DisplayName = "Residential"),
    Production UMETA(DisplayName = "Production"),
    Commercial UMETA(DisplayName = "Commercial"),
    Military UMETA(DisplayName = "Military"),
    Utility UMETA(DisplayName = "Utility"),
    Cultural UMETA(DisplayName = "Cultural"),
    Monument UMETA(DisplayName = "Monument")
};

USTRUCT(BlueprintType)
struct OATH_API FBuildingData
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Name;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EBuildingType Type;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UStaticMesh* Mesh;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Level;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Size;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FResourceData, int32> ConstructionCost;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 GoldConstructionCost;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AestheticValue;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxOccupants;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DailyGoldContribution;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, float> DailyResourceContribution;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> RequiredKingdomTiers;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, float> AdjacencyBonuses;
};