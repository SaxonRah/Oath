// FQuestNode.h
#pragma once

#include "CoreMinimal.h"
#include "FTANode.h"
#include "QuestTypes.h"
#include "FQuestNode.generated.h"

/**
 * Quest-specific node for representing quest states
 */
class TREEAUTOMATA_API FQuestNode : public FTANode
{
public:
    // Constructor
    FQuestNode();
    
    // Quest status (Available, Active, Completed, Failed)
    EQuestStatus Status;
    
    // Quest title
    FText Title;
    
    // Quest description
    FText Description;
    
    // Experience reward
    int32 XPReward;
    
    // Item rewards
    TArray<FItemReward> ItemRewards;
    
    // Quest objectives
    TArray<FQuestObjective> Objectives;
    
    // Is this quest visible in the journal?
    bool bVisibleInJournal;
    
    // Is this a main quest?
    bool bIsMainQuest;
    
    // Quest category
    FString QuestCategory;
    
    // Quest tags for filtering
    TArray<FString> QuestTags;
    
    // Override to handle quest-specific logic
    virtual bool EvaluateTransitions(const FTAContext& Context, TSharedPtr<FTANode>& OutNextNode) override;
    
    // Custom implementation for quest completion
    virtual void ExecuteEntryActions(const FTAContext& Context) override;
    
    // Execute exit actions with quest-specific logic
    virtual void ExecuteExitActions(const FTAContext& Context) override;
    
    // Clone this node
    virtual TSharedPtr<FTANode> Clone() const override;
    
    // Get descriptive string
    virtual FString ToString() const override;
    
    // Serialize quest node
    virtual void Serialize(FArchive& Ar) override;
    
    // Check if all objectives are complete
    bool AreAllObjectivesComplete() const;
    
    // Check if a specific objective is complete
    bool IsObjectiveComplete(const FGuid& ObjectiveID) const;
    
    // Update objective progress
    void UpdateObjectiveProgress(const FGuid& ObjectiveID, int32 Progress);
    
    // Set objective as complete
    void CompleteObjective(const FGuid& ObjectiveID);
    
    // Get completion percentage
    float GetCompletionPercentage() const;
};

/**
 * Quest objective completion condition
 */
class TREEAUTOMATA_API FQuestObjectiveCompletedCondition : public FTACondition
{
public:
    // Constructor
    FQuestObjectiveCompletedCondition();
    
    // ID of the objective that must be completed
    FGuid ObjectiveID;
    
    // All objectives must be complete
    bool bAllObjectives;
    
    // Implementation
    virtual bool Evaluate(const FTAContext& Context) const override;
    virtual FString GetDescription() const override;
    virtual TSharedPtr<FTACondition> Clone() const override;
    virtual void Serialize(FArchive& Ar) override;

private:
   // Helper to find the quest node
   TSharedPtr<FQuestNode> FindQuestNode(const FTAContext& Context) const;
};

/**
* Quest status condition
*/
class TREEAUTOMATA_API FQuestStatusCondition : public FTACondition
{
public:
   // Constructor
   FQuestStatusCondition();
   
   // Required quest status
   EQuestStatus RequiredStatus;
   
   // Implementation
   virtual bool Evaluate(const FTAContext& Context) const override;
   virtual FString GetDescription() const override;
   virtual TSharedPtr<FTACondition> Clone() const override;
   virtual void Serialize(FArchive& Ar) override;
};

/**
* Add quest action
*/
class TREEAUTOMATA_API FAddQuestAction : public FTAAction
{
public:
   // Constructor
   FAddQuestAction();
   
   // Quest ID to add
   FString QuestID;
   
   // Auto-activate the quest
   bool bAutoActivate;
   
   // Implementation
   virtual void Execute(const FTAContext& Context) const override;
   virtual FString GetDescription() const override;
   virtual TSharedPtr<FTAAction> Clone() const override;
   virtual void Serialize(FArchive& Ar) override;
};

/**
* Complete objective action
*/
class TREEAUTOMATA_API FCompleteObjectiveAction : public FTAAction
{
public:
   // Constructor
   FCompleteObjectiveAction();
   
   // Objective ID to complete
   FGuid ObjectiveID;
   
   // Update progress instead of completing
   bool bUpdateProgressOnly;
   
   // Progress amount to add (if updating progress)
   int32 ProgressAmount;
   
   // Implementation
   virtual void Execute(const FTAContext& Context) const override;
   virtual FString GetDescription() const override;
   virtual TSharedPtr<FTAAction> Clone() const override;
   virtual void Serialize(FArchive& Ar) override;
};