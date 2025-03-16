// KingdomUIWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "KingdomManager.h"
#include "FollowerData.h"
#include "BuildingData.h"
#include "KingdomUIWidget.generated.h"

UCLASS()
class OATH_API UKingdomUIWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UKingdomUIWidget(const FObjectInitializer& ObjectInitializer);
    
    virtual void NativeConstruct() override;
    
    // Kingdom overview
    UFUNCTION(BlueprintCallable, Category = "UI|Kingdom")
    void UpdateKingdomInfo(const FString& KingdomName, EKingdomTier KingdomTier, 
                           int32 FollowerCount, int32 BuildingCount, float DailyIncome);
    
    // Buildings tab
    UFUNCTION(BlueprintCallable, Category = "UI|Kingdom")
    void UpdateBuildingsList(const TArray<FBuildingData>& Buildings);
    
    UFUNCTION(BlueprintCallable, Category = "UI|Kingdom")
    void UpdateAvailableBuildings(const TArray<FBuildingData>& AvailableBuildings);
    
    UFUNCTION(BlueprintCallable, Category = "UI|Kingdom")
    void SelectBuilding(const FBuildingData& BuildingData);
    
    UFUNCTION(BlueprintCallable, Category = "UI|Kingdom")
    bool CanBuildSelected() const;
    
    UFUNCTION(BlueprintCallable, Category = "UI|Kingdom")
    void ConstructSelectedBuilding();
    
    // Followers tab
    UFUNCTION(BlueprintCallable, Category = "UI|Kingdom")
    void UpdateFollowersList(const TArray<FFollowerData>& Followers);
    
    UFUNCTION(BlueprintCallable, Category = "UI|Kingdom")
    void SelectFollower(const FFollowerData& FollowerData);
    
    UFUNCTION(BlueprintCallable, Category = "UI|Kingdom")
    void AssignFollowerToBuilding(const FFollowerData& Follower, const FBuildingData& Building);
    
    UFUNCTION(BlueprintCallable, Category = "UI|Kingdom")
    void AssignFollowerToQuest(const FFollowerData& Follower, UQuest* Quest);
    
    // Resources tab
    UFUNCTION(BlueprintCallable, Category = "UI|Kingdom")
    void UpdateResourceProduction(const TMap<FString, float>& DailyProduction);
    
    UFUNCTION(BlueprintCallable, Category = "UI|Kingdom")
    void UpdateResourceConsumption(const TMap<FString, float>& DailyConsumption);
    
    // Events tab
    UFUNCTION(BlueprintCallable, Category = "UI|Kingdom")
    void AddEventToLog(const FKingdomEvent& Event);
    
    UFUNCTION(BlueprintCallable, Category = "UI|Kingdom")
    void ClearEventLog();
    
    // Implement in Blueprints
    UFUNCTION(BlueprintImplementableEvent, Category = "UI|Kingdom")
    void OnKingdomInfoUpdated(const FString& KingdomName, EKingdomTier KingdomTier, 
                             int32 FollowerCount, int32 BuildingCount, float DailyIncome);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "UI|Kingdom")
    void OnBuildingsListUpdated(const TArray<FBuildingData>& Buildings);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "UI|Kingdom")
    void OnAvailableBuildingsUpdated(const TArray<FBuildingData>& AvailableBuildings);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "UI|Kingdom")
    void OnBuildingSelected(const FBuildingData& BuildingData);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "UI|Kingdom")
    void OnFollowersListUpdated(const TArray<FFollowerData>& Followers);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "UI|Kingdom")
    void OnFollowerSelected(const FFollowerData& FollowerData);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "UI|Kingdom")
    void OnResourceProductionUpdated(const TMap<FString, float>& DailyProduction);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "UI|Kingdom")
    void OnResourceConsumptionUpdated(const TMap<FString, float>& DailyConsumption);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "UI|Kingdom")
    void OnEventAdded(const FKingdomEvent& Event);
    
private:
    // Reference to the kingdom manager
    UPROPERTY()
    AKingdomManager* KingdomManager;
    
    // Current selections
    UPROPERTY()
    FBuildingData SelectedBuilding;
    
    UPROPERTY()
    FFollowerData SelectedFollower;
    
    // Helper methods
    void FindKingdomManager();
    bool HasSufficientResources(const FBuildingData& Building) const;
};