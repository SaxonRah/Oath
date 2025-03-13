#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FactionComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RPGSTATEMACHINES_API UFactionComponent : public UActorComponent
{
    GENERATED_BODY()

public:    
    UFactionComponent();

    virtual void BeginPlay() override;
    
    UFUNCTION(BlueprintCallable, Category = "Faction")
    int32 GetReputationWithFaction(FName FactionID) const;
    
    UFUNCTION(BlueprintCallable, Category = "Faction")
    void SetReputationWithFaction(FName FactionID, int32 Value);
    
    UFUNCTION(BlueprintCallable, Category = "Faction")
    void ModifyReputationWithFaction(FName FactionID, int32 Delta);
    
    UFUNCTION(BlueprintCallable, Category = "Faction")
    void ModifyCurrentNPCReputation(int32 Delta);
    
    UFUNCTION(BlueprintCallable, Category = "Faction")
    void SetCurrentNPC(FName FactionID);
    
private:
    UPROPERTY(VisibleAnywhere, Category = "Faction")
    TMap<FName, int32> FactionReputations;
    
    UPROPERTY(VisibleAnywhere, Category = "Faction")
    FName CurrentNPCFaction;
};