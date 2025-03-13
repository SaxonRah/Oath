#include "SkillTreeNode.h"
#include "PlayerSkillComponent.h"
#include "PlayerStatsComponent.h"
#include "PlayerAbilityComponent.h"

FSkillTreeNode::FSkillTreeNode()
{
    // Set default values
}

bool FSkillTreeNode::Link(FStateTreeLinker& Linker)
{
    Linker.LinkExternalData(SkillsHandle);
    Linker.LinkExternalData(StatsHandle);
    Linker.LinkExternalData(AbilitiesHandle);
    return true;
}

EStateTreeRunStatus FSkillTreeNode::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    const UPlayerSkillComponent* Skills = Context.GetExternalData(SkillsHandle);
    
    if (!Skills)
    {
        return EStateTreeRunStatus::Failed;
    }
    
    // Check current skill level
    int32 CurrentLevel = Skills->GetSkillLevel(SkillID);
    
    // If already at max level, no need to process further
    if (CurrentLevel >= MaxLevel)
    {
        return EStateTreeRunStatus::Succeeded;
    }
    
    return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FSkillTreeNode::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
    UPlayerSkillComponent* Skills = Context.GetExternalData(SkillsHandle);
    UPlayerStatsComponent* Stats = Context.GetExternalData(StatsHandle);
    UPlayerAbilityComponent* Abilities = Context.GetExternalData(AbilitiesHandle);
    
    if (!Skills || !Stats)
    {
        return EStateTreeRunStatus::Failed;
    }
    
    // Check if player is attempting to improve this skill
    if (Skills->IsAttemptingToImprove(SkillID))
    {
        int32 CurrentLevel = Skills->GetSkillLevel(SkillID);
        int32 AvailablePoints = Skills->GetAvailableSkillPoints();
        
        // Check if we can level up this skill
        bool bCanLevelUp = true;
        
        // Check character level requirement
        if (Stats->GetCharacterLevel() < CharacterLevelRequired)
        {
            bCanLevelUp = false;
        }
        
        // Check skill point cost
        if (AvailablePoints < SkillPointCost)
        {
            bCanLevelUp = false;
        }
        
        // Check prerequisites
        for (const FSkillPrerequisite& Prereq : Prerequisites)
        {
            int32 PrereqLevel = Skills->GetSkillLevel(Prereq.PrerequisiteSkill);
            if (PrereqLevel < Prereq.RequiredLevel)
            {
                bCanLevelUp = false;
                break;
            }
        }
        
        // Process skill improvement if possible
        if (bCanLevelUp)
        {
            // Increase skill level
            Skills->ImproveSkill(SkillID, SkillPointCost);
            
            // Apply skill effects
            for (const FSkillEffect& Effect : SkillEffects)
            {
                float ValueToAdd = Effect.ValuePerLevel;
                if (Effect.bIsPercentage)
                {
                    Stats->AddStatPercentageBonus(Effect.StatModified, ValueToAdd);
                }
                else
                {
                    Stats->AddStatBonus(Effect.StatModified, ValueToAdd);
                }
            }
            
            // Unlock abilities if reaching required levels
            if (Abilities)
            {
                int32 NewLevel = Skills->GetSkillLevel(SkillID);
                for (const FName& AbilityID : UnlockedAbilities)
                {
                    Abilities->UnlockAbility(AbilityID);
                }
            }
            
            // Notify about potential child skills becoming available
            for (const FName& ChildSkill : ChildSkills)
            {
                Skills->NotifySkillAvailable(ChildSkill);
            }
            
            return EStateTreeRunStatus::Succeeded;
        }
    }
    
    return EStateTreeRunStatus::Running;
}