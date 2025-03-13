#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerKnowledgeComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RPGSTATEMACHINES_API UPlayerKnowledgeComponent : public UActorComponent
{
    GENERATED_BODY()

public:    
    UPlayerKnowledgeComponent();

    virtual void BeginPlay() override;
    
    UFUNCTION(BlueprintCallable, Category = "Knowledge")
    bool HasKnowledge(FName KnowledgeID) const;
    
    UFUNCTION(BlueprintCallable, Category = "Knowledge")
    void AddKnowledge(FName KnowledgeID);
    
    UFUNCTION(BlueprintCallable, Category = "Knowledge")
    void RemoveKnowledge(FName KnowledgeID);
    
private:
    UPROPERTY(VisibleAnywhere, Category = "Knowledge")
    TSet<FName> AcquiredKnowledge;
};