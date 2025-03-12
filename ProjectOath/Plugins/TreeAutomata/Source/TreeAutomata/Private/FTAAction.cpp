// FTAAction.cpp
#include "FTAAction.h"
#include "TALogging.h"

//------------------------------------------------------------------------------
// FFTAAction
//------------------------------------------------------------------------------

FFTAAction::FFTAAction()
    : ActionID(FGuid::NewGuid())
    , ActionName(TEXT("Base Action"))
{
}

FFTAAction::~FFTAAction()
{
}

void FFTAAction::Execute(const FTAContext& Context) const
{
    // Base implementation does nothing
}

FString FFTAAction::GetDescription() const
{
    return ActionName;
}

TSharedPtr<FFTAAction> FFTAAction::Clone() const
{
    TSharedPtr<FFTAAction> Clone = MakeShared<FFTAAction>();
    Clone->ActionID = FGuid::NewGuid();
    Clone->ActionName = ActionName;
    return Clone;
}

void FFTAAction::Serialize(FArchive& Ar)
{
    Ar << ActionID;
    Ar << ActionName;
}

//------------------------------------------------------------------------------
// FSetVariableAction
//------------------------------------------------------------------------------

FSetVariableAction::FSetVariableAction()
{
    ActionName = TEXT("Set Variable");
    VariableName = TEXT("");
    bUseGlobalState = false;
}

void FSetVariableAction::Execute(const FTAContext& Context) const
{
    if (bUseGlobalState)
    {
        // Modify global state
        // Need to cast away const for this operation
        FTAContext& MutableContext = const_cast<FTAContext&>(Context);
        MutableContext.GlobalState.Add(VariableName, Value);
    }
    else
    {
        // This would modify node state in a real implementation
        // Simplified for this example
        UE_LOG(LogTreeAutomata, Display, TEXT("SetVariableAction: Would set node variable '%s' to %s"),
            *VariableName, *Value.ToString());
    }
}

FString FSetVariableAction::GetDescription() const
{
    return FString::Printf(TEXT("Set %s variable '%s' to %s"),
        bUseGlobalState ? TEXT("global") : TEXT("node"),
        *VariableName,
        *Value.ToString());
}

TSharedPtr<FFTAAction> FSetVariableAction::Clone() const
{
    TSharedPtr<FSetVariableAction> Clone = MakeShared<FSetVariableAction>();
    Clone->ActionID = FGuid::NewGuid();
    Clone->ActionName = ActionName;
    Clone->VariableName = VariableName;
    Clone->Value = Value;
    Clone->bUseGlobalState = bUseGlobalState;
    return Clone;
}

void FSetVariableAction::Serialize(FArchive& Ar)
{
    Super::Serialize(Ar);
    Ar << VariableName;
    Ar << bUseGlobalState;

    // Value serialization would depend on FTAVariant implementation
}

//------------------------------------------------------------------------------
// FLogMessageAction
//------------------------------------------------------------------------------

FLogMessageAction::FLogMessageAction()
{
    ActionName = TEXT("Log Message");
    Message = TEXT("");
    Verbosity = (uint8)ELogVerbosity::Display;
}

void FLogMessageAction::Execute(const FTAContext& Context) const
{
    FString FormattedMessage = Message;

    // Replace variables in message
    // {Global:VarName} for global variables
    // {Input:ParamName} for input parameters

    TArray<FString> GlobalVars;
    TArray<FString> InputVars;

    // Extract variable placeholders
    FRegexPattern GlobalPattern(TEXT("\\{Global:([^}]+)\\}"));
    FRegexMatcher GlobalMatcher(GlobalPattern, FormattedMessage);

    while (GlobalMatcher.FindNext())
    {
        FString VarName = GlobalMatcher.GetCaptureGroup(1);
        const FTAVariant* Value = Context.GlobalState.Find(VarName);
        if (Value)
        {
            FString ValueStr = Value->ToString();
            FormattedMessage = FormattedMessage.Replace(*FString::Printf(TEXT("{Global:%s}"), *VarName), *ValueStr);
        }
    }

    FRegexPattern InputPattern(TEXT("\\{Input:([^}]+)\\}"));
    FRegexMatcher InputMatcher(InputPattern, FormattedMessage);

    while (InputMatcher.FindNext())
    {
        FString ParamName = InputMatcher.GetCaptureGroup(1);
        const FTAVariant* Value = Context.InputParams.Find(ParamName);
        if (Value)
        {
            FString ValueStr = Value->ToString();
            FormattedMessage = FormattedMessage.Replace(*FString::Printf(TEXT("{Input:%s}"), *ParamName), *ValueStr);
        }
    }

    // Log with appropriate verbosity
    switch ((ELogVerbosity::Type)Verbosity)
    {
    case ELogVerbosity::Fatal:
        UE_LOG(LogTreeAutomata, Fatal, TEXT("%s"), *FormattedMessage);
        break;
    case ELogVerbosity::Error:
        UE_LOG(LogTreeAutomata, Error, TEXT("%s"), *FormattedMessage);
        break;
    case ELogVerbosity::Warning:
        UE_LOG(LogTreeAutomata, Warning, TEXT("%s"), *FormattedMessage);
        break;
    case ELogVerbosity::Display:
        UE_LOG(LogTreeAutomata, Display, TEXT("%s"), *FormattedMessage);
        break;
    case ELogVerbosity::Log:
        UE_LOG(LogTreeAutomata, Log, TEXT("%s"), *FormattedMessage);
        break;
    case ELogVerbosity::Verbose:
        UE_LOG(LogTreeAutomata, Verbose, TEXT("%s"), *FormattedMessage);
        break;
    case ELogVerbosity::VeryVerbose:
        UE_LOG(LogTreeAutomata, VeryVerbose, TEXT("%s"), *FormattedMessage);
        break;
    default:
        UE_LOG(LogTreeAutomata, Display, TEXT("%s"), *FormattedMessage);
        break;
    }
}

FString FLogMessageAction::GetDescription() const
{
    return FString::Printf(TEXT("Log message: \"%s\""), *Message);
}

TSharedPtr<FFTAAction> FLogMessageAction::Clone() const
{
    TSharedPtr<FLogMessageAction> Clone = MakeShared<FLogMessageAction>();
    Clone->ActionID = FGuid::NewGuid();
    Clone->ActionName = ActionName;
    Clone->Message = Message;
    Clone->Verbosity = Verbosity;
    return Clone;
}

void FLogMessageAction::Serialize(FArchive& Ar)
{
    Super::Serialize(Ar);
    Ar << Message;
    Ar << Verbosity;
}

//------------------------------------------------------------------------------
// FSpawnActorAction
//------------------------------------------------------------------------------

FSpawnActorAction::FSpawnActorAction()
{
    ActionName = TEXT("Spawn Actor");
    ActorClass = nullptr;
    SpawnLocationType = ETAActorSpawnLocation::PlayerLocation;
    SpawnOffset = FVector::ZeroVector;
    ActorTag = NAME_None;
}

void FSpawnActorAction::Execute(const FTAContext& Context) const
{
    if (!Context.World || !ActorClass)
    {
        UE_LOG(LogTreeAutomata, Warning, TEXT("FSpawnActorAction::Execute: Invalid world or actor class"));
        return;
    }

    // Determine spawn location
    FVector SpawnLocation = FVector::ZeroVector;
    FRotator SpawnRotation = FRotator::ZeroRotator;

    switch (SpawnLocationType.GetValue())
    {
    case ETAActorSpawnLocation::PlayerLocation:
        if (Context.PlayerActor)
        {
            SpawnLocation = Context.PlayerActor->GetActorLocation();
            SpawnRotation = Context.PlayerActor->GetActorRotation();
        }
        break;

    case ETAActorSpawnLocation::PlayerViewpoint:
        if (Context.PlayerActor)
        {
            // Handle player viewpoint
            APlayerController* PC = Cast<APlayerController>(Context.PlayerActor->GetInstigatorController());
            if (PC)
            {
                FVector ViewLocation;
                FRotator ViewRotation;
                PC->GetPlayerViewPoint(ViewLocation, ViewRotation);

                SpawnLocation = ViewLocation;
                SpawnRotation = ViewRotation;
            }
            else
            {
                SpawnLocation = Context.PlayerActor->GetActorLocation();
                SpawnRotation = Context.PlayerActor->GetActorRotation();
            }
        }
        break;

    case ETAActorSpawnLocation::CustomLocation:
        // Custom location would be fetched from parameters
        const FTAVariant* LocationVar = Context.InputParams.Find(TEXT("SpawnLocation"));
        if (LocationVar && LocationVar->IsType<FVector>())
        {
            SpawnLocation = LocationVar->AsVector();
        }

        const FTAVariant* RotationVar = Context.InputParams.Find(TEXT("SpawnRotation"));
        if (RotationVar && RotationVar->IsType<FRotator>())
        {
            SpawnRotation = RotationVar->AsRotator();
        }
        break;
    }

    // Apply offset
    SpawnLocation += SpawnOffset;

    // Spawn parameters
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    // Spawn the actor
    AActor* SpawnedActor = Context.World->SpawnActor<AActor>(ActorClass, SpawnLocation, SpawnRotation, SpawnParams);

    if (SpawnedActor)
    {
        if (ActorTag != NAME_None)
        {
            SpawnedActor->Tags.AddUnique(ActorTag);
        }

        UE_LOG(LogTreeAutomata, Display, TEXT("Spawned actor of class %s at location %s"),
            *ActorClass->GetName(), *SpawnLocation.ToString());
    }
    else
    {
        UE_LOG(LogTreeAutomata, Warning, TEXT("Failed to spawn actor of class %s"),
            *ActorClass->GetName());
    }
}

FString FSpawnActorAction::GetDescription() const
{
    FString LocationTypeString;
    switch (SpawnLocationType.GetValue())
    {
    case ETAActorSpawnLocation::PlayerLocation:
        LocationTypeString = TEXT("at player location");
        break;
    case ETAActorSpawnLocation::PlayerViewpoint:
        LocationTypeString = TEXT("at player viewpoint");
        break;
    case ETAActorSpawnLocation::CustomLocation:
        LocationTypeString = TEXT("at custom location");
        break;
    default:
        LocationTypeString = TEXT("at unknown location");
        break;
    }

    return FString::Printf(TEXT("Spawn %s %s%s"),
        ActorClass ? *ActorClass->GetName() : TEXT("NULL"),
        *LocationTypeString,
        SpawnOffset.IsZero() ? TEXT("") : *FString::Printf(TEXT(" with offset %s"), *SpawnOffset.ToString()));
}

TSharedPtr<FFTAAction> FSpawnActorAction::Clone() const
{
    TSharedPtr<FSpawnActorAction> Clone = MakeShared<FSpawnActorAction>();
    Clone->ActionID = FGuid::NewGuid();
    Clone->ActionName = ActionName;
    Clone->ActorClass = ActorClass;
    Clone->SpawnLocationType = SpawnLocationType;
    Clone->SpawnOffset = SpawnOffset;
    Clone->ActorTag = ActorTag;
    return Clone;
}

void FSpawnActorAction::Serialize(FArchive& Ar)
{
    Super::Serialize(Ar);

    // Actor class reference would be serialized as a path in real implementation

    uint8 LocationTypeInt = (uint8)SpawnLocationType.GetValue();
    Ar << LocationTypeInt;
    SpawnLocationType = LocationTypeInt;
    // note: could be 'TEnumAsByte<ETAActorSpawnLocation> &TEnumAsByte<ETAActorSpawnLocation>::operator =(const TEnumAsByte<ETAActorSpawnLocation> &)'
    Ar << SpawnOffset;
    Ar << ActorTag;
}