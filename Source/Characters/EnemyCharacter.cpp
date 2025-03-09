// EnemyCharacter.cpp
#include "EnemyCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "OathCharacter.h"
#include "AIController.h"
#include "Engine/World.h"
#include "TimerManager.h"

AEnemyCharacter::AEnemyCharacter()
{
    // Set this character to call Tick() every frame
    PrimaryActorTick.bCanEverTick = true;
    
    // Create AI perception components
    PawnSensingComponent = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComponent"));
    PawnSensingComponent->SetPeripheralVisionAngle(60.0f);
    PawnSensingComponent->SightRadius = 1000.0f;
    PawnSensingComponent->HearingThreshold = 600.0f;
    PawnSensingComponent->LOSHearingThreshold = 1200.0f;
    
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    
    // Create behavior tree components
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    
    // Set default values
    EnemyName = TEXT("Enemy");
    EnemyType = EEnemyType::Regular;
    Level = 1;
    MaxHealth = 100.0f;
    CurrentHealth = MaxHealth;
    AttackPower = 10.0f;
    AttackRange = 150.0f;
    DetectionRange = 800.0f;
    AggroRange = 600.0f;
    PatrolRadius = 500.0f;
    bIsDead = false;
    RenownValue = 10.0f;
    GoldRewardMin = 5;
    GoldRewardMax = 15;
    
    // Configure character movement
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 200.0f, 0.0f);
    
    // Don't rotate when the controller rotates
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;
    
    // Set team ID for AI perception system
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
    
    // Set default tags
    Tags.Add(FName("Enemy"));
}

void AEnemyCharacter::BeginPlay()
{
    Super::BeginPlay();
    
    // Store the initial location for returning after chasing
    HomeLocation = GetActorLocation();
    
    // Initialize health based on level and type
    MaxHealth = 50.0f + (Level * 25.0f);
    
    switch (EnemyType)
    {
        case EEnemyType::Elite:
            MaxHealth *= 2.0f;
            AttackPower *= 1.5f;
            break;
        case EEnemyType::Boss:
            MaxHealth *= 5.0f;
            AttackPower *= 2.0f;
            break;
        case EEnemyType::Notorious:
            MaxHealth *= 3.0f;
            AttackPower *= 1.75f;
            break;
        default:
            break;
    }
    
    CurrentHealth = MaxHealth;
    
    // Calculate renown value based on enemy level and type
    RenownValue = Level * 5.0f;
    
    switch (EnemyType)
    {
        case EEnemyType::Elite:
            RenownValue *= 2.0f;
            break;
        case EEnemyType::Boss:
            RenownValue *= 5.0f;
            break;
        case EEnemyType::Notorious:
            RenownValue *= 4.0f;
            break;
        default:
            break;
    }
    
    // Bind perception events
    if (PawnSensingComponent)
    {
        PawnSensingComponent->OnSeePawn.AddDynamic(this, &AEnemyCharacter::OnSeePlayer);
        PawnSensingComponent->OnHearNoise.AddDynamic(this, &AEnemyCharacter::OnHearNoise);
    }
}

void AEnemyCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // AI logic should mostly be handled by behavior tree,
    // but can add additional logic here if needed
}

float AEnemyCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    // Calculate final damage after resistances
    float FinalDamage = CalculateFinalDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    
    // Apply damage
    CurrentHealth = FMath::Max(CurrentHealth - FinalDamage, 0.0f);
    
    // Play hit reaction
    if (!bIsDead && FinalDamage > 0)
    {
        PlayHitReactionMontage();
    }
    
    // Alert AI that we've been hit
    AAIController* AIController = Cast<AAIController>(GetController());
    if (AIController && BlackboardComponent && DamageCauser)
    {
        // Set the attacker as the target in the blackboard
        BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), DamageCauser);
        
        // Set LastDamageTaken for AI decision making
        BlackboardComponent->SetValueAsFloat(TEXT("LastDamageTaken"), FinalDamage);
        
        // Set LastDamageTime
        BlackboardComponent->SetValueAsFloat(TEXT("LastDamageTime"), GetWorld()->GetTimeSeconds());
    }
    
    // Check if dead
    if (CurrentHealth <= 0 && !bIsDead)
    {
        Die();
    }
    
    return FinalDamage;
}

float AEnemyCharacter::CalculateFinalDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    // Base damage calculation
    float FinalDamage = DamageAmount;
    
    // Apply resistances based on enemy type
    switch (EnemyType)
    {
        case EEnemyType::Elite:
            FinalDamage *= 0.9f; // 10% resistance
            break;
        case EEnemyType::Boss:
            FinalDamage *= 0.75f; // 25% resistance
            break;
        case EEnemyType::Notorious:
            FinalDamage *= 0.8f; // 20% resistance
            break;
        default:
            break;
    }
    
    // Additional damage type calculations could go here
    
    // Ensure damage is at least 1
    FinalDamage = FMath::Max(FinalDamage, 1.0f);
    
    return FinalDamage;
}

void AEnemyCharacter::Die()
{
    // Mark as dead
    bIsDead = true;
    
    // Stop all movement and actions
    GetCharacterMovement()->StopMovementImmediately();
    GetController()->StopMovement();
    
    // Cancel any pending attacks
    GetWorldTimerManager().ClearTimer(AttackTimerHandle);
    
    // Play death animation
    PlayDeathMontage();
    
    // Disable collision
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    
    // Call blueprint event
    OnDeath();
    
    // Destroy after delay or leave corpse
    if (EnemyType != EEnemyType::Boss && EnemyType != EEnemyType::Notorious)
    {
        // Regular enemies despawn after a delay
        SetLifeSpan(10.0f);
    }
    else
    {
        // Bosses and notorious enemies leave corpses for looting
        // They can be despawned elsewhere when looted
    }
}

void AEnemyCharacter::AttackTarget(AActor* Target)
{
    if (bIsDead || bIsAttacking || !Target)
    {
        return;
    }
    
    // Check if target is in range
    float DistanceToTarget = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
    if (DistanceToTarget > AttackRange)
    {
        return;
    }
    
    // Face the target
    FVector Direction = Target->GetActorLocation() - GetActorLocation();
    Direction.Z = 0.0f;
    FRotator NewRotation = Direction.Rotation();
    SetActorRotation(NewRotation);
    
    // Set attacking flag
    bIsAttacking = true;
    
    // Play attack animation
    PlayAttackMontage();
    
    // Apply damage after a delay (matching the animation)
    FTimerDelegate DamageDelegate;
    DamageDelegate.BindUFunction(this, FName("ApplyDamageToTarget"), Target, AttackPower);
    GetWorldTimerManager().SetTimer(AttackTimerHandle, DamageDelegate, 0.5f, false);
    
    // Reset attacking flag after attack animation
    float AttackDuration = AttackMontage ? AttackMontage->GetPlayLength() : 1.0f;
    FTimerHandle ResetAttackingHandle;
    FTimerDelegate ResetDelegate;
    ResetDelegate.BindLambda([this]() { bIsAttacking = false; });
    GetWorldTimerManager().SetTimer(ResetAttackingHandle, ResetDelegate, AttackDuration, false);
}

void AEnemyCharacter::OnSeePlayer(APawn* Pawn)
{
    if (bIsDead)
    {
        return;
    }
    
    // Check if this is actually a player
    AOathCharacter* PlayerCharacter = Cast<AOathCharacter>(Pawn);
    if (!PlayerCharacter || PlayerCharacter->IsDead())
    {
        return;
    }
    
    // Update blackboard
    AAIController* AIController = Cast<AAIController>(GetController());
    if (AIController && BlackboardComponent)
    {
        BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), Pawn);
        BlackboardComponent->SetValueAsVector(TEXT("LastKnownLocation"), Pawn->GetActorLocation());
        BlackboardComponent->SetValueAsFloat(TEXT("LastSeenTime"), GetWorld()->GetTimeSeconds());
    }
    
    // Call blueprint event
    OnSpotPlayer(Pawn);
}

void AEnemyCharacter::OnHearNoise(APawn* PawnInstigator, const FVector& Location, float Volume)
{
    if (bIsDead)
    {
        return;
    }
    
    // Check if this is actually a player
    AOathCharacter* PlayerCharacter = Cast<AOathCharacter>(PawnInstigator);
    if (!PlayerCharacter || PlayerCharacter->IsDead())
    {
        return;
    }
    
    // Update blackboard if the noise was loud enough
    if (Volume > 0.5f)
    {
        AAIController* AIController = Cast<AAIController>(GetController());
        if (AIController && BlackboardComponent)
        {
            BlackboardComponent->SetValueAsVector(TEXT("NoiseLocation"), Location);
            BlackboardComponent->SetValueAsFloat(TEXT("LastNoiseTime"), GetWorld()->GetTimeSeconds());
            BlackboardComponent->SetValueAsFloat(TEXT("NoiseVolume"), Volume);
        }
    }
}

void AEnemyCharacter::PlayAttackMontage()
{
    if (AttackMontage)
    {
        PlayAnimMontage(AttackMontage);
    }
}

void AEnemyCharacter::PlayHitReactionMontage()
{
    if (HitReactionMontage)
    {
        PlayAnimMontage(HitReactionMontage);
    }
}

void AEnemyCharacter::PlayDeathMontage()
{
    if (DeathMontage)
    {
        PlayAnimMontage(DeathMontage);
    }
}

TArray<FLootItem> AEnemyCharacter::GenerateLoot()
{
    TArray<FLootItem> GeneratedLoot;
    
    // Roll for each possible loot item
    for (const FLootTable& LootEntry : PossibleLoot)
    {
        float RollChance = FMath::FRand(); // 0.0 to 1.0
        
        if (RollChance <= LootEntry.DropChance)
        {
            // Item drops, determine quantity
            FLootItem DroppedItem = LootEntry.Item;
            
            // Determine quantity if applicable
            int32 Quantity = FMath::RandRange(LootEntry.MinQuantity, LootEntry.MaxQuantity);
            
            // Only add if quantity > 0
            if (Quantity > 0)
            {
                // For stackable items, we'd set the quantity
                // For now, just duplicate the entry
                for (int32 i = 0; i < Quantity; i++)
                {
                    GeneratedLoot.Add(DroppedItem);
                }
            }
        }
    }
    
    // Special handling for bosses and notorious enemies
    if (EnemyType == EEnemyType::Boss || EnemyType == EEnemyType::Notorious)
    {
        // Guarantee at least one rare item for Notorious enemies
        if (EnemyType == EEnemyType::Notorious && GeneratedLoot.Num() == 0)
        {
            // Find a rare item in the loot table
            for (const FLootTable& LootEntry : PossibleLoot)
            {
                if (LootEntry.Item.Rarity >= EResourceRarity::Rare)
                {
                    GeneratedLoot.Add(LootEntry.Item);
                    break;
                }
            }
        }
        
        // Guarantee at least one legendary item for Bosses
        if (EnemyType == EEnemyType::Boss && !ContainsLegendaryItem(GeneratedLoot))
        {
            // Find a legendary item in the loot table
            for (const FLootTable& LootEntry : PossibleLoot)
            {
                if (LootEntry.Item.Rarity >= EResourceRarity::Legendary)
                {
                    GeneratedLoot.Add(LootEntry.Item);
                    break;
                }
            }
        }
    }
    
    return GeneratedLoot;
}

bool AEnemyCharacter::ContainsLegendaryItem(const TArray<FLootItem>& Loot)
{
    for (const FLootItem& Item : Loot)
    {
        if (Item.Rarity >= EResourceRarity::Legendary)
        {
            return true;
        }
    }
    return false;
}

TMap<FResourceData, int32> AEnemyCharacter::GenerateMaterials()
{
    TMap<FResourceData, int32> GeneratedMaterials;
    
    // Roll for each possible material
    for (const FMaterialDrop& MaterialEntry : PossibleMaterials)
    {
        float RollChance = FMath::FRand(); // 0.0 to 1.0
        
        if (RollChance <= MaterialEntry.DropChance)
        {
            // Material drops, determine quantity
            int32 Quantity = FMath::RandRange(MaterialEntry.MinQuantity, MaterialEntry.MaxQuantity);
            
            // Only add if quantity > 0
            if (Quantity > 0)
            {
                // Add to the map or increment existing quantity
                if (GeneratedMaterials.Contains(MaterialEntry.Material))
                {
                    GeneratedMaterials[MaterialEntry.Material] += Quantity;
                }
                else
                {
                    GeneratedMaterials.Add(MaterialEntry.Material, Quantity);
                }
            }
        }
    }
    
    return GeneratedMaterials;
}

int32 AEnemyCharacter::GetGoldReward()
{
    // Base gold drop
    int32 GoldAmount = FMath::RandRange(GoldRewardMin, GoldRewardMax);
    
    // Modify based on enemy type
    switch (EnemyType)
    {
        case EEnemyType::Elite:
            GoldAmount *= 2;
            break;
        case EEnemyType::Boss:
            GoldAmount *= 5;
            break;
        case EEnemyType::Notorious:
            GoldAmount *= 3;
            break;
        default:
            break;
    }
    
    // Modify based on level
    GoldAmount += (Level - 1) * 5;
    
    return GoldAmount;
}

// Missing function from earlier
UFUNCTION()
void AEnemyCharacter::ApplyDamageToTarget(AActor* Target, float DamageAmount)
{
    if (Target && !bIsDead)
    {
        FDamageEvent DamageEvent;
        Target->TakeDamage(DamageAmount, DamageEvent, GetController(), this);
    }
}