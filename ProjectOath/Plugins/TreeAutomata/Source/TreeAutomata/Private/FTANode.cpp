// FTANode.cpp
#include "FTANode.h"
#include "TANodePool.h"

FTANode::FTANode()
    : NodeID(FGuid::NewGuid())
    , NodeName(TEXT("Default Node"))
    , NodeType(TEXT("Base"))
    , bIsAcceptingState(false)
{
}

FTANode::FTANode(const FTANode& Other)
    : NodeID(FGuid::NewGuid()) // Generate new ID for copy
    , NodeName(Other.NodeName)
    , NodeType(Other.NodeType)
    , StateData(Other.StateData)
    , bIsAcceptingState(Other.bIsAcceptingState)
{
    // Deep copy transitions
    for (const FTATransition& Transition : Other.Transitions)
    {
        FTATransition NewTransition = Transition;
        NewTransition.TargetNode = nullptr; // Will be set later during full tree cloning
        Transitions.Add(NewTransition);
    }
    
    // Deep copy actions
    for (const TSharedPtr<FTAAction>& Action : Other.EntryActions)
    {
        if (Action.IsValid())
        {
            EntryActions.Add(Action->Clone());
        }
    }
    
    for (const TSharedPtr<FTAAction>& Action : Other.ExitActions)
    {
        if (Action.IsValid())
        {
            ExitActions.Add(Action->Clone());
        }
    }
    
    // Children will be copied by the parent clone operation
}

FTANode::~FTANode()
{
    // Clear references to prevent memory leaks
    Children.Empty();
    EntryActions.Empty();
    ExitActions.Empty();
    Transitions.Empty();
}

bool FTANode::EvaluateTransitions(const FTAContext& Context, TSharedPtr<FTANode>& OutNextNode)
{
    // Sort transitions by priority
    Transitions.Sort([](const FTATransition& A, const FTATransition& B) {
        return A.Priority > B.Priority;
    });
    
    // Evaluate each transition in priority order
    for (const FTATransition& Transition : Transitions)
    {
        if (Transition.Evaluate(Context))
        {
            OutNextNode = Transition.TargetNode;
            return true;
        }
    }
    
    return false;
}

void FTANode::ExecuteEntryActions(const FTAContext& Context)
{
    for (TSharedPtr<FTAAction>& Action : EntryActions)
    {
        if (Action.IsValid())
        {
            Action->Execute(Context);
        }
    }
}

void FTANode::ExecuteExitActions(const FTAContext& Context)
{
    for (TSharedPtr<FTAAction>& Action : ExitActions)
    {
        if (Action.IsValid())
        {
            Action->Execute(Context);
        }
    }
}

TSharedPtr<FTANode> FTANode::FindChildByID(const FGuid& ChildID)
{
    for (TSharedPtr<FTANode>& Child : Children)
    {
        if (Child.IsValid() && Child->NodeID == ChildID)
        {
            return Child;
        }
    }
    
    return nullptr;
}

TSharedPtr<FTANode> FTANode::FindChildByName(const FString& ChildName)
{
    for (TSharedPtr<FTANode>& Child : Children)
    {
        if (Child.IsValid() && Child->NodeName == ChildName)
        {
            return Child;
        }
    }
    
    return nullptr;
}

void FTANode::AddChild(TSharedPtr<FTANode> Child)
{
    if (Child.IsValid())
    {
        Children.Add(Child);
        Child->Parent = AsShared();
    }
}

bool FTANode::RemoveChild(const FGuid& ChildID)
{
    for (int32 i = 0; i < Children.Num(); ++i)
    {
        if (Children[i].IsValid() && Children[i]->NodeID == ChildID)
        {
            Children.RemoveAt(i);
            return true;
        }
    }
    
    return false;
}

void FTANode::AddTransition(const FTATransition& Transition)
{
    Transitions.Add(Transition);
}

bool FTANode::RemoveTransition(int32 Index)
{
    if (Transitions.IsValidIndex(Index))
    {
        Transitions.RemoveAt(Index);
        return true;
    }
    
    return false;
}

TSharedPtr<FTANode> FTANode::Clone() const
{
    // Create a new node of the same type
    TSharedPtr<FTANode> ClonedNode = MakeShared<FTANode>(*this);
    
    // Clone children
    for (const TSharedPtr<FTANode>& Child : Children)
    {
        if (Child.IsValid())
        {
            TSharedPtr<FTANode> ClonedChild = Child->Clone();
            ClonedNode->AddChild(ClonedChild);
        }
    }
    
    return ClonedNode;
}

void FTANode::Serialize(FArchive& Ar)
{
    // Basic properties
    Ar << NodeID;
    Ar << NodeName;
    Ar << NodeType;
    Ar << bIsAcceptingState;
    
    // Serialize state data
    int32 StateCount = StateData.Num();
    Ar << StateCount;
    
    if (Ar.IsLoading())
    {
        StateData.Empty(StateCount);
        for (int32 i = 0; i < StateCount; ++i)
        {
            FString Key;
            FVariant Value;
            Ar << Key;
            Ar << Value;
            StateData.Add(Key, Value);
        }
    }
    else
    {
        for (auto& Pair : StateData)
        {
            FString Key = Pair.Key;
            Ar << Key;
            Ar << Pair.Value;
        }
    }
    
    // Serialize transitions
    int32 TransitionCount = Transitions.Num();
    Ar << TransitionCount;
    
    if (Ar.IsLoading())
    {
        Transitions.Empty(TransitionCount);
        for (int32 i = 0; i < TransitionCount; ++i)
        {
            FTATransition Transition;
            
            // Serialize basic transition properties
            Ar << Transition.TransitionID;
            Ar << Transition.TransitionName;
            Ar << Transition.Priority;
            
            // Target node will be resolved in a second pass after all nodes are loaded
            FGuid TargetNodeID;
            Ar << TargetNodeID;
            
            // Store target ID for later resolution
            Transition.StateData.Add(TEXT("TargetNodeID"), FVariant(TargetNodeID.ToString()));
            
            // Serialize conditions
            int32 ConditionCount;
            Ar << ConditionCount;
            
            for (int32 j = 0; j < ConditionCount; ++j)
            {
                // Serialize condition type
                FString ConditionType;
                Ar << ConditionType;
                
                // Create appropriate condition based on type
                TSharedPtr<FTACondition> Condition;
                
                if (ConditionType == TEXT("Boolean"))
                {
                    Condition = MakeShared<FBooleanCondition>();
                }
                else if (ConditionType == TEXT("Variable"))
                {
                    Condition = MakeShared<FVariableCondition>();
                }
                else if (ConditionType == TEXT("Input"))
                {
                    Condition = MakeShared<FInputCondition>();
                }
                else if (ConditionType == TEXT("QuestObjectiveCompleted"))
                {
                    Condition = MakeShared<FQuestObjectiveCompletedCondition>();
                }
                else if (ConditionType == TEXT("RelationshipLevel"))
                {
                    Condition = MakeShared<FRelationshipLevelCondition>();
                }
                else
                {
                    // Create a default condition for unknown types
                    Condition = MakeShared<FBooleanCondition>();
                    UE_LOG(LogTreeAutomata, Warning, TEXT("Unknown condition type during deserialization: %s"), *ConditionType);
                }
                
                // Deserialize the condition data
                if (Condition.IsValid())
                {
                    Condition->Serialize(Ar);
                    Transition.Conditions.Add(Condition);
                }
            }
            
            Transitions.Add(Transition);
        }
    }
    else // Saving
    {
        for (const FTATransition& Transition : Transitions)
        {
            // Serialize basic transition properties
            Ar << Transition.TransitionID;
            Ar << Transition.TransitionName;
            Ar << Transition.Priority;
            
            // Serialize target node ID
            FGuid TargetNodeID = Transition.TargetNode.IsValid() ? Transition.TargetNode->NodeID : FGuid();
            Ar << TargetNodeID;
            
            // Serialize conditions
            int32 ConditionCount = Transition.Conditions.Num();
            Ar << ConditionCount;
            
            for (const TSharedPtr<FTACondition>& Condition : Transition.Conditions)
            {
                if (Condition.IsValid())
                {
                    // Determine condition type
                    FString ConditionType;
                    
                    if (Cast<FBooleanCondition>(Condition.Get()))
                    {
                        ConditionType = TEXT("Boolean");
                    }
                    else if (Cast<FVariableCondition>(Condition.Get()))
                    {
                        ConditionType = TEXT("Variable");
                    }
                    else if (Cast<FInputCondition>(Condition.Get()))
                    {
                        ConditionType = TEXT("Input");
                    }
                    else if (Cast<FQuestObjectiveCompletedCondition>(Condition.Get()))
                    {
                        ConditionType = TEXT("QuestObjectiveCompleted");
                    }
                    else if (Cast<FRelationshipLevelCondition>(Condition.Get()))
                    {
                        ConditionType = TEXT("RelationshipLevel");
                    }
                    else
                    {
                        ConditionType = TEXT("Unknown");
                    }
                    
                    // Serialize condition type
                    Ar << ConditionType;
                    
                    // Serialize condition data
                    Condition->Serialize(Ar);
                }
            }
        }
    }
    
    // Serialize entry actions
    int32 EntryActionCount = EntryActions.Num();
    Ar << EntryActionCount;
    
    if (Ar.IsLoading())
    {
        EntryActions.Empty(EntryActionCount);
        for (int32 i = 0; i < EntryActionCount; ++i)
        {
            // Serialize action type
            FString ActionType;
            Ar << ActionType;
            
            // Create appropriate action based on type
            TSharedPtr<FTAAction> Action;
            
            if (ActionType == TEXT("SetVariable"))
            {
                Action = MakeShared<FSetVariableAction>();
            }
            else if (ActionType == TEXT("LogMessage"))
            {
                Action = MakeShared<FLogMessageAction>();
            }
            else if (ActionType == TEXT("SpawnActor"))
            {
                Action = MakeShared<FSpawnActorAction>();
            }
            else if (ActionType == TEXT("AddQuest"))
            {
                Action = MakeShared<FAddQuestAction>();
            }
            else if (ActionType == TEXT("CompleteObjective"))
            {
                Action = MakeShared<FCompleteObjectiveAction>();
            }
            else if (ActionType == TEXT("ModifyRelationship"))
            {
                Action = MakeShared<FModifyRelationshipAction>();
            }
            else
            {
                // Create a default action for unknown types
                Action = MakeShared<FLogMessageAction>();
                UE_LOG(LogTreeAutomata, Warning, TEXT("Unknown action type during deserialization: %s"), *ActionType);
            }
            
            // Deserialize the action data
            if (Action.IsValid())
            {
                Action->Serialize(Ar);
                EntryActions.Add(Action);
            }
        }
    }
    else // Saving
    {
        for (const TSharedPtr<FTAAction>& Action : EntryActions)
        {
            if (Action.IsValid())
            {
                // Determine action type
                FString ActionType;
                
                if (Cast<FSetVariableAction>(Action.Get()))
                {
                    ActionType = TEXT("SetVariable");
                }
                else if (Cast<FLogMessageAction>(Action.Get()))
                {
                    ActionType = TEXT("LogMessage");
                }
                else if (Cast<FSpawnActorAction>(Action.Get()))
                {
                    ActionType = TEXT("SpawnActor");
                }
                else if (Cast<FAddQuestAction>(Action.Get()))
                {
                    ActionType = TEXT("AddQuest");
                }
                else if (Cast<FCompleteObjectiveAction>(Action.Get()))
                {
                    ActionType = TEXT("CompleteObjective");
                }
                else if (Cast<FModifyRelationshipAction>(Action.Get()))
                {
                    ActionType = TEXT("ModifyRelationship");
                }
                else
                {
                    ActionType = TEXT("Unknown");
                }
                
                // Serialize action type
                Ar << ActionType;
                
                // Serialize action data
                Action->Serialize(Ar);
            }
        }
    }
    
    // Serialize exit actions using the same pattern as entry actions
    int32 ExitActionCount = ExitActions.Num();
    Ar << ExitActionCount;
    
    if (Ar.IsLoading())
    {
        ExitActions.Empty(ExitActionCount);
        for (int32 i = 0; i < ExitActionCount; ++i)
        {
            // Serialize action type
            FString ActionType;
            Ar << ActionType;
            
            // Create appropriate action based on type
            TSharedPtr<FTAAction> Action;
            
            if (ActionType == TEXT("SetVariable"))
            {
                Action = MakeShared<FSetVariableAction>();
            }
            else if (ActionType == TEXT("LogMessage"))
            {
                Action = MakeShared<FLogMessageAction>();
            }
            else if (ActionType == TEXT("SpawnActor"))
            {
                Action = MakeShared<FSpawnActorAction>();
            }
            else if (ActionType == TEXT("AddQuest"))
            {
                Action = MakeShared<FAddQuestAction>();
            }
            else if (ActionType == TEXT("CompleteObjective"))
            {
                Action = MakeShared<FCompleteObjectiveAction>();
            }
            else if (ActionType == TEXT("ModifyRelationship"))
            {
                Action = MakeShared<FModifyRelationshipAction>();
            }
            else
            {
                // Create a default action for unknown types
                Action = MakeShared<FLogMessageAction>();
                UE_LOG(LogTreeAutomata, Warning, TEXT("Unknown action type during deserialization: %s"), *ActionType);
            }
            
            // Deserialize the action data
            if (Action.IsValid())
            {
                Action->Serialize(Ar);
                ExitActions.Add(Action);
            }
        }
    }
    else // Saving
    {
        for (const TSharedPtr<FTAAction>& Action : ExitActions)
        {
            if (Action.IsValid())
            {
                // Determine action type
                FString ActionType;
                
                if (Cast<FSetVariableAction>(Action.Get()))
                {
                    ActionType = TEXT("SetVariable");
                }
                else if (Cast<FLogMessageAction>(Action.Get()))
                {
                    ActionType = TEXT("LogMessage");
                }
                else if (Cast<FSpawnActorAction>(Action.Get()))
                {
                    ActionType = TEXT("SpawnActor");
                }
                else if (Cast<FAddQuestAction>(Action.Get()))
                {
                    ActionType = TEXT("AddQuest");
                }
                else if (Cast<FCompleteObjectiveAction>(Action.Get()))
                {
                    ActionType = TEXT("CompleteObjective");
                }
                else if (Cast<FModifyRelationshipAction>(Action.Get()))
                {
                    ActionType = TEXT("ModifyRelationship");
                }
                else
                {
                    ActionType = TEXT("Unknown");
                }
                
                // Serialize action type
                Ar << ActionType;
                
                // Serialize action data
                Action->Serialize(Ar);
            }
        }
    }
    
    // Serialize child nodes
    int32 ChildCount = Children.Num();
    Ar << ChildCount;
    
    if (Ar.IsLoading())
    {
        Children.Empty(ChildCount);
        for (int32 i = 0; i < ChildCount; ++i)
        {
            // Create a new node
            TSharedPtr<FTANode> ChildNode = MakeShared<FTANode>();
            
            // Deserialize the child node
            ChildNode->Serialize(Ar);
            
            // Set parent relationship
            ChildNode->Parent = AsShared();
            
            // Add to children list
            Children.Add(ChildNode);
        }
    }
    else // Saving
    {
        for (const TSharedPtr<FTANode>& Child : Children)
        {
            if (Child.IsValid())
            {
                // Serialize the child node
                Child->Serialize(Ar);
            }
        }
    }
}

void FTANode::ResolveNodeReferences(TMap<FGuid, TSharedPtr<FTANode>>& NodeMap)
{
    // Resolve transition targets
    for (FTATransition& Transition : Transitions)
    {
        // Get stored target node ID
        const FVariant* TargetIDVar = Transition.StateData.Find(TEXT("TargetNodeID"));
        if (TargetIDVar && TargetIDVar->IsType<FString>())
        {
            FString TargetIDStr = TargetIDVar->AsString();
            FGuid TargetID;
            if (FGuid::Parse(TargetIDStr, TargetID))
            {
                // Look up the node in the map
                TSharedPtr<FTANode>* FoundNode = NodeMap.Find(TargetID);
                if (FoundNode && FoundNode->IsValid())
                {
                    Transition.TargetNode = *FoundNode;
                }
            }
        }
        
        // Remove temporary state data
        Transition.StateData.Remove(TEXT("TargetNodeID"));
    }
    
    // Recursively resolve references for child nodes
    for (TSharedPtr<FTANode>& Child : Children)
    {
        if (Child.IsValid())
        {
            Child->ResolveNodeReferences(NodeMap);
        }
    }
}

FString FTANode::ToString() const
{
    return FString::Printf(TEXT("[Node: %s (%s) ID: %s]"), 
        *NodeName, 
        *NodeType, 
        *NodeID.ToString());
}