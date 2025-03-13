// TreeAutomataManager.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Tree_Automata.h"
#include "RPGAutomataExtensions.h"
#include "TreeAutomataManager.generated.h"

// Forward declarations for the types we'll use
class UQuestAutomaton;
class UDialogueAutomaton;
class USkillTreeAutomaton;
class UCraftingAutomaton;

/**
 * Manager class to handle multiple automata systems and provide a central interface
 */
UCLASS(Blueprintable)
class TREEAUTOMATA_API ATreeAutomataManager : public AActor
{
    GENERATED_BODY()
    
public:
    ATreeAutomataManager();
    
    virtual void BeginPlay() override;
    
    // Get the quest automaton
    UFUNCTION(BlueprintCallable, Category = "Automata Manager")
    UQuestAutomaton* GetQuestAutomaton() const;
    
    // Get the dialogue automaton
    UFUNCTION(BlueprintCallable, Category = "Automata Manager")
    UDialogueAutomaton* GetDialogueAutomaton() const;
    
    // Get the skill tree automaton
    UFUNCTION(BlueprintCallable, Category = "Automata Manager")
    USkillTreeAutomaton* GetSkillTreeAutomaton() const;
    
    // Get the crafting automaton
    UFUNCTION(BlueprintCallable, Category = "Automata Manager")
    UCraftingAutomaton* GetCraftingAutomaton() const;
    
    // Save all automata to JSON
    UFUNCTION(BlueprintCallable, Category = "Automata Manager")
    bool SaveToFile(const FString& FilePath);
    
    // Load all automata from JSON
    UFUNCTION(BlueprintCallable, Category = "Automata Manager")
    bool LoadFromFile(const FString& FilePath);
    
protected:
    // Quest system
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Automata Manager")
    UQuestAutomaton* QuestAutomaton;
    
    // Dialogue system
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Automata Manager")
    UDialogueAutomaton* DialogueAutomaton;
    
    // Skill tree system
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Automata Manager")
    USkillTreeAutomaton* SkillTreeAutomaton;
    
    // Crafting system
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Automata Manager")
    UCraftingAutomaton* CraftingAutomaton;
};