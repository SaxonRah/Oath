#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DialogueTreeNode.h" // For FDialogueOption
#include "DialogueManagerComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RPGSTATEMACHINES_API UDialogueManagerComponent : public UActorComponent
{
    GENERATED_BODY()

public:    
    UDialogueManagerComponent();

    virtual void BeginPlay() override;
    
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SetCurrentNPCText(const FText& Text);
    
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FText GetCurrentNPCText() const;
    
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SetCurrentDialogueOptions(const TArray<FDialogueOption>& Options);
    
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    TArray<FDialogueOption> GetCurrentDialogueOptions() const;
    
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SelectOption(int32 OptionIndex);
    
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    int32 GetSelectedOptionIndex() const;
    
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void TransitionToDialogueNode(FName NodeID);
    
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SetCurrentPlayer(AActor* Player);
    
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void TriggerQuest(FName QuestID);
    
private:
    UPROPERTY(VisibleAnywhere, Category = "Dialogue")
    FText CurrentNPCText;
    
    UPROPERTY(VisibleAnywhere, Category = "Dialogue")
    TArray<FDialogueOption> CurrentOptions;
    
    UPROPERTY(VisibleAnywhere, Category = "Dialogue")
    int32 SelectedOptionIndex;
    
    UPROPERTY(VisibleAnywhere, Category = "Dialogue")
    AActor* CurrentPlayer;
    
    UPROPERTY(VisibleAnywhere, Category = "Dialogue")
    FName CurrentNodeID;
};