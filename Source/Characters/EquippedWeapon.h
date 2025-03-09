// EquippedWeapon.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ResourceData.h"
#include "EquippedWeapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
    Sword UMETA(DisplayName = "Sword"),
    Axe UMETA(DisplayName = "Axe"),
    Mace UMETA(DisplayName = "Mace"),
    Dagger UMETA(DisplayName = "Dagger"),
    Bow UMETA(DisplayName = "Bow"),
    Staff UMETA(DisplayName = "Staff"),
    Wand UMETA(DisplayName = "Wand")
};

UCLASS()
class OATH_API AEquippedWeapon : public AActor
{
    GENERATED_BODY()

public:
    AEquippedWeapon();
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    FString WeaponName;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    EWeaponType WeaponType;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    EResourceRarity Rarity;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    float BaseDamage;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    float AttackSpeed;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    float CriticalChance;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    float CriticalMultiplier;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    float DurabilityMax;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    float DurabilityCurrent;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    int32 RequiredLevel;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    TMap<FString, float> StatBonuses;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    TArray<FString> SpecialEffects;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    UStaticMeshComponent* WeaponMesh;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    UParticleSystemComponent* WeaponEffects;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    UAudioComponent* WeaponSounds;
    
    UFUNCTION(BlueprintCallable, Category = "Weapon")
    float GetDamageValue() const;
    
    UFUNCTION(BlueprintCallable, Category = "Weapon")
    bool UseDurability(float Amount);
    
    UFUNCTION(BlueprintCallable, Category = "Weapon")
    void RepairWeapon(float Amount);
    
    UFUNCTION(BlueprintCallable, Category = "Weapon")
    float GetDurabilityPercentage() const;
    
    UFUNCTION(BlueprintCallable, Category = "Weapon")
    UAnimMontage* GetAttackMontage() const;
    
    UFUNCTION(BlueprintCallable, Category = "Weapon")
    void EnableWeaponEffects(bool bEnable);
    
    UFUNCTION(BlueprintCallable, Category = "Weapon")
    void PlayWeaponSound(FString SoundType);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Weapon")
    void OnWeaponEquipped(AActor* Owner);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Weapon")
    void OnWeaponUnequipped(AActor* Owner);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Weapon")
    void OnWeaponAttack();
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Weapon")
    void OnWeaponCritical();
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Weapon")
    void OnWeaponBroken();

protected:
    virtual void BeginPlay() override;
    
    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Animation")
    UAnimMontage* PrimaryAttackMontage;
    
    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Animation")
    UAnimMontage* SecondaryAttackMontage;
    
    UPROPERTY(EditDefaultsOnly, Category = "Weapon|Animation")
    UAnimMontage* SpecialAttackMontage;

private:
    void InitializeWeaponStats();
    float CalculateRarityBonus() const;
    
    UPROPERTY()
    TMap<FString, USoundBase*> WeaponSoundEffects;
};