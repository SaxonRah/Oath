// FDialogueNode.h
#pragma once

#include "CoreMinimal.h"
#include "FTANode.h"
#include "DialogueTypes.h"
#include "FDialogueNode.generated.h"

/**
 * Dialogue-specific node for conversation trees
 */
class TREEAUTOMATA_API FDialogueNode : public FTANode
{
public:
    // Constructor
    FDialogueNode();
    
    // Speaker ID
    FString SpeakerID;
    
    // Dialogue text
    FText DialogueText;
    
    // Audio cue
    USoundBase* VoiceClip;
    
    // Animation to play
    UAnimMontage* SpeakAnimation;
    
    // Camera setup
    FCameraSetup CameraSetup;
    
    // Dialogue options (responses)
    TArray<FDialogueOption> Options;
    
    // Is this a player response?
    bool bIsPlayerNode;
    
    // Show this dialogue in subtitles?
    bool bShowSubtitles;
    
    // Emotion for this dialogue
    EDialogueEmotion Emotion;
    
    // Wait time before continuing auto-dialogue
    float AutoContinueDelay;
    
    // Override for dialogue-specific logic
    virtual bool EvaluateTransitions(const FTAContext& Context, TSharedPtr<FTANode>& OutNextNode) override;
    
    // Custom entry actions
    virtual void ExecuteEntryActions(const FTAContext& Context) override;
    
    // Custom exit actions
    virtual void ExecuteExitActions(const FTAContext& Context) override;
    
    // Clone this node
    virtual TSharedPtr<FTANode> Clone() const override;
    
    // Get descriptive string
    virtual FString ToString() const override;
    
    // Serialize dialogue node
    virtual void Serialize(FArchive& Ar) override;
    
    // Add dialogue option
    void AddOption(const FDialogueOption& Option);
    
    // Remove dialogue option
    bool RemoveOption(int32 OptionIndex);
    
    // Get filtered options based on conditions
    TArray<FDialogueOption> GetFilteredOptions(const FTAContext& Context) const;
    
    // Get emotion string
    FString GetEmotionString() const;
};

/**
 * Dialogue option selected condition
 */
class TREEAUTOMATA_API FDialogueOptionSelectedCondition : public FTACondition
{
public:
    // Constructor
    FDialogueOptionSelectedCondition();
    
    // Option index that was selected
    int32 OptionIndex;
    
    // Implementation
    virtual bool Evaluate(const FTAContext& Context) const override;
    virtual FString GetDescription() const override;
    virtual TSharedPtr<FTACondition> Clone() const override;
    virtual void Serialize(FArchive& Ar) override;
};

/**
 * Relationship level condition
 */
class TREEAUTOMATA_API FRelationshipLevelCondition : public FTACondition
{
public:
    // Constructor
    FRelationshipLevelCondition();
    
    // NPC ID to check relationship with
    FString NPCID;
    
    // Required relationship level
    int32 RequiredLevel;
    
    // Comparison operator
    EComparisonOperator Operator;
    
    // Implementation
    virtual bool Evaluate(const FTAContext& Context) const override;
    virtual FString GetDescription() const override;
    virtual TSharedPtr<FTACondition> Clone() const override;
    virtual void Serialize(FArchive& Ar) override;
};

/**
 * Modify relationship action
 */
class TREEAUTOMATA_API FModifyRelationshipAction : public FTAAction
{
public:
    // Constructor
    FModifyRelationshipAction();
    
    // NPC ID to modify relationship with
    FString NPCID;
    
    // Amount to change by
    int32 ChangeAmount;
    
    // Implementation
    virtual void Execute(const FTAContext& Context) const override;
    virtual FString GetDescription() const override;
    virtual TSharedPtr<FTAAction> Clone() const override;
    virtual void Serialize(FArchive& Ar) override;
};