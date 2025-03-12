// UTANodeWrapper.cpp
#include "UTANodeWrapper.h"
#include "FQuestNode.h"
#include "FDialogueNode.h"
#include "TALogging.h"

UTANodeWrapper::UTANodeWrapper()
{
}

UTANodeWrapper* UTANodeWrapper::CreateNodeWrapper(UObject* Outer)
{
    if (!Outer)
    {
        UE_LOG(LogTreeAutomata, Warning, TEXT("CreateNodeWrapper: Invalid outer object"));
        return nullptr;
    }
    
    UTANodeWrapper* Wrapper = NewObject<UTANodeWrapper>(Outer);
    Wrapper->InternalNode = MakeShared<FTANode>();
    Wrapper->InternalNode->NodeID = FGuid::NewGuid();
    Wrapper->InternalNode->NodeName = TEXT("New Node");
    Wrapper->InternalNode->NodeType = TEXT("Default");
    
    return Wrapper;
}

UTANodeWrapper* UTANodeWrapper::CreateQuestNode(UObject* Outer, const FString& NodeName)
{
    if (!Outer)
    {
        UE_LOG(LogTreeAutomata, Warning, TEXT("CreateQuestNode: Invalid outer object"));
        return nullptr;
    }
    
    UTANodeWrapper* Wrapper = NewObject<UTANodeWrapper>(Outer);
    Wrapper->InternalNode = MakeShared<FQuestNode>();
    Wrapper->InternalNode->NodeID = FGuid::NewGuid();
    Wrapper->InternalNode->NodeName = NodeName.IsEmpty() ? TEXT("New Quest") : NodeName;
    Wrapper->InternalNode->NodeType = TEXT("Quest");
    
    return Wrapper;
}

UTANodeWrapper* UTANodeWrapper::CreateDialogueNode(UObject* Outer, const FString& NodeName)
{
    if (!Outer)
    {
        UE_LOG(LogTreeAutomata, Warning, TEXT("CreateDialogueNode: Invalid outer object"));
        return nullptr;
    }
    
    UTANodeWrapper* Wrapper = NewObject<UTANodeWrapper>(Outer);
    Wrapper->InternalNode = MakeShared<FDialogueNode>();
    Wrapper->InternalNode->NodeID = FGuid::NewGuid();
    Wrapper->InternalNode->NodeName = NodeName.IsEmpty() ? TEXT("New Dialogue") : NodeName;
    Wrapper->InternalNode->NodeType = TEXT("Dialogue");
    
    return Wrapper;
}

FGuid UTANodeWrapper::GetNodeID() const
{
    if (InternalNode.IsValid())
    {
        return InternalNode->NodeID;
    }
    
    return FGuid();
}

FString UTANodeWrapper::GetNodeName() const
{
    if (InternalNode.IsValid())
    {
        return InternalNode->NodeName;
    }
    
    return TEXT("");
}

void UTANodeWrapper::SetNodeName(const FString& NewName)
{
    if (InternalNode.IsValid())
    {
        InternalNode->NodeName = NewName;
    }
}

FString UTANodeWrapper::GetNodeType() const
{
    if (InternalNode.IsValid())
    {
        return InternalNode->NodeType;
    }
    
    return TEXT("");
}

void UTANodeWrapper::AddChild(UTANodeWrapper* ChildWrapper)
{
    if (!InternalNode.IsValid())
    {
        UE_LOG(LogTreeAutomata, Warning, TEXT("AddChild: Invalid internal node"));
        return;
    }
    
    if (!ChildWrapper || !ChildWrapper->InternalNode.IsValid())
    {
        UE_LOG(LogTreeAutomata, Warning, TEXT("AddChild: Invalid child node"));
        return;
    }
    
    InternalNode->AddChild(ChildWrapper->InternalNode);
}

void UTANodeWrapper::AddTransition(UTANodeWrapper* TargetNode, const FString& TransitionName, int32 Priority)
{
    if (!InternalNode.IsValid())
    {
        UE_LOG(LogTreeAutomata, Warning, TEXT("AddTransition: Invalid internal node"));
        return;
    }
    
    if (!TargetNode || !TargetNode->InternalNode.IsValid())
    {
        UE_LOG(LogTreeAutomata, Warning, TEXT("AddTransition: Invalid target node"));
        return;
    }
    
    FTATransition Transition;
    Transition.TransitionID = FGuid::NewGuid();
    Transition.TransitionName = TransitionName.IsEmpty() ? TEXT("New Transition") : TransitionName;
    Transition.TargetNode = TargetNode->InternalNode;
    Transition.Priority = Priority;
    
    InternalNode->AddTransition(Transition);
}

bool UTANodeWrapper::IsQuestNode() const
{
    if (!InternalNode.IsValid())
    {
        return false;
    }
    
    // Try to cast to quest node
    return InternalNode->NodeType == TEXT("Quest");
}

bool UTANodeWrapper::IsDialogueNode() const
{
    if (!InternalNode.IsValid())
    {
        return false;
    }
    
    // Try to cast to dialogue node
    return InternalNode->NodeType == TEXT("Dialogue");
}

void UTANodeWrapper::SetStateVariable(const FString& Name, const FTAVariant& Value)
{
    if (InternalNode.IsValid())
    {
        InternalNode->StateData.Add(Name, Value);
    }
}

FTAVariant UTANodeWrapper::GetStateVariable(const FString& Name, const FTAVariant& DefaultValue) const
{
    if (InternalNode.IsValid())
    {
        const FTAVariant* Found = InternalNode->StateData.Find(Name);
        if (Found)
        {
            return *Found;
        }
    }
    
    return DefaultValue;
}

void UTANodeWrapper::SetAcceptingState(bool bIsAccepting)
{
    if (InternalNode.IsValid())
    {
        InternalNode->bIsAcceptingState = bIsAccepting;
    }
}

bool UTANodeWrapper::IsAcceptingState() const
{
    if (InternalNode.IsValid())
    {
        return InternalNode->bIsAcceptingState;
    }
    
    return false;
}

UTANodeWrapper* UTANodeWrapper::GetParentNode() const
{
    if (!InternalNode.IsValid())
    {
        return nullptr;
    }
    
    // Check if parent is valid
    TSharedPtr<FTANode> ParentNode = InternalNode->Parent.Pin();
    if (!ParentNode.IsValid())
    {
        return nullptr;
    }
    
    // Create a wrapper for the parent
    return CreateFromInternalNode(GetOuter(), ParentNode);
}

UTANodeWrapper* UTANodeWrapper::CreateFromInternalNode(UObject* Outer, TSharedPtr<FTANode> Node)
{
    if (!Outer || !Node.IsValid())
    {
        return nullptr;
    }
    
    UTANodeWrapper* Wrapper = NewObject<UTANodeWrapper>(Outer);
    Wrapper->InternalNode = Node;
    
    return Wrapper;
}