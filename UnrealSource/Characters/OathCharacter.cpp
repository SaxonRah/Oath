// OathCharacter.cpp
#include "OathCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/World.h"
#include "OathGameMode.h"
#include "InventoryComponent.h"
#include "ReputationComponent.h"
#include "Kismet/GameplayStatics.h"

AOathCharacter::AOathCharacter()
{
    // Set this character to call Tick() every frame
    PrimaryActorTick.bCanEverTick = true;
    
    // Create a camera boom
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 300.0f;
    CameraBoom->bUsePawnControlRotation = true;
    
    // Create a follow camera
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;
    
    // Create reputation component
    ReputationComponent = CreateDefaultSubobject<UReputationComponent>(TEXT("ReputationComponent"));
    
    // Create inventory component
    InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
    
    // Set default values
    BaseAttackPower = 10.0f;
    CharacterClass = ECharacterClass::Warrior;
    KingdomVision = TEXT("Military Stronghold");
    
    // Configure character movement
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
    GetCharacterMovement()->JumpZVelocity = 600.f;
    GetCharacterMovement()->AirControl = 0.2f;
    
    // Don't rotate when the controller rotates
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;
}

void AOathCharacter::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize stats based on character class
    InitializeClassStats();
    // Initialize starting inventory based on character class
    InitializeStartingInventory();
    
    // Apply kingdom vision bonuses
    ApplyKingdomVisionBonuses();
}

void AOathCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update character status effects if any
    UpdateStatusEffects(DeltaTime);
}

void AOathCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    
    // Set up gameplay key bindings
    PlayerInputComponent->BindAxis("MoveForward", this, &AOathCharacter::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &AOathCharacter::MoveRight);
    
    PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
    PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
    
    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
    PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
    
    PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &AOathCharacter::BeginAttack);
    PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &AOathCharacter::Interact);
}

void AOathCharacter::MoveForward(float Value)
{
    if ((Controller != nullptr) && (Value != 0.0f))
    {
        // Find out which way is forward
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);
        
        // Get forward vector
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        AddMovementInput(Direction, Value);
    }
}

void AOathCharacter::MoveRight(float Value)
{
    if ((Controller != nullptr) && (Value != 0.0f))
    {
        // Find out which way is right
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);
        
        // Get right vector 
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        AddMovementInput(Direction, Value);
    }
}


void AOathCharacter::InitializeStartingInventory()
{
    // Give some starting gold
    InventoryComponent->AddGold(100);
    
    // Add basic materials based on class
    FResourceData Wood;
    Wood.Name = "Wood";
    Wood.Rarity = EResourceRarity::Common;
    Wood.BaseValue = 1;
    
    FResourceData Stone;
    Stone.Name = "Stone";
    Stone.Rarity = EResourceRarity::Common;
    Stone.BaseValue = 1;
    
    FResourceData Herb;
    Herb.Name = "Herb";
    Herb.Rarity = EResourceRarity::Common;
    Herb.BaseValue = 2;
    
    // Give different starting materials based on class
    switch (CharacterClass)
    {
        case ECharacterClass::Warrior:
            InventoryComponent->AddMaterial(Wood, 10);
            InventoryComponent->AddMaterial(Stone, 15);
            break;
            
        case ECharacterClass::Mage:
            InventoryComponent->AddMaterial(Herb, 20);
            InventoryComponent->AddMaterial(Wood, 5);
            break;
            
        case ECharacterClass::Rogue:
            InventoryComponent->AddMaterial(Wood, 8);
            InventoryComponent->AddMaterial(Stone, 8);
            InventoryComponent->AddMaterial(Herb, 8);
            break;
            
        case ECharacterClass::Ranger:
            InventoryComponent->AddMaterial(Wood, 15);
            InventoryComponent->AddMaterial(Herb, 10);
            break;
            
        default:
            InventoryComponent->AddMaterial(Wood, 10);
            InventoryComponent->AddMaterial(Stone, 10);
            break;
    }
    
    // Add a starting weapon
    FLootItem StartingWeapon;
    StartingWeapon.Name = GetStartingWeaponName();
    StartingWeapon.Rarity = EResourceRarity::Common;
    StartingWeapon.Value = 10;
    StartingWeapon.Description = TEXT("A simple weapon to start your journey.");
    StartingWeapon.Stats.Add(TEXT("Damage"), GetStartingWeaponDamage());
    
    InventoryComponent->AddItem(StartingWeapon);
}


FString AOathCharacter::GetStartingWeaponName() const
{
    switch (CharacterClass)
    {
        case ECharacterClass::Warrior:
            return TEXT("Rusty Sword");
        case ECharacterClass::Mage:
            return TEXT("Apprentice Staff");
        case ECharacterClass::Rogue:
            return TEXT("Dull Dagger");
        case ECharacterClass::Ranger:
            return TEXT("Simple Bow");
        default:
            return TEXT("Sturdy Stick");
    }
}

float AOathCharacter::GetStartingWeaponDamage() const
{
    switch (CharacterClass)
    {
        case ECharacterClass::Warrior:
            return 5.0f;
        case ECharacterClass::Mage:
            return 3.0f;
        case ECharacterClass::Rogue:
            return 4.0f;
        case ECharacterClass::Ranger:
            return 4.5f;
        default:
            return 3.0f;
    }
}

void AOathCharacter::HarvestResource(AResourceNode* ResourceNode)
{
    if (!ResourceNode)
    {
        return;
    }
    
    // Check if player has the appropriate tool
    if (!HasAppropriateHarvestingTool(ResourceNode->GetResourceType()))
    {
        // Notify player they need the right tool
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("You need the appropriate tool to harvest this resource!"));
        return;
    }
    
    // Get the resource data from the node
    FResourceData Resource = ResourceNode->GetResourceData();
    int32 Amount = ResourceNode->GetHarvestAmount();
    
    // Add to inventory
    InventoryComponent->AddMaterial(Resource, Amount);
    
    // Grant a small amount of renown for gathering resources
    ReputationComponent->GainCombatRenown(0.5f);
    
    // Notify the resource node it was harvested
    ResourceNode->OnHarvested();
    
    // Play harvest animation
    PlayHarvestAnimation(ResourceNode->GetResourceType());
}

bool AOathCharacter::HasAppropriateHarvestingTool(EResourceType ResourceType) const
{
    // Simple implementation - in a real game, you'd check inventory for specific tools
    // This is just a placeholder assuming the character always has basic tools
    return true;
}

void AOathCharacter::PlayHarvestAnimation(EResourceType ResourceType)
{
    // Play animation based on resource type
    // This would typically trigger a montage
    UAnimMontage* HarvestMontage = nullptr;
    
    switch (ResourceType)
    {
        case EResourceType::Wood:
            HarvestMontage = WoodcuttingMontage;
            break;
        case EResourceType::Stone:
            HarvestMontage = MiningMontage;
            break;
        case EResourceType::Herb:
            HarvestMontage = HerbGatheringMontage;
            break;
        default:
            HarvestMontage = BasicHarvestMontage;
            break;
    }
    
    if (HarvestMontage)
    {
        PlayAnimMontage(HarvestMontage);
    }
}

bool AOathCharacter::UseItem(FLootItem Item)
{
    // Check if the item exists in inventory
    int32 ItemIndex = -1;
    for (int32 i = 0; i < InventoryComponent->Items.Num(); i++)
    {
        if (InventoryComponent->Items[i].Name == Item.Name)
        {
            ItemIndex = i;
            break;
        }
    }
    
    if (ItemIndex == -1)
    {
        return false;
    }
    
    // Handle different item types
    if (Item.Name.Contains(TEXT("Potion")))
    {
        // Handle potion effects
        if (Item.Name.Contains(TEXT("Health")))
        {
            // Heal the player
            Health += 50.0f;
            if (Health > MaxHealth)
            {
                Health = MaxHealth;
            }
        }
        else if (Item.Name.Contains(TEXT("Strength")))
        {
            // Apply strength buff
            BaseAttackPower *= 1.5f;
            GetWorldTimerManager().SetTimer(StrengthBuffTimer, this, &AOathCharacter::EndStrengthBuff, 30.0f, false);
        }
        
        // Remove the item after use
        InventoryComponent->RemoveItem(Item);
        return true;
    }
    else if (Item.Name.Contains(TEXT("Map")))
    {
        // Reveal part of the map
        // This would typically call into your world/map system
        // For now, just log it
        UE_LOG(LogTemp, Display, TEXT("Map used! Area revealed."));
        
        // Maps might be reusable, so don't remove
        return true;
    }
    else if (Item.Name.Contains(TEXT("Blueprint")))
    {
        // Learn a new building blueprint
        // This would typically call into your building system
        FString BuildingName = Item.Name.Replace(TEXT("Blueprint: "), TEXT(""));
        UE_LOG(LogTemp, Display, TEXT("Blueprint learned: %s"), *BuildingName);
        
        // Remove the blueprint after learning
        InventoryComponent->RemoveItem(Item);
        return true;
    }
    
    // Default case if no specific handling
    UE_LOG(LogTemp, Display, TEXT("Used item: %s"), *Item.Name);
    return true;
}

void AOathCharacter::EndStrengthBuff()
{
    BaseAttackPower /= 1.5f;
}

void AOathCharacter::BeginAttack()
{
    // Find the nearest enemy within attack range
    AActor* Target = FindNearestEnemy();
    
    if (Target)
    {
        AttackEnemy(Target);
    }
    else
    {
        // Play attack animation anyway
        PlayAttackMontage();
    }
}

AActor* AOathCharacter::FindNearestEnemy()
{
    // Find all enemies within range
    TArray<AActor*> FoundEnemies;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemyCharacter::StaticClass(), FoundEnemies);
    
    float ClosestDistance = AttackRange;
    AActor* ClosestEnemy = nullptr;
    
    for (AActor* Enemy : FoundEnemies)
    {
        float Distance = FVector::Dist(GetActorLocation(), Enemy->GetActorLocation());
        if (Distance < ClosestDistance)
        {
            ClosestDistance = Distance;
            ClosestEnemy = Enemy;
        }
    }
    
    return ClosestEnemy;
}

void AOathCharacter::AttackEnemy(AActor* Target)
{
    if (!Target)
    {
        return;
    }
    
    // Calculate attack direction
    FVector AttackDirection = Target->GetActorLocation() - GetActorLocation();
    AttackDirection.Z = 0.0f;
    AttackDirection.Normalize();
    
    // Rotate character towards the target
    SetActorRotation(AttackDirection.Rotation());
    
    // Play attack animation
    PlayAttackMontage();
    
    // Apply damage
    float DamageAmount = CalculateAttackDamage();
    FDamageEvent DamageEvent;
    Target->TakeDamage(DamageAmount, DamageEvent, GetController(), this);
    
    // Check if the enemy died from this attack
    AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(Target);
    if (Enemy && Enemy->IsDead())
    {
        // Gain combat renown based on enemy strength
        float RenownGain = Enemy->GetRenownValue();
        ReputationComponent->GainCombatRenown(RenownGain);
        
        // Add loot to inventory
        TArray<FLootItem> Loot = Enemy->GenerateLoot();
        for (const FLootItem& Item : Loot)
        {
            InventoryComponent->AddItem(Item);
        }
        
        // Add materials
        TMap<FResourceData, int32> Materials = Enemy->GenerateMaterials();
        for (const TPair<FResourceData, int32>& Material : Materials)
        {
            InventoryComponent->AddMaterial(Material.Key, Material.Value);
        }
        
        // Add gold
        int32 GoldReward = Enemy->GetGoldReward();
        InventoryComponent->AddGold(GoldReward);
    }
}

float AOathCharacter::CalculateAttackDamage()
{
    // Base damage calculation
    float Damage = BaseAttackPower;
    
    // Add weapon damage if equipped
    if (EquippedWeapon)
    {
        Damage += EquippedWeapon->GetDamageValue();
    }
    
    // Apply class modifiers
    switch (CharacterClass)
    {
        case ECharacterClass::Warrior:
            Damage *= 1.2f;
            break;
        case ECharacterClass::Rogue:
            Damage *= 1.0f;
            break;
        case ECharacterClass::Mage:
            Damage *= 0.8f;
            break;
        case ECharacterClass::Ranger:
            Damage *= 1.1f;
            break;
        default:
            break;
    }
    
    // Apply random variation (±10%)
    float RandomFactor = FMath::RandRange(0.9f, 1.1f);
    Damage *= RandomFactor;
    
    return Damage;
}

void AOathCharacter::PlayAttackMontage()
{
    // Play appropriate attack animation based on equipped weapon and character class
    UAnimMontage* AttackMontage = nullptr;
    
    if (EquippedWeapon)
    {
        AttackMontage = EquippedWeapon->GetAttackMontage();
    }
    else
    {
        // Use default attack montage based on class
        switch (CharacterClass)
        {
            case ECharacterClass::Warrior:
                AttackMontage = DefaultWarriorAttackMontage;
                break;
            case ECharacterClass::Rogue:
                AttackMontage = DefaultRogueAttackMontage;
                break;
            case ECharacterClass::Mage:
                AttackMontage = DefaultMageAttackMontage;
                break;
            case ECharacterClass::Ranger:
                AttackMontage = DefaultRangerAttackMontage;
                break;
            default:
                AttackMontage = DefaultAttackMontage;
                break;
        }
    }
    
    if (AttackMontage)
    {
        PlayAnimMontage(AttackMontage);
    }
}

void AOathCharacter::Interact()
{
    // Find the nearest interactable object
    AActor* InteractableActor = FindNearestInteractable();
    
    if (InteractableActor)
    {
        // Call the interaction interface
        IInteractableInterface* Interactable = Cast<IInteractableInterface>(InteractableActor);
        if (Interactable)
        {
            Interactable->Interact(this);
        }
    }
}

AActor* AOathCharacter::FindNearestInteractable()
{
    // Find all interactable objects within range
    TArray<AActor*> FoundInteractables;
    UGameplayStatics::GetAllActorsWithInterface(GetWorld(), UInteractableInterface::StaticClass(), FoundInteractables);
    
    float ClosestDistance = InteractionRange;
    AActor* ClosestInteractable = nullptr;
    
    for (AActor* Interactable : FoundInteractables)
    {
        float Distance = FVector::Dist(GetActorLocation(), Interactable->GetActorLocation());
        if (Distance < ClosestDistance)
        {
            ClosestDistance = Distance;
            ClosestInteractable = Interactable;
        }
    }
    
    return ClosestInteractable;
}

void AOathCharacter::CompleteQuest(UQuest* Quest)
{
    if (!Quest)
    {
        return;
    }
    
    // Complete the quest if all objectives are done
    if (Quest->AreAllObjectivesComplete())
    {
        Quest->CompleteQuest();
        
        // Add rewards to player
        InventoryComponent->AddGold(Quest->GoldReward);
        
        // Add material rewards
        for (const TPair<FResourceData, int32>& Material : Quest->MaterialRewards)
        {
            InventoryComponent->AddMaterial(Material.Key, Material.Value);
        }
        
        // Add item rewards
        for (const FLootItem& Item : Quest->ItemRewards)
        {
            InventoryComponent->AddItem(Item);
        }
        
        // Gain quest renown
        ReputationComponent->GainQuestRenown(Quest->QuestRenownReward);
        
        // Modify faction reputation if applicable
        if (!Quest->FactionName.IsEmpty())
        {
            ReputationComponent->ModifyFactionReputation(Quest->FactionName, Quest->FactionReputationReward);
        }
        
        // Notify player of completed quest
        OnQuestCompleted.Broadcast(Quest);
    }
}

void AOathCharacter::InitializeClassStats()
{
    // Set starting stats based on character class
    switch (CharacterClass)
    {
        case ECharacterClass::Warrior:
            BaseAttackPower = 15.0f;
            GetCharacterMovement()->MaxWalkSpeed = 500.0f;
            break;
            
        case ECharacterClass::Rogue:
            BaseAttackPower = 12.0f;
            GetCharacterMovement()->MaxWalkSpeed = 600.0f;
            break;
            
        case ECharacterClass::Mage:
            BaseAttackPower = 8.0f;
            GetCharacterMovement()->MaxWalkSpeed = 450.0f;
            break;
            
        case ECharacterClass::Ranger:
            BaseAttackPower = 10.0f;
            GetCharacterMovement()->MaxWalkSpeed = 550.0f;
            break;
            
        default:
            BaseAttackPower = 10.0f;
            GetCharacterMovement()->MaxWalkSpeed = 500.0f;
            break;
    }
    
    // Initialize starting health and other stats (could be expanded)
    MaxHealth = 100.0f;
    CurrentHealth = MaxHealth;
    AttackRange = 150.0f;
    InteractionRange = 200.0f;
}

void AOathCharacter::ApplyKingdomVisionBonuses()
{
    // Apply bonuses based on kingdom vision
    if (KingdomVision == TEXT("Military Stronghold"))
    {
        // Combat-focused bonuses
        BaseAttackPower *= 1.1f;
        ReputationComponent->CombatRenownMultiplier = 1.2f;
    }
    else if (KingdomVision == TEXT("Trade Hub"))
    {
        // Economy-focused bonuses
        InventoryComponent->GoldGainMultiplier = 1.2f;
        InventoryComponent->InventoryCapacity += 10;
    }
    else if (KingdomVision == TEXT("Magical Haven"))
    {
        // Magic-focused bonuses (assuming we have magic abilities)
        MagicEffectiveness = 1.2f;
        ReputationComponent->QuestRenownMultiplier = 1.1f;
    }
    // Add more kingdom visions as needed
}

void AOathCharacter::UpdateStatusEffects(float DeltaTime)
{
    // Update any active buffs or debuffs
    TArray<FStatusEffect> ExpiredEffects;
    
    for (FStatusEffect& Effect : ActiveStatusEffects)
    {
        // Reduce the remaining duration
        Effect.RemainingDuration -= DeltaTime;
        
        // Check if the effect has expired
        if (Effect.RemainingDuration <= 0.0f)
        {
            ExpiredEffects.Add(Effect);
        }
        else
        {
            // Apply over-time effects if any
            if (Effect.bAppliesOverTime)
            {
                ApplyStatusEffectOverTime(Effect, DeltaTime);
            }
        }
    }
    
    // Remove expired effects
    for (const FStatusEffect& ExpiredEffect : ExpiredEffects)
    {
        RemoveStatusEffect(ExpiredEffect.EffectId);
    }
}

void AOathCharacter::ApplyStatusEffectOverTime(const FStatusEffect& Effect, float DeltaTime)
{
    // Apply different effects based on the type
    switch (Effect.EffectType)
    {
        case EStatusEffectType::Damage:
            TakeDamage(Effect.EffectStrength * DeltaTime, FDamageEvent(), nullptr, nullptr);
            break;
            
        case EStatusEffectType::Healing:
            Heal(Effect.EffectStrength * DeltaTime);
            break;
            
        case EStatusEffectType::SpeedBoost:
            // Speed is handled as a multiplier in the movement component
            break;
            
        default:
            break;
    }
}

void AOathCharacter::AddStatusEffect(const FStatusEffect& NewEffect)
{
    // Check if the effect already exists
    for (FStatusEffect& ExistingEffect : ActiveStatusEffects)
    {
        if (ExistingEffect.EffectId == NewEffect.EffectId)
        {
            // Replace or refresh the existing effect
            ExistingEffect = NewEffect;
            return;
        }
    }
    
    // Add the new effect
    ActiveStatusEffects.Add(NewEffect);
    
    // Apply immediate effects if any
    if (!NewEffect.bAppliesOverTime)
    {
        ApplyImmediateStatusEffect(NewEffect);
    }
    
    // Broadcast that a status effect was added
    OnStatusEffectAdded.Broadcast(NewEffect);
}

void AOathCharacter::RemoveStatusEffect(const FString& EffectId)
{
    // Find and remove the effect
    for (int32 i = 0; i < ActiveStatusEffects.Num(); i++)
    {
        if (ActiveStatusEffects[i].EffectId == EffectId)
        {
            FStatusEffect RemovedEffect = ActiveStatusEffects[i];
            
            // Revert any stat changes
            RevertStatusEffectChanges(RemovedEffect);
            
            // Remove from array
            ActiveStatusEffects.RemoveAt(i);
            
            // Broadcast that a status effect was removed
            OnStatusEffectRemoved.Broadcast(RemovedEffect);
            
            break;
        }
    }
}

void AOathCharacter::ApplyImmediateStatusEffect(const FStatusEffect& Effect)
{
    // Apply different effects based on the type
    switch (Effect.EffectType)
    {
        case EStatusEffectType::Damage:
            TakeDamage(Effect.EffectStrength, FDamageEvent(), nullptr, nullptr);
            break;
            
        case EStatusEffectType::Healing:
            Heal(Effect.EffectStrength);
            break;
            
        case EStatusEffectType::SpeedBoost:
            GetCharacterMovement()->MaxWalkSpeed *= (1.0f + Effect.EffectStrength);
            break;
            
        case EStatusEffectType::AttackBoost:
            BaseAttackPower *= (1.0f + Effect.EffectStrength);
            break;
            
        default:
            break;
    }
}

void AOathCharacter::RevertStatusEffectChanges(const FStatusEffect& Effect)
{
    // Revert different effects based on the type
    switch (Effect.EffectType)
    {
        case EStatusEffectType::SpeedBoost:
            GetCharacterMovement()->MaxWalkSpeed /= (1.0f + Effect.EffectStrength);
            break;
            
        case EStatusEffectType::AttackBoost:
            BaseAttackPower /= (1.0f + Effect.EffectStrength);
            break;
            
        default:
            break;
    }
}

void AOathCharacter::Heal(float Amount)
{
    CurrentHealth = FMath::Min(CurrentHealth + Amount, MaxHealth);
    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

float AOathCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    // Calculate final damage after armor and resistances
    float FinalDamage = CalculateFinalDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    
    // Apply damage
    CurrentHealth = FMath::Max(CurrentHealth - FinalDamage, 0.0f);
    
    // Broadcast health changed event
    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
    
    // Check if character died
    if (CurrentHealth <= 0)
    {
        Die();
    }
    
    return FinalDamage;
}

float AOathCharacter::CalculateFinalDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    // Start with base damage
    float FinalDamage = DamageAmount;
    
    // Apply armor reduction if we have equipment component
    if (EquipmentComponent)
    {
        float ArmorValue = EquipmentComponent->GetTotalArmorValue();
        float DamageReduction = ArmorValue / (ArmorValue + 100.0f); // Simple armor formula
        FinalDamage *= (1.0f - DamageReduction);
    }
    
    // Apply damage type resistances if applicable
    // This would depend on having a damage type system
    
    // Ensure damage is at least 1
    FinalDamage = FMath::Max(FinalDamage, 1.0f);
    
    return FinalDamage;
}

void AOathCharacter::Die()
{
    // Handle character death
    bIsDead = true;
    
    // Disable collision
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    
    // Ragdoll if enabled
    GetMesh()->SetSimulatePhysics(true);
    
    // Disable input
    DisableInput(Cast<APlayerController>(GetController()));
    
    // Broadcast death event
    OnCharacterDied.Broadcast();
    
    // Handle respawn or game over after delay
    FTimerHandle RespawnTimerHandle;
    GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &AOathCharacter::HandleRespawn, 5.0f, false);
}

void AOathCharacter::HandleRespawn()
{
    // Get the game mode and tell it to respawn the player
    AOathGameMode* GameMode = Cast<AOathGameMode>(UGameplayStatics::GetGameMode(this));
    if (GameMode)
    {
        GameMode->RespawnPlayer(GetController());
    }
}