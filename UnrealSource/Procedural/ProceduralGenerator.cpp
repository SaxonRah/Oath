// ProceduralGenerator.cpp
#include "ProceduralGenerator.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

UProceduralGenerator::UProceduralGenerator()
{
    // Initialize name pools
    FollowerNamePool = {
        "Aldric", "Brenna", "Caius", "Dalia", "Eamon", "Farrah",
        "Gareth", "Hilda", "Isran", "Jora", "Keelan", "Lyra",
        "Magnus", "Nessa", "Orrin", "Pera", "Quincy", "Rowena",
        "Silas", "Thea", "Uriel", "Vessa", "Weston", "Xandra",
        "Yorick", "Zara"
    };
    
    QuestTemplates = {
        "Retrieve the {item} from {location}",
        "Defeat the {monster} plaguing {location}",
        "Escort {person} to {location} safely",
        "Investigate the strange occurrences at {location}",
        "Build a {building} to improve {location}",
        "Negotiate peace between {faction1} and {faction2}",
        "Solve the mystery of the missing {item} from {location}"
    };
    
    ResourceNamePool = {
        "Iron", "Wood", "Stone", "Gold Ore", "Crystal", "Leather",
        "Cloth", "Herbs", "Bones", "Scales", "Gemstones", "Steel",
        "Mithril", "Dragonhide", "Stardust", "Ancient Relics", "Ectoplasm"
    };
    
    ItemNamePool = {
        "Sword", "Shield", "Bow", "Staff", "Amulet", "Ring",
        "Helmet", "Chestplate", "Boots", "Gauntlets", "Potion", "Scroll",
        "Wand", "Dagger", "Axe", "Spear", "Talisman", "Orb"
    };
    
    TraitPool = {
        "Brave", "Cautious", "Clever", "Determined", "Efficient", "Friendly",
        "Generous", "Hardworking", "Ingenious", "Jovial", "Kind", "Loyal",
        "Meticulous", "Noble", "Optimistic", "Patient", "Quick-witted", "Resourceful",
        "Strong", "Tactful", "Understanding", "Vigilant", "Wise", "Zealous"
    };
}

AActor* UProceduralGenerator::GenerateNotoriousMonsterEncounter(int32 Tier, FString Difficulty)
{
    // Get the world
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }
    
    // Find a suitable spawn location
    FVector SpawnLocation = FVector::ZeroVector;
    AActor* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (PlayerPawn)
    {
        // Find a location some distance from the player
        float SpawnDistance = FMath::RandRange(1000.0f, 2000.0f);
        float SpawnAngle = FMath::RandRange(0.0f, 360.0f);
        
        FVector Offset = FVector(
            FMath::Cos(FMath::DegreesToRadians(SpawnAngle)) * SpawnDistance,
            FMath::Sin(FMath::DegreesToRadians(SpawnAngle)) * SpawnDistance,
            0.0f
        );
        
        SpawnLocation = PlayerPawn->GetActorLocation() + Offset;
        
        // Adjust Z to ground level
        FHitResult HitResult;
        FVector TraceStart = SpawnLocation + FVector(0, 0, 1000);
        FVector TraceEnd = SpawnLocation - FVector(0, 0, 1000);
        
        if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility))
        {
            SpawnLocation = HitResult.Location + FVector(0, 0, 100); // Offset slightly above ground
        }
    }
    
    // Determine which monster to spawn based on tier and difficulty
    FString MonsterType;
    if (Difficulty == "Legendary")
    {
        TArray<FString> LegendaryMonsters = {
            "BP_Dragon", "BP_AncientGolem", "BP_EldritchHorror",
            "BP_PhoenixLord", "BP_ShadowTitan"
        };
        MonsterType = LegendaryMonsters[FMath::RandRange(0, LegendaryMonsters.Num() - 1)];
    }
    else if (Difficulty == "Elite")
    {
        TArray<FString> EliteMonsters = {
            "BP_Cyclops", "BP_FrostGiant", "BP_Chimera",
            "BP_DemonKnight", "BP_ShadowStalker", "BP_StormElemental"
        };
        MonsterType = EliteMonsters[FMath::RandRange(0, EliteMonsters.Num() - 1)];
    }
    else // Standard
    {
        TArray<FString> StandardMonsters = {
            "BP_OgreChief", "BP_VeteranTroll", "BP_BanditLord",
            "BP_DireWolf", "BP_GiantSpider", "BP_SkeletonWarlord"
        };
        MonsterType = StandardMonsters[FMath::RandRange(0, StandardMonsters.Num() - 1)];
    }
    
    // Construct full path for the blueprint
    FString BlueprintPath = FString::Printf(TEXT("/Game/Blueprints/Monsters/%s.%s_C"), *MonsterType, *MonsterType);
    
    // Try to load the blueprint class
    UClass* MonsterClass = LoadObject<UClass>(nullptr, *BlueprintPath);
    if (!MonsterClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to load monster blueprint: %s"), *BlueprintPath);
        return nullptr;
    }
    
    // Spawn the monster
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    AActor* SpawnedMonster = World->SpawnActor<AActor>(MonsterClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
    if (SpawnedMonster)
    {
        // Set monster properties based on tier
        // This would typically be implemented on the monster blueprint side,
        // but we can set some basic properties here too
        
        // Notify player about the notorious monster
        APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
        if (PC)
        {
            AOathPlayerController* OathPC = Cast<AOathPlayerController>(PC);
            if (OathPC)
            {
                FString MonsterName = MonsterType.RightChop(3); // Remove the "BP_" prefix
                OathPC->ShowNotification(
                    FString::Printf(TEXT("Notorious %s Appeared"), *MonsterName),
                    FString::Printf(TEXT("A %s %s has been spotted nearby. Defeat it for great rewards!"), 
                                   *Difficulty, *MonsterName),
                    15.0f
                );
            }
        }
    }
    
    return SpawnedMonster;
}

void UProceduralGenerator::GenerateSpecialQuestChain(FString ChainType, int32 QuestCount, int32 BaseDifficulty)
{
    // Get the world
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Determine quest chain theme
    FString ChainTheme;
    TArray<EQuestType> QuestTypes;
    
    if (ChainType == "Kingdom")
    {
        TArray<FString> KingdomThemes = {
            "Ancient Prophecy", "Royal Bloodline", "Forbidden Magic",
            "Lost Kingdom", "Divine Blessing", "Legendary Artifact"
        };
        ChainTheme = KingdomThemes[FMath::RandRange(0, KingdomThemes.Num() - 1)];
        
        QuestTypes = { EQuestType::Explore, EQuestType::Kill, EQuestType::Mystery };
    }
    else if (ChainType == "Faction")
    {
        TArray<FString> FactionThemes = {
            "Internal Conflict", "Ancient Rivalry", "Lost Knowledge",
            "Sacred Ritual", "Corruption Within", "Rising Threat"
        };
        ChainTheme = FactionThemes[FMath::RandRange(0, FactionThemes.Num() - 1)];
        
        QuestTypes = { EQuestType::Diplomatic, EQuestType::Fetch, EQuestType::Kill };
    }
    else
    {
        TArray<FString> StandardThemes = {
            "Missing Person", "Monster Infestation", "Bandit Troubles",
            "Resource Shortage", "Natural Disaster", "Mysterious Illness"
        };
        ChainTheme = StandardThemes[FMath::RandRange(0, StandardThemes.Num() - 1)];
        
        QuestTypes = { EQuestType::Fetch, EQuestType::Kill, EQuestType::Escort };
    }
    
    // Create quest chain structure
    TArray<UQuest*> QuestChain;
    
    // Randomly select a faction for the quest chain
    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    AOathCharacter* PlayerCharacter = nullptr;
    FString SelectedFaction;
    
    if (PC)
    {
        PlayerCharacter = Cast<AOathCharacter>(PC->GetPawn());
        if (PlayerCharacter && PlayerCharacter->ReputationComponent)
        {
            // Select from factions where the player has positive reputation
            TArray<FString> EligibleFactions;
            for (const auto& Pair : PlayerCharacter->ReputationComponent->FactionReputation)
            {
                if (Pair.Value >= 0)
                {
                    EligibleFactions.Add(Pair.Key);
                }
            }
            
            if (EligibleFactions.Num() > 0)
            {
                SelectedFaction = EligibleFactions[FMath::RandRange(0, EligibleFactions.Num() - 1)];
            }
            else
            {
                // Fallback to a random faction
                UOathGameInstance* GameInstance = Cast<UOathGameInstance>(UGameplayStatics::GetGameInstance(World));
                if (GameInstance)
                {
                    TArray<FString> AllFactions = GameInstance->GetAllFactions();
                    if (AllFactions.Num() > 0)
                    {
                        SelectedFaction = AllFactions[FMath::RandRange(0, AllFactions.Num() - 1)];
                    }
                    else
                    {
                        SelectedFaction = "Hunters Guild"; // Default fallback
                    }
                }
            }
        }
    }
    
    // Generate the quest chain
    for (int32 i = 0; i < QuestCount; ++i)
    {
        // Alternate quest types from the available ones
        EQuestType QuestType = QuestTypes[i % QuestTypes.Num()];
        
        // Increase difficulty as chain progresses
        int32 QuestDifficulty = BaseDifficulty + i;
        
        // Generate the quest
        UQuest* NewQuest = GenerateQuest(QuestDifficulty, SelectedFaction, QuestType);
        if (NewQuest)
        {
            // Modify quest to fit the chain theme
            NewQuest->QuestName = FString::Printf(TEXT("%s: Part %d"), *ChainTheme, i + 1);
            
            // Only the first quest is available initially
            if (i == 0)
            {
                NewQuest->Status = EQuestStatus::Available;
            }
            else
            {
                NewQuest->Status = EQuestStatus::Available; // We'll handle availability through game logic
                
                // Store reference to previous quest
                NewQuest->PrerequisiteQuestName = QuestChain[i - 1]->QuestName;
            }
            
            // Add to chain
            QuestChain.Add(NewQuest);
            
            // Notify player about the first quest
            if (i == 0 && PC)
            {
                AOathPlayerController* OathPC = Cast<AOathPlayerController>(PC);
                if (OathPC)
                {
                    OathPC->ShowNotification(
                        FString::Printf(TEXT("Special Quest Chain: %s"), *ChainTheme),
                        FString::Printf(TEXT("A special quest chain has become available from the %s."), *SelectedFaction),
                        15.0f
                    );
                    
                    OathPC->UpdateQuestLog(NewQuest, true, false);
                }
            }
        }
    }
    
    // Store quest chain in game instance for tracking
    UOathGameInstance* GameInstance = Cast<UOathGameInstance>(UGameplayStatics::GetGameInstance(World));
    if (GameInstance)
    {
        GameInstance->RegisterQuestChain(QuestChain);
    }
}

UQuest* UProceduralGenerator::GenerateSpecialQuest(int32 Difficulty)
{
    // Select a random quest type
    TArray<EQuestType> QuestTypes = {
        EQuestType::Fetch, EQuestType::Kill, EQuestType::Escort,
        EQuestType::Explore, EQuestType::Build, EQuestType::Diplomatic,
        EQuestType::Mystery
    };
    EQuestType QuestType = QuestTypes[FMath::RandRange(0, QuestTypes.Num() - 1)];
    
    // Select a random faction
    UOathGameInstance* GameInstance = Cast<UOathGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
    FString SelectedFaction;
    
    if (GameInstance)
    {
        TArray<FString> AllFactions = GameInstance->GetAllFactions();
        if (AllFactions.Num() > 0)
        {
            SelectedFaction = AllFactions[FMath::RandRange(0, AllFactions.Num() - 1)];
        }
        else
        {
            SelectedFaction = "Hunters Guild"; // Default fallback
        }
    }
    
    // Generate the quest
    UQuest* NewQuest = GenerateQuest(Difficulty, SelectedFaction, QuestType);
    if (NewQuest)
    {
        // Make it special
        NewQuest->QuestName = FString::Printf(TEXT("Special: %s"), *NewQuest->QuestName);
        NewQuest->QuestRenownReward *= 1.5f; // 50% bonus
        NewQuest->GoldReward *= 2; // Double gold
        
        // Add special objective
        FQuestObjective BonusObjective;
        BonusObjective.Description = "Complete the quest with exceptional performance";
        BonusObjective.CurrentProgress = 0;
        BonusObjective.RequiredProgress = 1;
        BonusObjective.bIsCompleted = false;
        
        NewQuest->Objectives.Add(BonusObjective);
        
        // Notify player
        APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (PC)
        {
            AOathPlayerController* OathPC = Cast<AOathPlayerController>(PC);
            if (OathPC)
            {
                OathPC->ShowNotification(
                    "Special Quest Available",
                    FString::Printf(TEXT("A special quest has become available from the %s."), *SelectedFaction),
                    10.0f
                );
                
                OathPC->UpdateQuestLog(NewQuest, true, false);
            }
        }
    }
    
    return NewQuest;
}

UQuest* UProceduralGenerator::GenerateQuest(int32 DifficultyLevel, FString FactionName, EQuestType PreferredType)
{
    // Create a new quest object
    UQuest* NewQuest = NewObject<UQuest>(this);
    if (!NewQuest)
    {
        return nullptr;
    }
    
    // Set basic properties
    NewQuest->DifficultyLevel = DifficultyLevel;
    NewQuest->Type = PreferredType;
    NewQuest->Status = EQuestStatus::Available;
    NewQuest->FactionName = FactionName;
    
    // Generate quest details based on type
    switch (PreferredType)
    {
        case EQuestType::Fetch:
            GenerateFetchQuest(NewQuest);
            break;
        case EQuestType::Kill:
            GenerateKillQuest(NewQuest);
            break;
        case EQuestType::Escort:
            GenerateEscortQuest(NewQuest);
            break;
        case EQuestType::Explore:
            GenerateExploreQuest(NewQuest);
            break;
        case EQuestType::Build:
            GenerateBuildQuest(NewQuest);
            break;
        case EQuestType::Diplomatic:
            GenerateDiplomaticQuest(NewQuest);
            break;
        case EQuestType::Mystery:
            GenerateMysteryQuest(NewQuest);
            break;
        default:
            // Default to fetch quest if something goes wrong
            GenerateFetchQuest(NewQuest);
            break;
    }
    
    // Scale rewards based on difficulty
    NewQuest->QuestRenownReward = 10.0f * DifficultyLevel;
    NewQuest->GoldReward = 50 * DifficultyLevel;
    NewQuest->FactionReputationReward = 5.0f * DifficultyLevel;
    
    // Add material rewards
    for (int32 i = 0; i < FMath::Max(1, DifficultyLevel / 2); ++i)
    {
        FResourceData Resource = GenerateResource(
            DifficultyLevel <= 2 ? EResourceRarity::Common : EResourceRarity::Uncommon,
            DifficultyLevel >= 5 ? EResourceRarity::Legendary : EResourceRarity::Rare
        );
        
        int32 Amount = FMath::RandRange(1, 3) * DifficultyLevel;
        NewQuest->MaterialRewards.Add(Resource, Amount);
    }
    
    // Add item rewards for higher difficulty quests
    if (DifficultyLevel >= 3)
    {
        for (int32 i = 0; i < FMath::Min(3, DifficultyLevel / 2); ++i)
        {
            FLootItem Item = GenerateLoot(DifficultyLevel, 1.0f);
            NewQuest->ItemRewards.Add(Item);
        }
    }
    
    // Determine if this quest can be assigned to followers
    NewQuest->bCanBeAssignedToFollower = (DifficultyLevel <= 3);
    
    return NewQuest;
}


UQuest* UProceduralGenerator::GenerateQuest2(int32 DifficultyLevel, FString FactionName, EQuestType PreferredType)
{
    UQuest* NewQuest = NewObject<UQuest>();
    
    // Set basic properties
    NewQuest->DifficultyLevel = FMath::Clamp(DifficultyLevel, 1, 5);
    NewQuest->FactionName = FactionName;
    NewQuest->Type = PreferredType;
    NewQuest->Status = EQuestStatus::Available;
    
    // Set rewards based on difficulty
    NewQuest->QuestRenownReward = 10.0f * DifficultyLevel;
    NewQuest->GoldReward = 50 * DifficultyLevel;
    NewQuest->FactionReputationReward = 5.0f * DifficultyLevel;
    
    // Determine if quest can be assigned to followers
    // Higher difficulty quests are less likely to be assignable
    NewQuest->bCanBeAssignedToFollower = (FMath::RandRange(1, 5) > NewQuest->DifficultyLevel);
    
    // Generate quest name and description based on type
    switch (PreferredType)
    {
        case EQuestType::Fetch:
            GenerateFetchQuest(NewQuest);
            break;
        case EQuestType::Kill:
            GenerateKillQuest(NewQuest);
            break;
        case EQuestType::Escort:
            GenerateEscortQuest(NewQuest);
            break;
        case EQuestType::Explore:
            GenerateExploreQuest(NewQuest);
            break;
        case EQuestType::Build:
            GenerateBuildQuest(NewQuest);
            break;
        case EQuestType::Diplomatic:
            GenerateDiplomaticQuest(NewQuest);
            break;
        case EQuestType::Mystery:
            GenerateMysteryQuest(NewQuest);
            break;
        default:
            // Generate a random type if preferred type is invalid
            int32 RandomType = FMath::RandRange(0, static_cast<int32>(EQuestType::Mystery));
            return GenerateQuest(DifficultyLevel, FactionName, static_cast<EQuestType>(RandomType));
    }
    
    // Generate material rewards
    int32 NumMaterialRewards = FMath::RandRange(1, 2 + DifficultyLevel / 2);
    for (int32 i = 0; i < NumMaterialRewards; ++i)
    {
        EResourceRarity MinRarity = EResourceRarity::Common;
        EResourceRarity MaxRarity = static_cast<EResourceRarity>(FMath::Min(static_cast<int32>(EResourceRarity::Legendary), DifficultyLevel));
        
        FResourceData Material = GenerateResource(MinRarity, MaxRarity);
        int32 Amount = FMath::RandRange(1, 3) * DifficultyLevel;
        
        NewQuest->MaterialRewards.Add(Material, Amount);
    }

    // Generate item rewards (higher chance for better items with higher difficulty)
   int32 NumItemRewards = FMath::RandRange(0, 1 + DifficultyLevel / 2);
   for (int32 i = 0; i < NumItemRewards; ++i)
   {
       float RarityModifier = 0.2f * DifficultyLevel;
       FLootItem Item = GenerateLoot(DifficultyLevel, RarityModifier);
       NewQuest->ItemRewards.Add(Item);
   }
   
   return NewQuest;
}

void UProceduralGenerator::GenerateFetchQuest(UQuest* Quest)
{
   // Generate a random resource to fetch
   EResourceRarity MinRarity = EResourceRarity::Common;
   EResourceRarity MaxRarity = static_cast<EResourceRarity>(FMath::Min(static_cast<int32>(EResourceRarity::Legendary), Quest->DifficultyLevel));
   
   FResourceData Resource = GenerateResource(MinRarity, MaxRarity);
   int32 Amount = FMath::RandRange(3, 5) * Quest->DifficultyLevel;
   
   // Create quest name and description
   TArray<FString> FetchVerbs = {"Gather", "Collect", "Acquire", "Obtain", "Procure", "Harvest"};
   int32 VerbIndex = FMath::RandRange(0, FetchVerbs.Num() - 1);
   
   Quest->QuestName = FString::Printf(TEXT("%s %s"), *FetchVerbs[VerbIndex], *Resource.Name);
   
   TArray<FString> ReasonTemplates = {
       "They are needed for an important research project.",
       "They are crucial ingredients for a special potion.",
       "They are required to repair damaged equipment.",
       "They are needed to strengthen our defenses.",
       "A rare ceremony requires these materials.",
       "Trade negotiations depend on acquiring these goods."
   };
   
   int32 ReasonIndex = FMath::RandRange(0, ReasonTemplates.Num() - 1);
   
   Quest->Description = FString::Printf(TEXT("I need you to %s %d %s. %s"), 
       *FetchVerbs[VerbIndex].ToLower(), 
       Amount, 
       *Resource.Name, 
       *ReasonTemplates[ReasonIndex]);
   
   // Create an objective
   FQuestObjective Objective;
   Objective.Description = FString::Printf(TEXT("%s %d %s"), *FetchVerbs[VerbIndex], Amount, *Resource.Name);
   Objective.CurrentProgress = 0;
   Objective.RequiredProgress = Amount;
   Objective.bIsCompleted = false;
   
   Quest->Objectives.Add(Objective);
}

void UProceduralGenerator::GenerateKillQuest(UQuest* Quest)
{
   // Generate enemy types based on difficulty
   TArray<FString> EnemyTypes;
   
   if (Quest->DifficultyLevel <= 2)
   {
       EnemyTypes = {"Wolves", "Bandits", "Goblins", "Spiders", "Skeletons"};
   }
   else if (Quest->DifficultyLevel <= 4)
   {
       EnemyTypes = {"Trolls", "Dark Knights", "Cultists", "Wyverns", "Wendigos"};
   }
   else
   {
       EnemyTypes = {"Demons", "Dragons", "Ancient Guardians", "Shadow Beasts", "Eldritch Horrors"};
   }
   
   FString EnemyType = EnemyTypes[FMath::RandRange(0, EnemyTypes.Num() - 1)];
   int32 Amount = FMath::RandRange(5, 10) + (Quest->DifficultyLevel * 2);
   
   // Create quest name and description
   TArray<FString> KillVerbs = {"Hunt", "Slay", "Defeat", "Eliminate", "Exterminate", "Destroy"};
   int32 VerbIndex = FMath::RandRange(0, KillVerbs.Num() - 1);
   
   Quest->QuestName = FString::Printf(TEXT("%s the %s"), *KillVerbs[VerbIndex], *EnemyType);
   
   TArray<FString> ReasonTemplates = {
       "They've been terrorizing travelers in the region.",
       "They're threatening our supply lines.",
       "They've taken up residence in an important location.",
       "They carry a plague that must be stopped.",
       "They serve a dark power that must be weakened.",
       "They're disrupting the natural balance."
   };
   
   int32 ReasonIndex = FMath::RandRange(0, ReasonTemplates.Num() - 1);
   
   Quest->Description = FString::Printf(TEXT("I need you to %s %d %s. %s"), 
       *KillVerbs[VerbIndex].ToLower(), 
       Amount, 
       *EnemyType, 
       *ReasonTemplates[ReasonIndex]);
   
   // Create an objective
   FQuestObjective Objective;
   Objective.Description = FString::Printf(TEXT("%s %d %s"), *KillVerbs[VerbIndex], Amount, *EnemyType);
   Objective.CurrentProgress = 0;
   Objective.RequiredProgress = Amount;
   Objective.bIsCompleted = false;
   
   Quest->Objectives.Add(Objective);
}

void UProceduralGenerator::GenerateEscortQuest(UQuest* Quest)
{
   // Generate NPC types based on difficulty
   TArray<FString> NPCTypes;
   
   if (Quest->DifficultyLevel <= 2)
   {
       NPCTypes = {"Merchant", "Scholar", "Pilgrim", "Refugee", "Messenger"};
   }
   else if (Quest->DifficultyLevel <= 4)
   {
       NPCTypes = {"Noble", "Diplomat", "Priest", "Knight", "Spellcaster"};
   }
   else
   {
       NPCTypes = {"High Priest", "Royal Emissary", "Archmage", "Legendary Hero", "Oracle"};
   }
   
   FString NPCType = NPCTypes[FMath::RandRange(0, NPCTypes.Num() - 1)];
   FString NPCName = GenerateName(true);
   
   // Generate destination
   TArray<FString> Destinations = {
       "the Crystal Caverns", "Shadowvale", "the Ancient Temple", 
       "the Mountain Pass", "the Coastal Fort", "the Enchanted Grove",
       "the Capital City", "the Border Outpost", "the Hidden Valley"
   };
   
   FString Destination = Destinations[FMath::RandRange(0, Destinations.Num() - 1)];
   
   // Create quest name and description
   Quest->QuestName = FString::Printf(TEXT("Escort the %s to %s"), *NPCType, *Destination);
   
   TArray<FString> ReasonTemplates = {
       "The journey is dangerous, but their mission is vital.",
       "There have been reports of ambushes along the way.",
       "They carry important information that must reach its destination.",
       "Their presence will strengthen our alliance.",
       "They possess knowledge that could change everything.",
       "Their safety is paramount to our future."
   };
   
   int32 ReasonIndex = FMath::RandRange(0, ReasonTemplates.Num() - 1);
   
   Quest->Description = FString::Printf(TEXT("I need you to escort %s the %s safely to %s. %s"), 
       *NPCName, 
       *NPCType, 
       *Destination, 
       *ReasonTemplates[ReasonIndex]);
   
   // Create multiple objectives
   FQuestObjective Objective1;
   Objective1.Description = FString::Printf(TEXT("Meet %s the %s"), *NPCName, *NPCType);
   Objective1.CurrentProgress = 0;
   Objective1.RequiredProgress = 1;
   Objective1.bIsCompleted = false;
   
   FQuestObjective Objective2;
   Objective2.Description = FString::Printf(TEXT("Escort %s to %s"), *NPCName, *Destination);
   Objective2.CurrentProgress = 0;
   Objective2.RequiredProgress = 1;
   Objective2.bIsCompleted = false;
   
   // Optionally add combat objective based on difficulty
   if (Quest->DifficultyLevel >= 3)
   {
       TArray<FString> EnemyTypes = {"Bandits", "Mercenaries", "Assassins", "Wild Beasts", "Cultists"};
       FString EnemyType = EnemyTypes[FMath::RandRange(0, EnemyTypes.Num() - 1)];
       int32 Amount = FMath::RandRange(3, 5) + Quest->DifficultyLevel;
       
       FQuestObjective Objective3;
       Objective3.Description = FString::Printf(TEXT("Defeat %s attacking the escort (%d)"), *EnemyType, Amount);
       Objective3.CurrentProgress = 0;
       Objective3.RequiredProgress = Amount;
       Objective3.bIsCompleted = false;
       
       Quest->Objectives.Add(Objective1);
       Quest->Objectives.Add(Objective3);
       Quest->Objectives.Add(Objective2);
   }
   else
   {
       Quest->Objectives.Add(Objective1);
       Quest->Objectives.Add(Objective2);
   }
}

void UProceduralGenerator::GenerateExploreQuest(UQuest* Quest)
{
   // Generate location types based on difficulty
   TArray<FString> LocationTypes;
   
   if (Quest->DifficultyLevel <= 2)
   {
       LocationTypes = {"Abandoned Mine", "Ancient Ruins", "Hidden Cave", "Forgotten Shrine", "Bandit Camp"};
   }
   else if (Quest->DifficultyLevel <= 4)
   {
       LocationTypes = {"Lost Temple", "Sunken City", "Haunted Fortress", "Mystic Grove", "Dragon's Lair"};
   }
   else
   {
       LocationTypes = {"Planar Gateway", "Ancient Ziggurat", "Void Nexus", "Elder God's Vault", "Primordial Chasm"};
   }
   
   FString LocationType = LocationTypes[FMath::RandRange(0, LocationTypes.Num() - 1)];
   
   // Generate region
   TArray<FString> Regions = {
       "the Western Wildlands", "the Northern Peaks", "the Eastern Deserts", 
       "the Southern Jungles", "the Coastal Cliffs", "the Central Plains",
       "the Misty Mountains", "the Forgotten Forest", "the Sundered Sea"
   };
   
   FString Region = Regions[FMath::RandRange(0, Regions.Num() - 1)];
   
   // Create quest name and description
   TArray<FString> ExploreVerbs = {"Explore", "Investigate", "Scout", "Survey", "Chart", "Map"};
   int32 VerbIndex = FMath::RandRange(0, ExploreVerbs.Num() - 1);
   
   Quest->QuestName = FString::Printf(TEXT("%s the %s"), *ExploreVerbs[VerbIndex], *LocationType);
   
   TArray<FString> ReasonTemplates = {
       "Ancient artifacts are rumored to be hidden there.",
       "Strange occurrences have been reported in the area.",
       "We need to assess potential threats in the region.",
       "Lost knowledge may be recovered from this place.",
       "The location might serve as an outpost for your kingdom.",
       "A mysterious power emanates from within."
   };
   
   int32 ReasonIndex = FMath::RandRange(0, ReasonTemplates.Num() - 1);
   
   Quest->Description = FString::Printf(TEXT("I need you to %s the %s in %s. %s"), 
       *ExploreVerbs[VerbIndex].ToLower(), 
       *LocationType, 
       *Region, 
       *ReasonTemplates[ReasonIndex]);
   
   // Create multiple objectives
   FQuestObjective Objective1;
   Objective1.Description = FString::Printf(TEXT("Find the %s in %s"), *LocationType, *Region);
   Objective1.CurrentProgress = 0;
   Objective1.RequiredProgress = 1;
   Objective1.bIsCompleted = false;
   
   FQuestObjective Objective2;
   Objective2.Description = FString::Printf(TEXT("Explore the %s"), *LocationType);
   Objective2.CurrentProgress = 0;
   Objective2.RequiredProgress = 1;
   Objective2.bIsCompleted = false;
   
   FQuestObjective Objective3;
   Objective3.Description = TEXT("Return with information");
   Objective3.CurrentProgress = 0;
   Objective3.RequiredProgress = 1;
   Objective3.bIsCompleted = false;
   
   Quest->Objectives.Add(Objective1);
   Quest->Objectives.Add(Objective2);
   Quest->Objectives.Add(Objective3);
   
   // Add optional objectives based on difficulty
   if (Quest->DifficultyLevel >= 3)
   {
       FQuestObjective Objective4;
       Objective4.Description = TEXT("Collect ancient inscriptions (3)");
       Objective4.CurrentProgress = 0;
       Objective4.RequiredProgress = 3;
       Objective4.bIsCompleted = false;
       
       Quest->Objectives.Insert(Objective4, 2);
   }
}

void UProceduralGenerator::GenerateBuildQuest(UQuest* Quest)
{
   // Generate building types based on difficulty
   TArray<FString> BuildingTypes;
   
   if (Quest->DifficultyLevel <= 2)
   {
       BuildingTypes = {"Watchtower", "Bridge", "Trading Post", "Shrine", "Mill"};
   }
   else if (Quest->DifficultyLevel <= 4)
   {
       BuildingTypes = {"Fortress", "Temple", "Guild Hall", "Academy", "Port"};
   }
   else
   {
       BuildingTypes = {"Castle", "Cathedral", "Arcane Spire", "Grand Arena", "Royal Palace"};
   }
   
   FString BuildingType = BuildingTypes[FMath::RandRange(0, BuildingTypes.Num() - 1)];
   
   // Generate required resources
   TArray<EResourceRarity> PossibleRarities;
   for (int32 i = 0; i <= FMath::Min(static_cast<int32>(EResourceRarity::Legendary), Quest->DifficultyLevel); i++)
   {
       PossibleRarities.Add(static_cast<EResourceRarity>(i));
   }
   
   int32 NumResourceTypes = FMath::RandRange(2, 3 + Quest->DifficultyLevel / 2);
   TMap<FString, int32> RequiredResources;
   
   for (int32 i = 0; i < NumResourceTypes; i++)
   {
       EResourceRarity Rarity = PossibleRarities[FMath::RandRange(0, PossibleRarities.Num() - 1)];
       FResourceData Resource = GenerateResource(Rarity, Rarity);
       int32 Amount = FMath::RandRange(10, 20) * (Quest->DifficultyLevel + 1);
       
       RequiredResources.Add(Resource.Name, Amount);
   }
   
   // Create quest name and description
   Quest->QuestName = FString::Printf(TEXT("Construct a %s"), *BuildingType);
   
   TArray<FString> ReasonTemplates = {
       "It will strengthen our presence in the region.",
       "It will provide important services to the local population.",
       "It will serve as a beacon of hope in these troubled times.",
       "It will demonstrate our commitment to the area's development.",
       "It will honor an ancient pact with the local spirits.",
       "It will secure our supply lines through this territory."
   };
   
   int32 ReasonIndex = FMath::RandRange(0, ReasonTemplates.Num() - 1);
   
   FString ResourceList;
   bool bFirst = true;
   
   for (const TPair<FString, int32>& Resource : RequiredResources)
   {
       if (bFirst)
       {
           ResourceList = FString::Printf(TEXT("%d %s"), Resource.Value, *Resource.Key);
           bFirst = false;
       }
       else
       {
           ResourceList += FString::Printf(TEXT(", %d %s"), Resource.Value, *Resource.Key);
       }
   }
   
   Quest->Description = FString::Printf(TEXT("I need you to construct a %s. We'll need %s. %s"), 
       *BuildingType, 
       *ResourceList, 
       *ReasonTemplates[ReasonIndex]);
   
   // Create collection objectives for each resource
   for (const TPair<FString, int32>& Resource : RequiredResources)
   {
       FQuestObjective Objective;
       Objective.Description = FString::Printf(TEXT("Gather %s (%d)"), *Resource.Key, Resource.Value);
       Objective.CurrentProgress = 0;
       Objective.RequiredProgress = Resource.Value;
       Objective.bIsCompleted = false;
       
       Quest->Objectives.Add(Objective);
   }
   
   // Add final construction objective
   FQuestObjective FinalObjective;
   FinalObjective.Description = FString::Printf(TEXT("Construct the %s"), *BuildingType);
   FinalObjective.CurrentProgress = 0;
   FinalObjective.RequiredProgress = 1;
   FinalObjective.bIsCompleted = false;
   
   Quest->Objectives.Add(FinalObjective);
}

void UProceduralGenerator::GenerateDiplomaticQuest(UQuest* Quest)
{
   // Generate faction types based on difficulty
   TArray<FString> FactionTypes;
   
   if (Quest->DifficultyLevel <= 2)
   {
       FactionTypes = {"Village Elders", "Trade Guild", "Local Militia", "Druid Circle", "Hunter's Lodge"};
   }
   else if (Quest->DifficultyLevel <= 4)
   {
       FactionTypes = {"Noble House", "Mage College", "Warrior Clan", "Merchant Consortium", "Religious Order"};
   }
   else
   {
       FactionTypes = {"Royal Court", "Arcane Council", "Ancient Bloodline", "Planar Embassy", "Divine Pantheon"};
   }
   
   FString FactionType = FactionTypes[FMath::RandRange(0, FactionTypes.Num() - 1)];
   FString FactionName = GenerateName(false) + " " + FactionType;
   
   // Generate diplomatic task
   TArray<FString> DiplomaticTasks;
   
   if (Quest->DifficultyLevel <= 2)
   {
       DiplomaticTasks = {"Deliver a peace offering", "Negotiate a minor trade deal", "Mediate a small dispute", "Extend an invitation", "Request assistance"};
   }
   else if (Quest->DifficultyLevel <= 4)
   {
       DiplomaticTasks = {"Forge an alliance", "Negotiate a non-aggression pact", "Secure trade rights", "Exchange cultural ambassadors", "Request military aid"};
   }
   else
   {
       DiplomaticTasks = {"Negotiate a royal marriage", "Form a grand coalition", "Establish exclusive resource rights", "Create a mutual defense treaty", "Forge a magical compact"};
   }
   
   FString DiplomaticTask = DiplomaticTasks[FMath::RandRange(0, DiplomaticTasks.Num() - 1)];
   
   // Create quest name and description
   Quest->QuestName = FString::Printf(TEXT("Diplomacy with the %s"), *FactionType);
   
   TArray<FString> ReasonTemplates = {
       "Their support would greatly benefit our kingdom.",
       "Their resources are vital for our expansion.",
       "Their knowledge could solve many of our problems.",
       "Their military might would deter our enemies.",
       "Their influence extends beyond our borders.",
       "Their ancient wisdom could guide our future."
   };
   
   int32 ReasonIndex = FMath::RandRange(0, ReasonTemplates.Num() - 1);
   
   Quest->Description = FString::Printf(TEXT("I need you to %s with the %s. %s"), 
       *DiplomaticTask.ToLower(), 
       *FactionName, 
       *ReasonTemplates[ReasonIndex]);
   
   // Create multiple objectives
   FQuestObjective Objective1;
   Objective1.Description = FString::Printf(TEXT("Meet with the %s representatives"), *FactionType);
   Objective1.CurrentProgress = 0;
   Objective1.RequiredProgress = 1;
   Objective1.bIsCompleted = false;
   
   FQuestObjective Objective2;
   Objective2.Description = FString::Printf(TEXT("%s"), *DiplomaticTask);
   Objective2.CurrentProgress = 0;
   Objective2.RequiredProgress = 1;
   Objective2.bIsCompleted = false;
   
   FQuestObjective Objective3;
   Objective3.Description = TEXT("Return with the diplomatic outcome");
   Objective3.CurrentProgress = 0;
   Objective3.RequiredProgress = 1;
   Objective3.bIsCompleted = false;
   
   Quest->Objectives.Add(Objective1);
   Quest->Objectives.Add(Objective2);
   Quest->Objectives.Add(Objective3);
   
   // Add optional gift objective based on difficulty
   if (Quest->DifficultyLevel >= 3)
   {
       EResourceRarity MinRarity = static_cast<EResourceRarity>(FMath::Min(static_cast<int32>(EResourceRarity::Rare), Quest->DifficultyLevel - 2));
       EResourceRarity MaxRarity = static_cast<EResourceRarity>(FMath::Min(static_cast<int32>(EResourceRarity::Legendary), Quest->DifficultyLevel));
       
       FResourceData Resource = GenerateResource(MinRarity, MaxRarity);
       int32 Amount = FMath::RandRange(1, 3) * Quest->DifficultyLevel;
       
       FQuestObjective GiftObjective;
       GiftObjective.Description = FString::Printf(TEXT("Acquire diplomatic gifts: %s (%d)"), *Resource.Name, Amount);
       GiftObjective.CurrentProgress = 0;
       GiftObjective.RequiredProgress = Amount;
       GiftObjective.bIsCompleted = false;
       
       Quest->Objectives.Insert(GiftObjective, 1);
   }
}

void UProceduralGenerator::GenerateMysteryQuest(UQuest* Quest)
{
   // Generate mystery types based on difficulty
   TArray<FString> MysteryTypes;
   
   if (Quest->DifficultyLevel <= 2)
   {
       MysteryTypes = {"Strange Disappearances", "Unusual Animal Behavior", "Mysterious Illness", "Stolen Artifact", "Unexplained Phenomena"};
   }
   else if (Quest->DifficultyLevel <= 4)
   {
       MysteryTypes = {"Ancient Curse", "Haunting", "Magical Anomaly", "Conspiracy", "Lost Expedition"};
   }
   else
   {
       MysteryTypes = {"Planar Incursion", "Prophetic Vision", "Divine Intervention", "Cosmic Anomaly", "Temporal Distortion"};
   }
   
   FString MysteryType = MysteryTypes[FMath::RandRange(0, MysteryTypes.Num() - 1)];
   
   // Generate location
   TArray<FString> Locations = {
       "Whispering Woods", "Foggy Hollow", "Moonlight Bay", "Thunderpeak", "Shadowvale",
       "Lost Harbor", "Forgotten Hills", "Ancient Crossroads", "Crimson Valley", "Frostfall"
   };
   
   FString Location = Locations[FMath::RandRange(0, Locations.Num() - 1)];
   
   // Create quest name and description
   TArray<FString> MysteryVerbs = {"Investigate", "Solve", "Unravel", "Uncover", "Explore", "Decipher"};
   int32 VerbIndex = FMath::RandRange(0, MysteryVerbs.Num() - 1);
   
   Quest->QuestName = FString::Printf(TEXT("%s the %s"), *MysteryVerbs[VerbIndex], *MysteryType);
   
   TArray<FString> ReasonTemplates = {
       "Local residents are terrified and need your help.",
       "The consequences could be dire if left unchecked.",
       "Ancient warnings speak of such occurrences.",
       "Similar events preceded great calamities in the past.",
       "Strange energies have been detected that could be useful.",
       "Knowledge of this phenomenon could prove invaluable."
   };
   
   int32 ReasonIndex = FMath::RandRange(0, ReasonTemplates.Num() - 1);
   
   Quest->Description = FString::Printf(TEXT("I need you to %s the %s in %s. %s"), 
       *MysteryVerbs[VerbIndex].ToLower(), 
       *MysteryType, 
       *Location, 
       *ReasonTemplates[ReasonIndex]);
   
   // Create multiple objectives with hidden elements (revealed as quest progresses)
   FQuestObjective Objective1;
   Objective1.Description = FString::Printf(TEXT("%s the %s in %s"), *MysteryVerbs[VerbIndex], *MysteryType, *Location);
   Objective1.CurrentProgress = 0;
   Objective1.RequiredProgress = 1;
   Objective1.bIsCompleted = false;
   
   FQuestObjective Objective2;
   Objective2.Description = TEXT("Gather evidence (0/3)");
   Objective2.CurrentProgress = 0;
   Objective2.RequiredProgress = 3;
   Objective2.bIsCompleted = false;
   
   FQuestObjective Objective3;
   Objective3.Description = TEXT("????? (Quest objective will be revealed as you progress)");
   Objective3.CurrentProgress = 0;
   Objective3.RequiredProgress = 1;
   Objective3.bIsCompleted = false;
   
   Quest->Objectives.Add(Objective1);
   Quest->Objectives.Add(Objective2);
   Quest->Objectives.Add(Objective3);
   
   // Harder mysteries have more objectives
   if (Quest->DifficultyLevel >= 4)
   {
       FQuestObjective Objective4;
       Objective4.Description = TEXT("????? (Quest objective will be revealed as you progress)");
       Objective4.CurrentProgress = 0;
       Objective4.RequiredProgress = 1;
       Objective4.bIsCompleted = false;
       
       Quest->Objectives.Add(Objective4);
   }
}

FString UProceduralGenerator::GenerateName(bool bIsFollower) const
{
   if (bIsFollower)
   {
       // Generate character name
       TArray<FString> FirstNames = {
           "Aldric", "Brenna", "Cedric", "Dalia", "Eamon", "Fiona", "Gareth", "Helga",
           "Ignis", "Jorah", "Kira", "Leif", "Mira", "Niles", "Orla", "Phineas",
           "Quinn", "Rowan", "Selene", "Thorne", "Una", "Varian", "Willow", "Xander",
           "Yvaine", "Zephyr"
       };
       
       TArray<FString> LastNames = {
           "Blackwood", "Crestfall", "Dawnforge", "Emberstone", "Frostwind", "Grimthorn", "Heartfield",
           "Ironbark", "Jaystone", "Kindleflame", "Lightbringer", "Moonshadow", "Nightvale", "Oakenheart",
           "Proudmoor", "Quicksilver", "Ravencrest", "Silverleaf", "Thornheart", "Underhill",
           "Valesong", "Whitewater", "Yellowfield", "Zephyrwind"
       };
       
       FString FirstName = FirstNames[FMath::RandRange(0, FirstNames.Num() - 1)];
       FString LastName = LastNames[FMath::RandRange(0, LastNames.Num() - 1)];
       
       return FirstName + " " + LastName;
   }
   else
   {
       // Generate faction/location name
       TArray<FString> Prefixes = {
           "Black", "Bright", "Crystal", "Dark", "Dawn", "Dusk", "Ever", "Frost",
           "Ghost", "Golden", "Iron", "Light", "Moon", "Night", "Raven", "Red",
           "Shadow", "Silver", "Storm", "Sun", "Thunder", "Twin", "Wild", "Winter"
       };
       
       TArray<FString> Suffixes = {
           "crest", "fall", "field", "ford", "gate", "haven", "hold", "keep",
           "light", "mist", "peak", "point", "reach", "rock", "root", "shield",
           "spire", "star", "stone", "vale", "valley", "watch", "wind", "wood"
       };
       
       FString Prefix = Prefixes[FMath::RandRange(0, Prefixes.Num() - 1)];
       FString Suffix = Suffixes[FMath::RandRange(0, Suffixes.Num() - 1)];
       
       return Prefix + Suffix;
   }
}

FResourceData UProceduralGenerator::GenerateResource(EResourceRarity MinRarity, EResourceRarity MaxRarity)
{
    FResourceData Resource;
    
    // Ensure valid rarity range
    MinRarity = static_cast<EResourceRarity>(FMath::Clamp(static_cast<int32>(MinRarity), 0, static_cast<int32>(EResourceRarity::Artifact)));
    MaxRarity = static_cast<EResourceRarity>(FMath::Clamp(static_cast<int32>(MaxRarity), static_cast<int32>(MinRarity), static_cast<int32>(EResourceRarity::Artifact)));
    
    // Randomly select a rarity within the specified range
    int32 RarityRange = static_cast<int32>(MaxRarity) - static_cast<int32>(MinRarity) + 1;
    EResourceRarity SelectedRarity = static_cast<EResourceRarity>(static_cast<int32>(MinRarity) + FMath::RandRange(0, RarityRange - 1));
    
    Resource.Rarity = SelectedRarity;
    
    // Generate resource name based on rarity
    TArray<FString> ResourceTypes;
    TArray<FString> ResourceQualities;
    TArray<FString> ResourceOrigins;
    
    // Populate name components based on resource category
    switch (FMath::RandRange(0, 3))
    {
        // Metals
        case 0:
            ResourceTypes = {"Iron", "Steel", "Copper", "Silver", "Gold", "Platinum", "Mithril", "Adamantium", "Orichalcum", "Starmetal"};
            ResourceQualities = {"Crude", "Raw", "Refined", "Pure", "Polished", "Pristine", "Radiant", "Enchanted", "Celestial"};
            ResourceOrigins = {"Ore", "Ingot", "Bar", "Alloy", "Composite", "Mineral"};
            break;
            
        // Plant materials
        case 1:
            ResourceTypes = {"Oak", "Ash", "Pine", "Maple", "Willow", "Ebony", "Rosewood", "Ironwood", "Elderwood", "Spiritwood"};
            ResourceQualities = {"Common", "Sturdy", "Flexible", "Strong", "Ancient", "Primal", "Mystic", "Ethereal"};
            ResourceOrigins = {"Wood", "Timber", "Lumber", "Bark", "Root", "Fibers", "Resin"};
            break;
            
        // Creature materials
        case 2:
            ResourceTypes = {"Wolf", "Bear", "Spider", "Drake", "Troll", "Giant", "Wyrm", "Dragon", "Phoenix", "Behemoth"};
            ResourceQualities = {"Damaged", "Intact", "Tough", "Resilient", "Superior", "Flawless", "Magnificent", "Legendary"};
            ResourceOrigins = {"Hide", "Leather", "Pelt", "Scales", "Bone", "Claw", "Fang", "Horn"};
            break;
            
        // Magical materials
        case 3:
            ResourceTypes = {"Arcane", "Fire", "Frost", "Storm", "Shadow", "Light", "Chaos", "Order", "Void", "Astral"};
            ResourceQualities = {"Flickering", "Stable", "Potent", "Concentrated", "Brilliant", "Transcendent", "Primordial"};
            ResourceOrigins = {"Crystal", "Gem", "Dust", "Essence", "Shard", "Extract", "Residue", "Catalyst"};
            break;
    }
    
    // Select name components based on rarity
    FString SelectedType;
    FString SelectedQuality;
    FString SelectedOrigin;
    
    // Higher rarities get more exotic types
    int32 TypeIndex = FMath::Min(static_cast<int32>(SelectedRarity) + FMath::RandRange(0, 2), ResourceTypes.Num() - 1);
    SelectedType = ResourceTypes[TypeIndex];
    
    // Higher rarities get better qualities
    int32 QualityIndex = FMath::Min(
        FMath::Max(static_cast<int32>(SelectedRarity) - 1 + FMath::RandRange(0, 2), 0),
        ResourceQualities.Num() - 1
    );
    SelectedQuality = ResourceQualities[QualityIndex];
    
    // Select a random origin
    SelectedOrigin = ResourceOrigins[FMath::RandRange(0, ResourceOrigins.Num() - 1)];
    
    // For artifact rarity, add special prefix
    if (SelectedRarity == EResourceRarity::Artifact)
    {
        TArray<FString> ArtifactPrefixes = {
            "Ancient", "Legendary", "Divine", "Mythical", "Eternal", "Cosmic", "Primeval", "Forgotten"
        };
        SelectedQuality = ArtifactPrefixes[FMath::RandRange(0, ArtifactPrefixes.Num() - 1)];
    }
    
    // Construct the resource name (format depends on rarity)
    if (FMath::RandBool() && SelectedRarity >= EResourceRarity::Uncommon)
    {
        // Format: Quality Type Origin (e.g., "Pristine Gold Ore")
        Resource.Name = FString::Printf(TEXT("%s %s %s"), *SelectedQuality, *SelectedType, *SelectedOrigin);
    }
    else
    {
        // Format: Type Origin (e.g., "Iron Ore")
        Resource.Name = FString::Printf(TEXT("%s %s"), *SelectedType, *SelectedOrigin);
    }
    
    // Generate a description
    TArray<FString> Descriptions;
    switch (SelectedRarity)
    {
        case EResourceRarity::Common:
            Descriptions = {
                "A common resource found throughout the land.",
                "Basic material used in many crafting recipes.",
                "Widely available and relatively inexpensive.",
                "Standard quality material with many practical uses."
            };
            break;
            
        case EResourceRarity::Uncommon:
            Descriptions = {
                "A resource of above-average quality and moderate rarity.",
                "Sought after by craftsmen for its superior properties.",
                "Less common than standard materials, but not exceptionally rare.",
                "Possesses unique characteristics that make it valuable."
            };
            break;
            
        case EResourceRarity::Rare:
            Descriptions = {
                "A rare and valuable resource found in specific regions.",
                "Highly prized by master craftsmen for exceptional results.",
                "Difficult to obtain but worth the effort for its properties.",
                "Has specialized applications in advanced crafting."
            };
            break;
            
        case EResourceRarity::Legendary:
            Descriptions = {
                "An extremely rare resource with extraordinary properties.",
                "Few living craftsmen have worked with this legendary material.",
                "Sought after by kings and powerful entities.",
                "Its origins are shrouded in mystery and ancient lore."
            };
            break;
            
        case EResourceRarity::Artifact:
            Descriptions = {
                "A mythical substance said to exist only in legends.",
                "Possesses otherworldly properties that defy conventional understanding.",
                "Said to be used in the creation of items of power in ancient times.",
                "Its mere presence seems to alter the fabric of reality around it."
            };
            break;
    }
    
    Resource.Description = Descriptions[FMath::RandRange(0, Descriptions.Num() - 1)];
    
    // Set base value based on rarity (exponential scaling)
    int32 BaseValues[] = {10, 50, 250, 1000, 5000};
    Resource.BaseValue = BaseValues[static_cast<int32>(SelectedRarity)] * FMath::RandRange(8, 12) / 10;
    
    // Note: In a real implementation, you would also assign the Icon property
    // Resource.Icon = LoadObject<UTexture2D>(nullptr, *IconPath);
    
    return Resource;
}

FLootItem UProceduralGenerator::GenerateLoot(int32 PlayerLevel, float RarityModifier)
{
    FLootItem Item;
    
    // Clamp inputs to reasonable ranges
    PlayerLevel = FMath::Clamp(PlayerLevel, 1, 50);
    RarityModifier = FMath::Clamp(RarityModifier, 0.0f, 2.0f);
    
    // Determine item rarity based on player level and rarity modifier
    float RarityRoll = FMath::FRand() + (PlayerLevel / 50.0f) + RarityModifier;
    
    EResourceRarity ItemRarity;
    if (RarityRoll < 1.0f)
    {
        ItemRarity = EResourceRarity::Common;
    }
    else if (RarityRoll < 1.5f)
    {
        ItemRarity = EResourceRarity::Uncommon;
    }
    else if (RarityRoll < 1.9f)
    {
        ItemRarity = EResourceRarity::Rare;
    }
    else if (RarityRoll < 2.3f)
    {
        ItemRarity = EResourceRarity::Legendary;
    }
    else
    {
        ItemRarity = EResourceRarity::Artifact;
    }
    
    Item.Rarity = ItemRarity;
    
    // Determine item type
    enum class EItemType
    {
        Weapon,
        Armor,
        Accessory,
        Consumable,
        Trophy
    };
    
    EItemType ItemType = static_cast<EItemType>(FMath::RandRange(0, 4));
    
    // Generate name based on item type and rarity
    switch (ItemType)
    {
        case EItemType::Weapon:
            GenerateWeaponItem(Item, PlayerLevel, ItemRarity);
            break;
            
        case EItemType::Armor:
            GenerateArmorItem(Item, PlayerLevel, ItemRarity);
            break;
            
        case EItemType::Accessory:
            GenerateAccessoryItem(Item, PlayerLevel, ItemRarity);
            break;
            
        case EItemType::Consumable:
            GenerateConsumableItem(Item, PlayerLevel, ItemRarity);
            break;
            
        case EItemType::Trophy:
            GenerateTrophyItem(Item, PlayerLevel, ItemRarity);
            break;
    }
    
    // Set value based on rarity and player level
    float RarityMultiplier = 1.0f;
    switch (ItemRarity)
    {
        case EResourceRarity::Common:
            RarityMultiplier = 1.0f;
            break;
        case EResourceRarity::Uncommon:
            RarityMultiplier = 2.5f;
            break;
        case EResourceRarity::Rare:
            RarityMultiplier = 6.0f;
            break;
        case EResourceRarity::Legendary:
            RarityMultiplier = 15.0f;
            break;
        case EResourceRarity::Artifact:
            RarityMultiplier = 40.0f;
            break;
    }
    
    Item.Value = FMath::RoundToInt((PlayerLevel * 10.0f) * RarityMultiplier * FMath::RandRange(0.8f, 1.2f));
    
    // Note: In a real implementation, you would also assign the Icon property
    // Item.Icon = LoadObject<UTexture2D>(nullptr, *IconPath);
    
    return Item;
}

void UProceduralGenerator::GenerateWeaponItem(FLootItem& Item, int32 PlayerLevel, EResourceRarity Rarity)
{
    // Arrays of weapon prefixes by rarity
    TArray<TArray<FString>> RarityPrefixes = {
        // Common
        {"Sturdy", "Reliable", "Serviceable", "Standard", "Practical"},
        // Uncommon
        {"Fine", "Superior", "Quality", "Reinforced", "Balanced"},
        // Rare
        {"Exceptional", "Masterwork", "Ornate", "Precision", "Devastating"},
        // Legendary
        {"Ancient", "Heroic", "Mythical", "Legendary", "Celestial"},
        // Artifact
        {"Godslayer", "Worldender", "Transcendent", "Divine", "Primordial"}
    };
    
    // Weapon types
    TArray<FString> WeaponTypes = {
        "Sword", "Axe", "Mace", "Spear", "Bow", "Dagger", 
        "Warhammer", "Staff", "Greatsword", "Halberd", "Crossbow"
    };
    
    // Weapon suffixes
    TArray<FString> WeaponSuffixes = {
        "of Power", "of Might", "of Swiftness", "of the Bear", "of the Eagle",
        "of the Serpent", "of Slaying", "of Destruction", "of Vengeance", "of Victory"
    };
    
    // Select prefix based on rarity
    FString Prefix = "";
    if (static_cast<int32>(Rarity) < RarityPrefixes.Num())
    {
        const TArray<FString>& AvailablePrefixes = RarityPrefixes[static_cast<int32>(Rarity)];
        Prefix = AvailablePrefixes[FMath::RandRange(0, AvailablePrefixes.Num() - 1)];
    }
    
    // Select weapon type
    FString WeaponType = WeaponTypes[FMath::RandRange(0, WeaponTypes.Num() - 1)];
    
    // Rare and above items get a suffix
    FString Suffix = "";
    if (Rarity >= EResourceRarity::Rare)
    {
        Suffix = WeaponSuffixes[FMath::RandRange(0, WeaponSuffixes.Num() - 1)];
    }
    
    // Construct name based on rarity
    if (Rarity >= EResourceRarity::Legendary)
    {
        // Legendary and Artifact weapons might have unique names
        TArray<FString> UniqueNames = {
            "Dawnbreaker", "Nightfall", "Soulreaver", "Worldcarver", "Stormcaller",
            "Dragonbane", "Shadowfang", "Lightbringer", "Frostmourn", "Destiny"
        };
        
        Item.Name = UniqueNames[FMath::RandRange(0, UniqueNames.Num() - 1)];
    }
    else if (Rarity == EResourceRarity::Common)
    {
        // Common weapons might just be "Iron Sword" etc.
        TArray<FString> Materials = {"Iron", "Steel", "Bronze", "Wooden", "Stone"};
        Item.Name = Materials[FMath::RandRange(0, Materials.Num() - 1)] + " " + WeaponType;
    }
    else
    {
        // Construct from prefix, type, and possibly suffix
        Item.Name = Prefix + " " + WeaponType;
        if (!Suffix.IsEmpty())
        {
            Item.Name += " " + Suffix;
        }
    }
    
    // Generate description
    TArray<FString> DescriptionTemplates = {
        "A %s designed for both efficiency and reliability.",
        "This %s has served many warriors in battle.",
        "The balance and craftsmanship of this %s are %s.",
        "A %s that %s in the hands of a skilled warrior.",
        "The edge of this %s %s with deadly promise."
    };
    
    TArray<FString> QualityDescriptors = {"acceptable", "fine", "impressive", "remarkable", "extraordinary"};
    TArray<FString> ActionDescriptors = {"performs well", "excels", "sings", "dances", "becomes legendary"};
    TArray<FString> EdgeDescriptors = {"gleams", "shimmers", "pulses", "glows", "radiates"};
    
    int32 TemplateIndex = FMath::RandRange(0, DescriptionTemplates.Num() - 1);
    FString DescTemplate = DescriptionTemplates[TemplateIndex];
    
    // Fill in template based on which one was selected
    switch (TemplateIndex)
    {
        case 0:
        case 1:
            Item.Description = FString::Printf(*DescTemplate, *WeaponType.ToLower());
            break;
            
        case 2:
            Item.Description = FString::Printf(*DescTemplate, 
                *WeaponType.ToLower(),
                *QualityDescriptors[FMath::Min(static_cast<int32>(Rarity), QualityDescriptors.Num() - 1)]);
            break;
            
        case 3:
            Item.Description = FString::Printf(*DescTemplate, 
                *WeaponType.ToLower(),
                *ActionDescriptors[FMath::Min(static_cast<int32>(Rarity), ActionDescriptors.Num() - 1)]);
            break;
            
        case 4:
            Item.Description = FString::Printf(*DescTemplate, 
                *WeaponType.ToLower(),
                *EdgeDescriptors[FMath::Min(static_cast<int32>(Rarity), EdgeDescriptors.Num() - 1)]);
            break;
    }
    
    // Add stats based on player level and rarity
    // Base damage
    float BaseDamage = PlayerLevel * 2.0f * (1.0f + 0.5f * static_cast<int32>(Rarity));
    Item.Stats.Add("Damage", BaseDamage * FMath::RandRange(0.9f, 1.1f));
    
    // Attack speed
    Item.Stats.Add("AttackSpeed", FMath::RandRange(0.8f, 1.5f));
    
    // Add special stats based on rarity
    if (Rarity >= EResourceRarity::Uncommon)
    {
        // Chance for critical hit
        Item.Stats.Add("CritChance", FMath::Min(5.0f + (5.0f * static_cast<int32>(Rarity)), 25.0f));
    }
    
    if (Rarity >= EResourceRarity::Rare)
    {
        // Critical damage multiplier
        Item.Stats.Add("CritDamage", 1.5f + (0.25f * static_cast<int32>(Rarity)));
        
        // Chance for elemental effects
        TArray<FString> ElementalTypes = {"Fire", "Ice", "Lightning", "Poison", "Arcane"};
        FString ElementType = ElementalTypes[FMath::RandRange(0, ElementalTypes.Num() - 1)];
        
        float ElementalDamage = BaseDamage * 0.3f * static_cast<int32>(Rarity - EResourceRarity::Rare + 1);
        Item.Stats.Add(ElementType + "Damage", ElementalDamage);
    }
    
    if (Rarity >= EResourceRarity::Legendary)
    {
        // Special effects
        TArray<FString> SpecialEffects = {
            "LifeSteal", "ArmorPenetration", "StunChance", "BleedChance", "Executioner"
        };
        
        FString SpecialEffect = SpecialEffects[FMath::RandRange(0, SpecialEffects.Num() - 1)];
        Item.Stats.Add(SpecialEffect, 5.0f + (5.0f * (static_cast<int32>(Rarity) - static_cast<int32>(EResourceRarity::Legendary))));
    }
}

void UProceduralGenerator::GenerateArmorItem(FLootItem& Item, int32 PlayerLevel, EResourceRarity Rarity)
{
    // Arrays of armor prefixes by rarity
    TArray<TArray<FString>> RarityPrefixes = {
        // Common
        {"Sturdy", "Reliable", "Serviceable", "Standard", "Practical"},
        // Uncommon
        {"Reinforced", "Hardy", "Resilient", "Durable", "Solid"},
        // Rare
        {"Exceptional", "Masterwork", "Ornate", "Impenetrable", "Formidable"},
        // Legendary
        {"Ancient", "Heroic", "Mythical", "Legendary", "Celestial"},
        // Artifact
        {"Godforged", "Ethereal", "Transcendent", "Divine", "Primordial"}
    };
    
    // Armor types
    TArray<FString> ArmorTypes = {
        "Helmet", "Chestplate", "Gauntlets", "Boots", "Shield", 
        "Pauldrons", "Greaves", "Vambraces", "Cuirass", "Armor"
    };
    
    // Armor materials
    TArray<TArray<FString>> RarityMaterials = {
        // Common
        {"Iron", "Leather", "Bronze", "Chain", "Hide"},
        // Uncommon
        {"Steel", "Reinforced Leather", "Silver", "Scale", "Hardened Hide"},
        // Rare
        {"Mithril", "Dragonhide", "Obsidian", "Enchanted", "Crystal"},
        // Legendary
        {"Adamantite", "Ancient Scale", "Starforged", "Ethereal", "Godsteel"},
        // Artifact
        {"Celestial", "Void", "Primordial", "Cosmic", "Eternal"}
    };
    
    // Armor suffixes
    TArray<FString> ArmorSuffixes = {
        "of Protection", "of Warding", "of Shielding", "of the Guardian", "of the Defender",
        "of Resilience", "of Fortitude", "of Endurance", "of Deftness", "of Valor"
    };
    
    // Select components based on rarity
    FString Prefix = "";
    if (static_cast<int32>(Rarity) < RarityPrefixes.Num())
    {
        const TArray<FString>& AvailablePrefixes = RarityPrefixes[static_cast<int32>(Rarity)];
        Prefix = AvailablePrefixes[FMath::RandRange(0, AvailablePrefixes.Num() - 1)];
    }
    
    FString Material = "";
    if (static_cast<int32>(Rarity) < RarityMaterials.Num())
    {
        const TArray<FString>& AvailableMaterials = RarityMaterials[static_cast<int32>(Rarity)];
        Material = AvailableMaterials[FMath::RandRange(0, AvailableMaterials.Num() - 1)];
    }
    
    // Select armor type
    FString ArmorType = ArmorTypes[FMath::RandRange(0, ArmorTypes.Num() - 1)];
    
    // Rare and above items get a suffix
    FString Suffix = "";
    if (Rarity >= EResourceRarity::Rare)
    {
        Suffix = ArmorSuffixes[FMath::RandRange(0, ArmorSuffixes.Num() - 1)];
    }
    
    // Construct name based on rarity
    if (Rarity >= EResourceRarity::Legendary)
    {
        // Legendary and Artifact armor might have unique names
        TArray<FString> UniqueNames = {
            "Dragonscale " + ArmorType, "Soulshield", "Heartguard", "Eclipse " + ArmorType,
            "Aegis of the Immortal", "Bastion of Stars", "Veil of Shadows", 
            "Bulwark of Dawn", "Guardian's Embrace", "Eternity's Shell"
        };
        
        Item.Name = UniqueNames[FMath::RandRange(0, UniqueNames.Num() - 1)];
    }
    else if (Rarity == EResourceRarity::Common)
    {
        // Common armor is just material + type
        Item.Name = Material + " " + ArmorType;
    }
    else
    {
        // Construct from prefix, material, type, and possibly suffix
        Item.Name = Prefix + " " + Material + " " + ArmorType;
        if (!Suffix.IsEmpty())
        {
            Item.Name += " " + Suffix;
        }
    }
    
    // Generate description
    TArray<FString> DescriptionTemplates = {
        "A %s piece of armor offering reliable protection.",
        "This %s %s has protected many warriors in battle.",
        "The craftsmanship of this %s is %s, providing excellent defense.",
        "A %s that %s even the fiercest blows.",
        "The surface of this %s %s with protective enchantments."
    };
    
    TArray<FString> QualityDescriptors = {"acceptable", "fine", "impressive", "remarkable", "extraordinary"};
    TArray<FString> ActionDescriptors = {"deflects", "repels", "withstands", "neutralizes", "transcends"};
    TArray<FString> SurfaceDescriptors = {"gleams", "shimmers", "pulses", "glows", "radiates"};
    
    int32 TemplateIndex = FMath::RandRange(0, DescriptionTemplates.Num() - 1);
    FString DescTemplate = DescriptionTemplates[TemplateIndex];
    
    // Fill in template based on which one was selected
    switch (TemplateIndex)
    {
        case 0:
            Item.Description = FString::Printf(*DescTemplate, 
                *QualityDescriptors[FMath::Min(static_cast<int32>(Rarity), QualityDescriptors.Num() - 1)]);
            break;
            
        case 1:
            Item.Description = FString::Printf(*DescTemplate, 
                *Material.ToLower(),
                *ArmorType.ToLower());
            break;
            
        case 2:
            Item.Description = FString::Printf(*DescTemplate, 
                *ArmorType.ToLower(),
                *QualityDescriptors[FMath::Min(static_cast<int32>(Rarity), QualityDescriptors.Num() - 1)]);
            break;
            
        case 3:
            Item.Description = FString::Printf(*DescTemplate, 
                *ArmorType.ToLower(),
                *ActionDescriptors[FMath::Min(static_cast<int32>(Rarity), ActionDescriptors.Num() - 1)]);
            break;
            
        case 4:
            Item.Description = FString::Printf(*DescTemplate, 
                *ArmorType.ToLower(),
                *SurfaceDescriptors[FMath::Min(static_cast<int32>(Rarity), SurfaceDescriptors.Num() - 1)]);
            break;
    }
    
    // Add stats based on player level and rarity
    // Base armor value
    float BaseArmor = PlayerLevel * 1.5f * (1.0f + 0.5f * static_cast<int32>(Rarity));
    Item.Stats.Add("Armor", BaseArmor * FMath::RandRange(0.9f, 1.1f));
    
    // Add special stats based on rarity
    if (Rarity >= EResourceRarity::Uncommon)
    {
        // Add health bonus
        float HealthBonus = PlayerLevel * 5.0f * static_cast<int32>(Rarity);
        Item.Stats.Add("HealthBonus", HealthBonus);
    }
    
    if (Rarity >= EResourceRarity::Rare)
    {
        // Add resistance to different damage types
        TArray<FString> ResistanceTypes = {"Physical", "Fire", "Ice", "Lightning", "Poison", "Arcane"};
        
        for (int32 i = 0; i < 1 + static_cast<int32>(Rarity) - static_cast<int32>(EResourceRarity::Rare); i++)
        {
            if (i >= ResistanceTypes.Num()) break;
            
            FString ResistType = ResistanceTypes[i];
            float ResistValue = 5.0f + (5.0f * static_cast<int32>(Rarity - EResourceRarity::Rare));
            
            Item.Stats.Add(ResistType + "Resist", ResistValue);
        }
    }
    
    if (Rarity >= EResourceRarity::Legendary)
    {
        // Special effects
        TArray<FString> SpecialEffects = {
            "Thorns", "Regeneration", "ReflectDamage", "ChanceToEvade", "AuraOfProtection"
        };
        
        FString SpecialEffect = SpecialEffects[FMath::RandRange(0, SpecialEffects.Num() - 1)];
        Item.Stats.Add(SpecialEffect, 5.0f + (5.0f * (static_cast<int32>(Rarity) - static_cast<int32>(EResourceRarity::Legendary))));
    }
}

void UProceduralGenerator::GenerateAccessoryItem(FLootItem& Item, int32 PlayerLevel, EResourceRarity Rarity)
{
    // Arrays of accessory prefixes by rarity
    TArray<TArray<FString>> RarityPrefixes = {
        // Common
        {"Simple", "Basic", "Plain", "Common", "Modest"},
        // Uncommon
        {"Polished", "Gleaming", "Quality", "Ornamental", "Elegant"},
        // Rare
        {"Exceptional", "Exquisite", "Radiant", "Mystical", "Enchanted"},
        // Legendary
        {"Ancient", "Heroic", "Mythical", "Legendary", "Celestial"},
        // Artifact
        {"Divine", "Eternal", "Transcendent", "Cosmic", "Primordial"}
    };
    
    // Accessory types
    TArray<FString> AccessoryTypes = {
        "Ring", "Amulet", "Necklace", "Pendant", "Bracelet", 
        "Earring", "Brooch", "Charm", "Talisman", "Circlet"
    };
    
    // Accessory materials
    TArray<TArray<FString>> RarityMaterials = {
        // Common
        {"Iron", "Copper", "Bone", "Wooden", "Stone"},
        // Uncommon
        {"Silver", "Jade", "Amber", "Pearl", "Bronze"},
        // Rare
        {"Gold", "Sapphire", "Ruby", "Emerald", "Diamond"},
        // Legendary
        {"Platinum", "Opal", "Moonstone", "Stargem", "Soulcrystal"},
        // Artifact
        {"Celestial Silver", "Void Crystal", "Dragonglass", "Living Metal", "Eternium"}
    };
    
    // Accessory suffixes
    TArray<FString> AccessorySuffixes = {
        "of Power", "of Wisdom", "of Vitality", "of the Sage", "of the Warrior",
        "of Protection", "of Fortune", "of the Elements", "of Mysteries", "of Time"
    };
    
    // Select components based on rarity
   FString Prefix = "";
   if (static_cast<int32>(Rarity) < RarityPrefixes.Num())
   {
       const TArray<FString>& AvailablePrefixes = RarityPrefixes[static_cast<int32>(Rarity)];
       Prefix = AvailablePrefixes[FMath::RandRange(0, AvailablePrefixes.Num() - 1)];
   }
   
   FString Material = "";
   if (static_cast<int32>(Rarity) < RarityMaterials.Num())
   {
       const TArray<FString>& AvailableMaterials = RarityMaterials[static_cast<int32>(Rarity)];
       Material = AvailableMaterials[FMath::RandRange(0, AvailableMaterials.Num() - 1)];
   }
   
   // Select accessory type
   FString AccessoryType = AccessoryTypes[FMath::RandRange(0, AccessoryTypes.Num() - 1)];
   
   // Rare and above items get a suffix
   FString Suffix = "";
   if (Rarity >= EResourceRarity::Rare)
   {
       Suffix = AccessorySuffixes[FMath::RandRange(0, AccessorySuffixes.Num() - 1)];
   }
   
   // Construct name based on rarity
   if (Rarity >= EResourceRarity::Legendary)
   {
       // Legendary and Artifact accessories might have unique names
       TArray<FString> UniqueNames = {
           "Heart of the Mountain", "Eye of Eternity", "Star's Embrace", 
           "Whisper of Fate", "Dream Weaver", "Soul Binder",
           "Void Walker's " + AccessoryType, "Twilight Keeper", 
           "Oracle's Vision", "Essence of Creation"
       };
       
       Item.Name = UniqueNames[FMath::RandRange(0, UniqueNames.Num() - 1)];
   }
   else if (Rarity == EResourceRarity::Common)
   {
       // Common accessories are just material + type
       Item.Name = Material + " " + AccessoryType;
   }
   else
   {
       // Construct from prefix, material, type, and possibly suffix
       Item.Name = Prefix + " " + Material + " " + AccessoryType;
       if (!Suffix.IsEmpty())
       {
           Item.Name += " " + Suffix;
       }
   }
   
   // Generate description
   TArray<FString> DescriptionTemplates = {
       "A %s %s with subtle magical properties.",
       "This %s has been crafted with %s attention to detail.",
       "The %s emanates a %s aura of power.",
       "A %s that %s the wearer's natural abilities.",
       "The %s %s with mystical energy, promising great benefits to its owner."
   };
   
   TArray<FString> QualityDescriptors = {"simple", "fine", "remarkable", "exquisite", "transcendent"};
   TArray<FString> AuraDescriptors = {"faint", "noticeable", "vibrant", "powerful", "overwhelming"};
   TArray<FString> ActionDescriptors = {"subtly enhances", "enhances", "amplifies", "greatly amplifies", "dramatically enhances"};
   TArray<FString> EnergyDescriptors = {"glimmers", "pulses", "resonates", "surges", "overflows"};
   
   int32 TemplateIndex = FMath::RandRange(0, DescriptionTemplates.Num() - 1);
   FString DescTemplate = DescriptionTemplates[TemplateIndex];
   
   // Fill in template based on which one was selected
   switch (TemplateIndex)
   {
       case 0:
           Item.Description = FString::Printf(*DescTemplate, 
               *QualityDescriptors[FMath::Min(static_cast<int32>(Rarity), QualityDescriptors.Num() - 1)],
               *AccessoryType.ToLower());
           break;
           
       case 1:
           Item.Description = FString::Printf(*DescTemplate, 
               *AccessoryType.ToLower(),
               *QualityDescriptors[FMath::Min(static_cast<int32>(Rarity), QualityDescriptors.Num() - 1)]);
           break;
           
       case 2:
           Item.Description = FString::Printf(*DescTemplate, 
               *Material.ToLower() + " " + *AccessoryType.ToLower(), 
               *AuraDescriptors[FMath::Min(static_cast<int32>(Rarity), AuraDescriptors.Num() - 1)]);
           break;
           
       case 3:
           Item.Description = FString::Printf(*DescTemplate, 
               *AccessoryType.ToLower(),
               *ActionDescriptors[FMath::Min(static_cast<int32>(Rarity), ActionDescriptors.Num() - 1)]);
           break;
           
       case 4:
           Item.Description = FString::Printf(*DescTemplate, 
               *Material.ToLower() + " " + *AccessoryType.ToLower(),
               *EnergyDescriptors[FMath::Min(static_cast<int32>(Rarity), EnergyDescriptors.Num() - 1)]);
           break;
   }
   
   // Add stats based on player level and rarity
   // Accessories primarily boost attributes and secondary statistics
   
   // Select 1-3 attributes to boost based on rarity
   int32 NumStats = FMath::Min(1 + static_cast<int32>(Rarity), 5);
   
   TArray<FString> PossibleStats = {
       "Strength", "Dexterity", "Intelligence", "Wisdom", "Constitution",
       "CritChance", "CritDamage", "MoveSpeed", "AttackSpeed", "Regeneration",
       "MagicFind", "ExperienceBonus", "GoldFind", "ResourceFind", "FollowerCharm"
   };
   
   // Shuffle the stats array to randomize selection
   for (int32 i = 0; i < PossibleStats.Num(); i++)
   {
       int32 SwapIndex = FMath::RandRange(i, PossibleStats.Num() - 1);
       if (i != SwapIndex)
       {
           PossibleStats.Swap(i, SwapIndex);
       }
   }
   
   // Add the selected stats
   for (int32 i = 0; i < NumStats; i++)
   {
       FString StatName = PossibleStats[i];
       float StatValue = 0.0f;
       
       // Calculate stat value based on stat type and rarity
       if (i < 5) // Primary attributes
       {
           StatValue = FMath::RoundToInt(PlayerLevel * 0.3f * (1.0f + 0.5f * static_cast<int32>(Rarity)));
       }
       else if (i < 10) // Secondary stats
       {
           if (StatName == "CritChance" || StatName == "MoveSpeed" || StatName == "AttackSpeed")
           {
               StatValue = 3.0f + (2.0f * static_cast<int32>(Rarity));
           }
           else
           {
               StatValue = 5.0f + (5.0f * static_cast<int32>(Rarity));
           }
       }
       else // Special bonuses
       {
           StatValue = 5.0f + (5.0f * static_cast<int32>(Rarity));
       }
       
       // Add some randomness to the stat value
       StatValue *= FMath::RandRange(0.9f, 1.1f);
       
       Item.Stats.Add(StatName, StatValue);
   }
   
   // Special effects for legendary and artifact accessories
   if (Rarity >= EResourceRarity::Legendary)
   {
       TArray<FString> SpecialEffects = {
           "AuraOfPower", "PassiveHealing", "ElementalImmunity", "SecondWind", "ShadowStep",
           "TimeWarp", "ManaShield", "BerserkRage", "ElementalEmpowerment", "SoulLink"
       };
       
       FString SpecialEffect = SpecialEffects[FMath::RandRange(0, SpecialEffects.Num() - 1)];
       Item.Stats.Add(SpecialEffect, 1.0f);
   }
}

void UProceduralGenerator::GenerateConsumableItem(FLootItem& Item, int32 PlayerLevel, EResourceRarity Rarity)
{
   // Arrays of consumable prefixes by rarity
   TArray<TArray<FString>> RarityPrefixes = {
       // Common
       {"Minor", "Small", "Basic", "Common", "Simple"},
       // Uncommon
       {"Moderate", "Standard", "Quality", "Enhanced", "Refined"},
       // Rare
       {"Major", "Superior", "Potent", "Exceptional", "Concentrated"},
       // Legendary
       {"Grand", "Master", "Mythical", "Legendary", "Supreme"},
       // Artifact
       {"Divine", "Eternal", "Transcendent", "Cosmic", "Ultimate"}
   };
   
   // Consumable types
   TArray<FString> ConsumableTypes = {
       "Potion", "Elixir", "Tonic", "Draught", "Philter", 
       "Brew", "Extract", "Solution", "Concoction", "Essence"
   };
   
   // Consumable effects
   TArray<FString> ConsumableEffects = {
       "Healing", "Mana", "Strength", "Dexterity", "Intelligence",
       "Wisdom", "Constitution", "Resistance", "Swiftness", "Invisibility",
       "Night Vision", "Water Breathing", "Levitation", "Fire Resistance", "Antidote"
   };
   
   // Select components based on rarity
   FString Prefix = "";
   if (static_cast<int32>(Rarity) < RarityPrefixes.Num())
   {
       const TArray<FString>& AvailablePrefixes = RarityPrefixes[static_cast<int32>(Rarity)];
       Prefix = AvailablePrefixes[FMath::RandRange(0, AvailablePrefixes.Num() - 1)];
   }
   
   // Select consumable type
   FString ConsumableType = ConsumableTypes[FMath::RandRange(0, ConsumableTypes.Num() - 1)];
   
   // Select effect
   FString Effect = ConsumableEffects[FMath::RandRange(0, ConsumableEffects.Num() - 1)];
   
   // Construct name based on rarity
   if (Rarity >= EResourceRarity::Legendary)
   {
       // Legendary and Artifact consumables might have unique names
       TArray<FString> UniqueNames = {
           "Phoenix Tears", "Elixir of Immortality", "Draught of the Titans",
           "Essence of Eternity", "Starlight Extract", "Liquid Courage",
           "Philosopher's Tincture", "Dragon's Blood", "Ambrosia", "Panacea"
       };
       
       Item.Name = UniqueNames[FMath::RandRange(0, UniqueNames.Num() - 1)];
   }
   else
   {
       // Standard naming format
       Item.Name = Prefix + " " + ConsumableType + " of " + Effect;
   }
   
   // Generate description
   TArray<FString> DescriptionTemplates = {
       "A %s that restores %s when consumed.",
       "This %s provides a %s boost to %s for a limited time.",
       "A %s alchemical mixture that %s the consumer with %s effects.",
       "When consumed, this %s offers %s protection against harm.",
       "The %s within this %s temporarily enhances %s abilities."
   };
   
   TArray<FString> StrengthDescriptors = {"mild", "moderate", "significant", "powerful", "overwhelming"};
   TArray<FString> ActionDescriptors = {"infuses", "empowers", "imbues", "blesses", "transforms"};
   TArray<FString> ProtectionDescriptors = {"limited", "moderate", "substantial", "exceptional", "divine"};
   
   int32 TemplateIndex = FMath::RandRange(0, DescriptionTemplates.Num() - 1);
   FString DescTemplate = DescriptionTemplates[TemplateIndex];
   
   // Fill in template based on which one was selected
   switch (TemplateIndex)
   {
       case 0:
           Item.Description = FString::Printf(*DescTemplate, 
               *ConsumableType.ToLower(),
               Effect == "Healing" ? "health" : (Effect == "Mana" ? "mana" : *Effect.ToLower() + " energy"));
           break;
           
       case 1:
           Item.Description = FString::Printf(*DescTemplate, 
               *ConsumableType.ToLower(),
               *StrengthDescriptors[FMath::Min(static_cast<int32>(Rarity), StrengthDescriptors.Num() - 1)],
               *Effect.ToLower());
           break;
           
       case 2:
           Item.Description = FString::Printf(*DescTemplate, 
               *StrengthDescriptors[FMath::Min(static_cast<int32>(Rarity), StrengthDescriptors.Num() - 1)],
               *ActionDescriptors[FMath::Min(static_cast<int32>(Rarity), ActionDescriptors.Num() - 1)],
               *Effect.ToLower());
           break;
           
       case 3:
           Item.Description = FString::Printf(*DescTemplate, 
               *ConsumableType.ToLower(),
               *ProtectionDescriptors[FMath::Min(static_cast<int32>(Rarity), ProtectionDescriptors.Num() - 1)]);
           break;
           
       case 4:
           Item.Description = FString::Printf(*DescTemplate, 
               *Effect.ToLower() + " essence",
               *ConsumableType.ToLower(),
               *Effect.ToLower());
           break;
   }
   
   // Add stats based on effect and rarity
   float EffectPower = PlayerLevel * (1.0f + static_cast<int32>(Rarity));
   float EffectDuration = 30.0f * (1.0f + 0.5f * static_cast<int32>(Rarity));
   
   // Add effect power
   if (Effect == "Healing" || Effect == "Mana")
   {
       // Instant effects
       Item.Stats.Add(Effect + "Amount", EffectPower * 5.0f);
   }
   else
   {
       // Buff effects
       Item.Stats.Add(Effect + "Bonus", EffectPower);
       Item.Stats.Add("Duration", EffectDuration);
   }
   
   // Higher rarity consumables might have additional effects
   if (Rarity >= EResourceRarity::Rare)
   {
       // Add a secondary effect
       TArray<FString> SecondaryEffects = ConsumableEffects;
       SecondaryEffects.Remove(Effect); // Don't duplicate the primary effect
       
       if (SecondaryEffects.Num() > 0)
       {
           FString SecondaryEffect = SecondaryEffects[FMath::RandRange(0, SecondaryEffects.Num() - 1)];
           float SecondaryPower = EffectPower * 0.5f;
           
           if (SecondaryEffect == "Healing" || SecondaryEffect == "Mana")
           {
               Item.Stats.Add(SecondaryEffect + "Amount", SecondaryPower * 5.0f);
           }
           else
           {
               Item.Stats.Add(SecondaryEffect + "Bonus", SecondaryPower);
           }
       }
   }
   
   // Legendary and artifact consumables might have unique effects
   if (Rarity >= EResourceRarity::Legendary)
   {
       TArray<FString> SpecialEffects = {
           "Invulnerability", "TimeFreeze", "MassRevive", "TeleportToSafety", "SummonAlly"
       };
       
       FString SpecialEffect = SpecialEffects[FMath::RandRange(0, SpecialEffects.Num() - 1)];
       float SpecialDuration = FMath::Min(10.0f * static_cast<int32>(Rarity - EResourceRarity::Legendary + 1), 30.0f);
       
       Item.Stats.Add(SpecialEffect, 1.0f);
       Item.Stats.Add(SpecialEffect + "Duration", SpecialDuration);
   }
}

void UProceduralGenerator::GenerateTrophyItem(FLootItem& Item, int32 PlayerLevel, EResourceRarity Rarity)
{
   // Arrays of trophy sources by rarity
   TArray<TArray<FString>> RaritySources = {
       // Common
       {"Wolf", "Boar", "Bandit", "Goblin", "Snake"},
       // Uncommon
       {"Bear", "Troll", "Cultist", "Giant Spider", "Wyvern"},
       // Rare
       {"Dire Wolf", "Stone Golem", "Dark Knight", "Wendigo", "Manticore"},
       // Legendary
       {"Ancient Dragon", "Demon Lord", "Lich King", "Behemoth", "Kraken"},
       // Artifact
       {"Elder God", "Primordial Titan", "Cosmic Horror", "World Serpent", "Phoenix"}
   };
   
   // Trophy types
   TArray<FString> TrophyTypes = {
       "Head", "Claw", "Fang", "Horn", "Pelt", 
       "Scale", "Heart", "Eye", "Skull", "Talon"
   };
   
   // Select components based on rarity
   FString Source = "";
   if (static_cast<int32>(Rarity) < RaritySources.Num())
   {
       const TArray<FString>& AvailableSources = RaritySources[static_cast<int32>(Rarity)];
       Source = AvailableSources[FMath::RandRange(0, AvailableSources.Num() - 1)];
   }
   
   // Select trophy type
   FString TrophyType = TrophyTypes[FMath::RandRange(0, TrophyTypes.Num() - 1)];
   
   // Construct name based on rarity
   if (Rarity >= EResourceRarity::Legendary)
   {
       // Some legendary and artifact trophies might have unique names
       if (FMath::RandBool())
       {
           TArray<FString> UniqueNames = {
               "Heart of the Mountain", "Eye of the Storm", "Crown of Thorns",
               "Breath of Winter", "Essence of Flame", "Tear of the Ocean",
               "Voice of Thunder", "Shadow of Death", "Light of Creation", "Key of Eternity"
           };
           
           Item.Name = UniqueNames[FMath::RandRange(0, UniqueNames.Num() - 1)];
       }
       else
       {
           Item.Name = Source + "'s " + TrophyType;
       }
   }
   else
   {
       // Standard naming format
       Item.Name = Source + " " + TrophyType;
   }
   
   // Generate description
   TArray<FString> DescriptionTemplates = {
       "A trophy collected from a %s, proving your combat prowess.",
       "This %s from a %s is a symbol of your victory in battle.",
       "A %s %s taken as a trophy after a difficult battle.",
       "This rare %s was claimed from a %s you defeated.",
       "The %s of a %s, a prized trophy for any collector."
   };
   
   int32 TemplateIndex = FMath::RandRange(0, DescriptionTemplates.Num() - 1);
   FString DescTemplate = DescriptionTemplates[TemplateIndex];
   
   // Fill in template based on which one was selected
   switch (TemplateIndex)
   {
       case 0:
       case 3:
           Item.Description = FString::Printf(*DescTemplate, *Source.ToLower());
           break;
           
       case 1:
       case 4:
           Item.Description = FString::Printf(*DescTemplate, 
               *TrophyType.ToLower(),
               *Source.ToLower());
           break;
           
       case 2:
           Item.Description = FString::Printf(*DescTemplate, 
               *Source.ToLower(),
               *TrophyType.ToLower());
           break;
   }
   
   // Add lore to higher rarity trophies
   if (Rarity >= EResourceRarity::Rare)
   {
       TArray<FString> LoreAdditions = {
           " It radiates a strange energy.",
           " Legends speak of its mystical properties.",
           " Some believe it brings good fortune to its owner.",
           " The material seems otherworldly upon close inspection.",
           " It sometimes moves slightly of its own accord..."
       };
       
       Item.Description += LoreAdditions[FMath::RandRange(0, LoreAdditions.Num() - 1)];
   }
   
   // Add kingdom display value stat
   float DisplayValue = PlayerLevel * 2.0f * (1.0f + static_cast<int32>(Rarity));
   Item.Stats.Add("KingdomPrestige", DisplayValue);
   
   // Add kingdom reputation when displayed
   float ReputationBonus = PlayerLevel * 0.5f * (1.0f + static_cast<int32>(Rarity));
   Item.Stats.Add("CombatRenownBonus", ReputationBonus);
   
   // Higher rarity trophies might attract specific followers
   if (Rarity >= EResourceRarity::Rare)
   {
       TArray<FString> FollowerTypes = {
           "Hunter", "Warrior", "Mage", "Scholar", "Merchant"
       };
       
       FString FollowerType = FollowerTypes[FMath::RandRange(0, FollowerTypes.Num() - 1)];
       float AttractionBonus = 5.0f * static_cast<int32>(Rarity - EResourceRarity::Rare + 1);
       
       Item.Stats.Add(FollowerType + "AttractionBonus", AttractionBonus);
   }
   
   // Legendary and artifact trophies might have unique kingdom effects
   if (Rarity >= EResourceRarity::Legendary)
   {
       TArray<FString> KingdomEffects = {
           "DefenseBonus", "ResourceGenerationBonus", "BuildingCostReduction", 
           "FollowerHappinessBonus", "TaxIncomeBonus"
       };
       
       FString KingdomEffect = KingdomEffects[FMath::RandRange(0, KingdomEffects.Num() - 1)];
       float EffectValue = 5.0f + (5.0f * static_cast<int32>(Rarity - EResourceRarity::Legendary));
       
       Item.Stats.Add(KingdomEffect, EffectValue);
   }
}