#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WorldStateTreeNode.h" // For EWorldEventType
#include "WorldStateComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnWorldEventStarted, FName, EventID, FText, EventName, FText, EventDescription, EWorldEventType, EventType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWorldEventEnded, FName, EventID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWorldVariableChanged, FName, VariableName, float, NewValue);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RPGSTATEMACHINES_API UWorldStateComponent : public UActorComponent
{
    GENERATED_BODY()

public:    
    UWorldStateComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "World State")
    float GetWorldVariable(FName VariableName) const;
    
    UFUNCTION(BlueprintCallable, Category = "World State")
    void SetWorldVariable(FName VariableName, float Value);
    
    UFUNCTION(BlueprintCallable, Category = "World State")
    void ChangeWorldVariable(FName VariableName, float Delta);
    
    UFUNCTION(BlueprintCallable, Category = "World State")
    void SetWorldVariableMultiplier(FName VariableName, float Multiplier);
    
    UFUNCTION(BlueprintCallable, Category = "World State")
    void RegisterPossibleEvent(FName EventID);
    
    UFUNCTION(BlueprintCallable, Category = "World State")
    void NotifyWorldEventStarted(FName EventID, FText EventName, FText EventDescription, EWorldEventType EventType);
    
    UFUNCTION(BlueprintCallable, Category = "World State")
    void NotifyWorldEventEnded(FName EventID);
    
    UPROPERTY(BlueprintAssignable, Category = "World State")
    FOnWorldEventStarted OnWorldEventStarted;
    
    UPROPERTY(BlueprintAssignable, Category = "World State")
    FOnWorldEventEnded OnWorldEventEnded;
    
    UPROPERTY(BlueprintAssignable, Category = "World State")
    FOnWorldVariableChanged OnWorldVariableChanged;
    
private:
    UPROPERTY(VisibleAnywhere, Category = "World State")
    TMap<FName, float> WorldVariables;
    
    UPROPERTY(VisibleAnywhere, Category = "World State")
    TMap<FName, float> WorldVariableMultipliers;
    
    UPROPERTY(VisibleAnywhere, Category = "World State")
    TArray<FName> ActiveEvents;
    
    UPROPERTY(VisibleAnywhere, Category = "World State")
    TArray<FName> PossibleEvents;
};