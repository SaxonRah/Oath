// RPGAutomataExtensions.h
#pragma once

#include "CoreMinimal.h"
#include "Tree_Automata.h"

#include "RPGAutomataExtensions.generated.h"

/**
 * Quest automaton extension for RPG games
 */
UCLASS(Blueprintable)
class TREEAUTOMATA_API UQuestAutomaton : public UTreeAutomata
{
    GENERATED_BODY()
    
public:
    // Initialize a new quest with the given name
    UFUNCTION(BlueprintCallable, Category = "Quest Automaton")
    FGuid CreateQuest(const FString& QuestName, const FString& QuestDescription);
    
    // Add an objective to a quest
    UFUNCTION(BlueprintCallable, Category = "Quest Automaton")
    FGuid AddObjective(const FGuid& QuestId, const FString& ObjectiveName, const FString& Description);
    
    // Mark an objective as completed
    UFUNCTION(BlueprintCallable, Category = "Quest Automaton")
    bool CompleteObjective(const FGuid& ObjectiveId);
    
    // Check if a quest is complete
    UFUNCTION(BlueprintCallable, Category = "Quest Automaton")
    bool IsQuestComplete(const FGuid& QuestId) const;
    
    // Get all active quests
    UFUNCTION(BlueprintCallable, Category = "Quest Automaton")
    TArray<FAutomatonNode> GetActiveQuests() const;
    
    // Get objectives for a quest
    UFUNCTION(BlueprintCallable, Category = "Quest Automaton")
    TArray<FAutomatonNode> GetQuestObjectives(const FGuid& QuestId) const;
};

/**
 * Dialogue system extension for RPG games
 */
UCLASS(Blueprintable)
class TREEAUTOMATA_API UDialogueAutomaton : public UTreeAutomata
{
    GENERATED_BODY()
    
public:
    // Create a dialogue tree for an NPC
    UFUNCTION(BlueprintCallable, Category = "Dialogue Automaton")
    FGuid CreateDialogueTree(const FString& NPCName);
    
    // Add a dialogue node
    UFUNCTION(BlueprintCallable, Category = "Dialogue Automaton")
    FGuid AddDialogueNode(const FGuid& ParentNodeId, const FString& DialogueText, 
                          const FString& PlayerResponseText);
    
    // Set conditions for when a dialogue option is available
    UFUNCTION(BlueprintCallable, Category = "Dialogue Automaton")
    void SetDialogueCondition(const FGuid& DialogueNodeId, const FString& Condition);
    
    // Get available dialogue options based on current game state
    UFUNCTION(BlueprintCallable, Category = "Dialogue Automaton")
    TArray<FAutomatonNode> GetAvailableDialogueOptions(const FGuid& CurrentNodeId, UObject* GameState);
};

/**
 * Skill tree extension for RPG games
 */
UCLASS(Blueprintable)
class TREEAUTOMATA_API USkillTreeAutomaton : public UTreeAutomata
{
    GENERATED_BODY()
    
public:
    // Create a skill tree for a character class
    UFUNCTION(BlueprintCallable, Category = "Skill Tree Automaton")
    FGuid CreateSkillTree(const FString& ClassName);
    
    // Add a skill to the tree
    UFUNCTION(BlueprintCallable, Category = "Skill Tree Automaton")
    FGuid AddSkill(const FGuid& ParentSkillId, const FString& SkillName, 
                   int32 PointsRequired, const FString& Description);
    
    // Unlock a skill if prerequisites are met
    UFUNCTION(BlueprintCallable, Category = "Skill Tree Automaton")
    bool UnlockSkill(const FGuid& SkillId, int32 AvailablePoints);
    
    // Get available skills that can be unlocked
    UFUNCTION(BlueprintCallable, Category = "Skill Tree Automaton")
    TArray<FAutomatonNode> GetAvailableSkills(int32 AvailablePoints) const;
    
    // Get all unlocked skills
    UFUNCTION(BlueprintCallable, Category = "Skill Tree Automaton")
    TArray<FAutomatonNode> GetUnlockedSkills() const;
};

/**
 * Crafting system extension for RPG games
 */
UCLASS(Blueprintable)
class TREEAUTOMATA_API UCraftingAutomaton : public UTreeAutomata
{
    GENERATED_BODY()
    
public:
    // Create a crafting system
    UFUNCTION(BlueprintCallable, Category = "Crafting Automaton")
    FGuid CreateCraftingSystem(const FString& SystemName);
    
    // Add a recipe to the crafting system
    UFUNCTION(BlueprintCallable, Category = "Crafting Automaton")
    FGuid AddRecipe(const FGuid& ParentRecipeId, const FString& RecipeName, 
                    const TArray<FString>& RequiredItems, const FString& ResultItem);
    
    // Discover a recipe
    UFUNCTION(BlueprintCallable, Category = "Crafting Automaton")
    bool DiscoverRecipe(const FGuid& RecipeId);
    
    // Get all discovered recipes
    UFUNCTION(BlueprintCallable, Category = "Crafting Automaton")
    TArray<FAutomatonNode> GetDiscoveredRecipes() const;
    
    // Check if recipe can be crafted with available items
    UFUNCTION(BlueprintCallable, Category = "Crafting Automaton")
    bool CanCraftRecipe(const FGuid& RecipeId, const TArray<FString>& AvailableItems) const;
};