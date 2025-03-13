// RPGTreeAutomataExample.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TreeAutomataManager.h"
#include "RPGTreeAutomataHelpers.h"
#include "RPGTreeAutomataExample.generated.h"

/**
 * Example class demonstrating usage of the Tree Automata system for RPG games
 */
UCLASS(Blueprintable)
class TREEAUTOMATA_API ARPGTreeAutomataExample : public AActor
{
    GENERATED_BODY()
    
public:
    ARPGTreeAutomataExample();
    
    virtual void BeginPlay() override;
    
    // Set up a sample quest system
    UFUNCTION(BlueprintCallable, Category = "RPG Example")
    void SetupQuestSystem();
    
    // Set up a sample dialogue system
    UFUNCTION(BlueprintCallable, Category = "RPG Example")
    void SetupDialogueSystem();
    
    // Set up a sample skill tree
    UFUNCTION(BlueprintCallable, Category = "RPG Example")
    void SetupSkillTree();
    
    // Set up a sample crafting system
    UFUNCTION(BlueprintCallable, Category = "RPG Example")
    void SetupCraftingSystem();
    
    // Demo accepting and completing a quest
    UFUNCTION(BlueprintCallable, Category = "RPG Example")
    void DemoQuestCompletion();
    
    // Demo progressing through dialogue
    UFUNCTION(BlueprintCallable, Category = "RPG Example")
    void DemoDialogueProgression();
    
    // Demo unlocking skills
    UFUNCTION(BlueprintCallable, Category = "RPG Example")
    void DemoSkillUnlocking();
    
    // Demo crafting items
    UFUNCTION(BlueprintCallable, Category = "RPG Example")
    void DemoCrafting();
    
protected:
    // The automata manager
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RPG Example")
    ATreeAutomataManager* AutomataManager;
    
    // Game state for the demo
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RPG Example")
    URPGGameState* GameState;
    
    // Condition evaluator
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RPG Example")
    URPGConditionEvaluator* ConditionEvaluator;
    
    // Action performer
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "RPG Example")
    URPGActionPerformer* ActionPerformer;
    
    // Store quest IDs for the demo
    UPROPERTY(VisibleAnywhere, Category = "RPG Example")
    TMap<FString, FGuid> QuestIDs;
    
    // Store dialogue IDs for the demo
    UPROPERTY(VisibleAnywhere, Category = "RPG Example")
    TMap<FString, FGuid> DialogueIDs;
    
    // Store skill IDs for the demo
    UPROPERTY(VisibleAnywhere, Category = "RPG Example")
    TMap<FString, FGuid> SkillIDs;
    
    // Store recipe IDs for the demo
    UPROPERTY(VisibleAnywhere, Category = "RPG Example")
    TMap<FString, FGuid> RecipeIDs;
};