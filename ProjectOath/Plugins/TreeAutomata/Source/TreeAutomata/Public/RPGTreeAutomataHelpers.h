// RPGTreeAutomataHelpers.h
#pragma once

#include "CoreMinimal.h"
#include "Tree_Automata.h"
#include "RPGTreeAutomataHelpers.generated.h"

/**
 * Example RPG condition evaluator
 */
UCLASS(Blueprintable)
class TREEAUTOMATA_API URPGConditionEvaluator : public UConditionEvaluator
{
    GENERATED_BODY()
    
public:
    // Override the base condition evaluator
    virtual bool EvaluateCondition_Implementation(const FString& Condition, UObject* Context) override;
    
    // Helper function to parse and evaluate a condition string
    bool ParseAndEvaluateCondition(const FString& Condition, UObject* GameState);
};

/**
 * Example RPG action performer
 */
UCLASS(Blueprintable)
class TREEAUTOMATA_API URPGActionPerformer : public UActionPerformer
{
    GENERATED_BODY()
    
public:
    // Override the base action performer
    virtual void PerformAction_Implementation(const FString& Action, UObject* Context) override;
    
    // Helper function to parse and perform an action string
    void ParseAndPerformAction(const FString& Action, UObject* GameState);
};

/**
 * Example RPG game state that can be used with the tree automata
 */
UCLASS(Blueprintable)
class TREEAUTOMATA_API URPGGameState : public UObject
{
    GENERATED_BODY()
    
public:
    // Player stats
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Level;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 SkillPoints;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, int32> Stats;
    
    // Player inventory
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> Inventory;
    
    // Known facts and quest progress flags
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, bool> Flags;
    
    // Faction reputation
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, int32> Reputation;
    
    // Initialize with default values
    URPGGameState();
    
    // Check if player has a specific item
    UFUNCTION(BlueprintCallable)
    bool HasItem(const FString& ItemName) const;
    
    // Check if player has enough of a stat
    UFUNCTION(BlueprintCallable)
    bool HasStat(const FString& StatName, int32 MinValue) const;
    
    // Check if a flag is set
    UFUNCTION(BlueprintCallable)
    bool IsFlagSet(const FString& FlagName) const;
    
    // Check faction reputation level
    UFUNCTION(BlueprintCallable)
    bool HasReputation(const FString& Faction, int32 MinValue) const;
    
    // Add an item to inventory
    UFUNCTION(BlueprintCallable)
    void AddItem(const FString& ItemName);
    
    // Remove an item from inventory
    UFUNCTION(BlueprintCallable)
    bool RemoveItem(const FString& ItemName);
    
    // Set a flag
    UFUNCTION(BlueprintCallable)
    void SetFlag(const FString& FlagName, bool Value);
    
    // Modify a stat
    UFUNCTION(BlueprintCallable)
    void ModifyStat(const FString& StatName, int32 Delta);
    
    // Modify faction reputation
    UFUNCTION(BlueprintCallable)
    void ModifyReputation(const FString& Faction, int32 Delta);
};