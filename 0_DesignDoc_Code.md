# Unreal Engine 5 C++ System Design for "Oath"

## Core Class Structure

### 1. Player Character

```cpp
// OathCharacter.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ReputationComponent.h"
#include "InventoryComponent.h"
#include "OathCharacter.generated.h"

UENUM(BlueprintType)
enum class ECharacterClass : uint8
{
    Warrior UMETA(DisplayName = "Warrior"),
    Rogue UMETA(DisplayName = "Rogue"),
    Mage UMETA(DisplayName = "Mage"),
    Ranger UMETA(DisplayName = "Ranger"),
    Custom UMETA(DisplayName = "Custom")
};

UCLASS()
class OATH_API AOathCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AOathCharacter();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    ECharacterClass CharacterClass;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString KingdomVision;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UReputationComponent* ReputationComponent;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UInventoryComponent* InventoryComponent;
    
    // Combat functions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void AttackEnemy(AActor* Target);
    
    // Quest functions
    UFUNCTION(BlueprintCallable, Category = "Quests")
    void CompleteQuest(UQuest* Quest);
    
protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(EditDefaultsOnly, Category = "Stats")
    float BaseAttackPower;
};
```

### 2. Reputation System

```cpp
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

protected:
    virtual void BeginPlay() override;
    
private:
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnReputationChangedDelegate, FString, ReputationType, float, OldValue, float, NewValue);
    
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnReputationChangedDelegate OnReputationChanged;
};
```

### 3. Resource & Economy System

```cpp
// InventoryComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ResourceData.h"
#include "InventoryComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class OATH_API UInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UInventoryComponent();
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    int32 Gold;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    TMap<FResourceData, int32> Materials;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    TArray<FLootItem> Items;
    
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void AddGold(int32 Amount, bool bNotify = true);
    
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool SpendGold(int32 Amount, bool bNotify = true);
    
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void AddMaterial(FResourceData Material, int32 Amount, bool bNotify = true);
    
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool SpendMaterial(FResourceData Material, int32 Amount, bool bNotify = true);
    
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void AddItem(FLootItem Item, bool bNotify = true);
    
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool RemoveItem(FLootItem Item, bool bNotify = true);
    
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void RegisterInventoryChangedDelegate(const FOnInventoryChangedDelegate& Delegate);

protected:
    virtual void BeginPlay() override;
    
private:
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnInventoryChangedDelegate, FString, ResourceType, UObject*, Resource, int32, NewAmount);
    
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnInventoryChangedDelegate OnInventoryChanged;
};
```

### 4. Resource Data Structure

```cpp
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
```

### 5. Kingdom Management

```cpp
// KingdomManager.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FollowerData.h"
#include "BuildingData.h"
#include "KingdomManager.generated.h"

UENUM(BlueprintType)
enum class EKingdomTier : uint8
{
    Camp UMETA(DisplayName = "Camp"),
    Village UMETA(DisplayName = "Village"),
    Town UMETA(DisplayName = "Town"),
    City UMETA(DisplayName = "City"),
    Kingdom UMETA(DisplayName = "Kingdom")
};

UCLASS()
class OATH_API AKingdomManager : public AActor
{
    GENERATED_BODY()

public:
    AKingdomManager();
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kingdom")
    FString KingdomName;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kingdom")
    EKingdomTier CurrentTier;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kingdom")
    float KingdomAlignment; // -1.0 to 1.0
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kingdom")
    TArray<FFollowerData> Followers;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kingdom")
    TArray<FBuildingData> Buildings;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kingdom")
    int32 TaxRate;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kingdom")
    float DailyIncome;
    
    UFUNCTION(BlueprintCallable, Category = "Kingdom")
    bool RecruitFollower(FFollowerData Follower);
    
    UFUNCTION(BlueprintCallable, Category = "Kingdom")
    bool ConstructBuilding(FBuildingData Building, FVector Location);
    
    UFUNCTION(BlueprintCallable, Category = "Kingdom")
    void CalculateDailyIncome();
    
    UFUNCTION(BlueprintCallable, Category = "Kingdom")
    void UpdateKingdomTier();
    
    UFUNCTION(BlueprintCallable, Category = "Kingdom")
    void ProcessDailyUpdate();
    
    UFUNCTION(BlueprintCallable, Category = "Kingdom")
    void TriggerRandomEvent();
    
    UFUNCTION(BlueprintCallable, Category = "Kingdom")
    int32 GetMaxFollowers() const;
    
    UFUNCTION(BlueprintCallable, Category = "Kingdom")
    int32 GetMaxBuildings() const;
    
protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    
private:
    float DayTimer;
    float DaysElapsed;
    
    UPROPERTY(EditDefaultsOnly, Category = "Kingdom")
    float DayLength; // In real seconds
    
    UPROPERTY(EditDefaultsOnly, Category = "Kingdom")
    TArray<FKingdomEvent> PossibleEvents;
};
```

### 6. Follower System

```cpp
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
```

### 7. Building System

```cpp
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
```

### 8. Quest System

```cpp
// Quest.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ResourceData.h"
#include "Quest.generated.h"

UENUM(BlueprintType)
enum class EQuestType : uint8
{
    Fetch UMETA(DisplayName = "Fetch"),
    Kill UMETA(DisplayName = "Kill"),
    Escort UMETA(DisplayName = "Escort"),
    Explore UMETA(DisplayName = "Explore"),
    Build UMETA(DisplayName = "Build"),
    Diplomatic UMETA(DisplayName = "Diplomatic"),
    Mystery UMETA(DisplayName = "Mystery")
};

UENUM(BlueprintType)
enum class EQuestStatus : uint8
{
    Available UMETA(DisplayName = "Available"),
    InProgress UMETA(DisplayName = "In Progress"),
    Completed UMETA(DisplayName = "Completed"),
    Failed UMETA(DisplayName = "Failed")
};

USTRUCT(BlueprintType)
struct OATH_API FQuestObjective
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 CurrentProgress;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RequiredProgress;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsCompleted;
};

UCLASS(BlueprintType, Blueprintable)
class OATH_API UQuest : public UObject
{
    GENERATED_BODY()

public:
    UQuest();
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestName;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuestType Type;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuestStatus Status;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 DifficultyLevel;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float QuestRenownReward;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 GoldReward;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TMap<FResourceData, int32> MaterialRewards;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FLootItem> ItemRewards;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuestObjective> Objectives;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestGiver;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString FactionName;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float FactionReputationReward;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bCanBeAssignedToFollower;
    
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void UpdateObjective(int32 ObjectiveIndex, int32 Progress);
    
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool AreAllObjectivesComplete();
    
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CompleteQuest();
    
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void FailQuest();
};
```

### 9. Procedural Generator

```cpp
// ProceduralGenerator.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "FollowerData.h"
#include "ResourceData.h"
#include "Quest.h"
#include "ProceduralGenerator.generated.h"

UCLASS(BlueprintType, Blueprintable)
class OATH_API UProceduralGenerator : public UObject
{
    GENERATED_BODY()

public:
    UProceduralGenerator();
    
    UFUNCTION(BlueprintCallable, Category = "Procedural")
    FFollowerData GenerateFollower(int32 MinLevel, int32 MaxLevel, bool bCanBeHero);
    
    UFUNCTION(BlueprintCallable, Category = "Procedural")
    UQuest* GenerateQuest(int32 DifficultyLevel, FString FactionName, EQuestType PreferredType);
    
    UFUNCTION(BlueprintCallable, Category = "Procedural")
    FResourceData GenerateResource(EResourceRarity MinRarity, EResourceRarity MaxRarity);
    
    UFUNCTION(BlueprintCallable, Category = "Procedural")
    FLootItem GenerateLoot(int32 PlayerLevel, float RarityModifier);
    
    UFUNCTION(BlueprintCallable, Category = "Procedural")
    void GenerateWorldChunk(FVector2D ChunkCoordinates, TArray<AActor*>& OutSpawnedActors);
    
    UFUNCTION(BlueprintCallable, Category = "Procedural")
    AActor* GenerateNotoriousMonster(int32 PlayerLevel, FString BiomeType);
    
    UFUNCTION(BlueprintCallable, Category = "Procedural")
    FKingdomEvent GenerateRandomEvent(EKingdomTier KingdomTier, float KingdomAlignment);

private:
    UPROPERTY()
    TArray<FString> FollowerNamePool;
    
    UPROPERTY()
    TArray<FString> QuestTemplates;
    
    UPROPERTY()
    TArray<FString> ResourceNamePool;
    
    UPROPERTY()
    TArray<FString> ItemNamePool;
    
    UPROPERTY()
    TArray<FString> TraitPool;
    
    int32 GetRandomSeed() const;
    FString GenerateName(bool bIsFollower) const;
};
```

### 10. Game Mode

```cpp
// OathGameMode.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "KingdomManager.h"
#include "ProceduralGenerator.h"
#include "OathGameMode.generated.h"

UCLASS()
class OATH_API AOathGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AOathGameMode();
    
    virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
    virtual void BeginPlay() override;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kingdom")
    AKingdomManager* KingdomManager;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural")
    UProceduralGenerator* ProceduralGenerator;
    
    UFUNCTION(BlueprintCallable, Category = "Game")
    void SaveGame();
    
    UFUNCTION(BlueprintCallable, Category = "Game")
    void LoadGame();
    
    UFUNCTION(BlueprintCallable, Category = "Game")
    void ProcessWorldTime(float DeltaTime);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Game")
    void OnDayPassed();
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Game")
    void OnSeasonChanged(int32 NewSeason);

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Game")
    float TimeScale;
    
    UPROPERTY(EditDefaultsOnly, Category = "Game")
    float DayLength; // In seconds
    
    UPROPERTY(EditDefaultsOnly, Category = "Game")
    int32 DaysPerSeason;
    
private:
    float CurrentDayTime;
    int32 CurrentDay;
    int32 CurrentSeason;
    
    void UpdateWorldTime(float DeltaTime);
    void SpawnRandomEncounters();
};
```

## Implementation Considerations

1. **Performance Optimization**:
   - Implement LOD (Level of Detail) for the procedural world
   - Use object pooling for frequently spawned entities (monsters, resources)
   - Chunk loading system for the open world

2. **Save/Load System**:
   - Create a custom save game object that serializes kingdom data
   - Use Unreal's SaveGame system with custom extensions for procedural content

3. **AI & Behavior**:
   - Use Behavior Trees for monster AI
   - Implement utility-based AI for followers' daily routines
   - Create a time-based system for follower activities

4. **Networking Considerations** (if applicable):
   - Design systems with replication in mind
   - Use RPC for important gameplay events
   - Consider dedicated server architecture
