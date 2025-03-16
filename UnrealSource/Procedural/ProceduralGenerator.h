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
    
    UFUNCTION(BlueprintCallable, Category = "Procedural|Characters")
    FFollowerData GenerateFollower(int32 MinLevel, int32 MaxLevel, bool bCanBeHero);
    UFUNCTION(BlueprintCallable, Category = "Procedural|Characters")
    FString GenerateName(bool bIsFollower) const;
    
    UFUNCTION(BlueprintCallable, Category = "Procedural|Kingdom")
    void GenerateWorldChunk(FVector2D ChunkCoordinates, TArray<AActor*>& OutSpawnedActors);
    UFUNCTION(BlueprintCallable, Category = "Procedural|Kingdom")
    FKingdomEvent GenerateRandomEvent(EKingdomTier KingdomTier, float KingdomAlignment);
    
    UFUNCTION(BlueprintCallable, Category = "Procedural|Enemies")
    AActor* GenerateNotoriousMonster(int32 PlayerLevel, FString BiomeType);
    UFUNCTION(BlueprintCallable, Category = "Procedural|Enemies")
    AActor* GenerateNotoriousMonsterEncounter(int32 Tier, FString Difficulty);
    
    UFUNCTION(BlueprintCallable, Category = "Procedural|Quests")
    UQuest* GenerateQuest(int32 DifficultyLevel, FString FactionName, EQuestType PreferredType);
    UFUNCTION(BlueprintCallable, Category = "Procedural|Quests")
    UQuest* GenerateQuest2(int32 DifficultyLevel, FString FactionName, EQuestType PreferredType);
    UFUNCTION(BlueprintCallable, Category = "Procedural|Quests")
    void GenerateSpecialQuestChain(FString ChainType, int32 QuestCount, int32 BaseDifficulty);
    UFUNCTION(BlueprintCallable, Category = "Procedural|Quests")
    UQuest* GenerateSpecialQuest(int32 Difficulty);
    UFUNCTION(BlueprintCallable, Category = "Procedural|Quests")
    void GenerateFetchQuest(UQuest* Quest);
    UFUNCTION(BlueprintCallable, Category = "Procedural|Quests")
    void GenerateKillQuest(UQuest* Quest);
    UFUNCTION(BlueprintCallable, Category = "Procedural|Quests")
    void GenerateEscortQuest(UQuest* Quest);
    UFUNCTION(BlueprintCallable, Category = "Procedural|Quests")
    void GenerateExploreQuest(UQuest* Quest);
    UFUNCTION(BlueprintCallable, Category = "Procedural|Quests")
    void GenerateBuildQuest(UQuest* Quest);
    UFUNCTION(BlueprintCallable, Category = "Procedural|Quests")
    void GenerateDiplomaticQuest(UQuest* Quest);
    UFUNCTION(BlueprintCallable, Category = "Procedural|Quests")
    void GenerateMysteryQuest(UQuest* Quest);
    
    UFUNCTION(BlueprintCallable, Category = "Procedural|Resource")
    FResourceData GenerateResource(EResourceRarity MinRarity, EResourceRarity MaxRarity);
    UFUNCTION(BlueprintCallable, Category = "Procedural|Resource")
    FLootItem GenerateLoot(int32 PlayerLevel, float RarityModifier);
    UFUNCTION(BlueprintCallable, Category = "Procedural|Resource")
    void GenerateWeaponItem(FLootItem& Item, int32 PlayerLevel, EResourceRarity Rarity);
    UFUNCTION(BlueprintCallable, Category = "Procedural|Resource")
    void GenerateArmorItem(FLootItem& Item, int32 PlayerLevel, EResourceRarity Rarity);
    UFUNCTION(BlueprintCallable, Category = "Procedural|Resource")
    void GenerateAccessoryItem(FLootItem& Item, int32 PlayerLevel, EResourceRarity Rarity);
    UFUNCTION(BlueprintCallable, Category = "Procedural|Resource")
    void GenerateConsumableItem(FLootItem& Item, int32 PlayerLevel, EResourceRarity Rarity);
    UFUNCTION(BlueprintCallable, Category = "Procedural|Resource")
    void GenerateTrophyItem(FLootItem& Item, int32 PlayerLevel, EResourceRarity Rarity);
    

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