// ReputationComponent.cpp
#include "ReputationComponent.h"
#include "Kismet/GameplayStatics.h"
#include "OathPlayerController.h"
#include "OathGameInstance.h"

UReputationComponent::UReputationComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize default reputation values
    CombatRenown = 0.0f;
    QuestRenown = 0.0f;
    KingdomReputation = 0.0f;
}

void UReputationComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize faction reputations if needed
    UOathGameInstance* GameInstance = Cast<UOathGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
    if (GameInstance)
    {
        TArray<FString> Factions = GameInstance->GetAllFactions();
        for (const FString& Faction : Factions)
        {
            if (!FactionReputation.Contains(Faction))
            {
                FactionReputation.Add(Faction, 0.0f);
            }
        }
    }
}

void UReputationComponent::GainCombatRenown(float Amount, bool bNotify)
{
    float OldValue = CombatRenown;
    CombatRenown += Amount;
    
    if (bNotify)
    {
        OnReputationChanged.Broadcast("Combat", OldValue, CombatRenown);
        
        // Notify player controller for UI updates
        if (AActor* Owner = GetOwner())
        {
            if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
            {
                if (AOathPlayerController* OathPC = Cast<AOathPlayerController>(PC))
                {
                    OathPC->OnReputationChanged("Combat", OldValue, CombatRenown);
                }
            }
        }
    }
    
    // Check for milestone achievements
    CheckCombatRenownMilestones();
}

void UReputationComponent::GainQuestRenown(float Amount, bool bNotify)
{
    float OldValue = QuestRenown;
    QuestRenown += Amount;
    
    if (bNotify)
    {
        OnReputationChanged.Broadcast("Quest", OldValue, QuestRenown);
        
        // Notify player controller for UI updates
        if (AActor* Owner = GetOwner())
        {
            if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
            {
                if (AOathPlayerController* OathPC = Cast<AOathPlayerController>(PC))
                {
                    OathPC->OnReputationChanged("Quest", OldValue, QuestRenown);
                }
            }
        }
    }
    
    // Check for milestone achievements
    CheckQuestRenownMilestones();
}

void UReputationComponent::GainKingdomReputation(float Amount, bool bNotify)
{
    float OldValue = KingdomReputation;
    KingdomReputation += Amount;
    
    if (bNotify)
    {
        OnReputationChanged.Broadcast("Kingdom", OldValue, KingdomReputation);
        
        // Notify player controller for UI updates
        if (AActor* Owner = GetOwner())
        {
            if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
            {
                if (AOathPlayerController* OathPC = Cast<AOathPlayerController>(PC))
                {
                    OathPC->OnReputationChanged("Kingdom", OldValue, KingdomReputation);
                }
            }
        }
    }
    
    // Notify Kingdom Manager about the reputation change
    UOathGameInstance* GameInstance = Cast<UOathGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
    if (GameInstance && GameInstance->GetKingdomManager())
    {
        GameInstance->GetKingdomManager()->OnKingdomReputationChanged(KingdomReputation);
    }
}

void UReputationComponent::ModifyFactionReputation(FString FactionName, float Amount, bool bNotify)
{
    if (!FactionReputation.Contains(FactionName))
    {
        FactionReputation.Add(FactionName, 0.0f);
    }
    
    float OldValue = FactionReputation[FactionName];
    FactionReputation[FactionName] += Amount;
    
    // Clamp faction reputation to a reasonable range (-100 to 100)
    FactionReputation[FactionName] = FMath::Clamp(FactionReputation[FactionName], -100.0f, 100.0f);
    
    if (bNotify)
    {
        FString ReputationType = FString::Printf(TEXT("Faction:%s"), *FactionName);
        OnReputationChanged.Broadcast(ReputationType, OldValue, FactionReputation[FactionName]);
        
        // Notify player controller for UI updates
        if (AActor* Owner = GetOwner())
        {
            if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
            {
                if (AOathPlayerController* OathPC = Cast<AOathPlayerController>(PC))
                {
                    OathPC->OnReputationChanged(ReputationType, OldValue, FactionReputation[FactionName]);
                }
            }
        }
    }
    
    // Handle faction-specific events based on reputation thresholds
    CheckFactionReputationThresholds(FactionName);
}

int32 UReputationComponent::GetKingdomTier() const
{
    // Kingdom tiers based on Kingdom Reputation
    // 0: Camp (0-99)
    // 1: Village (100-499)
    // 2: Town (500-1499)
    // 3: City (1500-4999)
    // 4: Kingdom (5000+)
    
    if (KingdomReputation >= 5000.0f)
    {
        return 4; // Kingdom
    }
    else if (KingdomReputation >= 1500.0f)
    {
        return 3; // City
    }
    else if (KingdomReputation >= 500.0f)
    {
        return 2; // Town
    }
    else if (KingdomReputation >= 100.0f)
    {
        return 1; // Village
    }
    else
    {
        return 0; // Camp
    }
}

void UReputationComponent::RegisterReputationChangedDelegate(const FOnReputationChangedDelegate& Delegate)
{
    OnReputationChanged.Add(Delegate);
}

void UReputationComponent::CheckCombatRenownMilestones()
{
    // Define milestone thresholds
    TArray<float> Milestones = { 100.0f, 500.0f, 1000.0f, 2500.0f, 5000.0f, 10000.0f };
    
    for (float Milestone : Milestones)
    {
        if (CombatRenown >= Milestone && PreviousCombatRenown < Milestone)
        {
            // Trigger milestone event
            OnCombatRenownMilestoneReached.Broadcast(Milestone);
            
            // Trigger game-wide events for this milestone
            UOathGameInstance* GameInstance = Cast<UOathGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
            if (GameInstance)
            {
                GameInstance->OnCombatRenownMilestoneReached(Milestone);
            }
            
            // Generate special encounters or quests based on milestone
            GenerateMilestoneContent("Combat", Milestone);
        }
    }
    
    PreviousCombatRenown = CombatRenown;
}

void UReputationComponent::CheckQuestRenownMilestones()
{
    // Define milestone thresholds
    TArray<float> Milestones = { 100.0f, 500.0f, 1000.0f, 2500.0f, 5000.0f, 10000.0f };
    
    for (float Milestone : Milestones)
    {
        if (QuestRenown >= Milestone && PreviousQuestRenown < Milestone)
        {
            // Trigger milestone event
            OnQuestRenownMilestoneReached.Broadcast(Milestone);
            
            // Trigger game-wide events for this milestone
            UOathGameInstance* GameInstance = Cast<UOathGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
            if (GameInstance)
            {
                GameInstance->OnQuestRenownMilestoneReached(Milestone);
            }
            
            // Generate special encounters or quests based on milestone
            GenerateMilestoneContent("Quest", Milestone);
        }
    }
    
    PreviousQuestRenown = QuestRenown;
}

void UReputationComponent::CheckFactionReputationThresholds(const FString& FactionName)
{
    // Key reputation thresholds that trigger events
    TArray<float> Thresholds = { -100.0f, -75.0f, -50.0f, -25.0f, 0.0f, 25.0f, 50.0f, 75.0f, 100.0f };
    float CurrentRep = FactionReputation[FactionName];
    
    // Find if we've crossed any thresholds
    for (float Threshold : Thresholds)
    {
        // Check if we've crossed this threshold (in either direction)
        if ((PreviousFactionReputations.Contains(FactionName) && 
             ((CurrentRep >= Threshold && PreviousFactionReputations[FactionName] < Threshold) ||
              (CurrentRep < Threshold && PreviousFactionReputations[FactionName] >= Threshold))))
        {
            // Broadcast faction threshold event
            OnFactionReputationThresholdCrossed.Broadcast(FactionName, Threshold, CurrentRep > PreviousFactionReputations[FactionName]);
            
            // Update faction availability in the game world
            UOathGameInstance* GameInstance = Cast<UOathGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
            if (GameInstance)
            {
                GameInstance->UpdateFactionAvailability(FactionName, CurrentRep);
            }
        }
    }
    
    // Update the previous value
    PreviousFactionReputations.FindOrAdd(FactionName) = CurrentRep;
}

void UReputationComponent::GenerateMilestoneContent(const FString& ReputationType, float Milestone)
{
    // Get necessary references
    UOathGameInstance* GameInstance = Cast<UOathGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
    if (!GameInstance || !GameInstance->GetProceduralGenerator())
    {
        return;
    }
    
    UProceduralGenerator* Generator = GameInstance->GetProceduralGenerator();
    
    // Generate appropriate content based on the type and milestone
    if (ReputationType == "Combat")
    {
        // For combat milestones, generate special monsters or combat challenges
        if (Milestone >= 5000.0f)
        {
            // Legendary monster challenge
            Generator->GenerateNotoriousMonsterEncounter(5, "Legendary");
        }
        else if (Milestone >= 1000.0f)
        {
            // Elite monster pack
            Generator->GenerateNotoriousMonsterEncounter(3, "Elite");
        }
        else
        {
            // Standard challenging monster
            Generator->GenerateNotoriousMonsterEncounter(1, "Standard");
        }
    }
    else if (ReputationType == "Quest")
    {
        // For quest milestones, generate special quests or quest chains
        EQuestType QuestType = EQuestType::Fetch; // Default
        
        if (Milestone >= 5000.0f)
        {
            // Kingdom-defining quest chain
            Generator->GenerateSpecialQuestChain("Kingdom", 5, 3);
        }
        else if (Milestone >= 1000.0f)
        {
            // Special faction quest
            Generator->GenerateSpecialQuestChain("Faction", 3, 2);
        }
        else
        {
            // Standard special quest
            Generator->GenerateSpecialQuest(2);
        }
    }
}