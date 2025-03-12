// TAContext.cpp
#include "TAContext.h"
#include "TALogging.h"

FTAContext::FTAContext()
    : World(nullptr)
    , PlayerActor(nullptr)
    , InputID(TEXT(""))
    , bDebugTraceEnabled(false)
{
}

FTAContext::FTAContext(UWorld* InWorld, AActor* InPlayerActor)
    : World(InWorld)
    , PlayerActor(InPlayerActor)
    , InputID(TEXT(""))
    , bDebugTraceEnabled(false)
{
}

void FTAContext::AddParam(const FString& Name, const FVariant& Value)
{
    InputParams.Add(Name, Value);
}

FVariant FTAContext::GetParam(const FString& Name, const FVariant& DefaultValue) const
{
    const FVariant* Found = InputParams.Find(Name);
    return Found ? *Found : DefaultValue;
}

void FTAContext::SetGlobal(const FString& Name, const FVariant& Value)
{
    GlobalState.Add(Name, Value);
}

FVariant FTAContext::GetGlobal(const FString& Name, const FVariant& DefaultValue) const
{
    const FVariant* Found = GlobalState.Find(Name);
    return Found ? *Found : DefaultValue;
}

void FTAContext::DebugTrace(const FString& Message) const
{
    if (bDebugTraceEnabled)
    {
        UE_LOG(LogTreeAutomata, Display, TEXT("[TA Debug] %s"), *Message);
    }
}

FTAContext FTAContext::Clone() const
{
    FTAContext NewContext;
    NewContext.World = World;
    NewContext.PlayerActor = PlayerActor;
    NewContext.InputID = InputID;
    NewContext.InputParams = InputParams;
    NewContext.GlobalState = GlobalState;
    NewContext.bDebugTraceEnabled = bDebugTraceEnabled;
    return NewContext;
}