// WorldChunkManager.cpp
#include "WorldChunkManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "OathCharacter.h"
#include "EngineUtils.h"
#include "ResourceNode.h"
#include "EnemyCharacter.h"

AWorldChunkManager::AWorldChunkManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Default chunk settings
    ChunkSize = 1000; // 1000 units (10 meters assuming 1 unit = 1 cm)
    ChunkVisibilityRadius = 2; // Load chunks up to 2 chunks away from player
    MaxConcurrentLoadedChunks = 25; // Maximum 5x5 grid of chunks
    
    // Create components
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
}

void AWorldChunkManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Ensure managers exist
    EnsureBiomeManagerExists();
    EnsureProceduralGeneratorExists();
    
    // Load saved exploration data
    LoadChunkExplorationData();
    
    // Initial chunk loading
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
    if (PlayerPawn)
    {
        FVector PlayerLocation = PlayerPawn->GetActorLocation();
        LoadChunksAroundLocation(PlayerLocation);
    }
}

void AWorldChunkManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update chunks based on player position
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
    if (PlayerPawn)
    {
        FVector PlayerLocation = PlayerPawn->GetActorLocation();
        
        // Check if player has moved to a new chunk
        FIntPoint NewPlayerChunk = WorldLocationToChunkCoordinates(PlayerLocation);
        
        if (NewPlayerChunk != CurrentPlayerChunk)
        {
            // Player has moved to a new chunk
            CurrentPlayerChunk = NewPlayerChunk;
            
            // Load chunks in new visibility radius
            LoadChunksAroundLocation(PlayerLocation);
            
            // Unload distant chunks
            UnloadDistantChunks(PlayerLocation);
            
            // Mark current chunk as explored
            MarkLocationExplored(PlayerLocation, ChunkSize * 0.5f);
        }
    }
    
    // Periodically clean up chunks to avoid memory bloat
    CleanupChunks();
}

void AWorldChunkManager::LoadChunksAroundLocation(const FVector& WorldLocation)
{
    FIntPoint CenterChunk = WorldLocationToChunkCoordinates(WorldLocation);
    
    // Generate chunks in visibility radius
    for (int32 X = -ChunkVisibilityRadius; X <= ChunkVisibilityRadius; X++)
    {
        for (int32 Y = -ChunkVisibilityRadius; Y <= ChunkVisibilityRadius; Y++)
        {
            FIntPoint ChunkCoord = FIntPoint(CenterChunk.X + X, CenterChunk.Y + Y);
            
            // Check if this chunk is already generated
            if (!ChunkMap.Contains(ChunkCoord) || !ChunkMap[ChunkCoord].bIsGenerated)
            {
                // Generate the chunk
                GenerateChunk(ChunkCoord);
            }
            else if (!ChunkMap[ChunkCoord].bIsVisible)
            {
                // Chunk exists but is not visible, make it visible
                UpdateVisibilityForChunk(ChunkMap[ChunkCoord], true);
            }
        }
    }
}

void AWorldChunkManager::UnloadDistantChunks(const FVector& WorldLocation)
{
    FIntPoint CenterChunk = WorldLocationToChunkCoordinates(WorldLocation);
    
    // Get all loaded chunks
    TArray<FIntPoint> LoadedChunks;
    ChunkMap.GetKeys(LoadedChunks);
    
    for (const FIntPoint& ChunkCoord : LoadedChunks)
    {
        // Calculate distance to the center chunk
        int32 Distance = FMath::Max(FMath::Abs(ChunkCoord.X - CenterChunk.X), FMath::Abs(ChunkCoord.Y - CenterChunk.Y));
        
        if (Distance > ChunkVisibilityRadius && ChunkMap[ChunkCoord].bIsVisible)
        {
            // This chunk is outside visibility radius and is currently visible
            // Hide it but don't destroy it yet (it will be cleaned up periodically)
            UpdateVisibilityForChunk(ChunkMap[ChunkCoord], false);
        }
    }
}

bool AWorldChunkManager::IsLocationExplored(const FVector& WorldLocation)
{
    FIntPoint ChunkCoord = WorldLocationToChunkCoordinates(WorldLocation);
    
    if (ChunkMap.Contains(ChunkCoord))
    {
        return ChunkMap[ChunkCoord].bIsExplored;
    }
    
    return false;
}

void AWorldChunkManager::MarkLocationExplored(const FVector& WorldLocation, float ExplorationRadius)
{
    FIntPoint CenterChunk = WorldLocationToChunkCoordinates(WorldLocation);
    
    // Mark center chunk as explored
    if (ChunkMap.Contains(CenterChunk))
    {
        ChunkMap[CenterChunk].bIsExplored = true;
    }
    
    // Calculate how many chunks the exploration radius covers
    int32 ChunkRadius = FMath::CeilToInt(ExplorationRadius / ChunkSize);
    
    // Mark surrounding chunks within exploration radius
    for (int32 X = -ChunkRadius; X <= ChunkRadius; X++)
    {
        for (int32 Y = -ChunkRadius; Y <= ChunkRadius; Y++)
        {
            FIntPoint ChunkCoord = FIntPoint(CenterChunk.X + X, CenterChunk.Y + Y);
            
            // Skip if we're checking a chunk beyond exploration radius
            FVector ChunkCenter = ChunkCoordinatesToWorldLocation(ChunkCoord);
            float DistanceSquared = FVector::DistSquared2D(WorldLocation, ChunkCenter);
            
            if (DistanceSquared <= ExplorationRadius * ExplorationRadius)
            {
                // This chunk is within exploration radius
                if (ChunkMap.Contains(ChunkCoord))
                {
                    ChunkMap[ChunkCoord].bIsExplored = true;
                }
                else
                {
                    // Create an entry for this chunk if it doesn't exist
                    FWorldChunk NewChunk;
                    NewChunk.ChunkCoordinates = ChunkCoord;
                    NewChunk.bIsExplored = true;
                    ChunkMap.Add(ChunkCoord, NewChunk);
                }
            }
        }
    }
    
    // Save exploration data after significant changes
    SaveChunkExplorationData();
}

FWorldChunk* AWorldChunkManager::GetChunkAtLocation(const FVector& WorldLocation)
{
    FIntPoint ChunkCoord = WorldLocationToChunkCoordinates(WorldLocation);
    
    if (ChunkMap.Contains(ChunkCoord))
    {
        return &ChunkMap[ChunkCoord];
    }
    
    return nullptr;
}

FIntPoint AWorldChunkManager::WorldLocationToChunkCoordinates(const FVector& WorldLocation)
{
    int32 ChunkX = FMath::FloorToInt(WorldLocation.X / ChunkSize);
    int32 ChunkY = FMath::FloorToInt(WorldLocation.Y / ChunkSize);
    
    return FIntPoint(ChunkX, ChunkY);
}

FVector AWorldChunkManager::ChunkCoordinatesToWorldLocation(const FIntPoint& ChunkCoordinates)
{
    // Return the center of the chunk
    float X = (ChunkCoordinates.X * ChunkSize) + (ChunkSize * 0.5f);
    float Y = (ChunkCoordinates.Y * ChunkSize) + (ChunkSize * 0.5f);
    
    return FVector(X, Y, 0.0f);
}

void AWorldChunkManager::GetAllExploredChunks(TArray<FIntPoint>& OutExploredChunks)
{
    OutExploredChunks.Empty();
    
    for (const TPair<FIntPoint, FWorldChunk>& ChunkPair : ChunkMap)
    {
        if (ChunkPair.Value.bIsExplored)
        {
            OutExploredChunks.Add(ChunkPair.Key);
        }
    }
}

bool AWorldChunkManager::GenerateChunk(const FIntPoint& ChunkCoordinates)
{
   // Skip generation if chunk already exists and is generated
   if (ChunkMap.Contains(ChunkCoordinates) && ChunkMap[ChunkCoordinates].bIsGenerated)
   {
       // Just update visibility
       UpdateVisibilityForChunk(ChunkMap[ChunkCoordinates], true);
       return true;
   }
   
   // Create a new chunk or update existing one
   FWorldChunk& Chunk = ChunkMap.Contains(ChunkCoordinates) 
       ? ChunkMap[ChunkCoordinates] 
       : ChunkMap.Add(ChunkCoordinates, FWorldChunk());
   
   // Set chunk coordinates and generate flag
   Chunk.ChunkCoordinates = ChunkCoordinates;
   Chunk.bIsGenerated = true;
   Chunk.bIsVisible = true;
   
   // Determine the dominant biome for this chunk
   DetermineDominantBiome(Chunk);
   
   // Use the procedural generator to populate the chunk
   if (ProceduralGenerator)
   {
       FVector ChunkCenter = ChunkCoordinatesToWorldLocation(ChunkCoordinates);
       ProceduralGenerator->GenerateWorldChunk(FVector2D(ChunkCenter.X, ChunkCenter.Y), Chunk.SpawnedActors);
   }
   
   return true;
}


void AWorldChunkManager::DestroyChunk(const FIntPoint& ChunkCoordinates)
{
   if (ChunkMap.Contains(ChunkCoordinates))
   {
       FWorldChunk& Chunk = ChunkMap[ChunkCoordinates];
       
       // Destroy all spawned actors in this chunk
       for (AActor* Actor : Chunk.SpawnedActors)
       {
           if (Actor && !Actor->IsPendingKill())
           {
               Actor->Destroy();
           }
       }
       
       // Clear the actor array
       Chunk.SpawnedActors.Empty();
       
       // Mark as not generated but preserve exploration status
       Chunk.bIsGenerated = false;
       Chunk.bIsVisible = false;
   }
}

void AWorldChunkManager::SaveChunkExplorationData()
{
   // This would be implemented to save exploration data to a save game object
   // For now it's a placeholder
   // USaveGame* SaveGameInstance = UGameplayStatics::CreateSaveGameObject(UWorldChunkSaveGame::StaticClass());
   // UWorldChunkSaveGame* ChunkSaveGame = Cast<UWorldChunkSaveGame>(SaveGameInstance);
   
   // if (ChunkSaveGame)
   // {
   //     // Save exploration status only
   //     for (const TPair<FIntPoint, FWorldChunk>& ChunkPair : ChunkMap)
   //     {
   //         if (ChunkPair.Value.bIsExplored)
   //         {
   //             ChunkSaveGame->ExploredChunks.Add(ChunkPair.Key);
   //         }
   //     }
   //     
   //     UGameplayStatics::SaveGameToSlot(ChunkSaveGame, "WorldChunks", 0);
   // }
}

void AWorldChunkManager::LoadChunkExplorationData()
{
   // This would be implemented to load exploration data from a save game
   // For now it's a placeholder
   // if (UGameplayStatics::DoesSaveGameExist("WorldChunks", 0))
   // {
   //     USaveGame* SaveGameInstance = UGameplayStatics::LoadGameFromSlot("WorldChunks", 0);
   //     UWorldChunkSaveGame* ChunkSaveGame = Cast<UWorldChunkSaveGame>(SaveGameInstance);
   //     
   //     if (ChunkSaveGame)
   //     {
   //         for (const FIntPoint& ExploredChunk : ChunkSaveGame->ExploredChunks)
   //         {
   //             if (!ChunkMap.Contains(ExploredChunk))
   //             {
   //                 // Create a new entry for this chunk
   //                 FWorldChunk NewChunk;
   //                 NewChunk.ChunkCoordinates = ExploredChunk;
   //                 NewChunk.bIsExplored = true;
   //                 ChunkMap.Add(ExploredChunk, NewChunk);
   //             }
   //             else
   //             {
   //                 // Update existing chunk
   //                 ChunkMap[ExploredChunk].bIsExplored = true;
   //             }
   //         }
   //     }
   // }
}

bool AWorldChunkManager::EnsureBiomeManagerExists()
{
   if (!BiomeManager)
   {
       // Try to find an existing biome manager
       for (TActorIterator<AActor> It(GetWorld()); It; ++It)
       {
           UBiomeManager* FoundBiomeManager = It->FindComponentByClass<UBiomeManager>();
           if (FoundBiomeManager)
           {
               BiomeManager = FoundBiomeManager;
               break;
           }
       }
       
       // Create a new biome manager if none exists
       if (!BiomeManager)
       {
           BiomeManager = NewObject<UBiomeManager>(this);
       }
   }
   
   return BiomeManager != nullptr;
}

bool AWorldChunkManager::EnsureProceduralGeneratorExists()
{
   if (!ProceduralGenerator)
   {
       // Try to find an existing procedural generator
       for (TActorIterator<AActor> It(GetWorld()); It; ++It)
       {
           UProceduralGenerator* FoundGenerator = It->FindComponentByClass<UProceduralGenerator>();
           if (FoundGenerator)
           {
               ProceduralGenerator = FoundGenerator;
               break;
           }
       }
       
       // Create a new procedural generator if none exists
       if (!ProceduralGenerator)
       {
           ProceduralGenerator = NewObject<UProceduralGenerator>(this);
       }
   }
   
   return ProceduralGenerator != nullptr;
}

void AWorldChunkManager::DetermineDominantBiome(FWorldChunk& Chunk)
{
   if (!BiomeManager)
   {
       Chunk.DominantBiome = EBiomeType::Plains;
       return;
   }
   
   // Get the center of the chunk
   FVector ChunkCenter = ChunkCoordinatesToWorldLocation(Chunk.ChunkCoordinates);
   
   // Use the biome manager to determine the biome at this location
   Chunk.DominantBiome = BiomeManager->GetBiomeTypeAtLocation(ChunkCenter);
}

void AWorldChunkManager::UpdateVisibilityForChunk(FWorldChunk& Chunk, bool bVisible)
{
   // Skip if visibility state is already matching
   if (Chunk.bIsVisible == bVisible)
   {
       return;
   }
   
   Chunk.bIsVisible = bVisible;
   
   // Update visibility of all actors in the chunk
   for (AActor* Actor : Chunk.SpawnedActors)
   {
       if (Actor && !Actor->IsPendingKill())
       {
           Actor->SetActorHiddenInGame(!bVisible);
           
           // Also disable physics/ticking for hidden actors to improve performance
           Actor->SetActorEnableCollision(bVisible);
           Actor->SetActorTickEnabled(bVisible);
       }
   }
}

void AWorldChunkManager::CleanupChunks()
{
   // Only perform cleanup every few seconds
   static float TimeSinceLastCleanup = 0.0f;
   TimeSinceLastCleanup += GetWorld()->GetDeltaSeconds();
   
   if (TimeSinceLastCleanup < 5.0f)
   {
       return;
   }
   
   TimeSinceLastCleanup = 0.0f;
   
   // Get current player location
   APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
   if (!PlayerPawn)
   {
       return;
   }
   
   FVector PlayerLocation = PlayerPawn->GetActorLocation();
   FIntPoint PlayerChunk = WorldLocationToChunkCoordinates(PlayerLocation);
   
   // Get all chunks
   TArray<FIntPoint> AllChunks;
   ChunkMap.GetKeys(AllChunks);
   
   // Determine how many chunks we need to unload
   int32 GeneratedChunkCount = 0;
   
   for (const FIntPoint& ChunkCoord : AllChunks)
   {
       if (ChunkMap[ChunkCoord].bIsGenerated)
       {
           GeneratedChunkCount++;
       }
   }
   
   // If we're under the limit, no need to clean up
   if (GeneratedChunkCount <= MaxConcurrentLoadedChunks)
   {
       return;
   }
   
   // Sort chunks by distance from player
   AllChunks.Sort([PlayerChunk](const FIntPoint& A, const FIntPoint& B) {
       int32 DistA = FMath::Max(FMath::Abs(A.X - PlayerChunk.X), FMath::Abs(A.Y - PlayerChunk.Y));
       int32 DistB = FMath::Max(FMath::Abs(B.X - PlayerChunk.X), FMath::Abs(B.Y - PlayerChunk.Y));
       return DistA > DistB; // Sort by descending distance (furthest first)
   });
   
   // Destroy chunks that are too far away until we're under the limit
   int32 ChunksToDestroy = GeneratedChunkCount - MaxConcurrentLoadedChunks;
   
   for (const FIntPoint& ChunkCoord : AllChunks)
   {
       if (ChunksToDestroy <= 0)
       {
           break;
       }
       
       if (ChunkMap[ChunkCoord].bIsGenerated && !ChunkMap[ChunkCoord].bIsVisible)
       {
           // We only destroy chunks that are generated but not visible
           DestroyChunk(ChunkCoord);
           ChunksToDestroy--;
       }
   }
}