// BiomeManager.cpp
#include "BiomeManager.h"
#include "Math/UnrealMathUtility.h"

UBiomeManager::UBiomeManager()
{
    // Default values
    BiomeNoiseScale = 0.001f;
    BiomeNoiseSeed = 12345;
    
    // Set up default biome settings
    // Forest biome
    FBiomeSettings ForestSettings;
    ForestSettings.BiomeColor = FLinearColor(0.0f, 0.5f, 0.0f);
    ForestSettings.VegetationDensity = 0.8f;
    ForestSettings.EnemySpawnRate = 0.5f;
    ForestSettings.ResourceNodeDensity = 0.6f;
    ForestSettings.ResourceRarityDistribution.Add(EResourceRarity::Common, 0.6f);
    ForestSettings.ResourceRarityDistribution.Add(EResourceRarity::Uncommon, 0.3f);
    ForestSettings.ResourceRarityDistribution.Add(EResourceRarity::Rare, 0.08f);
    ForestSettings.ResourceRarityDistribution.Add(EResourceRarity::Legendary, 0.02f);
    ForestSettings.SeasonResourceModifiers.Add(0, 1.2f); // Spring bonus
    ForestSettings.SeasonResourceModifiers.Add(1, 1.0f); // Summer normal
    ForestSettings.SeasonResourceModifiers.Add(2, 1.5f); // Fall bonus
    ForestSettings.SeasonResourceModifiers.Add(3, 0.7f); // Winter penalty
    ForestSettings.SeasonEnemyModifiers.Add(0, 1.0f);
    ForestSettings.SeasonEnemyModifiers.Add(1, 1.2f);
    ForestSettings.SeasonEnemyModifiers.Add(2, 1.0f);
    ForestSettings.SeasonEnemyModifiers.Add(3, 0.8f);
    BiomeSettings.Add(EBiomeType::Forest, ForestSettings);
    
    // Mountain biome
    FBiomeSettings MountainSettings;
    MountainSettings.BiomeColor = FLinearColor(0.5f, 0.5f, 0.5f);
    MountainSettings.VegetationDensity = 0.3f;
    MountainSettings.EnemySpawnRate = 0.7f;
    MountainSettings.ResourceNodeDensity = 0.9f;
    MountainSettings.ResourceRarityDistribution.Add(EResourceRarity::Common, 0.5f);
    MountainSettings.ResourceRarityDistribution.Add(EResourceRarity::Uncommon, 0.3f);
    MountainSettings.ResourceRarityDistribution.Add(EResourceRarity::Rare, 0.15f);
    MountainSettings.ResourceRarityDistribution.Add(EResourceRarity::Legendary, 0.05f);
    MountainSettings.SeasonResourceModifiers.Add(0, 0.8f);
    MountainSettings.SeasonResourceModifiers.Add(1, 1.2f);
    MountainSettings.SeasonResourceModifiers.Add(2, 1.0f);
    MountainSettings.SeasonResourceModifiers.Add(3, 0.5f);
    MountainSettings.SeasonEnemyModifiers.Add(0, 0.8f);
    MountainSettings.SeasonEnemyModifiers.Add(1, 1.0f);
    MountainSettings.SeasonEnemyModifiers.Add(2, 1.0f);
    MountainSettings.SeasonEnemyModifiers.Add(3, 1.3f);
    BiomeSettings.Add(EBiomeType::Mountain, MountainSettings);
    
    // Additional biomes would be set up similarly
    // ...
}

EBiomeType UBiomeManager::GetBiomeTypeAtLocation(const FVector& WorldLocation)
{
    // Use noise to determine biome type based on location
    float NoiseValue = GetNoise2D(WorldLocation.X, WorldLocation.Y);
    
    // Use elevation (Z) and a derived moisture value for more realistic biome distribution
    float Elevation = WorldLocation.Z / 10000.0f; // Normalize elevation 
    float Moisture = GetNoise2D(WorldLocation.X * 1.5f, WorldLocation.Y * 1.5f) * 0.5f + 0.5f;
    
    return DetermineBiomeFromNoise(NoiseValue, Elevation, Moisture);
}

FBiomeSettings UBiomeManager::GetBiomeSettingsAtLocation(const FVector& WorldLocation)
{
    EBiomeType BiomeType = GetBiomeTypeAtLocation(WorldLocation);
    
    // Return the settings for this biome
    if (BiomeSettings.Contains(BiomeType))
    {
        return BiomeSettings[BiomeType];
    }
    
    // Default to forest if biome not found
    return BiomeSettings[EBiomeType::Forest];
}

void UBiomeManager::GetVegetationTypesForBiome(EBiomeType BiomeType, TArray<TSubclassOf<AActor>>& OutVegetationTypes)
{
    if (BiomeSettings.Contains(BiomeType))
    {
        OutVegetationTypes = BiomeSettings[BiomeType].VegetationTypes;
    }
    else
    {
        OutVegetationTypes.Empty();
    }
}

void UBiomeManager::GetEnemyTypesForBiome(EBiomeType BiomeType, TArray<TSubclassOf<AActor>>& OutEnemyTypes)
{
    if (BiomeSettings.Contains(BiomeType))
    {
        OutEnemyTypes = BiomeSettings[BiomeType].EnemyTypes;
    }
    else
    {
        OutEnemyTypes.Empty();
    }
}

void UBiomeManager::GetResourceNodeTypesForBiome(EBiomeType BiomeType, TArray<TSubclassOf<AActor>>& OutResourceNodeTypes)
{
    if (BiomeSettings.Contains(BiomeType))
    {
        OutResourceNodeTypes = BiomeSettings[BiomeType].ResourceNodeTypes;
    }
    else
    {
        OutResourceNodeTypes.Empty();
    }
}

float UBiomeManager::GetResourceModifierForBiomeAndSeason(EBiomeType BiomeType, int32 Season)
{
    if (BiomeSettings.Contains(BiomeType) && BiomeSettings[BiomeType].SeasonResourceModifiers.Contains(Season))
    {
        return BiomeSettings[BiomeType].SeasonResourceModifiers[Season];
    }
    
    return 1.0f; // Default modifier if not found
}

float UBiomeManager::GetEnemyModifierForBiomeAndSeason(EBiomeType BiomeType, int32 Season)
{
    if (BiomeSettings.Contains(BiomeType) && BiomeSettings[BiomeType].SeasonEnemyModifiers.Contains(Season))
    {
        return BiomeSettings[BiomeType].SeasonEnemyModifiers[Season];
    }
    
    return 1.0f; // Default modifier if not found
}

bool UBiomeManager::GetRandomResourceRarityForBiome(EBiomeType BiomeType, EResourceRarity& OutRarity)
{
    if (!BiomeSettings.Contains(BiomeType))
    {
        return false;
    }
    
    const FBiomeSettings& Settings = BiomeSettings[BiomeType];
    
    // Calculate total weight of all rarities
    float TotalWeight = 0.0f;
    for (const TPair<EResourceRarity, float>& RarityPair : Settings.ResourceRarityDistribution)
    {
        TotalWeight += RarityPair.Value;
    }
    
    if (TotalWeight <= 0.0f)
    {
        return false;
    }
    
    // Roll for a random rarity based on weights
    float RandomRoll = FMath::FRand() * TotalWeight;
    float AccumulatedWeight = 0.0f;
    
    for (const TPair<EResourceRarity, float>& RarityPair : Settings.ResourceRarityDistribution)
    {
        AccumulatedWeight += RarityPair.Value;
        
        if (RandomRoll <= AccumulatedWeight)
        {
            OutRarity = RarityPair.Key;
            return true;
        }
    }
    
    // Fallback to Common if something went wrong
    OutRarity = EResourceRarity::Common;
    return true;
}

float UBiomeManager::GetNoise2D(float X, float Y) const
{
    // Simple 2D Perlin noise implementation
    // In a real game, you would use a proper noise library
    // This is just a placeholder implementation
    
    // Apply noise scale
    X *= BiomeNoiseScale;
    Y *= BiomeNoiseScale;
    
    // Apply seed
    X += BiomeNoiseSeed * 0.1f;
    Y += BiomeNoiseSeed * 0.7f;
    
    // Simplified noise calculation (in real implementation, use Perlin noise)
    float n1 = FMath::Sin(X * 12.9898f + Y * 78.233f) * 43758.5453f;
    float n2 = FMath::Sin(X * 39.346f + Y * 11.135f) * 53758.5453f;
    
    n1 = FMath::Fractional(n1);
    n2 = FMath::Fractional(n2);
    
    // Interpolate between different octaves
    return FMath::Lerp(n1, n2, 0.5f) * 2.0f - 1.0f;
}

EBiomeType UBiomeManager::DetermineBiomeFromNoise(float NoiseValue, float Elevation, float Moisture) const
{
    // Simple biome determination based on noise value, elevation, and moisture
    // In a full implementation, this would be more complex, using multiple noise layers
    
    // Convert noise to 0-1 range
    float NormalizedNoise = (NoiseValue + 1.0f) * 0.5f;
    
    // High elevation = mountains
    if (Elevation > 0.7f)
    {
        if (NormalizedNoise > 0.8f)
        {
            return EBiomeType::Volcanic;
        }
        return EBiomeType::Mountain;
    }
    
    // Low elevation with high moisture = swamp
    if (Elevation < 0.3f && Moisture > 0.7f)
    {
        return EBiomeType::Swamp;
    }
    
    // Low elevation with low moisture = desert
    if (Elevation < 0.4f && Moisture < 0.3f)
    {
        return EBiomeType::Desert;
    }
    
    // Moderate elevation with high moisture = forest
    if (Elevation >= 0.3f && Elevation <= 0.7f && Moisture > 0.5f)
    {
        return EBiomeType::Forest;
    }
    
    // Moderate elevation with low moisture = plains
    if (Elevation >= 0.3f && Elevation <= 0.7f && Moisture <= 0.5f)
    {
        return EBiomeType::Plains;
    }
    
    // Very low elevation = coast
    if (Elevation < 0.2f)
    {
        return EBiomeType::Coast;
    }
    
    // Very high noise + low moisture = tundra (cold)
    if (NormalizedNoise > 0.7f && Moisture < 0.4f)
    {
        return EBiomeType::Tundra;
    }
    
    // Default to plains
    return EBiomeType::Plains;
}