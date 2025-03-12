// FQuestNode.cpp
#include "FQuestNode.h"
#include "QuestSubsystem.h"
#include "TALogging.h"

//------------------------------------------------------------------------------
// FQuestNode
//------------------------------------------------------------------------------

FQuestNode::FQuestNode()
    : FTANode()
    , Status(EQuestStatus::Available)
    , XPReward(0)
    , bVisibleInJournal(true)
    , bIsMainQuest(false)
    , QuestCategory(TEXT("Misc"))
{
    NodeType = TEXT("Quest");
}

bool FQuestNode::EvaluateTransitions(const FTAContext& Context, TSharedPtr<FTANode>& OutNextNode)
{
    // First check if there's a direct input-based transition
    if (FTANode::EvaluateTransitions(Context, OutNextNode))
    {
        return true;
    }
    
    // If all objectives are complete, check for auto-transitions to completion nodes
    if (Status == EQuestStatus::Active && AreAllObjectivesComplete())
    {
        // Look for auto-complete transitions
        for (const FTATransition& Transition : Transitions)
        {
            if (Transition.TargetNode.IsValid())
            {
                TSharedPtr<FQuestNode> QuestNode = StaticCastSharedPtr<FQuestNode>(Transition.TargetNode);
                if (QuestNode.IsValid() && QuestNode->Status == EQuestStatus::Completed)
                {
                    // Check if this is an auto-complete transition (no inputs required)
                    if (Transition.Conditions.Num() == 0 || Transition.Evaluate(Context))
                    {
                        OutNextNode = Transition.TargetNode;
                        return true;
                    }
                }
            }
        }
    }
    
    return false;
}

void FQuestNode::ExecuteEntryActions(const FTAContext& Context)
{
    // Update quest status in journal
    UQuestSubsystem* QuestSubsystem = Context.World ? Context.World->GetGameInstance()->GetSubsystem<UQuestSubsystem>() : nullptr;
    if (QuestSubsystem)
    {
        QuestSubsystem->UpdateQuestStatus(NodeID, Status, Title, Description);
    }
    
    // Special handling for completed quests
    if (Status == EQuestStatus::Completed)
    {
        // Award XP
        if (XPReward > 0 && Context.PlayerActor)
        {
            // This would use the actual player character class in real implementation
            UE_LOG(LogTreeAutomata, Display, TEXT("Quest completed! Awarding %d XP to player"), XPReward);
            
            // Example of how rewards would be processed
            const_cast<FTAContext&>(Context).SetGlobal(TEXT("LastQuestXPReward"), FTAVariant(XPReward));
        }
        
        // Award items
        if (ItemRewards.Num() > 0 && Context.PlayerActor)
        {
            UE_LOG(LogTreeAutomata, Display, TEXT("Quest completed! Awarding %d items to player"), ItemRewards.Num());
            
            // Example of how item rewards would be processed
            const_cast<FTAContext&>(Context).SetGlobal(TEXT("LastQuestItemRewards"), FTAVariant(ItemRewards.Num()));
        }
    }
    
    // Execute standard entry actions
    FTANode::ExecuteEntryActions(Context);
}

void FQuestNode::ExecuteExitActions(const FTAContext& Context)
{
    // Execute standard exit actions
    FTANode::ExecuteExitActions(Context);
}

TSharedPtr<FTANode> FQuestNode::Clone() const
{
    TSharedPtr<FQuestNode> ClonedNode = StaticCastSharedPtr<FQuestNode>(FTANode::Clone());
    
    // Copy quest-specific properties
    if (ClonedNode.IsValid())
    {
        ClonedNode->Status = Status;
        ClonedNode->Title = Title;
        ClonedNode->Description = Description;
        ClonedNode->XPReward = XPReward;
        ClonedNode->ItemRewards = ItemRewards;
        ClonedNode->Objectives = Objectives;
        ClonedNode->bVisibleInJournal = bVisibleInJournal;
        ClonedNode->bIsMainQuest = bIsMainQuest;
        ClonedNode->QuestCategory = QuestCategory;
        ClonedNode->QuestTags = QuestTags;
    }
    
    return ClonedNode;
}

FString FQuestNode::ToString() const
{
    FString StatusString;
    switch (Status)
    {
        case EQuestStatus::Available: StatusString = TEXT("Available"); break;
        case EQuestStatus::Active: StatusString = TEXT("Active"); break;
        case EQuestStatus::Completed: StatusString = TEXT("Completed"); break;
        case EQuestStatus::Failed: StatusString = TEXT("Failed"); break;
        default: StatusString = TEXT("Unknown"); break;
    }
    
    return FString::Printf(TEXT("[Quest: %s (%s) ID: %s Status: %s Objectives: %d/%d]"), 
        *NodeName, 
        *Title.ToString(),
        *NodeID.ToString(),
        *StatusString,
        GetCompletedObjectiveCount(),
        Objectives.Num());
}

void FQuestNode::Serialize(FArchive& Ar)
{
    FTANode::Serialize(Ar);
    
    int32 StatusInt = (int32)Status;
    Ar << StatusInt;
    Status = (EQuestStatus)StatusInt;
    
    // Serialize title and description
    FString TitleStr = Title.ToString();
    Ar << TitleStr;
    Title = FText::FromString(TitleStr);
    
    FString DescStr = Description.ToString();
    Ar << DescStr;
    Description = FText::FromString(DescStr);
    
    Ar << XPReward;
    
    // Serialize item rewards
    int32 RewardCount = ItemRewards.Num();
    Ar << RewardCount;
    
    if (Ar.IsLoading())
    {
        ItemRewards.Empty(RewardCount);
        for (int32 i = 0; i < RewardCount; ++i)
        {
            FItemReward Reward;
            Ar << Reward;
            ItemRewards.Add(Reward);
        }
    }
    else
    {
        for (auto& Reward : ItemRewards)
        {
            Ar << Reward;
        }
    }
    
    // Serialize objectives
    int32 ObjectiveCount = Objectives.Num();
    Ar << ObjectiveCount;
    
    if (Ar.IsLoading())
    {
        Objectives.Empty(ObjectiveCount);
        for (int32 i = 0; i < ObjectiveCount; ++i)
        {
            FQuestObjective Objective;
            Ar << Objective;
            Objectives.Add(Objective);
        }
    }
    else
    {
        for (auto& Objective : Objectives)
        {
            Ar << Objective;
        }
    }
    
    Ar << bVisibleInJournal;
    Ar << bIsMainQuest;
    Ar << QuestCategory;
    
    // Serialize tags
    int32 TagCount = QuestTags.Num();
    Ar << TagCount;
    
    if (Ar.IsLoading())
    {
        QuestTags.Empty(TagCount);
        for (int32 i = 0; i < TagCount; ++i)
        {
            FString Tag;
            Ar << Tag;
            QuestTags.Add(Tag);
        }
    }
    else
    {
        for (const FString& Tag : QuestTags)
        {
            FString TagCopy = Tag;
            Ar << TagCopy;
        }
    }
}

bool FQuestNode::AreAllObjectivesComplete() const
{
    if (Objectives.Num() == 0)
    {
        return true;
    }
    
    for (const FQuestObjective& Objective : Objectives)
    {
        if (!Objective.bIsComplete)
        {
            return false;
        }
    }
    
    return true;
}

bool FQuestNode::IsObjectiveComplete(const FGuid& ObjectiveID) const
{
    for (const FQuestObjective& Objective : Objectives)
    {
        if (Objective.ObjectiveID == ObjectiveID)
        {
            return Objective.bIsComplete;
        }
    }
    
    // Objective not found
    return false;
}

void FQuestNode::UpdateObjectiveProgress(const FGuid& ObjectiveID, int32 Progress)
{
    for (FQuestObjective& Objective : Objectives)
    {
        if (Objective.ObjectiveID == ObjectiveID)
        {
            Objective.CurrentProgress += Progress;
            
            // Check if objective is now complete
            if (Objective.CurrentProgress >= Objective.RequiredProgress)
            {
                Objective.bIsComplete = true;
                Objective.CurrentProgress = Objective.RequiredProgress;
            }
            
            UE_LOG(LogTreeAutomata, Display, TEXT("Updated objective '%s' progress: %d/%d %s"), 
                *Objective.Title.ToString(),
                Objective.CurrentProgress,
                Objective.RequiredProgress,
                Objective.bIsComplete ? TEXT("(Complete)") : TEXT(""));
            
            break;
        }
    }
}

void FQuestNode::CompleteObjective(const FGuid& ObjectiveID)
{
    for (FQuestObjective& Objective : Objectives)
    {
        if (Objective.ObjectiveID == ObjectiveID)
        {
            Objective.bIsComplete = true;
            Objective.CurrentProgress = Objective.RequiredProgress;
            
            UE_LOG(LogTreeAutomata, Display, TEXT("Completed objective: %s"), *Objective.Title.ToString());
            break;
        }
    }
}

float FQuestNode::GetCompletionPercentage() const
{
    if (Objectives.Num() == 0)
    {
        return 1.0f;
    }
    
    int32 CompletedCount = 0;
    for (const FQuestObjective& Objective : Objectives)
    {
        if (Objective.bIsComplete)
        {
            CompletedCount++;
        }
    }
    
    return (float)CompletedCount / (float)Objectives.Num();
}

int32 FQuestNode::GetCompletedObjectiveCount() const
{
    int32 CompletedCount = 0;
    for (const FQuestObjective& Objective : Objectives)
    {
        if (Objective.bIsComplete)
        {
            CompletedCount++;
        }
    }
    
    return CompletedCount;
}

//------------------------------------------------------------------------------
// FQuestObjectiveCompletedCondition
//------------------------------------------------------------------------------

FQuestObjectiveCompletedCondition::FQuestObjectiveCompletedCondition()
    : FTACondition()
    , bAllObjectives(false)
{
    ConditionName = TEXT("Quest Objective Completed");
}

bool FQuestObjectiveCompletedCondition::Evaluate(const FTAContext& Context) const
{
    TSharedPtr<FQuestNode> QuestNode = FindQuestNode(Context);
    if (!QuestNode.IsValid())
    {
        return false;
    }
    
    bool Result;
    if (bAllObjectives)
    {
        Result = QuestNode->AreAllObjectivesComplete();
    }
    else
    {
        Result = QuestNode->IsObjectiveComplete(ObjectiveID);
    }
    
    return bInverted ? !Result : Result;
}

FString FQuestObjectiveCompletedCondition::GetDescription() const
{
    if (bAllObjectives)
    {
        return TEXT("All quest objectives are complete");
    }
    else
    {
        return FString::Printf(TEXT("Quest objective %s is complete"), *ObjectiveID.ToString());
    }
}

TSharedPtr<FTACondition> FQuestObjectiveCompletedCondition::Clone() const
{
    TSharedPtr<FQuestObjectiveCompletedCondition> Clone = MakeShared<FQuestObjectiveCompletedCondition>();
    Clone->bInverted = bInverted;
    Clone->ConditionName = ConditionName;
    Clone->ObjectiveID = ObjectiveID;
    Clone->bAllObjectives = bAllObjectives;
    return Clone;
}

void FQuestObjectiveCompletedCondition::Serialize(FArchive& Ar)
{
    FTACondition::Serialize(Ar);
    Ar << ObjectiveID;
    Ar << bAllObjectives;
}

TSharedPtr<FQuestNode> FQuestObjectiveCompletedCondition::FindQuestNode(const FTAContext& Context) const
{
    // In a real implementation, we would find the node in the current automaton
    // For this example, we'll assume the current node is available in the context
    
    // Try to get the current node from context
    const FTAVariant* CurrentNodeVar = Context.GlobalState.Find(TEXT("CurrentNode"));
    if (CurrentNodeVar && CurrentNodeVar->IsType<void*>())
    {
        FTANode* NodePtr = static_cast<FTANode*>(CurrentNodeVar->AsPtr());
        if (NodePtr)
        {
            return StaticCastSharedPtr<FQuestNode>(TSharedPtr<FTANode>(NodePtr));
        }
    }
    
    return nullptr;
}

//------------------------------------------------------------------------------
// FQuestStatusCondition
//------------------------------------------------------------------------------

FQuestStatusCondition::FQuestStatusCondition()
    : FTACondition()
    , RequiredStatus(EQuestStatus::Active)
{
    ConditionName = TEXT("Quest Status");
}

bool FQuestStatusCondition::Evaluate(const FTAContext& Context) const
{
    // Try to get the current node status
    const FTAVariant* StatusVar = Context.GlobalState.Find(TEXT("QuestStatus"));
    if (StatusVar && StatusVar->IsType<int32>())
    {
        EQuestStatus CurrentStatus = (EQuestStatus)StatusVar->AsInt();
        bool Result = (CurrentStatus == RequiredStatus);
        return bInverted ? !Result : Result;
    }
    
    return false;
}

FString FQuestStatusCondition::GetDescription() const
{
    FString StatusString;
    switch (RequiredStatus)
    {
        case EQuestStatus::Available: StatusString = TEXT("Available"); break;
        case EQuestStatus::Active: StatusString = TEXT("Active"); break;
        case EQuestStatus::Completed: StatusString = TEXT("Completed"); break;
        case EQuestStatus::Failed: StatusString = TEXT("Failed"); break;
        default: StatusString = TEXT("Unknown"); break;
    }
    
    return FString::Printf(TEXT("Quest status is %s"), *StatusString);
}

TSharedPtr<FTACondition> FQuestStatusCondition::Clone() const
{
    TSharedPtr<FQuestStatusCondition> Clone = MakeShared<FQuestStatusCondition>();
    Clone->bInverted = bInverted;
    Clone->ConditionName = ConditionName;
    Clone->RequiredStatus = RequiredStatus;
    return Clone;
}

void FQuestStatusCondition::Serialize(FArchive& Ar)
{
    FTACondition::Serialize(Ar);
    
    int32 StatusInt = (int32)RequiredStatus;
    Ar << StatusInt;
    RequiredStatus = (EQuestStatus)StatusInt;
}

//------------------------------------------------------------------------------
// FAddQuestAction
//------------------------------------------------------------------------------

FAddQuestAction::FAddQuestAction()
    : FTAAction()
    , QuestID(TEXT(""))
    , bAutoActivate(true)
{
    ActionName = TEXT("Add Quest");
}

void FAddQuestAction::Execute(const FTAContext& Context) const
{
    UQuestSubsystem* QuestSubsystem = Context.World ? Context.World->GetGameInstance()->GetSubsystem<UQuestSubsystem>() : nullptr;
    if (QuestSubsystem)
    {
        QuestSubsystem->AddQuest(QuestID, bAutoActivate);
        UE_LOG(LogTreeAutomata, Display, TEXT("Added quest: %s (Auto-activate: %s)"), 
            *QuestID, bAutoActivate ? TEXT("Yes") : TEXT("No"));
    }
    else
    {
        UE_LOG(LogTreeAutomata, Warning, TEXT("FAddQuestAction::Execute: Quest subsystem not found"));
    }
}

FString FAddQuestAction::GetDescription() const
{
    return FString::Printf(TEXT("Add quest '%s' (Auto-activate: %s)"), 
        *QuestID, bAutoActivate ? TEXT("Yes") : TEXT("No"));
}

TSharedPtr<FTAAction> FAddQuestAction::Clone() const
{
    TSharedPtr<FAddQuestAction> Clone = MakeShared<FAddQuestAction>();
    Clone->ActionName = ActionName;
    Clone->QuestID = QuestID;
    Clone->bAutoActivate = bAutoActivate;
    return Clone;
}

void FAddQuestAction::Serialize(FArchive& Ar)
{
    FTAAction::Serialize(Ar);
    Ar << QuestID;
    Ar << bAutoActivate;
}

//------------------------------------------------------------------------------
// FCompleteObjectiveAction
//------------------------------------------------------------------------------

FCompleteObjectiveAction::FCompleteObjectiveAction()
    : FTAAction()
    , bUpdateProgressOnly(false)
    , ProgressAmount(1)
{
    ActionName = TEXT("Complete Objective");
}

void FCompleteObjectiveAction::Execute(const FTAContext& Context) const
{
    // Get the current node from context
    const FTAVariant* CurrentNodeVar = Context.GlobalState.Find(TEXT("CurrentNode"));
    if (CurrentNodeVar && CurrentNodeVar->IsType<void*>())
    {
        FTANode* NodePtr = static_cast<FTANode*>(CurrentNodeVar->AsPtr());
        if (NodePtr)
        {
            // Try to cast to quest node
            FQuestNode* QuestNode = static_cast<FQuestNode*>(NodePtr);
            
            if (bUpdateProgressOnly)
            {
                QuestNode->UpdateObjectiveProgress(ObjectiveID, ProgressAmount);
            }
            else
            {
                QuestNode->CompleteObjective(ObjectiveID);
            }
            
            // Update quest subsystem if available
            UQuestSubsystem* QuestSubsystem = Context.World ? Context.World->GetGameInstance()->GetSubsystem<UQuestSubsystem>() : nullptr;
            if (QuestSubsystem)
            {
                QuestSubsystem->UpdateQuestObjective(QuestNode->NodeID, ObjectiveID, 
                    bUpdateProgressOnly ? ProgressAmount : INT32_MAX);
            }
        }
    }
}

FString FCompleteObjectiveAction::GetDescription() const
{
    if (bUpdateProgressOnly)
    {
        return FString::Printf(TEXT("Update objective %s progress by %d"), 
            *ObjectiveID.ToString(), ProgressAmount);
    }
    else
    {
        return FString::Printf(TEXT("Complete objective %s"), *ObjectiveID.ToString());
    }
}

TSharedPtr<FTAAction> FCompleteObjectiveAction::Clone() const
{
    TSharedPtr<FCompleteObjectiveAction> Clone = MakeShared<FCompleteObjectiveAction>();
    Clone->ActionName = ActionName;
    Clone->ObjectiveID = ObjectiveID;
    Clone->bUpdateProgressOnly = bUpdateProgressOnly;
    Clone->ProgressAmount = ProgressAmount;
    return Clone;
}

void FCompleteObjectiveAction::Serialize(FArchive& Ar)
{
    FTAAction::Serialize(Ar);
    Ar << ObjectiveID;
    Ar << bUpdateProgressOnly;
    Ar << ProgressAmount;
}