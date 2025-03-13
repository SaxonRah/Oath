#include "DialogueManagerComponent.h"
#include "QuestDataComponent.h"

UDialogueManagerComponent::UDialogueManagerComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SelectedOptionIndex = -1;
}

void UDialogueManagerComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UDialogueManagerComponent::SetCurrentNPCText(const FText& Text)
{
    CurrentNPCText = Text;
}

FText UDialogueManagerComponent::GetCurrentNPCText() const
{
    return CurrentNPCText;
}

void UDialogueManagerComponent::SetCurrentDialogueOptions(const TArray<FDialogueOption>& Options)
{
    CurrentOptions = Options;
    SelectedOptionIndex = -1; // Reset selection
}

TArray<FDialogueOption> UDialogueManagerComponent::GetCurrentDialogueOptions() const
{
    return CurrentOptions;
}

void UDialogueManagerComponent::SelectOption(int32 OptionIndex)
{
    if (CurrentOptions.IsValidIndex(OptionIndex))
    {
        SelectedOptionIndex = OptionIndex;
    }
}

int32 UDialogueManagerComponent::GetSelectedOptionIndex() const
{
    return SelectedOptionIndex;
}

void UDialogueManagerComponent::TransitionToDialogueNode(FName NodeID)
{
    CurrentNodeID = NodeID;
    SelectedOptionIndex = -1; // Reset selection
}

void UDialogueManagerComponent::SetCurrentPlayer(AActor* Player)
{
    CurrentPlayer = Player;
}

void UDialogueManagerComponent::TriggerQuest(FName QuestID)
{
    if (CurrentPlayer)
    {
        UQuestDataComponent* QuestData = CurrentPlayer->FindComponentByClass<UQuestDataComponent>();
        if (QuestData)
        {
            QuestData->SetQuestStatus(QuestID, EQuestStatus::Available);
            UE_LOG(LogTemp, Log, TEXT("Triggered quest: %s"), *QuestID.ToString());
        }
    }
}