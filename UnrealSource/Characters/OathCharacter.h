// OathCharacter.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ReputationComponent.h"
#include "InventoryComponent.h"
#include "OathCharacter.generated.h"

UENUM(BlueprintType)
enum class ECharacterClass : uint8
{
    Warrior UMETA(DisplayName = "Warrior"),
    Rogue UMETA(DisplayName = "Rogue"),
    Mage UMETA(DisplayName = "Mage"),
    Ranger UMETA(DisplayName = "Ranger"),
    Custom UMETA(DisplayName = "Custom")
};

UCLASS()
class OATH_API AOathCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AOathCharacter();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    ECharacterClass CharacterClass;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString KingdomVision;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|Components")
    UReputationComponent* ReputationComponent;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|Components")
    UInventoryComponent* InventoryComponent;
    
    // Combat functions
    UFUNCTION(BlueprintCallable, Category = "Character|Combat")
    void AttackEnemy(AActor* Target);
    
    // Quest functions
    UFUNCTION(BlueprintCallable, Category = "Character|Quests")
    void CompleteQuest(UQuest* Quest);
    
    UFUNCTION(BlueprintCallable, Category = "Character|Inventory")
    void InitializeStartingInventory();

    UFUNCTION(BlueprintCallable, Category = "Character|Combat")
    void AOathCharacter::EndStrengthBuff()
    
    UFUNCTION(BlueprintCallable, Category = "Character|Combat")
    bool AOathCharacter::UseItem(FLootItem Item)
    
    UFUNCTION(BlueprintCallable, Category = "Character|Harvest")
    void AOathCharacter::PlayHarvestAnimation(EResourceType ResourceType);
    
    UFUNCTION(BlueprintCallable, Category = "Character|Harvest")
    bool AOathCharacter::HasAppropriateHarvestingTool(EResourceType ResourceType) const;

    UFUNCTION(BlueprintCallable, Category = "Character|Harvest")
    void AOathCharacter::HarvestResource(AResourceNode* ResourceNode);
    
    UFUNCTION(BlueprintCallable, Category = "Character|Combat")
    float AOathCharacter::GetStartingWeaponDamage() const;
    
    UFUNCTION(BlueprintCallable, Category = "Character|Combat")
    FString AOathCharacter::GetStartingWeaponName() const;

    
    
protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(EditDefaultsOnly, Category = "Character|Stats")
    float BaseAttackPower;
};