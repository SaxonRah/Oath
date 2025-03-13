#include "PlayerStatsComponent.h"

UPlayerStatsComponent::UPlayerStatsComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerStatsComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize with default stats
    BaseStats.Add("Health", 100.0f);
    BaseStats.Add("Stamina", 100.0f);
    BaseStats.Add("Mana", 50.0f);
    BaseStats.Add("PhysicalDamage", 10.0f);
    BaseStats.Add("MagicDamage", 5.0f);
}

int32 UPlayerStatsComponent::GetCharacterLevel() const
{
    return CharacterLevel;
}

void UPlayerStatsComponent::SetCharacterLevel(int32 NewLevel)
{
    if (NewLevel > 0)
    {
        CharacterLevel = NewLevel;
    }
}

float UPlayerStatsComponent::GetStatValue(FName StatName) const
{
    float BaseValue = 0.0f;
    if (const float* Value = BaseStats.Find(StatName))
    {
        BaseValue = *Value;
    }
    
    float FlatBonus = 0.0f;
    if (const float* Bonus = StatBonuses.Find(StatName))
    {
        FlatBonus = *Bonus;
    }
    
    float PercentBonus = 0.0f;
    if (const float* Percent = StatPercentageBonuses.Find(StatName))
    {
        PercentBonus = *Percent;
    }
    
    // Apply flat bonus first, then percentage modifier
    return (BaseValue + FlatBonus) * (1.0f + PercentBonus / 100.0f);
}

void UPlayerStatsComponent::SetStatValue(FName StatName, float Value)
{
    if (Value >= 0.0f)
    {
        BaseStats.Add(StatName, Value);
    }
}

void UPlayerStatsComponent::AddStatBonus(FName StatName, float BonusValue)
{
    float CurrentBonus = 0.0f;
    if (const float* ExistingBonus = StatBonuses.Find(StatName))
    {
        CurrentBonus = *ExistingBonus;
    }
    
    StatBonuses.Add(StatName, CurrentBonus + BonusValue);
}

void UPlayerStatsComponent::AddStatPercentageBonus(FName StatName, float PercentBonus)
{
    float CurrentPercent = 0.0f;
    if (const float* ExistingPercent = StatPercentageBonuses.Find(StatName))
    {
        CurrentPercent = *ExistingPercent;
    }
    
    StatPercentageBonuses.Add(StatName, CurrentPercent + PercentBonus);
}