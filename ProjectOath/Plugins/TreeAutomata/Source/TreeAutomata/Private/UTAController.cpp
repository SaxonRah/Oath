// UTAController.cpp
#include "UTAController.h"
#include "UTAInstance.h"
#include "TALogging.h"
#include "TAStateSaveGame.h"
#include "Kismet/GameplayStatics.h"

UTAController::UTAController()
    : OwnerActor(nullptr)
    , World(nullptr)
    , bDebugMode(false)
    , bAutoSaveOnTransition(false)
{
}

void UTAController::Initialize(AActor* InOwner)
{
    OwnerActor = InOwner;
    
    if (OwnerActor)
    {
        World = OwnerActor->GetWorld();
    }
    
    UE_LOG(LogTreeAutomata, Display, TEXT("Tree Automata Controller initialized for %s"),
        OwnerActor ? *OwnerActor->GetName() : TEXT("NULL"));
}

UTAInstance* UTAController::CreateInstance(UObject* Template, const FString& InstanceName)
{
    // Check if instance name already exists
    if (ActiveInstances.Contains(InstanceName))
    {
        UE_LOG(LogTreeAutomata, Warning, TEXT("Tree Automata instance '%s' already exists"), *InstanceName);
        return nullptr;
    }
    
    // Create new instance
    UTAInstance* NewInstance = NewObject<UTAInstance>(this);
    NewInstance->Initialize(InstanceName, Template, OwnerActor);
    
    // Add to active instances
    ActiveInstances.Add(InstanceName, NewInstance);
    
    // Broadcast creation event
    OnAutomatonCreated.Broadcast(InstanceName);
    
    UE_LOG(LogTreeAutomata, Display, TEXT("Created Tree Automata instance '%s'%s"),
        *InstanceName,
        Template ? *FString::Printf(TEXT(" from template '%s'"), *Template->GetName()) : TEXT(""));
    
    return NewInstance;
}

UTAInstance* UTAController::CreateInstanceFromNodes(TSharedPtr<FTANode> RootNode, const FString& InstanceName)
{
    // Check if instance name already exists
    if (ActiveInstances.Contains(InstanceName))
    {
        UE_LOG(LogTreeAutomata, Warning, TEXT("Tree Automata instance '%s' already exists"), *InstanceName);
        return nullptr;
    }
    
    // Check for valid root node
    if (!RootNode.IsValid())
    {
        UE_LOG(LogTreeAutomata, Error, TEXT("Cannot create Tree Automata instance '%s' with invalid root node"), *InstanceName);
        return nullptr;
    }
    
    // Create new instance
    UTAInstance* NewInstance = NewObject<UTAInstance>(this);
    NewInstance->Initialize(InstanceName, nullptr, OwnerActor);
    NewInstance->SetRootNode(RootNode);
    
    // Add to active instances
    ActiveInstances.Add(InstanceName, NewInstance);
    
    // Broadcast creation event
    OnAutomatonCreated.Broadcast(InstanceName);
    
    UE_LOG(LogTreeAutomata, Display, TEXT("Created Tree Automata instance '%s' from node hierarchy"), *InstanceName);
    
    return NewInstance;
}

bool UTAController::ProcessInput(const FString& InstanceName, const FString& InputID, const TMap<FString, FVariant>& Params)
{
    // Find the instance
    UTAInstance* Instance = ActiveInstances.FindRef(InstanceName);
    if (!Instance)
    {
        UE_LOG(LogTreeAutomata, Warning, TEXT("ProcessInput: Tree Automata instance '%s' not found"), *InstanceName);
        return false;
    }
    
    // Create context
    FTAContext Context(World, OwnerActor);
    Context.InputID = InputID;
    Context.InputParams = Params;
    Context.GlobalState = GlobalState;
    Context.bDebugTraceEnabled = bDebugMode;
    
    // Process the input
    FGuid PreviousNodeID = Instance->GetCurrentNodeID();
    bool Result = Instance->ProcessInput(Context);
    
    // If node changed, broadcast event
    if (Result && Instance->GetCurrentNodeID() != PreviousNodeID)
    {
        OnAutomatonTransition.Broadcast(InstanceName, Instance->GetCurrentNodeID());
        
        // Auto-save if enabled
        if (bAutoSaveOnTransition)
        {
            SaveAutomataState(TEXT("AutoSave"));
        }
    }
    
    return Result;
}

TArray<FTAActionInfo> UTAController::GetAvailableActions(const FString& InstanceName)
{
    // Find the instance
    UTAInstance* Instance = ActiveInstances.FindRef(InstanceName);
    if (!Instance)
    {
        UE_LOG(LogTreeAutomata, Warning, TEXT("GetAvailableActions: Tree Automata instance '%s' not found"), *InstanceName);
        return TArray<FTAActionInfo>();
    }
    
    return Instance->GetAvailableActions();
}

bool UTAController::CanReachState(const FString& InstanceName, const FGuid& TargetNodeID)
{
    // Find the instance
    UTAInstance* Instance = ActiveInstances.FindRef(InstanceName);
    if (!Instance)
    {
        UE_LOG(LogTreeAutomata, Warning, TEXT("CanReachState: Tree Automata instance '%s' not found"), *InstanceName);
        return false;
    }
    
    return Instance->CanReachNode(TargetNodeID);
}

TArray<FTAPath> UTAController::FindPathsToState(const FString& InstanceName, const FGuid& TargetNodeID, int32 MaxPaths)
{
    // Find the instance
    UTAInstance* Instance = ActiveInstances.FindRef(InstanceName);
    if (!Instance)
    {
        UE_LOG(LogTreeAutomata, Warning, TEXT("FindPathsToState: Tree Automata instance '%s' not found"), *InstanceName);
        return TArray<FTAPath>();
    }
    
    return Instance->FindPathsToNode(TargetNodeID, MaxPaths);
}

bool UTAController::SaveAutomataState(const FString& SaveSlot)
{
    // Create save game object
    UTAStateSaveGame* SaveGame = Cast<UTAStateSaveGame>(UGameplayStatics::CreateSaveGameObject(UTAStateSaveGame::StaticClass()));
    if (!SaveGame)
    {
        UE_LOG(LogTreeAutomata, Error, TEXT("SaveAutomataState: Failed to create save game object"));
        return false;
    }
    
    // Save global state
    for (const auto& Pair : GlobalState)
    {
        SaveGame->GlobalVariables.Add(Pair.Key, Pair.Value.ToString());
    }
    
    // Save each instance
    for (const auto& Pair : ActiveInstances)
    {
        FString InstanceName = Pair.Key;
        UTAInstance* Instance = Pair.Value;
        
        if (Instance)
        {
            FTAInstanceSaveData InstanceData;
            Instance->SaveState(InstanceData);
            SaveGame->SavedInstances.Add(InstanceName, InstanceData);
        }
    }
    
    // Save version
    SaveGame->SaveVersion = 1;
    
    // Save to disk
    bool bSuccess = UGameplayStatics::SaveGameToSlot(SaveGame, SaveSlot, 0);
    
    if (bSuccess)
    {
        UE_LOG(LogTreeAutomata, Display, TEXT("Saved %d Tree Automata instances to save slot '%s'"), 
            SaveGame->SavedInstances.Num(), *SaveSlot);
    }
    else
    {
        UE_LOG(LogTreeAutomata, Error, TEXT("Failed to save Tree Automata state to slot '%s'"), *SaveSlot);
    }
    
    return bSuccess;
}

bool UTAController::LoadAutomataState(const FString& SaveSlot)
{
    // Check if save exists
    if (!UGameplayStatics::DoesSaveGameExist(SaveSlot, 0))
    {
        UE_LOG(LogTreeAutomata, Warning, TEXT("LoadAutomataState: Save slot '%s' does not exist"), *SaveSlot);
        return false;
    }
    
    // Load save game
    UTAStateSaveGame* SaveGame = Cast<UTAStateSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlot, 0));
    if (!SaveGame)
    {
        UE_LOG(LogTreeAutomata, Error, TEXT("LoadAutomataState: Failed to load save game from slot '%s'"), *SaveSlot);
        return false;
    }
    
    // Clear existing state
    ResetAllAutomata();
    GlobalState.Empty();
    
    // Load global state
    for (const auto& Pair : SaveGame->GlobalVariables)
    {
        GlobalState.Add(Pair.Key, FVariant(Pair.Value));
    }
    
    // Load each instance
    for (const auto& Pair : SaveGame->SavedInstances)
    {
        FString InstanceName = Pair.Key;
        const FTAInstanceSaveData& InstanceData = Pair.Value;
        
        // Load template if available
        UObject* Template = nullptr;
        if (!InstanceData.TemplatePath.IsNull())
        {
            Template = InstanceData.TemplatePath.TryLoad();
        }
        
        // Create instance
        UTAInstance* Instance = CreateInstance(Template, InstanceName);
        if (Instance)
        {
            Instance->LoadState(InstanceData);
        }
    }
    
    UE_LOG(LogTreeAutomata, Display, TEXT("Loaded %d Tree Automata instances from save slot '%s'"), 
        SaveGame->SavedInstances.Num(), *SaveSlot);
    
    return true;
}

void UTAController::ResetAllAutomata()
{
    // Reset each instance
    for (auto& Pair : ActiveInstances)
    {
        UTAInstance* Instance = Pair.Value;
        if (Instance)
        {
            Instance->Reset();
        }
    }
    
    UE_LOG(LogTreeAutomata, Display, TEXT("Reset all Tree Automata instances"));
}

bool UTAController::ResetAutomaton(const FString& InstanceName)
{
    // Find the instance
    UTAInstance* Instance = ActiveInstances.FindRef(InstanceName);
    if (!Instance)
    {
        UE_LOG(LogTreeAutomata, Warning, TEXT("ResetAutomaton: Tree Automata instance '%s' not found"), *InstanceName);
        return false;
    }
    
    // Reset the instance
    Instance->Reset();
    
    UE_LOG(LogTreeAutomata, Display, TEXT("Reset Tree Automata instance '%s'"), *InstanceName);
    
    return true;
}

bool UTAController::DestroyInstance(const FString& InstanceName)
{
    // Find the instance
    UTAInstance* Instance = ActiveInstances.FindRef(InstanceName);
    if (!Instance)
    {
        UE_LOG(LogTreeAutomata, Warning, TEXT("DestroyInstance: Tree Automata instance '%s' not found"), *InstanceName);
        return false;
    }
    
    // Remove from active instances
    ActiveInstances.Remove(InstanceName);
    
    // Broadcast destroy event
    OnAutomatonDestroyed.Broadcast(InstanceName);
    
    UE_LOG(LogTreeAutomata, Display, TEXT("Destroyed Tree Automata instance '%s'"), *InstanceName);
    
    return true;
}

FGuid UTAController::GetCurrentNodeID(const FString& InstanceName)
{
    // Find the instance
    UTAInstance* Instance = ActiveInstances.FindRef(InstanceName);
    if (!Instance)
    {
        UE_LOG(LogTreeAutomata, Warning, TEXT("GetCurrentNodeID: Tree Automata instance '%s' not found"), *InstanceName);
        return FGuid();
    }
    
    return Instance->GetCurrentNodeID();
}

FString UTAController::GetCurrentNodeName(const FString& InstanceName)
{
    // Find the instance
    UTAInstance* Instance = ActiveInstances.FindRef(InstanceName);
    if (!Instance)
    {
        UE_LOG(LogTreeAutomata, Warning, TEXT("GetCurrentNodeName: Tree Automata instance '%s' not found"), *InstanceName);
        return TEXT("");
    }
    
    return Instance->GetCurrentNodeName();
}

bool UTAController::ForceTransitionToNode(const FString& InstanceName, const FGuid& TargetNodeID)
{
   // Find the instance
   UTAInstance* Instance = ActiveInstances.FindRef(InstanceName);
   if (!Instance)
   {
       UE_LOG(LogTreeAutomata, Warning, TEXT("ForceTransitionToNode: Tree Automata instance '%s' not found"), *InstanceName);
       return false;
   }
   
   // Attempt to force transition
   bool Result = Instance->ForceTransitionToNode(TargetNodeID);
   
   if (Result)
   {
       // Broadcast transition event
       OnAutomatonTransition.Broadcast(InstanceName, TargetNodeID);
       
       UE_LOG(LogTreeAutomata, Display, TEXT("Forced transition in instance '%s' to node %s"), 
           *InstanceName, *TargetNodeID.ToString());
   }
   else
   {
       UE_LOG(LogTreeAutomata, Warning, TEXT("Failed to force transition in instance '%s' to node %s"), 
           *InstanceName, *TargetNodeID.ToString());
   }
   
   return Result;
}

bool UTAController::ForceTransitionToNodeByName(const FString& InstanceName, const FString& NodeName)
{
   // Find the instance
   UTAInstance* Instance = ActiveInstances.FindRef(InstanceName);
   if (!Instance)
   {
       UE_LOG(LogTreeAutomata, Warning, TEXT("ForceTransitionToNodeByName: Tree Automata instance '%s' not found"), *InstanceName);
       return false;
   }
   
   // Find the node by name
   TSharedPtr<FTANode> RootNode = Instance->GetRootNode();
   if (!RootNode.IsValid())
   {
       UE_LOG(LogTreeAutomata, Warning, TEXT("ForceTransitionToNodeByName: Instance '%s' has invalid root node"), *InstanceName);
       return false;
   }
   
   TSharedPtr<FTANode> TargetNode = FindNodeByName(RootNode, NodeName);
   if (!TargetNode.IsValid())
   {
       UE_LOG(LogTreeAutomata, Warning, TEXT("ForceTransitionToNodeByName: Node '%s' not found in instance '%s'"), 
           *NodeName, *InstanceName);
       return false;
   }
   
   // Force transition to the found node
   return ForceTransitionToNode(InstanceName, TargetNode->NodeID);
}

void UTAController::SetGlobalVariable(const FString& VariableName, const FVariant& Value)
{
   GlobalState.Add(VariableName, Value);
   
   if (bDebugMode)
   {
       UE_LOG(LogTreeAutomata, Display, TEXT("Set global variable '%s' = %s"), 
           *VariableName, *Value.ToString());
   }
}

FVariant UTAController::GetGlobalVariable(const FString& VariableName, const FVariant& DefaultValue)
{
   const FVariant* Found = GlobalState.Find(VariableName);
   return Found ? *Found : DefaultValue;
}

void UTAController::Serialize(FArchive& Ar)
{
   // Serialize global state
   int32 GlobalStateCount = GlobalState.Num();
   Ar << GlobalStateCount;
   
   if (Ar.IsLoading())
   {
       GlobalState.Empty(GlobalStateCount);
       for (int32 i = 0; i < GlobalStateCount; ++i)
       {
           FString Key;
           FVariant Value;
           Ar << Key;
           Ar << Value;
           GlobalState.Add(Key, Value);
       }
   }
   else
   {
       for (auto& Pair : GlobalState)
       {
           FString Key = Pair.Key;
           Ar << Key;
           Ar << Pair.Value;
       }
   }
   
   // Serialize active instances
   int32 InstanceCount = ActiveInstances.Num();
   Ar << InstanceCount;
   
   if (Ar.IsLoading())
   {
       // Clear existing instances
       for (auto& Pair : ActiveInstances)
       {
           OnAutomatonDestroyed.Broadcast(Pair.Key);
       }
       ActiveInstances.Empty(InstanceCount);
       
       // Load instances
       for (int32 i = 0; i < InstanceCount; ++i)
       {
           FString InstanceName;
           Ar << InstanceName;
           
           UTAInstance* Instance = NewObject<UTAInstance>(this);
           Ar << Instance;
           
           ActiveInstances.Add(InstanceName, Instance);
           OnAutomatonCreated.Broadcast(InstanceName);
       }
   }
   else
   {
       // Save instances
       for (auto& Pair : ActiveInstances)
       {
           FString InstanceName = Pair.Key;
           Ar << InstanceName;
           Ar << Pair.Value;
       }
   }
}

TSharedPtr<FTANode> UTAController::FindNodeByID(TSharedPtr<FTANode> StartNode, const FGuid& TargetID)
{
   if (!StartNode.IsValid())
   {
       return nullptr;
   }
   
   // Check if this is the target node
   if (StartNode->NodeID == TargetID)
   {
       return StartNode;
   }
   
   // Search children
   for (const TSharedPtr<FTANode>& Child : StartNode->Children)
   {
       TSharedPtr<FTANode> Found = FindNodeByID(Child, TargetID);
       if (Found.IsValid())
       {
           return Found;
       }
   }
   
   // Search transitions
   for (const FTATransition& Transition : StartNode->Transitions)
   {
       if (Transition.TargetNode.IsValid() && Transition.TargetNode->NodeID == TargetID)
       {
           return Transition.TargetNode;
       }
       
       TSharedPtr<FTANode> Found = FindNodeByID(Transition.TargetNode, TargetID);
       if (Found.IsValid())
       {
           return Found;
       }
   }
   
   return nullptr;
}

TSharedPtr<FTANode> UTAController::FindNodeByName(TSharedPtr<FTANode> StartNode, const FString& TargetName)
{
   if (!StartNode.IsValid())
   {
       return nullptr;
   }
   
   // Check if this is the target node
   if (StartNode->NodeName == TargetName)
   {
       return StartNode;
   }
   
   // Search children
   for (const TSharedPtr<FTANode>& Child : StartNode->Children)
   {
       TSharedPtr<FTANode> Found = FindNodeByName(Child, TargetName);
       if (Found.IsValid())
       {
           return Found;
       }
   }
   
   // Search transitions
   for (const FTATransition& Transition : StartNode->Transitions)
   {
       if (Transition.TargetNode.IsValid() && Transition.TargetNode->NodeName == TargetName)
       {
           return Transition.TargetNode;
       }
       
       TSharedPtr<FTANode> Found = FindNodeByName(Transition.TargetNode, TargetName);
       if (Found.IsValid())
       {
           return Found;
       }
   }
   
   return nullptr;
}