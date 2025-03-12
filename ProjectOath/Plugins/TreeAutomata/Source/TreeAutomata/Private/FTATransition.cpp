// FTATransition.cpp
#include "FTATransition.h"
#include "FTANode.h"

FTATransition::FTATransition()
    : TargetNode(nullptr)
    , Priority(0)
    , TransitionID(FGuid::NewGuid())
    , TransitionName(TEXT("Default Transition"))
{
}

FTATransition::FTATransition(TSharedPtr<FTANode> InTargetNode)
    : TargetNode(InTargetNode)
    , Priority(0)
    , TransitionID(FGuid::NewGuid())
    , TransitionName(TEXT("Default Transition"))
{
}

bool FTATransition::Evaluate(const FTAContext& Context) const
{
    // If no target node, transition can't be taken
    if (!TargetNode.IsValid())
    {
        return false;
    }
    
    // If no conditions, transition is always valid
    if (Conditions.Num() == 0)
    {
        return true;
    }
    
    // All conditions must be satisfied
    for (const TSharedPtr<FTACondition>& Condition : Conditions)
    {
        if (!Condition.IsValid() || !Condition->Evaluate(Context))
        {
            return false;
        }
    }
    
    return true;
}

void FTATransition::AddCondition(TSharedPtr<FTACondition> Condition)
{
    if (Condition.IsValid())
    {
        Conditions.Add(Condition);
    }
}

bool FTATransition::RemoveCondition(int32 Index)
{
    if (Conditions.IsValidIndex(Index))
    {
        Conditions.RemoveAt(Index);
        return true;
    }
    
    return false;
}

FTATransition FTATransition::Clone() const
{
    FTATransition ClonedTransition;
    ClonedTransition.TargetNode = TargetNode; // This is a reference, will be updated later
    ClonedTransition.Priority = Priority;
    ClonedTransition.TransitionName = TransitionName;
    ClonedTransition.TransitionID = FGuid::NewGuid(); // New ID for clone
    
    // Clone conditions
    for (const TSharedPtr<FTACondition>& Condition : Conditions)
    {
        if (Condition.IsValid())
        {
            ClonedTransition.AddCondition(Condition->Clone());
        }
    }
    
    return ClonedTransition;
}

void FTATransition::Serialize(FArchive& Ar)
{
    // Serialize basic properties
    Ar << TransitionID;
    Ar << TransitionName;
    Ar << Priority;
    
    // We don't serialize TargetNode directly because it would create circular references
    // Instead, we just serialize the ID during saving, and it gets resolved after loading
    
    // Serialize state data (for additional transition properties)
    int32 StateCount = StateData.Num();
    Ar << StateCount;
    
    if (Ar.IsLoading())
    {
        StateData.Empty(StateCount);
        for (int32 i = 0; i < StateCount; ++i)
        {
            FString Key;
            FTAVariant Value;
            Ar << Key;
            Ar << Value;
            StateData.Add(Key, Value);
        }
    }
    else // Saving
    {
        for (auto& Pair : StateData)
        {
            FString Key = Pair.Key;
            Ar << Key;
            Ar << Pair.Value;
        }
    }
    
    // Serialize conditions
    int32 ConditionCount = Conditions.Num();
    Ar << ConditionCount;
    
    if (Ar.IsLoading())
    {
        Conditions.Empty(ConditionCount);
        
        for (int32 i = 0; i < ConditionCount; ++i)
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
            else if (ConditionType == TEXT("QuestStatus"))
            {
                Condition = MakeShared<FQuestStatusCondition>();
            }
            else if (ConditionType == TEXT("DialogueOptionSelected"))
            {
                Condition = MakeShared<FDialogueOptionSelectedCondition>();
            }
            else if (ConditionType == TEXT("RelationshipLevel"))
            {
                Condition = MakeShared<FRelationshipLevelCondition>();
            }
            else
            {
                // Create a default condition for unknown types
                UE_LOG(LogTreeAutomata, Warning, TEXT("Unknown condition type during deserialization: %s"), *ConditionType);
                Condition = MakeShared<FBooleanCondition>();
            }
            
            // Deserialize the condition data
            if (Condition.IsValid())
            {
                Condition->Serialize(Ar);
                Conditions.Add(Condition);
            }
        }
    }
    else // Saving
    {
        for (const TSharedPtr<FTACondition>& Condition : Conditions)
        {
            if (Condition.IsValid())
            {
                // Determine condition type
                FString ConditionType;
                
                if (Condition->IsA<FBooleanCondition>())
                {
                    ConditionType = TEXT("Boolean");
                }
                else if (Condition->IsA<FVariableCondition>())
                {
                    ConditionType = TEXT("Variable");
                }
                else if (Condition->IsA<FInputCondition>())
                {
                    ConditionType = TEXT("Input");
                }
                else if (Condition->IsA<FQuestObjectiveCompletedCondition>())
                {
                    ConditionType = TEXT("QuestObjectiveCompleted");
                }
                else if (Condition->IsA<FQuestStatusCondition>())
                {
                    ConditionType = TEXT("QuestStatus");
                }
                else if (Condition->IsA<FDialogueOptionSelectedCondition>())
                {
                    ConditionType = TEXT("DialogueOptionSelected");
                }
                else if (Condition->IsA<FRelationshipLevelCondition>())
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
    
    // Custom data for specific transition types (optional)
    // This would be used for system-specific transition data
    bool bHasCustomData = false;
    Ar << bHasCustomData;
    
    if (bHasCustomData)
    {
        // Serialize transition type for custom data
        FString TransitionType;
        Ar << TransitionType;
        
        // For example, dialogue transitions might have additional properties
        if (TransitionType == TEXT("Dialogue"))
        {
            // Example: Serialize dialogue-specific data
            bool bIsSilent;
            float Delay;
            
            if (Ar.IsSaving())
            {
                // Extract values from state data or defaults
                bIsSilent = StateData.Contains(TEXT("IsSilent")) ? StateData[TEXT("IsSilent")].AsBool() : false;
                Delay = StateData.Contains(TEXT("Delay")) ? StateData[TEXT("Delay")].AsFloat() : 0.0f;
            }
            
            Ar << bIsSilent;
            Ar << Delay;
            
            if (Ar.IsLoading())
            {
                // Store in state data
                StateData.Add(TEXT("IsSilent"), FTAVariant(bIsSilent));
                StateData.Add(TEXT("Delay"), FTAVariant(Delay));
            }
        }
        else if (TransitionType == TEXT("Quest"))
        {
            // Example: Serialize quest-specific data
            bool bAutoComplete;
            int32 ReputationChange;
            
            if (Ar.IsSaving())
            {
                // Extract values from state data or defaults
                bAutoComplete = StateData.Contains(TEXT("AutoComplete")) ? StateData[TEXT("AutoComplete")].AsBool() : false;
                ReputationChange = StateData.Contains(TEXT("ReputationChange")) ? StateData[TEXT("ReputationChange")].AsInt() : 0;
            }
            
            Ar << bAutoComplete;
            Ar << ReputationChange;
            
            if (Ar.IsLoading())
            {
                // Store in state data
                StateData.Add(TEXT("AutoComplete"), FTAVariant(bAutoComplete));
                StateData.Add(TEXT("ReputationChange"), FTAVariant(ReputationChange));
            }
        }
    }
}

FString FTATransition::ToString() const
{
    FString TargetDesc = TargetNode.IsValid() ? TargetNode->NodeName : TEXT("Invalid Target");
    return FString::Printf(TEXT("[Transition: %s to %s, Priority: %d, Conditions: %d]"), 
        *TransitionName, 
        *TargetDesc, 
        Priority,
        Conditions.Num());
}