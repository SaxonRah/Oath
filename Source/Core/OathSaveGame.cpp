// OathSaveGame.cpp
#include "OathSaveGame.h"

UOathSaveGame::UOathSaveGame()
{
    // Initialize with default values
    PlayerLocation = FVector::ZeroVector;
    PlayerRotation = FRotator::ZeroRotator;
    CharacterClass = ECharacterClass::Warrior;
    KingdomVision = TEXT("Military Stronghold");
    
    CombatRenown = 0.0f;
    QuestRenown = 0.0f;
    KingdomReputation = 0.0f;
    
    Gold = 0;
    
    KingdomName = TEXT("New Kingdom");
    CurrentTier = EKingdomTier::Camp;
    KingdomAlignment = 0.0f;
    TaxRate = 10;
    DailyIncome = 0.0f;
    
    CurrentDayTime = 0.0f;
    CurrentDay = 1;
    CurrentSeason = 0;
}