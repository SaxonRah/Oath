// BiomeManager.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ResourceData.h"
#include "BiomeManager.generated.h"

UENUM(BlueprintType)
enum class EBiomeType : uint8
{
    Forest UMETA(DisplayName = "Forest"),
    Mountain UMETA(DisplayName = "Mountain"),
    Desert UMETA(DisplayName = "Desert"),
    Swamp UMETA(DisplayName = "Swamp"),
    Plains UMETA(DisplayName = "Plains"),
    Coast UMETA(DisplayName = "Coast"),
    Tundra UMETA(DisplayName = "Tundra"),
    Volcanic UMETA(DisplayName = "Volcanic")
};

USTRUCT(BlueprintType)
struct FBiomeSettings
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor BiomeColor;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<TSubclassOf<AActor>> VegetationTypes;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<TSubclassOf<AActor>> EnemyTypes;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<TSubclassOf<AActor>> ResourceNodeTypes;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float EnemySpawnRate;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float ResourceNodeDensity;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TMap<EResourceRarity, float> ResourceRarityDistribution;
    
    // Season modifiers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TMap<int32, float> SeasonResourceModifiers; // 0=Spring, 1=Summer, 2=Fall, 3=Winter
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TMap<int32, float> SeasonEnemyModifiers;
};

UCLASS(Blueprintable)
class OATH_API UBiomeManager : public UObject
{
    GENERATED_BODY()

public:
    UBiomeManager();
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TMap<EBiomeType, FBiomeSettings> BiomeSettings;
    
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    EBiomeType GetBiomeTypeAtLocation(const FVector& WorldLocation);
    
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    FBiomeSettings GetBiomeSettingsAtLocation(const FVector& WorldLocation);
    
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    void GetVegetationTypesForBiome(EBiomeType BiomeType, TArray<TSubclassOf<AActor>>& OutVegetationTypes);
    
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    void GetEnemyTypesForBiome(EBiomeType BiomeType, TArray<TSubclassOf<AActor>>& OutEnemyTypes);
    
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    void GetResourceNodeTypesForBiome(EBiomeType BiomeType, TArray<TSubclassOf<AActor>>& OutResourceNodeTypes);
    
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    float GetResourceModifierForBiomeAndSeason(EBiomeType BiomeType, int32 Season);
    
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    float GetEnemyModifierForBiomeAndSeason(EBiomeType BiomeType, int32 Season);
    
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    bool GetRandomResourceRarityForBiome(EBiomeType BiomeType, EResourceRarity& OutRarity);
    
    // Set up noise for procedural biome distribution
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    float BiomeNoiseScale;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    int32 BiomeNoiseSeed;
    
private:
    // Helper functions for noise generation and biome determination
    float GetNoise2D(float X, float Y) const;
    EBiomeType DetermineBiomeFromNoise(float NoiseValue, float Elevation, float Moisture) const;
};