// KingdomManager.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FollowerData.h"
#include "BuildingData.h"
#include "KingdomManager.generated.h"

UENUM(BlueprintType)
enum class EKingdomTier : uint8
{
    Camp UMETA(DisplayName = "Camp"),
    Village UMETA(DisplayName = "Village"),
    Town UMETA(DisplayName = "Town"),
    City UMETA(DisplayName = "City"),
    Kingdom UMETA(DisplayName = "Kingdom")
};

UCLASS()
class OATH_API AKingdomManager : public AActor
{
    GENERATED_BODY()

public:
    AKingdomManager();
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kingdom")
    FString KingdomName;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kingdom")
    EKingdomTier CurrentTier;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kingdom")
    float KingdomAlignment; // -1.0 to 1.0
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kingdom")
    TArray<FFollowerData> Followers;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kingdom")
    TArray<FBuildingData> Buildings;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kingdom")
    int32 TaxRate;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Kingdom")
    float DailyIncome;
    
    UFUNCTION(BlueprintCallable, Category = "Kingdom")
    bool RecruitFollower(FFollowerData Follower);
    
    UFUNCTION(BlueprintCallable, Category = "Kingdom")
    bool ConstructBuilding(FBuildingData Building, FVector Location);
    
    UFUNCTION(BlueprintCallable, Category = "Kingdom")
    void CalculateDailyIncome();
    
    UFUNCTION(BlueprintCallable, Category = "Kingdom")
    void UpdateKingdomTier();
    
    UFUNCTION(BlueprintCallable, Category = "Kingdom")
    void ProcessDailyUpdate();
    
    UFUNCTION(BlueprintCallable, Category = "Kingdom")
    void TriggerRandomEvent();
    
    UFUNCTION(BlueprintCallable, Category = "Kingdom")
    int32 GetMaxFollowers() const;
    
    UFUNCTION(BlueprintCallable, Category = "Kingdom")
    int32 GetMaxBuildings() const;
    
protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    
private:
    float DayTimer;
    float DaysElapsed;
    
    UPROPERTY(EditDefaultsOnly, Category = "Kingdom")
    float DayLength; // In real seconds
    
    UPROPERTY(EditDefaultsOnly, Category = "Kingdom")
    TArray<FKingdomEvent> PossibleEvents;
};