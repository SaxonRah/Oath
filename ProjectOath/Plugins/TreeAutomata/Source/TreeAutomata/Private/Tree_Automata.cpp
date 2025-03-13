// Tree_Automata.cpp
#include "Tree_Automata.h"
#include "JsonObjectConverter.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"

bool UConditionEvaluator::EvaluateCondition_Implementation(const FString& Condition, UObject* Context)
{
    // Base implementation always returns true
    // Derived classes should override this with actual condition logic
    return true;
}

void UActionPerformer::PerformAction_Implementation(const FString& Action, UObject* Context)
{
    // Base implementation does nothing
    // Derived classes should override this with actual action logic
}

UTreeAutomata::UTreeAutomata()
{
    PrimaryComponentTick.bCanEverTick = false;
    ConditionEvaluator = nullptr;
    ActionPerformer = nullptr;
}

FGuid UTreeAutomata::InitializeRoot(const FString& RootName)
{
    FAutomatonNode RootNode(RootName);
    RootNode.State = "Active"; // Root starts active by default
    
    RootNodeId = RootNode.NodeId;
    Nodes.Add(RootNodeId, RootNode);
    
    return RootNodeId;
}

FGuid UTreeAutomata::AddNode(const FString& NodeName, const FGuid& ParentId)
{
    // Validate parent exists
    if (!ParentId.IsValid() || !Nodes.Contains(ParentId))
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot add node: Invalid parent ID"));
        return FGuid();
    }
    
    // Create the new node
    FAutomatonNode NewNode(NodeName, ParentId);
    
    // Update parent's children list
    FAutomatonNode& ParentNode = Nodes[ParentId];
    ParentNode.ChildrenIds.Add(NewNode.NodeId);
    
    // Store the node
    Nodes.Add(NewNode.NodeId, NewNode);
    
    return NewNode.NodeId;
}

FAutomatonNode UTreeAutomata::GetNode(const FGuid& NodeId) const
{
    if (Nodes.Contains(NodeId))
    {
        return Nodes[NodeId];
    }
    
    return FAutomatonNode();
}

void UTreeAutomata::AddTransition(const FStateTransition& Transition)
{
    TransitionRules.Add(Transition);
}

bool UTreeAutomata::TriggerEvent(const FGuid& NodeId, const FString& EventName, UObject* Context)
{
    if (!Nodes.Contains(NodeId))
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot trigger event: Node does not exist"));
        return false;
    }
    
    FAutomatonNode& Node = Nodes[NodeId];
    TArray<FStateTransition> ApplicableTransitions = FindApplicableTransitions(Node.State, EventName);
    
    for (const FStateTransition& Transition : ApplicableTransitions)
    {
        // Check all conditions are met
        bool AllConditionsMet = true;
        
        for (const FString& Condition : Transition.Conditions)
        {
            if (ConditionEvaluator && !ConditionEvaluator->EvaluateCondition(Condition, Context))
            {
                AllConditionsMet = false;
                break;
            }
        }
        
        if (AllConditionsMet)
        {
            // Perform transition actions
            for (const FString& Action : Transition.Actions)
            {
                if (ActionPerformer)
                {
                    ActionPerformer->PerformAction(Action, Context);
                }
            }
            
            // Update node state
            Node.State = Transition.ToState;
            return true;
        }
    }
    
    return false;
}

void UTreeAutomata::SetConditionEvaluator(UConditionEvaluator* Evaluator)
{
    ConditionEvaluator = Evaluator;
}

void UTreeAutomata::SetActionPerformer(UActionPerformer* Performer)
{
    ActionPerformer = Performer;
}

TArray<FAutomatonNode> UTreeAutomata::GetChildren(const FGuid& NodeId) const
{
    TArray<FAutomatonNode> Children;
    
    if (Nodes.Contains(NodeId))
    {
        const FAutomatonNode& Node = Nodes[NodeId];
        
        for (const FGuid& ChildId : Node.ChildrenIds)
        {
            if (Nodes.Contains(ChildId))
            {
                Children.Add(Nodes[ChildId]);
            }
        }
    }
    
    return Children;
}

FAutomatonNode UTreeAutomata::GetParent(const FGuid& NodeId) const
{
    if (Nodes.Contains(NodeId))
    {
        const FAutomatonNode& Node = Nodes[NodeId];
        
        if (Node.ParentId.IsValid() && Nodes.Contains(Node.ParentId))
        {
            return Nodes[Node.ParentId];
        }
    }
    
    return FAutomatonNode();
}

TArray<FAutomatonNode> UTreeAutomata::FindNodesByState(const FString& State) const
{
    TArray<FAutomatonNode> MatchingNodes;
    
    for (const auto& NodePair : Nodes)
    {
        if (NodePair.Value.State == State)
        {
            MatchingNodes.Add(NodePair.Value);
        }
    }
    
    return MatchingNodes;
}

bool UTreeAutomata::PathExists(const FGuid& FromNodeId, const FGuid& ToNodeId) const
{
    if (!Nodes.Contains(FromNodeId) || !Nodes.Contains(ToNodeId))
    {
        return false;
    }
    
    // Simple BFS implementation to find path
    TArray<FGuid> Queue;
    TSet<FGuid> Visited;
    
    Queue.Add(FromNodeId);
    Visited.Add(FromNodeId);
    
    while (Queue.Num() > 0)
    {
        FGuid CurrentId = Queue[0];
        Queue.RemoveAt(0);
        
        if (CurrentId == ToNodeId)
        {
            return true;
        }
        
        const FAutomatonNode& CurrentNode = Nodes[CurrentId];
        
        // Check children
        for (const FGuid& ChildId : CurrentNode.ChildrenIds)
        {
            if (!Visited.Contains(ChildId))
            {
                Queue.Add(ChildId);
                Visited.Add(ChildId);
            }
        }
        
        // Also check parent (for bidirectional paths)
        if (CurrentNode.ParentId.IsValid() && !Visited.Contains(CurrentNode.ParentId))
        {
            Queue.Add(CurrentNode.ParentId);
            Visited.Add(CurrentNode.ParentId);
        }
    }
    
    return false;
}

FString UTreeAutomata::SerializeToJson() const
{
    TSharedPtr<FJsonObject> RootObject = MakeShareable(new FJsonObject);
    
    // Serialize nodes
    TArray<TSharedPtr<FJsonValue>> NodeArray;
    for (const auto& NodePair : Nodes)
    {
        TSharedPtr<FJsonObject> NodeObject = MakeShareable(new FJsonObject);
        
        NodeObject->SetStringField(TEXT("NodeId"), NodePair.Value.NodeId.ToString());
        NodeObject->SetStringField(TEXT("Name"), NodePair.Value.Name);
        NodeObject->SetStringField(TEXT("ParentId"), NodePair.Value.ParentId.ToString());
        NodeObject->SetStringField(TEXT("State"), NodePair.Value.State);
        
        // Serialize children
        TArray<TSharedPtr<FJsonValue>> ChildrenArray;
        for (const FGuid& ChildId : NodePair.Value.ChildrenIds)
        {
            ChildrenArray.Add(MakeShareable(new FJsonValueString(ChildId.ToString())));
        }
        NodeObject->SetArrayField(TEXT("ChildrenIds"), ChildrenArray);
        
        // Serialize metadata
        TSharedPtr<FJsonObject> MetadataObject = MakeShareable(new FJsonObject);
        for (const auto& MetaPair : NodePair.Value.Metadata)
        {
            MetadataObject->SetStringField(MetaPair.Key, MetaPair.Value);
        }
        NodeObject->SetObjectField(TEXT("Metadata"), MetadataObject);
        
        NodeArray.Add(MakeShareable(new FJsonValueObject(NodeObject)));
    }
    RootObject->SetArrayField(TEXT("Nodes"), NodeArray);
    
    // Serialize transitions
    TArray<TSharedPtr<FJsonValue>> TransitionArray;
    for (const FStateTransition& Transition : TransitionRules)
    {
        TSharedPtr<FJsonObject> TransitionObject = MakeShareable(new FJsonObject);
        
        TransitionObject->SetStringField(TEXT("FromState"), Transition.FromState);
        TransitionObject->SetStringField(TEXT("ToState"), Transition.ToState);
        TransitionObject->SetStringField(TEXT("TriggerEvent"), Transition.TriggerEvent);
        
        // Serialize conditions
        TArray<TSharedPtr<FJsonValue>> ConditionsArray;
        for (const FString& Condition : Transition.Conditions)
        {
            ConditionsArray.Add(MakeShareable(new FJsonValueString(Condition)));
        }
        TransitionObject->SetArrayField(TEXT("Conditions"), ConditionsArray);
        
        // Serialize actions
        TArray<TSharedPtr<FJsonValue>> ActionsArray;
        for (const FString& Action : Transition.Actions)
        {
            ActionsArray.Add(MakeShareable(new FJsonValueString(Action)));
        }
        TransitionObject->SetArrayField(TEXT("Actions"), ActionsArray);
        
        TransitionArray.Add(MakeShareable(new FJsonValueObject(TransitionObject)));
    }
    RootObject->SetArrayField(TEXT("Transitions"), TransitionArray);
    
    // Set root node ID
    RootObject->SetStringField(TEXT("RootNodeId"), RootNodeId.ToString());
    
    // Serialize to string
    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);
    
    return OutputString;
}

bool UTreeAutomata::DeserializeFromJson(const FString& JsonString)
{
    TSharedPtr<FJsonObject> RootObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);
    
    if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to deserialize Tree Automata from JSON"));
        return false;
    }
    
    // Clear existing data
    Nodes.Empty();
    TransitionRules.Empty();
    
    // Deserialize root node ID
    // FString RootIdStr = RootObject->GetStringField("RootNodeId");
	FString RootIdStr = RootObject->GetStringField(TEXT("RootNodeId"));
    
	RootNodeId = FGuid(RootIdStr);
    
    // Deserialize nodes
    // TArray<TSharedPtr<FJsonValue>> NodeArray = RootObject->GetArrayField("Nodes");
	TArray<TSharedPtr<FJsonValue>> NodeArray = RootObject->GetArrayField(TEXT("Nodes"));

    for (const TSharedPtr<FJsonValue>& NodeValue : NodeArray)
    {
        TSharedPtr<FJsonObject> NodeObject = NodeValue->AsObject();
        
        FAutomatonNode Node;
        // Node.NodeId = FGuid(NodeObject->GetStringField("NodeId"));
		Node.NodeId = FGuid(NodeObject->GetStringField(TEXT("NodeId")));
        
		// Node.Name = NodeObject->GetStringField("Name");
		Node.Name = NodeObject->GetStringField(TEXT("Name"));
        
		// Node.ParentId = FGuid(NodeObject->GetStringField("ParentId"));
		Node.ParentId = FGuid(NodeObject->GetStringField(TEXT("ParentId")));
        
		// Node.State = NodeObject->GetStringField("State");
		Node.State = NodeObject->GetStringField(TEXT("State"));
        
        // Deserialize children
        // TArray<TSharedPtr<FJsonValue>> ChildrenArray = NodeObject->GetArrayField("ChildrenIds");
		TArray<TSharedPtr<FJsonValue>> ChildrenArray = NodeObject->GetArrayField(TEXT("ChildrenIds"));

        for (const TSharedPtr<FJsonValue>& ChildValue : ChildrenArray)
        {
            Node.ChildrenIds.Add(FGuid(ChildValue->AsString()));
        }
        
        // Deserialize metadata
        // TSharedPtr<FJsonObject> MetadataObject = NodeObject->GetObjectField("Metadata");
		TSharedPtr<FJsonObject> MetadataObject = NodeObject->GetObjectField(TEXT("Metadata"));

        for (const auto& MetaPair : MetadataObject->Values)
        {
            Node.Metadata.Add(MetaPair.Key, MetaPair.Value->AsString());
        }
        
        Nodes.Add(Node.NodeId, Node);
    }
    
    // Deserialize transitions
    // TArray<TSharedPtr<FJsonValue>> TransitionArray = RootObject->GetArrayField("Transitions");
	TArray<TSharedPtr<FJsonValue>> TransitionArray = RootObject->GetArrayField(TEXT("Transitions"));

    for (const TSharedPtr<FJsonValue>& TransitionValue : TransitionArray)
    {
        TSharedPtr<FJsonObject> TransitionObject = TransitionValue->AsObject();
        
        FStateTransition Transition;
        // Transition.FromState = TransitionObject->GetStringField("FromState");
		Transition.FromState = TransitionObject->GetStringField(TEXT("FromState"));

        // Transition.ToState = TransitionObject->GetStringField("ToState");
		Transition.ToState = TransitionObject->GetStringField(TEXT("ToState"));

        // Transition.TriggerEvent = TransitionObject->GetStringField("TriggerEvent");
		Transition.TriggerEvent = TransitionObject->GetStringField(TEXT("TriggerEvent"));

        
        // Deserialize conditions
        // TArray<TSharedPtr<FJsonValue>> ConditionsArray = TransitionObject->GetArrayField("Conditions");
		TArray<TSharedPtr<FJsonValue>> ConditionsArray = TransitionObject->GetArrayField(TEXT("Conditions"));

        for (const TSharedPtr<FJsonValue>& ConditionValue : ConditionsArray)
        {
            Transition.Conditions.Add(ConditionValue->AsString());
        }
        
        // Deserialize actions
        // TArray<TSharedPtr<FJsonValue>> ActionsArray = TransitionObject->GetArrayField("Actions");
		TArray<TSharedPtr<FJsonValue>> ActionsArray = TransitionObject->GetArrayField(TEXT("Actions"));

        for (const TSharedPtr<FJsonValue>& ActionValue : ActionsArray)
        {
            Transition.Actions.Add(ActionValue->AsString());
        }
        
        TransitionRules.Add(Transition);
    }
    
    return true;
}

TArray<FStateTransition> UTreeAutomata::FindApplicableTransitions(const FString& CurrentState, const FString& EventName) const
{
    TArray<FStateTransition> ApplicableTransitions;
    
    for (const FStateTransition& Transition : TransitionRules)
    {
        if (Transition.FromState == CurrentState && Transition.TriggerEvent == EventName)
        {
            ApplicableTransitions.Add(Transition);
        }
    }
    
    return ApplicableTransitions;
}