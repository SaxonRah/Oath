// UTAInstance.cpp
#include "UTAInstance.h"
#include "TALogging.h"

UTAInstance::UTAInstance()
    : Template(nullptr)
    , InstanceName(TEXT(""))
    , OwnerActor(nullptr)
{
}

void UTAInstance::Initialize(const FString& InName, UObject* InTemplate, AActor* InOwner)
{
    InstanceName = InName;
    Template = InTemplate;
    OwnerActor = InOwner;
    
    // If template provided, load nodes from it
    if (Template)
    {
        // In a real implementation, this would load from the template asset
        // For this example, we'll create a simple root node
        RootNode = MakeShared<FTANode>();
        RootNode->NodeID = FGuid::NewGuid();
        RootNode->NodeName = TEXT("Root");
        RootNode->NodeType = TEXT("Root");
    }
    
    // Initialize to root node
    Reset();
}

bool UTAInstance::ProcessInput(const FTAContext& Context)
{
    if (!CurrentNode.IsValid())
    {
        UE_LOG(LogTreeAutomata, Warning, TEXT("%s: Cannot process input with invalid current node"), *InstanceName);
        return false;
    }
    
    // Clone context and add local state
    FTAContext LocalContext = Context;
    for (const auto& Pair : LocalState)
    {
        // Only add if not already present
        if (!LocalContext.GlobalState.Contains(Pair.Key))
        {
            LocalContext.GlobalState.Add(Pair.Key, Pair.Value);
        }
    }
    
    // Debug trace
    LocalContext.DebugTrace(FString::Printf(TEXT("%s: Processing input '%s' on node %s"),
        *InstanceName, *Context.InputID, *CurrentNode->NodeName));
    
    // Evaluate transitions
    TSharedPtr<FTANode> NextNode;
    bool bHasTransition = CurrentNode->EvaluateTransitions(LocalContext, NextNode);
    
    if (bHasTransition && NextNode.IsValid())
    {
        // Execute exit actions on current node
        CurrentNode->ExecuteExitActions(LocalContext);
        
        // Add to history
        History.Add(CurrentNode);
        
        // Update current node
        CurrentNode = NextNode;
        
        // Execute entry actions on new node
        CurrentNode->ExecuteEntryActions(LocalContext);
        
        LocalContext.DebugTrace(FString::Printf(TEXT("%s: Transitioned to node %s"),
            *InstanceName, *CurrentNode->NodeName));
        
        return true;
    }
    
    LocalContext.DebugTrace(FString::Printf(TEXT("%s: No valid transition from node %s for input '%s'"),
        *InstanceName, *CurrentNode->NodeName, *Context.InputID));
    
    return false;
}

TArray<FTAActionInfo> UTAInstance::GetAvailableActions() const
{
    TArray<FTAActionInfo> Actions;
    
    if (!CurrentNode.IsValid())
    {
        UE_LOG(LogTreeAutomata, Warning, TEXT("%s: Cannot get available actions with invalid current node"), *InstanceName);
        return Actions;
    }
    
    // In a real implementation, this would analyze outgoing transitions and return valid inputs
    // For this example, create a simple representation
    for (const FTATransition& Transition : CurrentNode->Transitions)
    {
        if (Transition.TargetNode.IsValid())
        {
            FTAActionInfo ActionInfo;
            ActionInfo.ActionID = FString::Printf(TEXT("Transition_%s"), *Transition.TransitionID.ToString());
            ActionInfo.DisplayName = FString::Printf(TEXT("Go to %s"), *Transition.TargetNode->NodeName);
            ActionInfo.TargetNodeID = Transition.TargetNode->NodeID;
            ActionInfo.TargetNodeName = Transition.TargetNode->NodeName;
            
            // Conditions would be analyzed here
            ActionInfo.AvailabilityStatus = ETAActionStatus::Available;
            
            Actions.Add(ActionInfo);
        }
    }
    
    return Actions;
}

bool UTAInstance::CanReachNode(const FGuid& TargetNodeID) const
{
    if (!CurrentNode.IsValid())
    {
        return false;
    }
    
    // Special case: current node is the target
    if (CurrentNode->NodeID == TargetNodeID)
    {
        return true;
    }
    
    // Create a set of visited nodes to prevent infinite loops
    TSet<FGuid> VisitedNodes;
    VisitedNodes.Add(CurrentNode->NodeID);
    
    // Queue of nodes to check
    TQueue<TSharedPtr<FTANode>> NodesToCheck;
    
    // Add all immediate transitions to queue
    for (const FTATransition& Transition : CurrentNode->Transitions)
    {
        if (Transition.TargetNode.IsValid())
        {
            NodesToCheck.Enqueue(Transition.TargetNode);
        }
    }
    
    // Breadth-first search
    while (!NodesToCheck.IsEmpty())
    {
        TSharedPtr<FTANode> Node;
        NodesToCheck.Dequeue(Node);
        
        if (!Node.IsValid())
        {
            continue;
        }
        
        // Check if this is the target
        if (Node->NodeID == TargetNodeID)
        {
            return true;
        }
        
        // Mark as visited
        VisitedNodes.Add(Node->NodeID);
        
        // Add all transitions to queue
        for (const FTATransition& Transition : Node->Transitions)
        {
            if (Transition.TargetNode.IsValid() && !VisitedNodes.Contains(Transition.TargetNode->NodeID))
            {
                NodesToCheck.Enqueue(Transition.TargetNode);
            }
        }
    }
    
    // Target not found
    return false;
}

TArray<FTAPath> UTAInstance::FindPathsToNode(const FGuid& TargetNodeID, int32 MaxPaths) const
{
    TArray<FTAPath> Paths;
    
    if (!CurrentNode.IsValid())
    {
        return Paths;
    }
    
    // Special case: current node is the target
    if (CurrentNode->NodeID == TargetNodeID)
    {
        FTAPath Path;
        FTAPathNode PathNode;
        PathNode.NodeID = CurrentNode->NodeID;
        PathNode.NodeName = CurrentNode->NodeName;
        Path.Nodes.Add(PathNode);
        Paths.Add(Path);
        return Paths;
    }
    
    // Start recursive search
    TArray<FTAPathNode> CurrentPath;
    FTAPathNode StartNode;
    StartNode.NodeID = CurrentNode->NodeID;
    StartNode.NodeName = CurrentNode->NodeName;
    CurrentPath.Add(StartNode);
    
    FindPathsRecursive(CurrentNode, TargetNodeID, CurrentPath, Paths, MaxPaths, 20);
    
    return Paths;
}

void UTAInstance::FindPathsRecursive(TSharedPtr<FTANode> CurrentNode, const FGuid& TargetID, 
    TArray<FTAPathNode>& CurrentPath, TArray<FTAPath>& Results, int32 MaxPaths, int32 MaxDepth) const
{
    // Stop if we have enough paths or reached max depth
    if (Results.Num() >= MaxPaths || CurrentPath.Num() > MaxDepth)
    {
        return;
    }
    
    // Check all transitions
    for (const FTATransition& Transition : CurrentNode->Transitions)
    {
        if (!Transition.TargetNode.IsValid())
        {
            continue;
        }
        
        // Check if we've already visited this node in current path (avoid cycles)
        bool bAlreadyVisited = false;
        for (const FTAPathNode& PathNode : CurrentPath)
        {
            if (PathNode.NodeID == Transition.TargetNode->NodeID)
            {
                bAlreadyVisited = true;
                break;
            }
        }
        
        if (bAlreadyVisited)
        {
            continue;
        }
        
        // Add node to current path
        FTAPathNode PathNode;
        PathNode.NodeID = Transition.TargetNode->NodeID;
        PathNode.NodeName = Transition.TargetNode->NodeName;
        PathNode.TransitionID = Transition.TransitionID;
        PathNode.TransitionName = Transition.TransitionName;
        CurrentPath.Add(PathNode);
        
        // Check if this is the target
        if (Transition.TargetNode->NodeID == TargetID)
        {
            // We found a path, add it to results
            FTAPath Path;
            Path.Nodes = CurrentPath;
            Results.Add(Path);
        }
        else
        {
            // Continue recursion
            FindPathsRecursive(Transition.TargetNode, TargetID, CurrentPath, Results, MaxPaths, MaxDepth);
        }
        
        // Remove node before trying next transition
        CurrentPath.RemoveAt(CurrentPath.Num() - 1);
        
        // Stop if we have enough paths
        if (Results.Num() >= MaxPaths)
        {
            return;
        }
    }
}

bool UTAInstance::ForceTransitionToNode(const FGuid& TargetNodeID)
{
    if (!RootNode.IsValid())
    {
        UE_LOG(LogTreeAutomata, Warning, TEXT("%s: Cannot force transition with invalid root node"), *InstanceName);
        return false;
    }
    
    // Find the target node
    TSharedPtr<FTANode> TargetNode = FindNodeByID(RootNode, TargetNodeID);
    if (!TargetNode.IsValid())
    {
        UE_LOG(LogTreeAutomata, Warning, TEXT("%s: Target node %s not found"), 
            *InstanceName, *TargetNodeID.ToString());
        return false;
    }
    
    // Create context for actions
    FTAContext Context;
    Context.World = OwnerActor ? OwnerActor->GetWorld() : nullptr;
    Context.PlayerActor = OwnerActor;
    
    // Add local state to context
    for (const auto& Pair : LocalState)
    {
        Context.GlobalState.Add(Pair.Key, Pair.Value);
    }
    
    // If we have a current node, execute exit actions
    if (CurrentNode.IsValid())
    {
        CurrentNode->ExecuteExitActions(Context);
        
        // Add to history
        History.Add(CurrentNode);
    }
    
    // Update current node
    CurrentNode = TargetNode;
    
    // Execute entry actions
    CurrentNode->ExecuteEntryActions(Context);
    
    UE_LOG(LogTreeAutomata, Display, TEXT("%s: Forced transition to node %s"), 
        *InstanceName, *CurrentNode->NodeName);
    
    return true;
}

void UTAInstance::Reset()
{
    // Clear history
    History.Empty();
    
    // Set current node to root
    CurrentNode = RootNode;
    
    // Execute entry actions on root if available
    if (CurrentNode.IsValid())
    {
        FTAContext Context;
        Context.World = OwnerActor ? OwnerActor->GetWorld() : nullptr;
        Context.PlayerActor = OwnerActor;
        
        for (const auto& Pair : LocalState)
        {
            Context.GlobalState.Add(Pair.Key, Pair.Value);
        }
        
        CurrentNode->ExecuteEntryActions(Context);
    }
    
    UE_LOG(LogTreeAutomata, Display, TEXT("%s: Reset to initial state"), *InstanceName);
}

void UTAInstance::SaveState(FTAInstanceSaveData& OutSaveData) const
{
    // Save template reference
    if (Template)
    {
        OutSaveData.TemplatePath = FSoftObjectPath(Template);
    }
    
    // Save current node ID
    OutSaveData.CurrentNodeID = CurrentNode.IsValid() ? CurrentNode->NodeID : FGuid();
    
    // Save history
    OutSaveData.HistoryNodeIDs.Empty(History.Num());
    for (const TSharedPtr<FTANode>& Node : History)
    {
        if (Node.IsValid())
        {
            OutSaveData.HistoryNodeIDs.Add(Node->NodeID);
        }
    }
    
    // Save variables
    OutSaveData.Variables.Empty(LocalState.Num());
    for (const auto& Pair : LocalState)
    {
        OutSaveData.Variables.Add(Pair.Key, Pair.Value.ToString());
    }
}

void UTAInstance::LoadState(const FTAInstanceSaveData& SaveData)
{
    // Clear history
    History.Empty();
    
    // Load node hierarchy from serialized data if present
    if (SaveData.SerializedNodes.Num() > 0)
    {
        // Create a memory archive to read from the serialized data
        FMemoryReader MemReader(SaveData.SerializedNodes);
        
        // Create a node map for reference resolution
        TMap<FGuid, TSharedPtr<FTANode>> NodeMap;
        
        // Read the root node flag
        bool bHasRootNode = false;
        MemReader << bHasRootNode;
        
        if (bHasRootNode)
        {
            // Deserialize the root node and its hierarchy
            RootNode = MakeShared<FTANode>();
            RootNode->Serialize(MemReader);
            
            // Build node map for reference resolution
            BuildNodeMap(RootNode, NodeMap);
            
            // Resolve references
            ResolveNodeReferences(RootNode, NodeMap);
        }
        else
        {
            UE_LOG(LogTreeAutomata, Warning, TEXT("%s: No root node in serialized data"), *InstanceName);
        }
    }
    else if (Template && Template->IsA<UTreeAutomatonAsset>())
    {
        // If no serialized data but we have a template, recreate from template
        UTreeAutomatonAsset* AutomatonAsset = Cast<UTreeAutomatonAsset>(Template);
        CreateNodesFromTemplate(AutomatonAsset);
    }
    
    // Set current node based on ID
    if (SaveData.CurrentNodeID.IsValid() && RootNode.IsValid())
    {
        // Find the node in our hierarchy
        TSharedPtr<FTANode> FoundNode = FindNodeByID(RootNode, SaveData.CurrentNodeID);
        if (FoundNode.IsValid())
        {
            CurrentNode = FoundNode;
            UE_LOG(LogTreeAutomata, Display, TEXT("%s: Set current node to %s"), 
                *InstanceName, *CurrentNode->NodeName);
        }
        else
        {
            // If node not found, try to force transition (which may rebuild connections)
            bool bSuccess = ForceTransitionToNode(SaveData.CurrentNodeID);
            
            if (!bSuccess)
            {
                UE_LOG(LogTreeAutomata, Warning, TEXT("%s: Failed to find current node %s, reverting to root"), 
                    *InstanceName, *SaveData.CurrentNodeID.ToString());
                CurrentNode = RootNode;
            }
        }
    }
    else
    {
        CurrentNode = RootNode;
    }
    
    // Rebuild history from IDs
    if (SaveData.HistoryNodeIDs.Num() > 0 && RootNode.IsValid())
    {
        for (const FGuid& NodeID : SaveData.HistoryNodeIDs)
        {
            TSharedPtr<FTANode> HistoryNode = FindNodeByID(RootNode, NodeID);
            if (HistoryNode.IsValid())
            {
                History.Add(HistoryNode);
            }
        }
        
        UE_LOG(LogTreeAutomata, Display, TEXT("%s: Reconstructed %d of %d history entries"), 
            *InstanceName, History.Num(), SaveData.HistoryNodeIDs.Num());
    }
    
    // Load variables
    LocalState.Empty(SaveData.Variables.Num());
    for (const auto& Pair : SaveData.Variables)
    {
        // Convert string to appropriate variant type
        FVariant Value;
        
        // Get type hint if available
        FString TypeHint;
        if (SaveData.VariableTypes.Contains(Pair.Key))
        {
            TypeHint = SaveData.VariableTypes[Pair.Key];
        }
        
        // Convert based on type hint
        if (TypeHint == TEXT("Integer"))
        {
            Value = FVariant(FCString::Atoi(*Pair.Value));
        }
        else if (TypeHint == TEXT("Float"))
        {
            Value = FVariant(FCString::Atof(*Pair.Value));
        }
        else if (TypeHint == TEXT("Boolean"))
        {
            Value = FVariant(Pair.Value.ToBool());
        }
        else if (TypeHint == TEXT("Vector"))
        {
            FVector Vec;
            Vec.InitFromString(Pair.Value);
            Value = FVariant(Vec);
        }
        else if (TypeHint == TEXT("Rotator"))
        {
            FRotator Rot;
            Rot.InitFromString(Pair.Value);
            Value = FVariant(Rot);
        }
        else
        {
            // Default to string if no type hint or unknown type
            Value = FVariant(Pair.Value);
        }
        
        LocalState.Add(Pair.Key, Value);
    }
    
    // Load completed transitions
    CompletedTransitions.Empty(SaveData.CompletedTransitions.Num());
    for (const FGuid& TransitionID : SaveData.CompletedTransitions)
    {
        CompletedTransitions.Add(TransitionID);
    }
    
    // Fire state loaded event
    OnStateLoaded.Broadcast(this);
    
    UE_LOG(LogTreeAutomata, Display, TEXT("%s: State loaded with %d variables, %d completed transitions"), 
        *InstanceName, LocalState.Num(), CompletedTransitions.Num());
}

// Helper method to build a map of node IDs to node pointers
void UTAInstance::BuildNodeMap(TSharedPtr<FTANode> Node, TMap<FGuid, TSharedPtr<FTANode>>& NodeMap)
{
    if (!Node.IsValid())
    {
        return;
    }
    
    // Add this node to the map
    NodeMap.Add(Node->NodeID, Node);
    
    // Process children recursively
    for (const TSharedPtr<FTANode>& Child : Node->Children)
    {
        if (Child.IsValid())
        {
            BuildNodeMap(Child, NodeMap);
        }
    }
}

// Helper method to resolve node references in the hierarchy
void UTAInstance::ResolveNodeReferences(TSharedPtr<FTANode> Node, TMap<FGuid, TSharedPtr<FTANode>>& NodeMap)
{
    if (!Node.IsValid())
    {
        return;
    }
    
    // Resolve transition targets
    for (FTATransition& Transition : Node->Transitions)
    {
        // Get stored target node ID
        const FVariant* TargetIDVar = Transition.StateData.Find(TEXT("TargetNodeID"));
        if (TargetIDVar && TargetIDVar->IsType<FString>())
        {
            FString TargetIDStr = TargetIDVar->AsString();
            FGuid TargetID;
            if (FGuid::Parse(TargetIDStr, TargetID))
            {
                // Look up the node in the map
                TSharedPtr<FTANode>* FoundNode = NodeMap.Find(TargetID);
                if (FoundNode && FoundNode->IsValid())
                {
                    Transition.TargetNode = *FoundNode;
                }
            }
        }
        
        // Remove temporary state data
        Transition.StateData.Remove(TEXT("TargetNodeID"));
    }
    
    // Recursively resolve references for child nodes
    for (TSharedPtr<FTANode>& Child : Node->Children)
    {
        if (Child.IsValid())
        {
            ResolveNodeReferences(Child, NodeMap);
        }
    }
}

// Helper method to create nodes from a template asset
void UTAInstance::CreateNodesFromTemplate(UTreeAutomatonAsset* TemplateAsset)
{
    if (!TemplateAsset)
    {
        UE_LOG(LogTreeAutomata, Warning, TEXT("%s: Invalid template asset"), *InstanceName);
        return;
    }
    
    // In a real implementation, this would load the nodes from the template asset
    // For this example, we'll create a simple hierarchy
    
    // Create root node
    RootNode = MakeShared<FTANode>();
    RootNode->NodeID = FGuid::NewGuid();
    RootNode->NodeName = TEXT("Root");
    RootNode->NodeType = TEXT("Root");
    
    // Create example child
    TSharedPtr<FTANode> ChildNode = MakeShared<FTANode>();
    ChildNode->NodeID = FGuid::NewGuid();
    ChildNode->NodeName = TEXT("Child");
    ChildNode->NodeType = TemplateAsset->DefaultNodeType;
    
    // Set up parent-child relationship
    RootNode->Children.Add(ChildNode);
    ChildNode->Parent = RootNode;
    
    // Create example transition
    FTATransition Transition;
    Transition.TransitionID = FGuid::NewGuid();
    Transition.TransitionName = TEXT("Default Transition");
    Transition.TargetNode = ChildNode;
    
    RootNode->Transitions.Add(Transition);
    
    UE_LOG(LogTreeAutomata, Display, TEXT("%s: Created nodes from template %s"), 
        *InstanceName, *TemplateAsset->GetName());
}

FGuid UTAInstance::GetCurrentNodeID() const
{
    return CurrentNode.IsValid() ? CurrentNode->NodeID : FGuid();
}

FString UTAInstance::GetCurrentNodeName() const
{
    return CurrentNode.IsValid() ? CurrentNode->NodeName : TEXT("");
}

void UTAInstance::SetRootNode(TSharedPtr<FTANode> InRootNode)
{
    RootNode = InRootNode;
    Reset();
}

TSharedPtr<FTANode> UTAInstance::GetRootNode() const
{
    return RootNode;
}

FVariant UTAInstance::GetLocalVariable(const FString& Name, const FVariant& DefaultValue) const
{
    const FVariant* Found = LocalState.Find(Name);
    return Found ? *Found : DefaultValue;
}

void UTAInstance::SetLocalVariable(const FString& Name, const FVariant& Value)
{
    LocalState.Add(Name, Value);
}

void UTAInstance::Serialize(FArchive& Ar)
{
    // Save basic info
    Ar << InstanceName;
    
    // Serialize local state
    int32 LocalStateCount = LocalState.Num();
    Ar << LocalStateCount;
    
    if (Ar.IsLoading())
    {
        LocalState.Empty(LocalStateCount);
        for (int32 i = 0; i < LocalStateCount; ++i)
        {
            FString Key;
            FVariant Value;
            Ar << Key;
            Ar << Value;
            LocalState.Add(Key, Value);
        }
    }
    else
    {
        for (auto& Pair : LocalState)
        {
            FString Key = Pair.Key;
            Ar << Key;
            Ar << Pair.Value;
        }
    }
    
    // Node hierarchy would be serialized here
    // For a full implementation, we need to handle full graph serialization
}

TSharedPtr<FTANode> UTAInstance::FindNodeByID(TSharedPtr<FTANode> StartNode, const FGuid& TargetID) const
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
        if (Transition.TargetNode.IsValid())
        {
            if (Transition.TargetNode->NodeID == TargetID)
            {
                return Transition.TargetNode;
            }
            
            TSharedPtr<FTANode> Found = FindNodeByID(Transition.TargetNode, TargetID);
            if (Found.IsValid())
            {
                return Found;
            }
        }
    }
    
    return nullptr;
}