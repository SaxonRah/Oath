// EnemyCharacter.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ResourceData.h"
#include "InventoryComponent.h"
#include "EnemyCharacter.generated.h"

UENUM(BlueprintType)
enum class EEnemyType : uint8
{
    Regular UMETA(DisplayName = "Regular"),
    Elite UMETA(DisplayName = "Elite"),
    Boss UMETA(DisplayName = "Boss"),
    Notorious UMETA(DisplayName = "Notorious")
};

UCLASS()
class OATH_API AEnemyCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AEnemyCharacter();
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
    FString EnemyName;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
    EEnemyType EnemyType;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
    int32 Level;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
    float MaxHealth;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
    float CurrentHealth;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
    float AttackPower;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
    float AttackRange;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
    float DetectionRange;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
    float AggroRange;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
    float PatrolRadius;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
    bool bIsDead;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Rewards")
    float RenownValue;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Rewards")
    int32 GoldRewardMin;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Rewards")
    int32 GoldRewardMax;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Rewards")
    TArray<FLootTable> PossibleLoot;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Rewards")
    TArray<FMaterialDrop> PossibleMaterials;
    
    UFUNCTION(BlueprintCallable, Category = "Enemy")
    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
    
    UFUNCTION(BlueprintCallable, Category = "Enemy")
    void AttackTarget(AActor* Target);
    
    UFUNCTION(BlueprintCallable, Category = "Enemy")
    TArray<FLootItem> GenerateLoot();
    
    UFUNCTION(BlueprintCallable, Category = "Enemy")
    TMap<FResourceData, int32> GenerateMaterials();
    
    UFUNCTION(BlueprintCallable, Category = "Enemy")
    int32 GetGoldReward();
    
    UFUNCTION(BlueprintPure, Category = "Enemy")
    bool IsDead() const { return bIsDead; }
    
    UFUNCTION(BlueprintPure, Category = "Enemy")
    float GetRenownValue() const { return RenownValue; }
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Enemy")
    void OnSpotPlayer(AActor* Player);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Enemy")
    void OnLostPlayer();
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Enemy")
    void OnDeath();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    
    UPROPERTY(EditDefaultsOnly, Category = "Enemy|Animation")
    UAnimMontage* AttackMontage;
    
    UPROPERTY(EditDefaultsOnly, Category = "Enemy|Animation")
    UAnimMontage* HitReactionMontage;
    
    UPROPERTY(EditDefaultsOnly, Category = "Enemy|Animation")
    UAnimMontage* DeathMontage;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|AI")
    class UPawnSensingComponent* PawnSensingComponent;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|AI")
    class UAIPerceptionComponent* AIPerceptionComponent;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Components")
    class UBehaviorTreeComponent* BehaviorTreeComponent;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Components")
    class UBlackboardComponent* BlackboardComponent;

private:
    UFUNCTION()
    void OnSeePlayer(APawn* Pawn);
    
    UFUNCTION()
    void OnHearNoise(APawn* PawnInstigator, const FVector& Location, float Volume);
    
    UFUNCTION()
    void Die();
    
    UFUNCTION()
    void PlayAttackMontage();
    
    UFUNCTION()
    void PlayHitReactionMontage();
    
    UFUNCTION()
    void PlayDeathMontage();
    
    UPROPERTY()
    FVector HomeLocation;
    
    UPROPERTY()
    FTimerHandle AttackTimerHandle;
    
    UPROPERTY()
    bool bIsAttacking;
    
    float CalculateFinalDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser);
};

// Helper structs for loot generation
USTRUCT(BlueprintType)
struct OATH_API FLootTable
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLootItem Item;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DropChance; // 0.0 to 1.0
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MinQuantity;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxQuantity;
};

USTRUCT(BlueprintType)
struct OATH_API FMaterialDrop
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FResourceData Material;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DropChance; // 0.0 to 1.0
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MinQuantity;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxQuantity;
};