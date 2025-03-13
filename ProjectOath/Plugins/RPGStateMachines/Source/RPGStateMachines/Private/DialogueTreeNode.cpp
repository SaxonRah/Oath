#include "DialogueTreeNode.h"
#include "PlayerSkillComponent.h"
#include "PlayerKnowledgeComponent.h"
#include "FactionComponent.h"
#include "DialogueManagerComponent.h"

FDialogueTreeNode::FDialogueTreeNode()
{
    // Set default values
}

bool FDialogueTreeNode::Link(FStateTreeLinker& Linker)
{
    Linker.LinkExternalData(SkillsHandle);
    Linker.LinkExternalData(KnowledgeHandle);
    Linker.LinkExternalData(FactionHandle);
    Linker.LinkExternalData(DialogueManagerHandle);
    return true;
}

EStateTreeRunStatus FDialogueTreeNode::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    UDialogueManagerComponent* DialogueManager = Context.GetExternalData(DialogueManagerHandle);
    
    if (!DialogueManager)
    {
        return EStateTreeRunStatus::Failed;
    }
    
    // Display NPC text
    DialogueManager->SetCurrentNPCText(NPCText);
    
    // Filter available dialogue options based on player status
    TArray<FDialogueOption> AvailableOptions;
    const UPlayerSkillComponent* Skills = Context.GetExternalData(SkillsHandle);
    const UPlayerKnowledgeComponent* Knowledge = Context.GetExternalData(KnowledgeHandle);
    const UFactionComponent* Faction = Context.GetExternalData(FactionHandle);
    
    for (const FDialogueOption& Option : DialogueOptions)
    {
        bool bOptionAvailable = true;
        
        // Check skill requirements
        if (!Option.RequiredSkill.IsNone() && Skills)
        {
            int32 PlayerSkillLevel = Skills->GetSkillLevel(Option.RequiredSkill);
            if (PlayerSkillLevel < Option.RequiredSkillLevel)
            {
                bOptionAvailable = false;
            }
        }
        
        // Check faction requirements
        if (!Option.RequiredFaction.IsNone() && Faction)
        {
            int32 PlayerReputation = Faction->GetReputationWithFaction(Option.RequiredFaction);
            if (PlayerReputation < Option.RequiredReputation)
            {
                bOptionAvailable = false;
            }
        }
        
        // Check knowledge requirements
        if (Knowledge)
        {
            for (const FName& RequiredKnowledge : Option.RequiredKnowledge)
            {
                if (!Knowledge->HasKnowledge(RequiredKnowledge))
                {
                    bOptionAvailable = false;
                    break;
                }
            }
        }
        
        if (bOptionAvailable)
        {
            AvailableOptions.Add(Option);
        }
    }
    
    // Set available options in dialogue manager
    DialogueManager->SetCurrentDialogueOptions(AvailableOptions);
    
    // Give player knowledge from this node
    if (Knowledge)
    {
        for (const FName& NewKnowledge : KnowledgeGained)
        {
            Knowledge->AddKnowledge(NewKnowledge);
        }
    }
    
    // Trigger quest if needed
    if (!QuestTriggered.IsNone())
    {
        DialogueManager->TriggerQuest(QuestTriggered);
    }
    
    return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FDialogueTreeNode::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
    UDialogueManagerComponent* DialogueManager = Context.GetExternalData(DialogueManagerHandle);
    
    if (!DialogueManager)
    {
        return EStateTreeRunStatus::Failed;
    }
    
    // Check if player has selected an option
    int32 SelectedOptionIndex = DialogueManager->GetSelectedOptionIndex();
    
    if (SelectedOptionIndex >= 0 && SelectedOptionIndex < DialogueOptions.Num())
    {
        // Apply reputation change if needed
        if (ReputationChangeOnSelect != 0)
        {
            UFactionComponent* Faction = Context.GetExternalData(FactionHandle);
            if (Faction)
            {
                Faction->ModifyCurrentNPCReputation(ReputationChangeOnSelect);
            }
        }
        
        // Transition to next dialogue node
        FName NextNodeID = DialogueOptions[SelectedOptionIndex].NextNodeID;
        DialogueManager->TransitionToDialogueNode(NextNodeID);
        
        return EStateTreeRunStatus::Succeeded;
    }
    
    return EStateTreeRunStatus::Running;
}