// KingdomManager.cpp
#include "KingdomManager.h"
#include "OathGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

// Constructor
AKingdomManager::AKingdomManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.0f; // Check every second
    
    // Default values
    KingdomName = TEXT("New Kingdom");
    CurrentTier = EKingdomTier::Camp;
    KingdomAlignment = 0.0f;
    TaxRate = 5;
    DailyIncome = 0.0f;
    DayTimer = 0.0f;
    DaysElapsed = 0.0f;
    DayLength = 600.0f; // 10 minutes = 1 in-game day
    
    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
}

// Called when the game starts or when spawned
void AKingdomManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize kingdom
    CalculateDailyIncome();
    UpdateKingdomTier();
    
    // Set up the random event timer
    float RandomEventCheckInterval = 300.0f; // Check for random events every 5 minutes
    GetWorldTimerManager().SetTimer(RandomEventTimerHandle, this, &AKingdomManager::CheckForRandomEvent, RandomEventCheckInterval, true);
}

// Called every frame
void AKingdomManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update day timer
    DayTimer += DeltaTime;
    
    // Check if a day has passed
    if (DayTimer >= DayLength)
    {
        DayTimer = FMath::Fmod(DayTimer, DayLength);
        DaysElapsed += 1.0f;
        ProcessDailyUpdate();
    }
}

// Recruit a new follower to the kingdom
bool AKingdomManager::RecruitFollower(FFollowerData Follower)
{
    // Check if we can add more followers
    if (Followers.Num() >= GetMaxFollowers())
    {
        UE_LOG(LogTemp, Warning, TEXT("Kingdom %s cannot recruit more followers (max: %d)"), *KingdomName, GetMaxFollowers());
        return false;
    }
    
    // Add the follower
    Followers.Add(Follower);
    
    // Update kingdom statistics
    CalculateDailyIncome();
    
    // Broadcast event
    OnFollowerRecruited.Broadcast(Follower);
    
    UE_LOG(LogTemp, Log, TEXT("Kingdom %s recruited follower %s"), *KingdomName, *Follower.Name);
    return true;
}

// Build a new structure in the kingdom
bool AKingdomManager::ConstructBuilding(FBuildingData Building, FVector Location)
{
    // Check if we can add more buildings
    if (Buildings.Num() >= GetMaxBuildings())
    {
        UE_LOG(LogTemp, Warning, TEXT("Kingdom %s cannot construct more buildings (max: %d)"), *KingdomName, GetMaxBuildings());
        return false;
    }
    
    // Check if the kingdom tier is high enough
    if (!Building.RequiredKingdomTiers.Contains(GetKingdomTierAsString()))
    {
        UE_LOG(LogTemp, Warning, TEXT("Kingdom %s cannot construct %s (required tier not met)"), *KingdomName, *Building.Name);
        return false;
    }
    
    // Check if we have an actor reference to spawn the physical building
    AOathGameMode* GameMode = Cast<AOathGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
    if (!GameMode)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get OathGameMode for building construction"));
        return false;
    }
    
    // Attempt to spend resources and gold
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PlayerController)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get PlayerController for resource deduction"));
        return false;
    }
    
    AOathCharacter* PlayerCharacter = Cast<AOathCharacter>(PlayerController->GetPawn());
    if (!PlayerCharacter || !PlayerCharacter->InventoryComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get player's inventory component"));
        return false;
    }
    
    // Check if player has enough gold
    if (!PlayerCharacter->InventoryComponent->SpendGold(Building.GoldConstructionCost, false))
    {
        UE_LOG(LogTemp, Warning, TEXT("Not enough gold to construct %s"), *Building.Name);
        return false;
    }
    
    // Check if player has enough materials
    bool hasAllMaterials = true;
    for (const TPair<FResourceData, int32>& MaterialCost : Building.ConstructionCost)
    {
        if (!PlayerCharacter->InventoryComponent->HasEnoughMaterial(MaterialCost.Key, MaterialCost.Value))
        {
            hasAllMaterials = false;
            UE_LOG(LogTemp, Warning, TEXT("Not enough %s to construct %s"), *MaterialCost.Key.Name, *Building.Name);
            break;
        }
    }
    
    // If missing materials, refund gold and return
    if (!hasAllMaterials)
    {
        PlayerCharacter->InventoryComponent->AddGold(Building.GoldConstructionCost, false);
        return false;
    }
    
    // Spend the materials
    for (const TPair<FResourceData, int32>& MaterialCost : Building.ConstructionCost)
    {
        PlayerCharacter->InventoryComponent->SpendMaterial(MaterialCost.Key, MaterialCost.Value, false);
    }
    
    // Finalize the transaction
    PlayerCharacter->InventoryComponent->NotifyInventoryChanged(TEXT("Construction"));
    
    // Spawn the building actor if the mesh is assigned
    AActor* BuildingActor = nullptr;
    if (Building.Mesh != nullptr)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
        
        TSubclassOf<AActor> BuildingClass = GameMode->GetBuildingClassForType(Building.Type);
        if (BuildingClass)
        {
            BuildingActor = GetWorld()->SpawnActor<AActor>(BuildingClass, Location, FRotator::ZeroRotator, SpawnParams);
            
            if (BuildingActor)
            {
                UStaticMeshComponent* MeshComponent = BuildingActor->FindComponentByClass<UStaticMeshComponent>();
                if (MeshComponent)
                {
                    MeshComponent->SetStaticMesh(Building.Mesh);
                }
                
                // Store the building reference
                UBuildingComponent* BuildingComponent = BuildingActor->FindComponentByClass<UBuildingComponent>();
                if (BuildingComponent)
                {
                    BuildingComponent->SetBuildingData(Building);
                }
            }
        }
    }
    
    // Add the building to our list
    Building.Location = Location;
    Building.BuildingActor = BuildingActor;
    Buildings.Add(Building);
    
    // Update kingdom statistics
    CalculateDailyIncome();
    
    // Broadcast event
    OnBuildingConstructed.Broadcast(Building);
    
    UE_LOG(LogTemp, Log, TEXT("Kingdom %s constructed building %s at location %s"), 
           *KingdomName, *Building.Name, *Location.ToString());
    
    return true;
}

// Calculate the daily income from all sources
void AKingdomManager::CalculateDailyIncome()
{
    float goldIncome = 0.0f;
    TMap<FString, float> resourceIncome;
    
    // Calculate income from followers
    for (const FFollowerData& Follower : Followers)
    {
        // Add gold contribution
        goldIncome += Follower.DailyGoldContribution * Follower.Productivity;
        
        // Add resource contributions
        for (const TPair<FString, float>& Resource : Follower.DailyResourceContribution)
        {
            float currentAmount = resourceIncome.Contains(Resource.Key) ? resourceIncome[Resource.Key] : 0.0f;
            resourceIncome.Add(Resource.Key, currentAmount + (Resource.Value * Follower.Productivity));
        }
    }
    
    // Calculate income from buildings
    for (const FBuildingData& Building : Buildings)
    {
        // Add gold contribution
        goldIncome += Building.DailyGoldContribution;
        
        // Add resource contributions
        for (const TPair<FString, float>& Resource : Building.DailyResourceContribution)
        {
            float currentAmount = resourceIncome.Contains(Resource.Key) ? resourceIncome[Resource.Key] : 0.0f;
            resourceIncome.Add(Resource.Key, currentAmount + Resource.Value);
        }
    }
    
    // Apply tax rate to gold income
    goldIncome *= (static_cast<float>(TaxRate) / 100.0f);
    
    // Update the kingdom's daily income values
    DailyIncome = goldIncome;
    DailyResourceIncome = resourceIncome;
    
    UE_LOG(LogTemp, Log, TEXT("Kingdom %s daily income: %f gold"), *KingdomName, DailyIncome);
}

// Update the kingdom tier based on reputation and size
void AKingdomManager::UpdateKingdomTier()
{
    // Get the player's reputation component
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PlayerController)
    {
        return;
    }
    
    AOathCharacter* PlayerCharacter = Cast<AOathCharacter>(PlayerController->GetPawn());
    if (!PlayerCharacter || !PlayerCharacter->ReputationComponent)
    {
        return;
    }
    
    // Get the current kingdom reputation
    float kingdomReputation = PlayerCharacter->ReputationComponent->KingdomReputation;
    
    // Define tier thresholds
    const float VillageThreshold = 1000.0f;
    const float TownThreshold = 5000.0f;
    const float CityThreshold = 15000.0f;
    const float KingdomThreshold = 50000.0f;
    
    // Determine new tier
    EKingdomTier newTier = EKingdomTier::Camp;
    
    if (kingdomReputation >= KingdomThreshold)
    {
        newTier = EKingdomTier::Kingdom;
    }
    else if (kingdomReputation >= CityThreshold)
    {
        newTier = EKingdomTier::City;
    }
    else if (kingdomReputation >= TownThreshold)
    {
        newTier = EKingdomTier::Town;
    }
    else if (kingdomReputation >= VillageThreshold)
    {
        newTier = EKingdomTier::Village;
    }
    
    // Only broadcast event if tier changed
    if (newTier != CurrentTier)
    {
        EKingdomTier oldTier = CurrentTier;
        CurrentTier = newTier;
        
        // Broadcast event
        OnKingdomTierChanged.Broadcast(oldTier, CurrentTier);
        
        UE_LOG(LogTemp, Log, TEXT("Kingdom %s tier changed from %s to %s"), 
               *KingdomName, *GetEnumValueAsString<EKingdomTier>("EKingdomTier", oldTier), 
               *GetEnumValueAsString<EKingdomTier>("EKingdomTier", CurrentTier));
    }
}

// Process the daily update for the kingdom
void AKingdomManager::ProcessDailyUpdate()
{
    // Get the player character
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PlayerController)
    {
        return;
    }
    
    AOathCharacter* PlayerCharacter = Cast<AOathCharacter>(PlayerController->GetPawn());
    if (!PlayerCharacter || !PlayerCharacter->InventoryComponent)
    {
        return;
    }
    
    // Add daily gold income
    PlayerCharacter->InventoryComponent->AddGold(FMath::RoundToInt(DailyIncome));
    
    // Add daily resource income
    for (const TPair<FString, float>& Resource : DailyResourceIncome)
    {
        // Find the matching resource data
        FResourceData resourceData;
        bool resourceFound = false;
        
        // This would be better handled with a centralized resource manager
        AOathGameMode* GameMode = Cast<AOathGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
        if (GameMode && GameMode->ResourceManager)
        {
            resourceData = GameMode->ResourceManager->GetResourceByName(Resource.Key, resourceFound);
        }
        
        if (resourceFound)
        {
            int32 resourceAmount = FMath::RoundToInt(Resource.Value);
            if (resourceAmount > 0)
            {
                PlayerCharacter->InventoryComponent->AddMaterial(resourceData, resourceAmount);
            }
        }
    }
    
    // Update follower happiness and productivity
    UpdateFollowerMorale();
    
    // Random chance for followers to leave if unhappy
    CheckUnhappyFollowers();
    
    // Random chance for new followers to be interested
    CheckForNewFollowerInterest();
    
    // Update kingdom tier
    UpdateKingdomTier();
    
    // Broadcast daily update event
    OnDailyUpdate.Broadcast(DaysElapsed);
    
    UE_LOG(LogTemp, Log, TEXT("Kingdom %s processed day %d update"), *KingdomName, FMath::RoundToInt(DaysElapsed));
}

// Trigger a random event in the kingdom
void AKingdomManager::CheckForRandomEvent()
{
    // Random chance to trigger an event (20% chance)
    if (FMath::FRand() <= 0.2f)
    {
        TriggerRandomEvent();
    }
}

void AKingdomManager::TriggerRandomEvent()
{
    // No events available
    if (PossibleEvents.Num() == 0)
    {
        return;
    }
    
    // Filter events based on current kingdom tier and alignment
    TArray<FKingdomEvent> eligibleEvents;
    for (const FKingdomEvent& Event : PossibleEvents)
    {
        if (Event.MinimumTier <= CurrentTier && Event.MaximumTier >= CurrentTier)
        {
            // Check alignment requirements
            if ((Event.RequiresPositiveAlignment && KingdomAlignment > 0.2f) ||
                (Event.RequiresNegativeAlignment && KingdomAlignment < -0.2f) ||
                (!Event.RequiresPositiveAlignment && !Event.RequiresNegativeAlignment))
            {
                eligibleEvents.Add(Event);
            }
        }
    }
    
    // If no eligible events, return
    if (eligibleEvents.Num() == 0)
    {
        return;
    }
    
    // Choose a random event
    int32 eventIndex = FMath::RandRange(0, eligibleEvents.Num() - 1);
    FKingdomEvent selectedEvent = eligibleEvents[eventIndex];
    
    // Apply event effects
    KingdomAlignment += selectedEvent.AlignmentEffect;
    
    // Get player character for resource effects
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PlayerController)
    {
        AOathCharacter* PlayerCharacter = Cast<AOathCharacter>(PlayerController->GetPawn());
        if (PlayerCharacter && PlayerCharacter->InventoryComponent)
        {
            // Apply gold effect
            if (selectedEvent.GoldEffect != 0)
            {
                if (selectedEvent.GoldEffect > 0)
                {
                    PlayerCharacter->InventoryComponent->AddGold(selectedEvent.GoldEffect);
                }
                else
                {
                    // Don't go below zero
                    int32 currentGold = PlayerCharacter->InventoryComponent->Gold;
                    int32 goldToRemove = FMath::Min(currentGold, FMath::Abs(selectedEvent.GoldEffect));
                    PlayerCharacter->InventoryComponent->SpendGold(goldToRemove);
                }
            }
            
            // Apply resource effects (would be implemented in a more robust system)
        }
        
        // Apply reputation effects
        if (PlayerCharacter && PlayerCharacter->ReputationComponent)
        {
            if (selectedEvent.CombatRenownEffect != 0.0f)
            {
                PlayerCharacter->ReputationComponent->GainCombatRenown(selectedEvent.CombatRenownEffect);
            }
            
            if (selectedEvent.QuestRenownEffect != 0.0f)
            {
                PlayerCharacter->ReputationComponent->GainQuestRenown(selectedEvent.QuestRenownEffect);
            }
            
            if (selectedEvent.KingdomReputationEffect != 0.0f)
            {
                PlayerCharacter->ReputationComponent->GainKingdomReputation(selectedEvent.KingdomReputationEffect);
            }
        }
    }
    
    // Handle follower effects
    if (selectedEvent.FollowerHappinessEffect != 0.0f)
    {
        for (FFollowerData& Follower : Followers)
        {
            Follower.Happiness = FMath::Clamp(Follower.Happiness + selectedEvent.FollowerHappinessEffect, 0.0f, 1.0f);
        }
    }
    
    // Handle building damage if applicable
    if (selectedEvent.ChanceToDestroyBuilding > 0.0f)
    {
        for (int32 i = Buildings.Num() - 1; i >= 0; --i)
        {
            if (FMath::FRand() <= selectedEvent.ChanceToDestroyBuilding)
            {
                FBuildingData destroyedBuilding = Buildings[i];
                
                // Remove the building from the list
                Buildings.RemoveAt(i);
                
                // Destroy the actor if it exists
                if (destroyedBuilding.BuildingActor)
                {
                    destroyedBuilding.BuildingActor->Destroy();
                }
                
                // Broadcast building destroyed event
                OnBuildingDestroyed.Broadcast(destroyedBuilding, selectedEvent.Name);
            }
        }
    }
    
    // Broadcast event
    OnKingdomEventTriggered.Broadcast(selectedEvent);
    
    UE_LOG(LogTemp, Log, TEXT("Kingdom %s experienced event: %s"), *KingdomName, *selectedEvent.Name);
}

// Get the maximum number of followers based on kingdom tier
int32 AKingdomManager::GetMaxFollowers() const
{
    switch (CurrentTier)
    {
        case EKingdomTier::Camp:
            return 5;
        case EKingdomTier::Village:
            return 15;
        case EKingdomTier::Town:
            return 30;
        case EKingdomTier::City:
            return 60;
        case EKingdomTier::Kingdom:
            return 100;
        default:
            return 5;
    }
}

// Get the maximum number of buildings based on kingdom tier
int32 AKingdomManager::GetMaxBuildings() const
{
    switch (CurrentTier)
    {
        case EKingdomTier::Camp:
            return 3;
        case EKingdomTier::Village:
            return 10;
        case EKingdomTier::Town:
            return 25;
        case EKingdomTier::City:
            return 50;
        case EKingdomTier::Kingdom:
            return 100;
        default:
            return 3;
    }
}

// Update follower morale based on kingdom conditions
void AKingdomManager::UpdateFollowerMorale()
{
    for (FFollowerData& Follower : Followers)
    {
        // Base daily happiness change (slightly negative to require attention)
        float happinessChange = -0.01f;
        
        // Tax rate impact on happiness
        if (TaxRate > 10)
        {
            happinessChange -= (TaxRate - 10) * 0.005f;
        }
        
        // Relationship impacts
        float relationshipFactor = 0.0f;
        int32 relationshipCount = 0;
        
        for (const TPair<FString, float>& Relationship : Follower.Relationships)
        {
            relationshipFactor += Relationship.Value;
            relationshipCount++;
        }
        
        if (relationshipCount > 0)
        {
            relationshipFactor /= relationshipCount;
            happinessChange += relationshipFactor * 0.02f;
        }
        
        // Kingdom alignment impact based on follower traits
        if (Follower.Traits.Contains("Honorable") && KingdomAlignment > 0.3f)
        {
            happinessChange += 0.02f;
        }
        else if (Follower.Traits.Contains("Corrupt") && KingdomAlignment < -0.3f)
        {
            happinessChange += 0.02f;
        }
        
        // Apply the happiness change
        Follower.Happiness = FMath::Clamp(Follower.Happiness + happinessChange, 0.0f, 1.0f);
        
        // Update productivity based on happiness
        Follower.Productivity = 0.5f + (Follower.Happiness * 0.5f);
    }
}

// Check if any followers are too unhappy and might leave
void AKingdomManager::CheckUnhappyFollowers()
{
    for (int32 i = Followers.Num() - 1; i >= 0; --i)
    {
        if (Followers[i].Happiness < 0.2f)
        {
            // 5% daily chance to leave if very unhappy
            if (FMath::FRand() <= 0.05f)
            {
                FFollowerData departingFollower = Followers[i];
                Followers.RemoveAt(i);
                
                // Broadcast event
                OnFollowerLeft.Broadcast(departingFollower, TEXT("Unhappiness"));
                
                UE_LOG(LogTemp, Log, TEXT("Follower %s left kingdom %s due to unhappiness"), 
                       *departingFollower.Name, *KingdomName);
            }
        }
    }
}

// Check if new followers might be interested in joining
void AKingdomManager::CheckForNewFollowerInterest()
{
    // Get the player's reputation
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PlayerController)
    {
        return;
    }
    
    AOathCharacter* PlayerCharacter = Cast<AOathCharacter>(PlayerController->GetPawn());
    if (!PlayerCharacter || !PlayerCharacter->ReputationComponent)
    {
        return;
    }
    
    float combatRenown = PlayerCharacter->ReputationComponent->CombatRenown;
    float questRenown = PlayerCharacter->ReputationComponent->QuestRenown;
    
    // Base chance for a new follower to appear
    float baseChance = 0.1f; // 10% daily chance
    
    // Modify chance based on reputation
    float renownModifier = (combatRenown + questRenown) / 1000.0f;
    renownModifier = FMath::Clamp(renownModifier, 0.0f, 0.5f);
    
    float finalChance = baseChance + renownModifier;
    
    // Check if we're at max capacity
    if (Followers.Num() >= GetMaxFollowers())
    {
        finalChance *= 0.2f; // Dramatically reduce chance if near capacity
    }
    
    // Roll for new follower
    if (FMath::FRand() <= finalChance)
    {
        // Generate a new potential follower
        AOathGameMode* GameMode = Cast<AOathGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
        if (GameMode && GameMode->ProceduralGenerator)
        {
            // Determine follower level based on kingdom tier
            int32 minLevel = 1;
            int32 maxLevel = 1;
            
            switch (CurrentTier)
            {
                case EKingdomTier::Camp:
                    maxLevel = 2;
                    break;
                case EKingdomTier::Village:
                    minLevel = 1;
                    maxLevel = 3;
                    break;
                case EKingdomTier::Town:
                    minLevel = 2;
                    maxLevel = 5;
                    break;
                case EKingdomTier::City:
                    minLevel = 3;
                    maxLevel = 7;
                    break;
                case EKingdomTier::Kingdom:
                    minLevel = 5;
                    maxLevel = 10;
                    break;
            }
            
            // Determine if this could be a hero follower (rare)
            bool canBeHero = (FMath::FRand() <= 0.1f);
            
            // Generate the follower
            FFollowerData potentialFollower = GameMode->ProceduralGenerator->GenerateFollower(minLevel, maxLevel, canBeHero);
            
            // Broadcast event for potential recruitment
            OnPotentialFollowerAppeared.Broadcast(potentialFollower);
            
            UE_LOG(LogTemp, Log, TEXT("Potential follower %s (%s) appeared at kingdom %s"), 
                   *potentialFollower.Name, *GetEnumValueAsString<EFollowerProfession>("EFollowerProfession", potentialFollower.Profession), 
                   *KingdomName);
        }
    }
}

// Convert kingdom tier enum to string
FString AKingdomManager::GetKingdomTierAsString() const
{
    return GetEnumValueAsString<EKingdomTier>("EKingdomTier", CurrentTier);
}

// Helper function to convert enum values to strings
template<typename TEnum>
FString AKingdomManager::GetEnumValueAsString(const FString& EnumName, TEnum Value)
{
    UEnum* Enum = FindObject<UEnum>(ANY_PACKAGE, *EnumName, true);
    if (!Enum)
    {
        return FString("Invalid");
    }
    return Enum->GetNameStringByValue(static_cast<int64>(Value));
}