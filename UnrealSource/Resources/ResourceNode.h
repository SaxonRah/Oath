// ResourceNode.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ResourceData.h"
#include "ResourceNode.generated.h"

UENUM(BlueprintType)
enum class EResourceType : uint8
{
    None UMETA(DisplayName = "None"),
    Wood UMETA(DisplayName = "Wood"),
    Stone UMETA(DisplayName = "Stone"),
    Herb UMETA(DisplayName = "Herb"),
    Ore UMETA(DisplayName = "Ore"),
    Crystal UMETA(DisplayName = "Crystal"),
    Animal UMETA(DisplayName = "Animal")
};

UCLASS()
class OATH_API AResourceNode : public AActor
{
    GENERATED_BODY()
    
public:
    AResourceNode();
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    EResourceType ResourceType;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    FResourceData ResourceData;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    int32 HarvestAmount;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    int32 MaxHarvests;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    float RespawnTime;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    UStaticMeshComponent* MeshComponent;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    class UInteractionComponent* InteractionComponent;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    UParticleSystem* HarvestEffect;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    USoundBase* HarvestSound;
    
    UFUNCTION(BlueprintCallable, Category = "Resource")
    void OnHarvested();
    
    UFUNCTION(BlueprintCallable, Category = "Resource")
    void Respawn();
    
    UFUNCTION(BlueprintPure, Category = "Resource")
    EResourceType GetResourceType() const { return ResourceType; }
    
    UFUNCTION(BlueprintPure, Category = "Resource")
    FResourceData GetResourceData() const { return ResourceData; }
    
    UFUNCTION(BlueprintPure, Category = "Resource")
    int32 GetHarvestAmount() const { return HarvestAmount; }
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Resource")
    void OnResourceDepleted();
    
protected:
    virtual void BeginPlay() override;
    
private:
    int32 CurrentHarvests;
    bool bIsDepleted;
    FTimerHandle RespawnTimerHandle;
};