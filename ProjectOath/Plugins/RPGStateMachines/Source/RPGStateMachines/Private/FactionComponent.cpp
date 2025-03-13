#include "FactionComponent.h"

UFactionComponent::UFactionComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UFactionComponent::BeginPlay()
{
    Super::BeginPlay();
}

int32 UFactionComponent::GetReputationWithFaction(FName FactionID) const
{
    if (const int32* Reputation = FactionReputations.Find(FactionID))
    {
        return *Reputation;
    }
    return 0; // Neutral reputation by default
}

void UFactionComponent::SetReputationWithFaction(FName FactionID, int32 Value)
{
    FactionReputations.Add(FactionID, Value);
}

void UFactionComponent::ModifyReputationWithFaction(FName FactionID, int32 Delta)
{
    int32 CurrentReputation = GetReputationWithFaction(FactionID);
    FactionReputations.Add(FactionID, CurrentReputation + Delta);
}

void UFactionComponent::ModifyCurrentNPCReputation(int32 Delta)
{
    if (!CurrentNPCFaction.IsNone())
    {
        ModifyReputationWithFaction(CurrentNPCFaction, Delta);
    }
}

void UFactionComponent::SetCurrentNPC(FName FactionID)
{
    CurrentNPCFaction = FactionID;
}