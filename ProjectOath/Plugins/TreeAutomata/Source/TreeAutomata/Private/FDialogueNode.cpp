// FDialogueNode.cpp
#include "FDialogueNode.h"
#include "DialogueSubsystem.h"
#include "TALogging.h"

//------------------------------------------------------------------------------
// FDialogueNode
//------------------------------------------------------------------------------

FDialogueNode::FDialogueNode()
    : FTANode()
    , SpeakerID(TEXT(""))
    , VoiceClip(nullptr)
    , SpeakAnimation(nullptr)
    , bIsPlayerNode(false)
    , bShowSubtitles(true)
    , Emotion(EDialogueEmotion::Neutral)
    , AutoContinueDelay(0.0f)
{
    NodeType = TEXT("Dialogue");
}

bool FDialogueNode::EvaluateTransitions(const FTAContext& Context, TSharedPtr<FTANode>& OutNextNode)
{
    // First check if there's a direct input-based transition
    if (FTANode::EvaluateTransitions(Context, OutNextNode))
    {
        return true;
    }
    
    // For dialogue nodes, check if we should auto-continue
    if (Options.Num() == 0 && AutoContinueDelay >= 0.0f)
    {
        // Look for the first valid transition with no conditions
        for (const FTATransition& Transition : Transitions)
        {
            if (Transition.TargetNode.IsValid() && 
                (Transition.Conditions.Num() == 0 || Transition.Evaluate(Context)))
            {
                OutNextNode = Transition.TargetNode;
                return true;
            }
        }
    }
    
    return false;
}

void FDialogueNode::ExecuteEntryActions(const FTAContext& Context)
{
    // Play dialogue in the dialogue system
    UDialogueSubsystem* DialogueSubsystem = Context.World ? Context.World->GetGameInstance()->GetSubsystem<UDialogueSubsystem>() : nullptr;
    if (DialogueSubsystem)
    {
        DialogueSubsystem->PlayDialogue(SpeakerID, DialogueText, VoiceClip, Emotion, GetFilteredOptions(Context));
    }
    
    // Play animations if available
    if (SpeakAnimation && !bIsPlayerNode)
    {
        // Find the speaker actor
        AActor* SpeakerActor = nullptr;
        
        // This would use a proper NPC system in a real implementation
        // For this example, just log it
        UE_LOG(LogTreeAutomata, Display, TEXT("Would play animation %s on speaker %s"), 
            *SpeakAnimation->GetName(), *SpeakerID);
    }
    
    // Execute standard entry actions
    FTANode::ExecuteEntryActions(Context);
}

void FDialogueNode::ExecuteExitActions(const FTAContext& Context)
{
    // Execute standard exit actions
    FTANode::ExecuteExitActions(Context);
}

TSharedPtr<FTANode> FDialogueNode::Clone() const
{
    TSharedPtr<FDialogueNode> ClonedNode = StaticCastSharedPtr<FDialogueNode>(FTANode::Clone());
    
    // Copy dialogue-specific properties
    if (ClonedNode.IsValid())
    {
        ClonedNode->SpeakerID = SpeakerID;
        ClonedNode->DialogueText = DialogueText;
        ClonedNode->VoiceClip = VoiceClip;
        ClonedNode->SpeakAnimation = SpeakAnimation;
        ClonedNode->CameraSetup = CameraSetup;
        ClonedNode->Options = Options;
        ClonedNode->bIsPlayerNode = bIsPlayerNode;
        ClonedNode->bShowSubtitles = bShowSubtitles;
        ClonedNode->Emotion = Emotion;
        ClonedNode->AutoContinueDelay = AutoContinueDelay;
    }
    
    return ClonedNode;
}

FString FDialogueNode::ToString() const
{
    return FString::Printf(TEXT("[Dialogue: %s Speaker: %s Text: \"%s\" Options: %d]"), 
        *NodeName, 
        *SpeakerID,
        *DialogueText.ToString().Left(30),
        Options.Num());
}

void FDialogueNode::Serialize(FArchive& Ar)
{
    FTANode::Serialize(Ar);
    
    Ar << SpeakerID;
    
    // Serialize dialogue text
    FString DialogueTextStr = DialogueText.ToString();
    Ar << DialogueTextStr;
    DialogueText = FText::FromString(DialogueTextStr);
    
    // References would be serialized as paths in a real implementation
    // VoiceClip, SpeakAnimation, etc.
    
    // Serialize options
    int32 OptionCount = Options.Num();
    Ar << OptionCount;
    
    if (Ar.IsLoading())
    {
        Options.Empty(OptionCount);
        for (int32 i = 0; i < OptionCount; ++i)
       {
           FDialogueOption Option;
           Ar << Option;
           Options.Add(Option);
       }
   }
   else
   {
       for (auto& Option : Options)
       {
           Ar << Option;
       }
   }
   
   Ar << bIsPlayerNode;
   Ar << bShowSubtitles;
   
   int32 EmotionInt = (int32)Emotion;
   Ar << EmotionInt;
   Emotion = (EDialogueEmotion)EmotionInt;
   
   Ar << AutoContinueDelay;
   
   // CameraSetup would be serialized in a real implementation
}

void FDialogueNode::AddOption(const FDialogueOption& Option)
{
   Options.Add(Option);
}

bool FDialogueNode::RemoveOption(int32 OptionIndex)
{
   if (Options.IsValidIndex(OptionIndex))
   {
       Options.RemoveAt(OptionIndex);
       return true;
   }
   
   return false;
}

TArray<FDialogueOption> FDialogueNode::GetFilteredOptions(const FTAContext& Context) const
{
   TArray<FDialogueOption> FilteredOptions;
   
   for (const FDialogueOption& Option : Options)
   {
       bool bShouldShow = true;
       
       // Check visibility conditions
       for (const TSharedPtr<FTACondition>& Condition : Option.VisibilityConditions)
       {
           if (Condition.IsValid() && !Condition->Evaluate(Context))
           {
               bShouldShow = false;
               break;
           }
       }
       
       if (bShouldShow)
       {
           FilteredOptions.Add(Option);
       }
   }
   
   return FilteredOptions;
}

FString FDialogueNode::GetEmotionString() const
{
   switch (Emotion)
   {
       case EDialogueEmotion::Happy: return TEXT("Happy");
       case EDialogueEmotion::Sad: return TEXT("Sad");
       case EDialogueEmotion::Angry: return TEXT("Angry");
       case EDialogueEmotion::Surprised: return TEXT("Surprised");
       case EDialogueEmotion::Frightened: return TEXT("Frightened");
       case EDialogueEmotion::Disgusted: return TEXT("Disgusted");
       case EDialogueEmotion::Confused: return TEXT("Confused");
       case EDialogueEmotion::Intrigued: return TEXT("Intrigued");
       default: return TEXT("Neutral");
   }
}

//------------------------------------------------------------------------------
// FDialogueOptionSelectedCondition
//------------------------------------------------------------------------------

FDialogueOptionSelectedCondition::FDialogueOptionSelectedCondition()
   : FTACondition()
   , OptionIndex(-1)
{
   ConditionName = TEXT("Dialogue Option Selected");
}

bool FDialogueOptionSelectedCondition::Evaluate(const FTAContext& Context) const
{
   // Check if the selected option index matches
   const FTAVariant* SelectedOptionVar = Context.InputParams.Find(TEXT("SelectedOption"));
   if (SelectedOptionVar && SelectedOptionVar->IsType<int32>())
   {
       int32 SelectedOption = SelectedOptionVar->AsInt();
       bool Result = (SelectedOption == OptionIndex);
       return bInverted ? !Result : Result;
   }
   
   return false;
}

FString FDialogueOptionSelectedCondition::GetDescription() const
{
   return FString::Printf(TEXT("Player selected dialogue option %d"), OptionIndex);
}

TSharedPtr<FTACondition> FDialogueOptionSelectedCondition::Clone() const
{
   TSharedPtr<FDialogueOptionSelectedCondition> Clone = MakeShared<FDialogueOptionSelectedCondition>();
   Clone->bInverted = bInverted;
   Clone->ConditionName = ConditionName;
   Clone->OptionIndex = OptionIndex;
   return Clone;
}

void FDialogueOptionSelectedCondition::Serialize(FArchive& Ar)
{
   FTACondition::Serialize(Ar);
   Ar << OptionIndex;
}

//------------------------------------------------------------------------------
// FRelationshipLevelCondition
//------------------------------------------------------------------------------

FRelationshipLevelCondition::FRelationshipLevelCondition()
   : FTACondition()
   , NPCID(TEXT(""))
   , RequiredLevel(0)
   , Operator(EComparisonOperator::GreaterEqual)
{
   ConditionName = TEXT("Relationship Level");
}

bool FRelationshipLevelCondition::Evaluate(const FTAContext& Context) const
{
   // In a real implementation, this would query a relationship system
   // For this example, check if the relationship level is in the global state
   FString RelationshipKey = FString::Printf(TEXT("Relationship_%s"), *NPCID);
   const FTAVariant* RelationshipVar = Context.GlobalState.Find(RelationshipKey);
   
   if (RelationshipVar && RelationshipVar->IsType<int32>())
   {
       int32 CurrentLevel = RelationshipVar->AsInt();
       
       bool Result = false;
       switch (Operator)
       {
           case EComparisonOperator::Equal:
               Result = (CurrentLevel == RequiredLevel);
               break;
           case EComparisonOperator::NotEqual:
               Result = (CurrentLevel != RequiredLevel);
               break;
           case EComparisonOperator::Greater:
               Result = (CurrentLevel > RequiredLevel);
               break;
           case EComparisonOperator::GreaterEqual:
               Result = (CurrentLevel >= RequiredLevel);
               break;
           case EComparisonOperator::Less:
               Result = (CurrentLevel < RequiredLevel);
               break;
           case EComparisonOperator::LessEqual:
               Result = (CurrentLevel <= RequiredLevel);
               break;
           default:
               Result = false;
               break;
       }
       
       return bInverted ? !Result : Result;
   }
   
   // Default to false if relationship not found
   return bInverted;
}

FString FRelationshipLevelCondition::GetDescription() const
{
   FString OperatorStr;
   switch (Operator)
   {
       case EComparisonOperator::Equal: OperatorStr = TEXT("=="); break;
       case EComparisonOperator::NotEqual: OperatorStr = TEXT("!="); break;
       case EComparisonOperator::Greater: OperatorStr = TEXT(">"); break;
       case EComparisonOperator::GreaterEqual: OperatorStr = TEXT(">="); break;
       case EComparisonOperator::Less: OperatorStr = TEXT("<"); break;
       case EComparisonOperator::LessEqual: OperatorStr = TEXT("<="); break;
       default: OperatorStr = TEXT("?"); break;
   }
   
   return FString::Printf(TEXT("Relationship with %s %s %d"), *NPCID, *OperatorStr, RequiredLevel);
}

TSharedPtr<FTACondition> FRelationshipLevelCondition::Clone() const
{
   TSharedPtr<FRelationshipLevelCondition> Clone = MakeShared<FRelationshipLevelCondition>();
   Clone->bInverted = bInverted;
   Clone->ConditionName = ConditionName;
   Clone->NPCID = NPCID;
   Clone->RequiredLevel = RequiredLevel;
   Clone->Operator = Operator;
   return Clone;
}

void FRelationshipLevelCondition::Serialize(FArchive& Ar)
{
   FTACondition::Serialize(Ar);
   Ar << NPCID;
   Ar << RequiredLevel;
   
   int32 OpInt = (int32)Operator;
   Ar << OpInt;
   Operator = (EComparisonOperator)OpInt;
}

//------------------------------------------------------------------------------
// FModifyRelationshipAction
//------------------------------------------------------------------------------

FModifyRelationshipAction::FModifyRelationshipAction()
   : FTAAction()
   , NPCID(TEXT(""))
   , ChangeAmount(0)
{
   ActionName = TEXT("Modify Relationship");
}

void FModifyRelationshipAction::Execute(const FTAContext& Context) const
{
   // In a real implementation, this would update a relationship system
   // For this example, update relationship in global state
   FString RelationshipKey = FString::Printf(TEXT("Relationship_%s"), *NPCID);
   
   // Get current relationship value
   int32 CurrentLevel = 0;
   const FTAVariant* RelationshipVar = Context.GlobalState.Find(RelationshipKey);
   if (RelationshipVar && RelationshipVar->IsType<int32>())
   {
       CurrentLevel = RelationshipVar->AsInt();
   }
   
   // Update value
   int32 NewLevel = CurrentLevel + ChangeAmount;
   
   // Clamp to reasonable range (e.g., -100 to 100)
   NewLevel = FMath::Clamp(NewLevel, -100, 100);
   
   // Store updated value
   const_cast<FTAContext&>(Context).GlobalState.Add(RelationshipKey, FTAVariant(NewLevel));
   
   UE_LOG(LogTreeAutomata, Display, TEXT("Modified relationship with %s: %d -> %d (%s%d)"), 
       *NPCID, CurrentLevel, NewLevel, 
       ChangeAmount >= 0 ? TEXT("+") : TEXT(""), ChangeAmount);
}

FString FModifyRelationshipAction::GetDescription() const
{
   return FString::Printf(TEXT("Change relationship with %s by %s%d"), 
       *NPCID, ChangeAmount >= 0 ? TEXT("+") : TEXT(""), ChangeAmount);
}

TSharedPtr<FTAAction> FModifyRelationshipAction::Clone() const
{
   TSharedPtr<FModifyRelationshipAction> Clone = MakeShared<FModifyRelationshipAction>();
   Clone->ActionName = ActionName;
   Clone->NPCID = NPCID;
   Clone->ChangeAmount = ChangeAmount;
   return Clone;
}

void FModifyRelationshipAction::Serialize(FArchive& Ar)
{
   FTAAction::Serialize(Ar);
   Ar << NPCID;
   Ar << ChangeAmount;
}