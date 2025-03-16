// EquippedWeapon.cpp
#include "EquippedWeapon.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"

AEquippedWeapon::AEquippedWeapon()
{
    // Set this actor to call Tick() every frame
    PrimaryActorTick.bCanEverTick = true;
    
    // Create weapon mesh component
    WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
    // EquippedWeapon.cpp continued
   RootComponent = WeaponMesh;
   WeaponMesh->SetCollisionProfileName(TEXT("Weapon"));
   WeaponMesh->SetGenerateOverlapEvents(false);
   
   // Create particle system component for weapon effects
   WeaponEffects = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("WeaponEffects"));
   WeaponEffects->SetupAttachment(WeaponMesh);
   WeaponEffects->bAutoActivate = false;
   
   // Create audio component for weapon sounds
   WeaponSounds = CreateDefaultSubobject<UAudioComponent>(TEXT("WeaponSounds"));
   WeaponSounds->SetupAttachment(WeaponMesh);
   WeaponSounds->bAutoActivate = false;
   
   // Set default values
   WeaponName = TEXT("Basic Weapon");
   WeaponType = EWeaponType::Sword;
   Rarity = EResourceRarity::Common;
   BaseDamage = 10.0f;
   AttackSpeed = 1.0f;
   CriticalChance = 0.05f;
   CriticalMultiplier = 1.5f;
   DurabilityMax = 100.0f;
   DurabilityCurrent = 100.0f;
   RequiredLevel = 1;
}

void AEquippedWeapon::BeginPlay()
{
   Super::BeginPlay();
   
   // Initialize weapon stats based on rarity and type
   InitializeWeaponStats();
   
   // Disable effects by default
   EnableWeaponEffects(false);
}

void AEquippedWeapon::InitializeWeaponStats()
{
   // Apply rarity-based bonuses
   float RarityMultiplier = CalculateRarityBonus();
   BaseDamage *= RarityMultiplier;
   
   // Apply weapon-type specific adjustments
   switch (WeaponType)
   {
       case EWeaponType::Sword:
           // Balanced weapon
           break;
           
       case EWeaponType::Axe:
           BaseDamage *= 1.2f;
           AttackSpeed *= 0.8f;
           break;
           
       case EWeaponType::Mace:
           BaseDamage *= 1.3f;
           AttackSpeed *= 0.7f;
           CriticalMultiplier *= 1.1f;
           break;
           
       case EWeaponType::Dagger:
           BaseDamage *= 0.7f;
           AttackSpeed *= 1.5f;
           CriticalChance *= 1.5f;
           break;
           
       case EWeaponType::Bow:
           // Range weapon with balanced stats
           break;
           
       case EWeaponType::Staff:
           BaseDamage *= 0.9f;
           // Would typically have magic bonuses
           break;
           
       case EWeaponType::Wand:
           BaseDamage *= 0.8f;
           AttackSpeed *= 1.2f;
           // Would typically have magic bonuses
           break;
   }
   
   // Adjust durability based on weapon type
   switch (WeaponType)
   {
       case EWeaponType::Sword:
       case EWeaponType::Bow:
           // Standard durability
           break;
           
       case EWeaponType::Axe:
       case EWeaponType::Mace:
           DurabilityMax *= 1.2f;
           break;
           
       case EWeaponType::Dagger:
       case EWeaponType::Wand:
           DurabilityMax *= 0.8f;
           break;
           
       case EWeaponType::Staff:
           DurabilityMax *= 1.1f;
           break;
   }
   
   // Set current durability to max
   DurabilityCurrent = DurabilityMax;
}

float AEquippedWeapon::CalculateRarityBonus() const
{
   // Apply different multipliers based on rarity
   switch (Rarity)
   {
       case EResourceRarity::Common:
           return 1.0f;
           
       case EResourceRarity::Uncommon:
           return 1.25f;
           
       case EResourceRarity::Rare:
           return 1.5f;
           
       case EResourceRarity::Legendary:
           return 2.0f;
           
       case EResourceRarity::Artifact:
           return 3.0f;
           
       default:
           return 1.0f;
   }
}

float AEquippedWeapon::GetDamageValue() const
{
   // Check if the weapon is broken
   if (DurabilityCurrent <= 0)
   {
       return BaseDamage * 0.5f; // Damaged weapons deal reduced damage
   }
   
   // Roll for critical hit
   float DamageMultiplier = 1.0f;
   float CritRoll = FMath::FRand(); // 0.0 to 1.0
   
   if (CritRoll <= CriticalChance)
   {
       DamageMultiplier = CriticalMultiplier;
       
       // Call the critical hit event
       const_cast<AEquippedWeapon*>(this)->OnWeaponCritical();
   }
   
   // Apply durability factor (reduced damage as durability gets lower)
   float DurabilityFactor = FMath::Max(0.8f, DurabilityCurrent / DurabilityMax);
   
   // Calculate final damage
   return BaseDamage * DamageMultiplier * DurabilityFactor;
}

bool AEquippedWeapon::UseDurability(float Amount)
{
   // Reduce durability
   DurabilityCurrent = FMath::Max(0.0f, DurabilityCurrent - Amount);
   
   // Check if weapon has broken
   if (DurabilityCurrent <= 0 && DurabilityMax > 0)
   {
       OnWeaponBroken();
       return true;
   }
   
   return false;
}

void AEquippedWeapon::RepairWeapon(float Amount)
{
   // Increase durability but cap at max
   DurabilityCurrent = FMath::Min(DurabilityMax, DurabilityCurrent + Amount);
}

float AEquippedWeapon::GetDurabilityPercentage() const
{
   if (DurabilityMax <= 0)
   {
       return 1.0f; // Indestructible weapons
   }
   
   return DurabilityCurrent / DurabilityMax;
}

UAnimMontage* AEquippedWeapon::GetAttackMontage() const
{
   // Return the appropriate attack animation based on the weapon type
   // For now, just return the primary attack montage
   return PrimaryAttackMontage;
}

void AEquippedWeapon::EnableWeaponEffects(bool bEnable)
{
   if (WeaponEffects)
   {
       if (bEnable)
       {
           WeaponEffects->Activate();
       }
       else
       {
           WeaponEffects->Deactivate();
       }
   }
}

void AEquippedWeapon::PlayWeaponSound(FString SoundType)
{
   // Check if we have a sound for this type
   if (WeaponSoundEffects.Contains(SoundType))
   {
       USoundBase* Sound = WeaponSoundEffects[SoundType];
       if (Sound && WeaponSounds)
       {
           WeaponSounds->SetSound(Sound);
           WeaponSounds->Play();
       }
   }
}