// WorldChunkManager.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BiomeManager.h"
#include "ProceduralGenerator.h"
#include "WorldChunkManager.generated.h"

// Structure to hold information about a world chunk
USTRUCT(BlueprintType)
struct FWorldChunk
{
    GENERATED_BODY()
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FIntPoint ChunkCoordinates;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bIsGenerated;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bIsVisible;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bIsExplored;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TArray<AActor*> SpawnedActors;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    EBiomeType DominantBiome;
    
    FWorldChunk()
        : ChunkCoordinates(FIntPoint::ZeroValue)
        , bIsGenerated(false)
        , bIsVisible(false)
        , bIsExplored(false)
        , DominantBiome(EBiomeType::Plains)
    {
    }
};

UCLASS()
class OATH_API AWorldChunkManager : public AActor
{
    GENERATED_BODY()

public:
    AWorldChunkManager();
    
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    
    // Configure chunk settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Chunks")
    int32 ChunkSize;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Chunks")
    int32 ChunkVisibilityRadius;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Chunks")
    int32 MaxConcurrentLoadedChunks;
    
    // Core functionality
    UFUNCTION(BlueprintCallable, Category = "World Chunks")
    void LoadChunksAroundLocation(const FVector& WorldLocation);
    
    UFUNCTION(BlueprintCallable, Category = "World Chunks")
    void UnloadDistantChunks(const FVector& WorldLocation);
    
    UFUNCTION(BlueprintCallable, Category = "World Chunks")
    bool IsLocationExplored(const FVector& WorldLocation);
    
    UFUNCTION(BlueprintCallable, Category = "World Chunks")
    void MarkLocationExplored(const FVector& WorldLocation, float ExplorationRadius);
    
    UFUNCTION(BlueprintCallable, Category = "World Chunks")
    FWorldChunk* GetChunkAtLocation(const FVector& WorldLocation);
    
    UFUNCTION(BlueprintCallable, Category = "World Chunks")
    FIntPoint WorldLocationToChunkCoordinates(const FVector& WorldLocation);
    
    UFUNCTION(BlueprintCallable, Category = "World Chunks")
    FVector ChunkCoordinatesToWorldLocation(const FIntPoint& ChunkCoordinates);
    
    UFUNCTION(BlueprintCallable, Category = "World Chunks")
    void GetAllExploredChunks(TArray<FIntPoint>& OutExploredChunks);
    
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    bool GenerateChunk(const FIntPoint& ChunkCoordinates);
    
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void DestroyChunk(const FIntPoint& ChunkCoordinates);
    
    // Save/Load functionality
    UFUNCTION(BlueprintCallable, Category = "World Chunks")
    void SaveChunkExplorationData();
    
    UFUNCTION(BlueprintCallable, Category = "World Chunks")
    void LoadChunkExplorationData();
    
    // References to other systems
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Chunks")
    UBiomeManager* BiomeManager;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Chunks")
    UProceduralGenerator* ProceduralGenerator;
    
private:
    // Map of all chunks indexed by their coordinates
    UPROPERTY()
    TMap<FIntPoint, FWorldChunk> ChunkMap;
    
    // Keep track of current player chunk for optimization
    FIntPoint CurrentPlayerChunk;
    
    // Helper methods
    bool EnsureBiomeManagerExists();
    bool EnsureProceduralGeneratorExists();
    void DetermineDominantBiome(FWorldChunk& Chunk);
    void UpdateVisibilityForChunk(FWorldChunk& Chunk, bool bVisible);
    void CleanupChunks();
};