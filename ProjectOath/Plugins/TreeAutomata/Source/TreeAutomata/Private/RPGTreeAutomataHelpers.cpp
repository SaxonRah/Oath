// RPGTreeAutomataHelpers.cpp
#include "RPGTreeAutomataHelpers.h"

bool URPGConditionEvaluator::EvaluateCondition_Implementation(const FString& Condition, UObject* Context)
{
    // Try to cast the context to our game state
    URPGGameState* GameState = Cast<URPGGameState>(Context);
    if (!GameState)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid game state provided for condition evaluation"));
        return false;
    }
    
    return ParseAndEvaluateCondition(Condition, GameState);
}

bool URPGConditionEvaluator::ParseAndEvaluateCondition(const FString& Condition, UObject* Context)
{
    URPGGameState* GameState = Cast<URPGGameState>(Context);
    if (!GameState)
    {
        return false;
    }
    
    // Parse condition string format: "Type:Parameter:Value"
    TArray<FString> Parts;
    Condition.ParseIntoArray(Parts, TEXT(":"), true);
    
    if (Parts.Num() < 2)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid condition format: %s"), *Condition);
        return false;
    }
    
    const FString& Type = Parts[0];
    
    // Handle different condition types
    if (Type == "HasItem")
    {
        return GameState->HasItem(Parts[1]);
    }
    else if (Type == "HasStat" && Parts.Num() >= 3)
    {
        return GameState->HasStat(Parts[1], FCString::Atoi(*Parts[2]));
    }
    else if (Type == "HasFlag")
    {
        return GameState->IsFlagSet(Parts[1]);
    }
    else if (Type == "HasReputation" && Parts.Num() >= 3)
    {
        return GameState->HasReputation(Parts[1], FCString::Atoi(*Parts[2]));
    }
    else if (Type == "HasLevel" && Parts.Num() >= 2)
    {
        return GameState->Level >= FCString::Atoi(*Parts[1]);
    }
    else if (Type == "HasSkillPoints" && Parts.Num() >= 2)
    {
        return GameState->SkillPoints >= FCString::Atoi(*Parts[1]);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Unknown condition type: %s"), *Type);
    return false;
}

void URPGActionPerformer::PerformAction_Implementation(const FString& Action, UObject* Context)
{
    URPGGameState* GameState = Cast<URPGGameState>(Context);
    if (!GameState)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid game state provided for action performance"));
        return;
    }
    
    ParseAndPerformAction(Action, GameState);
}

void URPGActionPerformer::ParseAndPerformAction(const FString& Action, UObject* Context)
{
    URPGGameState* GameState = Cast<URPGGameState>(Context);
    if (!GameState)
    {
        return;
    }
    
    // Parse action string format: "Type:Parameter:Value"
    TArray<FString> Parts;
    Action.ParseIntoArray(Parts, TEXT(":"), true);
    
    if (Parts.Num() < 2)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid action format: %s"), *Action);
        return;
    }
    
    const FString& Type = Parts[0];
    
    // Handle different action types
    if (Type == "GiveItem")
    {
        GameState->AddItem(Parts[1]);
    }
    else if (Type == "RemoveItem")
    {
        GameState->RemoveItem(Parts[1]);
    }
    else if (Type == "SetFlag" && Parts.Num() >= 3)
    {
        bool Value = Parts[2].ToBool();
        GameState->SetFlag(Parts[1], Value);
    }
    else if (Type == "ModifyStat" && Parts.Num() >= 3)
    {
        GameState->ModifyStat(Parts[1], FCString::Atoi(*Parts[2]));
    }
    else if (Type == "ModifyReputation" && Parts.Num() >= 3)
    {
        GameState->ModifyReputation(Parts[1], FCString::Atoi(*Parts[2]));
    }
    else if (Type == "GiveSkillPoints" && Parts.Num() >= 2)
    {
        GameState->SkillPoints += FCString::Atoi(*Parts[1]);
    }
    else if (Type == "GiveExperience" && Parts.Num() >= 2)
    {
        // Simple level up logic - would be more complex in a real game
        int32 ExpGained = FCString::Atoi(*Parts[1]);
        if (ExpGained > 100)  // Arbitrary threshold
        {
            GameState->Level += 1;
            GameState->SkillPoints += 1;
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Unknown action type: %s"), *Type);
    }
}

URPGGameState::URPGGameState()
    : Level(1)
    , SkillPoints(0)
{
    // Initialize default stats
    Stats.Add("Strength", 10);
    Stats.Add("Dexterity", 10);
    Stats.Add("Intelligence", 10);
    Stats.Add("Wisdom", 10);
    Stats.Add("Constitution", 10);
    Stats.Add("Charisma", 10);
    
    // Initialize empty inventory
    Inventory.Empty();
    
    // Initialize empty flags
    Flags.Empty();
    
    // Initialize neutral faction reputation
    Reputation.Add("Kingdom", 0);
    Reputation.Add("Rebels", 0);
    Reputation.Add("Merchants", 0);
}

bool URPGGameState::HasItem(const FString& ItemName) const
{
    return Inventory.Contains(ItemName);
}

bool URPGGameState::HasStat(const FString& StatName, int32 MinValue) const
{
    if (Stats.Contains(StatName))
    {
        return Stats[StatName] >= MinValue;
    }
    return false;
}

bool URPGGameState::IsFlagSet(const FString& FlagName) const
{
    if (Flags.Contains(FlagName))
    {
        return Flags[FlagName];
    }
    return false;
}

bool URPGGameState::HasReputation(const FString& Faction, int32 MinValue) const
{
    if (Reputation.Contains(Faction))
    {
        return Reputation[Faction] >= MinValue;
    }
    return false;
}

void URPGGameState::AddItem(const FString& ItemName)
{
   Inventory.Add(ItemName);
}

bool URPGGameState::RemoveItem(const FString& ItemName)
{
   return Inventory.Remove(ItemName) > 0;
}

void URPGGameState::SetFlag(const FString& FlagName, bool Value)
{
   Flags.Add(FlagName, Value);
}

void URPGGameState::ModifyStat(const FString& StatName, int32 Delta)
{
   if (Stats.Contains(StatName))
   {
       Stats[StatName] += Delta;
   }
   else
   {
       Stats.Add(StatName, Delta);
   }
}

void URPGGameState::ModifyReputation(const FString& Faction, int32 Delta)
{
   if (Reputation.Contains(Faction))
   {
       Reputation[Faction] += Delta;
   }
   else
   {
       Reputation.Add(Faction, Delta);
   }
}