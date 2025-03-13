#include "PlayerSkillComponent.h"

UPlayerSkillComponent::UPlayerSkillComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerSkillComponent::BeginPlay()
{
    Super::BeginPlay();
}

int32 UPlayerSkillComponent::GetSkillLevel(FName SkillID) const
{
    if (const FSkillData* SkillData = Skills.Find(SkillID))
    {
        return SkillData->CurrentLevel;
    }
    return 0;
}

bool UPlayerSkillComponent::IsSkillUnlocked(FName SkillID) const
{
    if (const FSkillData* SkillData = Skills.Find(SkillID))
    {
        return SkillData->bIsUnlocked;
    }
    return false;
}

void UPlayerSkillComponent::AddExperience(FName SkillID, float ExperienceAmount)
{
    if (!IsSkillUnlocked(SkillID))
    {
        return;
    }
    
    FSkillData& SkillData = Skills.FindOrAdd(SkillID);
    SkillData.CurrentExperience += ExperienceAmount;
    
    // Check for level up
    while (SkillData.CurrentExperience >= SkillData.ExperienceToNextLevel)
    {
        SkillData.CurrentExperience -= SkillData.ExperienceToNextLevel;
        SkillData.CurrentLevel++;
        SkillData.ExperienceToNextLevel = GetExperienceRequiredForLevel(SkillID, SkillData.CurrentLevel + 1);
        
        // Broadcast level up event
        OnSkillLevelChanged.Broadcast(SkillID, SkillData.CurrentLevel);
    }
}

void UPlayerSkillComponent::UnlockSkill(FName SkillID)
{
    FSkillData& SkillData = Skills.FindOrAdd(SkillID);
    
    if (!SkillData.bIsUnlocked)
    {
        SkillData.bIsUnlocked = true;
        SkillData.CurrentLevel = 1;
        SkillData.CurrentExperience = 0.0f;
        SkillData.ExperienceToNextLevel = GetExperienceRequiredForLevel(SkillID, 2);
        
        // Broadcast unlock event
        OnSkillUnlocked.Broadcast(SkillID);
        OnSkillLevelChanged.Broadcast(SkillID, 1);
    }
}

int32 UPlayerSkillComponent::GetAvailableSkillPoints() const
{
    return AvailableSkillPoints;
}

void UPlayerSkillComponent::AddSkillPoints(int32 Points)
{
    AvailableSkillPoints += Points;
}

bool UPlayerSkillComponent::ImproveSkill(FName SkillID, int32 PointCost)
{
    if (!IsSkillUnlocked(SkillID) || AvailableSkillPoints < PointCost)
    {
        return false;
    }
    
    FSkillData& SkillData = Skills.FindOrAdd(SkillID);
    SkillData.CurrentLevel++;
    AvailableSkillPoints -= PointCost;
    
    // Broadcast level up event
    OnSkillLevelChanged.Broadcast(SkillID, SkillData.CurrentLevel);
    
    return true;
}

void UPlayerSkillComponent::SetAttemptingToImprove(FName SkillID, bool bIsAttempting)
{
    if (bIsAttempting)
    {
        SkillBeingImproved = SkillID;
    }
    else if (SkillBeingImproved == SkillID)
    {
        SkillBeingImproved = NAME_None;
    }
}

bool UPlayerSkillComponent::IsAttemptingToImprove(FName SkillID) const
{
    return SkillBeingImproved == SkillID;
}

void UPlayerSkillComponent::NotifySkillAvailable(FName SkillID)
{
    // Add the skill to the map if it doesn't exist
    if (!Skills.Contains(SkillID))
    {
        FSkillData NewSkill;
        NewSkill.bIsUnlocked = false;
        Skills.Add(SkillID, NewSkill);
    }
}

float UPlayerSkillComponent::GetExperienceRequiredForLevel(FName SkillID, int32 Level) const
{
    // Simple exponential growth formula
    float BaseXP = 100.0f;
    float ScaleFactor = 1.5f;
    
    return BaseXP * FMath::Pow(ScaleFactor, Level - 1);
}