#include "WorldStateTreeNode.h"
#include "WorldStateComponent.h"
#include "QuestDataComponent.h"

FWorldStateTreeNode::FWorldStateTreeNode()
{
    // Set default values
}

bool FWorldStateTreeNode::Link(FStateTreeLinker& Linker)
{
    Linker.LinkExternalData(WorldStateHandle);
    Linker.LinkExternalData(QuestDataHandle);
    return true;
}

EStateTreeRunStatus FWorldStateTreeNode::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    const UWorldStateComponent* WorldState = Context.GetExternalData(WorldStateHandle);
    
    if (!WorldState)
    {
        return EStateTreeRunStatus::Failed;
    }
    
    // Reset timers
    const_cast<FWorldStateTreeNode*>(this)->CheckTimer = 0.0f;
    const_cast<FWorldStateTreeNode*>(this)->EventTimer = 0.0f;
    const_cast<FWorldStateTreeNode*>(this)->bEventActive = false;
    
    return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FWorldStateTreeNode::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
    UWorldStateComponent* WorldState = Context.GetExternalData(WorldStateHandle);
    UQuestDataComponent* QuestData = Context.GetExternalData(QuestDataHandle);
    
    if (!WorldState)
    {
        return EStateTreeRunStatus::Failed;
    }
    
    // Handle active event
    if (bEventActive)
    {
        // Update event timer if it's a temporary event
        if (EventDuration > 0.0f)
        {
            const_cast<FWorldStateTreeNode*>(this)->EventTimer += DeltaTime;
            
            // Check if event has expired
            if (EventTimer >= EventDuration)
            {
                // Revert effects
                for (const FWorldEventEffect& Effect : EventEffects)
                {
                    if (Effect.bIsMultiplier)
                    {
                        // Reset multiplier to 1.0
                        WorldState->SetWorldVariableMultiplier(Effect.VariableName, 1.0f);
                    }
                    else
                    {
                        // Reverse the change
                        WorldState->ChangeWorldVariable(Effect.VariableName, -Effect.ChangeValue);
                    }
                }
                
                // Reset event state
                const_cast<FWorldStateTreeNode*>(this)->bEventActive = false;
                const_cast<FWorldStateTreeNode*>(this)->EventTimer = 0.0f;
                
                // Notify that event has ended
                WorldState->NotifyWorldEventEnded(WorldEventID);
                
                // Handle cyclical events
                if (bCyclical)
                {
                    const_cast<FWorldStateTreeNode*>(this)->CheckTimer = 0.0f;
                    return EStateTreeRunStatus::Running;
                }
                
                return EStateTreeRunStatus::Succeeded;
            }
        }
        
        return EStateTreeRunStatus::Running;
    }
    
    // Not active yet, check conditions periodically
    const_cast<FWorldStateTreeNode*>(this)->CheckTimer += DeltaTime;
    
    if (CheckTimer >= TimeBetweenChecks)
    {
        const_cast<FWorldStateTreeNode*>(this)->CheckTimer = 0.0f;
        
        // Check if all conditions are met
        bool bAllConditionsMet = true;
        
        for (const FWorldEventCondition& Condition : RequiredConditions)
        {
            float CurrentValue = WorldState->GetWorldVariable(Condition.VariableName);
            bool bConditionMet = (CurrentValue >= Condition.MinValue && CurrentValue <= Condition.MaxValue);
            
            if (Condition.bInverseCondition)
            {
                bConditionMet = !bConditionMet;
            }
            
            if (!bConditionMet)
            {
                bAllConditionsMet = false;
                break;
            }
        }
        
        // If conditions are met, trigger the event
        if (bAllConditionsMet)
        {
            // Apply effects
            for (const FWorldEventEffect& Effect : EventEffects)
            {
                if (Effect.bIsMultiplier)
                {
                    WorldState->SetWorldVariableMultiplier(Effect.VariableName, Effect.ChangeValue);
                }
                else
                {
                    WorldState->ChangeWorldVariable(Effect.VariableName, Effect.ChangeValue);
                }
            }
            
            // Trigger quests if needed
            if (QuestData)
            {
                for (const FName& QuestID : QuestsTriggered)
                {
                    QuestData->SetQuestStatus(QuestID, EQuestStatus::Available);
                }
            }
            
            // Mark event as active
            const_cast<FWorldStateTreeNode*>(this)->bEventActive = true;
            
            // Register next possible events
            for (const FName& NextEvent : NextPossibleEvents)
            {
                WorldState->RegisterPossibleEvent(NextEvent);
            }
            
            // Notify that event has started
            WorldState->NotifyWorldEventStarted(WorldEventID, EventName, EventDescription, EventType);
            
            // If it's a permanent event with no duration, we're done
            if (EventDuration <= 0.0f && !bCyclical)
            {
                return EStateTreeRunStatus::Succeeded;
            }
        }
    }
    
    return EStateTreeRunStatus::Running;
}