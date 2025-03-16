// OathSaveGame.h (continued)
UPROPERTY(VisibleAnywhere, Category = "Characters")
ECharacterClass CharacterClass;

UPROPERTY(VisibleAnywhere, Category = "Characters")
FString KingdomVision;

// Reputation data
UPROPERTY(VisibleAnywhere, Category = "Reputation")
float CombatRenown;

UPROPERTY(VisibleAnywhere, Category = "Reputation")
float QuestRenown;

UPROPERTY(VisibleAnywhere, Category = "Reputation")
float KingdomReputation;

UPROPERTY(VisibleAnywhere, Category = "Reputation")
TMap<FString, float> FactionReputation;

// Inventory data
UPROPERTY(VisibleAnywhere, Category = "Inventory")
int32 Gold;

UPROPERTY(VisibleAnywhere, Category = "Inventory")
TMap<FResourceData, int32> Materials;

UPROPERTY(VisibleAnywhere, Category = "Inventory")
TArray<FLootItem> Items;

// Kingdom data
UPROPERTY(VisibleAnywhere, Category = "Kingdom")
FString KingdomName;

UPROPERTY(VisibleAnywhere, Category = "Kingdom")
EKingdomTier CurrentTier;

UPROPERTY(VisibleAnywhere, Category = "Kingdom")
float KingdomAlignment;

UPROPERTY(VisibleAnywhere, Category = "Kingdom")
TArray<FFollowerData> Followers;

UPROPERTY(VisibleAnywhere, Category = "Kingdom")
TArray<FBuildingData> Buildings;

UPROPERTY(VisibleAnywhere, Category = "Kingdom")
int32 TaxRate;

UPROPERTY(VisibleAnywhere, Category = "Kingdom")
float DailyIncome;

// Faction data
UPROPERTY(VisibleAnywhere, Category = "Faction")
TArray<FString> AvailableFactions;

UPROPERTY(VisibleAnywhere, Category = "Faction")
TArray<FString> FactionUnlocks;

// World data
UPROPERTY(VisibleAnywhere, Category = "World")
float CurrentDayTime;

UPROPERTY(VisibleAnywhere, Category = "World")
int32 CurrentDay;

UPROPERTY(VisibleAnywhere, Category = "World")
int32 CurrentSeason;
};