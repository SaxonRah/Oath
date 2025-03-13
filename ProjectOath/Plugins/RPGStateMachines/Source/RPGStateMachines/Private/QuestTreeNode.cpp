#include "QuestTreeNode.h"
#include "PlayerSkillComponent.h"
#include "QuestDataComponent.h"

FQuestTreeNode::FQuestTreeNode()
{
    // Set default values
}

bool FQuestTreeNode::Link(FStateTreeLinker& Linker)
{
    Linker.LinkExternalData(SkillsHandle);
    Linker.LinkExternalData(QuestDataHandle);
    return true;
}

EStateTreeRunStatus FQuestTreeNode::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    // Get external data
    const UPlayerSkillComponent* Skills = Context.GetExternalData(SkillsHandle);
    UQuestDataComponent* QuestData = Context.GetExternalData(QuestDataHandle);
    
    if (!Skills || !QuestData)
    {
        return EStateTreeRunStatus::Failed;
    }
    
    // Check if the quest can be started based on requirements
    bool bMeetsRequirements = true;
    
    // Check skill requirements
    if (!Requirements.RequiredSkill.IsNone())
    {
        int32 PlayerSkillLevel = Skills->GetSkillLevel(Requirements.RequiredSkill);
        if (PlayerSkillLevel < Requirements.RequiredLevel)
        {
            bMeetsRequirements = false;
        }
    }
    
    // Check prerequisite quests
    for (const FName& PrereqQuest : Requirements.RequiredQuestsCompleted)
    {
        if (QuestData->GetQuestStatus(PrereqQuest) != EQuestStatus::Completed)
        {
            bMeetsRequirements = false;
            break;
        }
    }
    
    // Update quest status if requirements are met
    if (bMeetsRequirements)
    {
        QuestData->SetQuestStatus(QuestID, EQuestStatus::Available);
        return EStateTreeRunStatus::Running;
    }
    
    return EStateTreeRunStatus::Succeeded;
}

EStateTreeRunStatus FQuestTreeNode::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
    UQuestDataComponent* QuestData = Context.GetExternalData(QuestDataHandle);
    
    if (!QuestData)
    {
        return EStateTreeRunStatus::Failed;
    }
    
    // Check if all sub-quests are completed
    bool bAllSubQuestsCompleted = true;
    
    for (const FName& SubQuestID : SubQuestIDs)
    {
        if (QuestData->GetQuestStatus(SubQuestID) != EQuestStatus::Completed)
        {
            bAllSubQuestsCompleted = false;
            break;
        }
    }
    
    // Complete the quest if all sub-quests are done
    if (bAllSubQuestsCompleted && QuestData->GetQuestStatus(QuestID) == EQuestStatus::InProgress)
    {
        QuestData->SetQuestStatus(QuestID, EQuestStatus::Completed);
        QuestData->GiveQuestRewards(RewardIDs);
        return EStateTreeRunStatus::Succeeded;
    }
    
    return EStateTreeRunStatus::Running;
}