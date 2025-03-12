// UTreeAutomataFunctionLibrary.cpp
#include "UTreeAutomataFunctionLibrary.h"
#include "TALogging.h"
#include "UTAController.h"

UTAController* UTreeAutomataFunctionLibrary::GetTreeAutomataController(AActor* Actor)
{
    if (!Actor)
    {
        UE_LOG(LogTreeAutomata, Warning, TEXT("GetTreeAutomataController: Invalid actor"));
        return nullptr;
    }
    
    // Look for existing controller component
    UTAController* Controller = Actor->FindComponentByClass<UTAController>();
    if (!Controller)
    {
        // In a real implementation, we would look for controller on the game instance or player controller
        // For this example, just return null
        UE_LOG(LogTreeAutomata, Verbose, TEXT("No Tree Automata controller found on %s"), *Actor->GetName());
    }
    
    return Controller;
}

UTAController* UTreeAutomataFunctionLibrary::CreateTreeAutomataController(AActor* Actor)
{
    if (!Actor)
    {
        UE_LOG(LogTreeAutomata, Warning, TEXT("CreateTreeAutomataController: Invalid actor"));
        return nullptr;
    }
    
    // Check if controller already exists
    UTAController* ExistingController = GetTreeAutomataController(Actor);
    if (ExistingController)
    {
        return ExistingController;
    }
    
    // Create new controller
    UTAController* NewController = NewObject<UTAController>(Actor);
    NewController->Initialize(Actor);
    
    UE_LOG(LogTreeAutomata, Display, TEXT("Created Tree Automata controller for %s"), *Actor->GetName());
    
    return NewController;
}

bool UTreeAutomataFunctionLibrary::ProcessAutomatonInput(AActor* Actor, const FString& AutomatonName, const FString& InputID, const TMap<FString, FVariant>& Params)
{
    UTAController* Controller = GetTreeAutomataController(Actor);
    if (!Controller)
    {
        UE_LOG(LogTreeAutomata, Warning, TEXT("ProcessAutomatonInput: No controller found on %s"), 
            Actor ? *Actor->GetName() : TEXT("NULL"));
        return false;
    }
    
    return Controller->ProcessInput(AutomatonName, InputID, Params);
}

TArray<FTAActionInfo> UTreeAutomataFunctionLibrary::GetAvailablePlayerActions(AActor* Actor, const FString& AutomatonName)
{
    UTAController* Controller = GetTreeAutomataController(Actor);
    if (!Controller)
    {
        UE_LOG(LogTreeAutomata, Warning, TEXT("GetAvailablePlayerActions: No controller found on %s"), 
            Actor ? *Actor->GetName() : TEXT("NULL"));
        return TArray<FTAActionInfo>();
    }
    
    return Controller->GetAvailableActions(AutomatonName);
}

void UTreeAutomataFunctionLibrary::SetGlobalVariable(AActor* Actor, const FString& VariableName, const FVariant& Value)
{
    UTAController* Controller = GetTreeAutomataController(Actor);
    if (!Controller)
    {
        UE_LOG(LogTreeAutomata, Warning, TEXT("SetGlobalVariable: No controller found on %s"), 
            Actor ? *Actor->GetName() : TEXT("NULL"));
        return;
    }
    
    Controller->SetGlobalVariable(VariableName, Value);
}

FVariant UTreeAutomataFunctionLibrary::GetGlobalVariable(AActor* Actor, const FString& VariableName, const FVariant& DefaultValue)
{
    UTAController* Controller = GetTreeAutomataController(Actor);
    if (!Controller)
    {
        UE_LOG(LogTreeAutomata, Warning, TEXT("GetGlobalVariable: No controller found on %s"), 
            Actor ? *Actor->GetName() : TEXT("NULL"));
        return DefaultValue;
    }
    
    return Controller->GetGlobalVariable(VariableName, DefaultValue);
}

TMap<FString, FVariant> UTreeAutomataFunctionLibrary::ConvertBlueprintMapToVariantMap(const TMap<FString, FString>& BlueprintMap)
{
    TMap<FString, FVariant> Result;
    
    for (const auto& Pair : BlueprintMap)
    {
        Result.Add(Pair.Key, FVariant(Pair.Value));
    }
    
    return Result;
}

UTAInstance* UTreeAutomataFunctionLibrary::CreateAutomaton(AActor* Actor, UObject* Template, const FString& InstanceName)
{
    UTAController* Controller = GetTreeAutomataController(Actor);
    if (!Controller)
    {
        Controller = CreateTreeAutomataController(Actor);
        if (!Controller)
        {
            UE_LOG(LogTreeAutomata, Warning, TEXT("CreateAutomaton: Failed to create controller on %s"), 
                Actor ? *Actor->GetName() : TEXT("NULL"));
            return nullptr;
        }
    }
    
    return Controller->CreateInstance(Template, InstanceName);
}

bool UTreeAutomataFunctionLibrary::ForceTransitionToNode(AActor* Actor, const FString& AutomatonName, const FString& NodeName)
{
    UTAController* Controller = GetTreeAutomataController(Actor);
    if (!Controller)
    {
        UE_LOG(LogTreeAutomata, Warning, TEXT("ForceTransitionToNode: No controller found on %s"), 
            Actor ? *Actor->GetName() : TEXT("NULL"));
        return false;
    }
    
    return Controller->ForceTransitionToNodeByName(AutomatonName, NodeName);
}