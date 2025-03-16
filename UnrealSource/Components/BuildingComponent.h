// BuildingComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuildingData.h"
#include "BuildingComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class OATH_API UBuildingComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UBuildingComponent();
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    FBuildingData BuildingData;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    int32 HealthPoints;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    int32 MaxHealthPoints;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    float Efficiency;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    TArray<AActor*> AssignedFollowers;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    bool bIsUnderConstruction;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building")
    float ConstructionProgress;
    
    UFUNCTION(BlueprintCallable, Category = "Building")
    void SetBuildingData(const FBuildingData& InBuildingData);
    
    UFUNCTION(BlueprintCallable, Category = "Building")
    void TakeDamage(int32 DamageAmount);
    
    UFUNCTION(BlueprintCallable, Category = "Building")
    void Repair(int32 RepairAmount);
    
    UFUNCTION(BlueprintCallable, Category = "Building")
    bool AssignFollower(AActor* Follower);
    
    UFUNCTION(BlueprintCallable, Category = "Building")
    bool RemoveFollower(AActor* Follower);
    
    UFUNCTION(BlueprintCallable, Category = "Building")
    void UpdateEfficiency();
    
    UFUNCTION(BlueprintCallable, Category = "Building")
    float GetResourceOutput(const FString& ResourceType) const;
    
    UFUNCTION(BlueprintCallable, Category = "Building")
    float GetGoldOutput() const;
    
protected:
    virtual void BeginPlay() override;
    
private:
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBuildingStatusChangedDelegate, bool, IsUnderConstruction, float, HealthPercentage);
    
    UPROPERTY(BlueprintAssignable, Category = "Building|Events")
    FOnBuildingStatusChangedDelegate OnBuildingStatusChanged;
};