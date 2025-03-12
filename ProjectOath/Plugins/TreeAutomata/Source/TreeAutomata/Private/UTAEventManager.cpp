// UTAEventManager.cpp
#include "UTAEventManager.h"
#include "TALogging.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UTAEventManager::UTAEventManager()
    : MaxEventHistoryLength(100)
{
}

UTAEventManager* UTAEventManager::Get(UWorld* World)
{
    if (!World)
    {
        return nullptr;
    }
    
    UTAEventManager* EventManager = World->GetGameInstance()->GetSubsystem<UTAEventManager>();
    if (!EventManager)
    {
        // In a real implementation, we would register this as a game instance subsystem
        // For this example, create a temporary instance
        EventManager = NewObject<UTAEventManager>(World->GetGameInstance());
    }
    
    return EventManager;
}

void UTAEventManager::RegisterEventListener(const FString& EventType, UObject* Listener, FName FunctionName)
{
    if (!Listener)
    {
        UE_LOG(LogTreeAutomata, Warning, TEXT("RegisterEventListener: Invalid listener"));
        return;
    }
    
    // Check if function exists
    UFunction* Function = Listener->FindFunction(FunctionName);
    if (!Function)
    {
        UE_LOG(LogTreeAutomata, Warning, TEXT("RegisterEventListener: Function %s not found on %s"), 
            *FunctionName.ToString(), *Listener->GetName());
        return;
    }
    
    // Create listener entry
    FTAEventListener EventListener;
    EventListener.Listener = Listener;
    EventListener.FunctionName = FunctionName;
    
    // Add to listeners map
    TArray<FTAEventListener>& Listeners = EventListeners.FindOrAdd(EventType);
    Listeners.Add(EventListener);
    
    UE_LOG(LogTreeAutomata, Display, TEXT("Registered listener %s for event type '%s'"), 
        *Listener->GetName(), *EventType);
}

void UTAEventManager::UnregisterEventListener(const FString& EventType, UObject* Listener)
{
    if (!Listener)
    {
        return;
    }
    
    // Find listeners for this event type
    TArray<FTAEventListener>* Listeners = EventListeners.Find(EventType);
    if (!Listeners)
    {
        return;
    }
    
    // Remove any listeners with matching object
    for (int32 i = Listeners->Num() - 1; i >= 0; --i)
    {
        if ((*Listeners)[i].Listener == Listener)
        {
            Listeners->RemoveAt(i);
        }
    }
    
    // If no more listeners, remove the event type
    if (Listeners->Num() == 0)
    {
        EventListeners.Remove(EventType);
    }
    
    UE_LOG(LogTreeAutomata, Display, TEXT("Unregistered listener %s from event type '%s'"), 
        *Listener->GetName(), *EventType);
}

void UTAEventManager::BroadcastEvent(const FString& EventType, const TMap<FString, FVariant>& EventData)
{
    // Add to history
    AddEventToHistory(EventType, EventData);
    
    // Broadcast to dynamic delegate
    OnTAEvent.Broadcast(EventType, EventData);
    
    // Find listeners for this event type
    TArray<FTAEventListener>* Listeners = EventListeners.Find(EventType);
    if (!Listeners)
    {
        return;
    }
    
    // Make a copy to handle listeners that might unregister during broadcast
    TArray<FTAEventListener> ListenersCopy = *Listeners;
    
    // Notify each listener
    for (const FTAEventListener& Listener : ListenersCopy)
    {
        if (Listener.Listener.IsValid())
        {
            // Create parameters for function call
            FTAEventParameters Params;
            Params.EventType = EventType;
            Params.EventData = EventData;
            
            // Call function
            Listener.Listener->ProcessEvent(
                Listener.Listener->FindFunction(Listener.FunctionName),
                &Params);
        }
    }
    
    UE_LOG(LogTreeAutomata, Verbose, TEXT("Broadcast event of type '%s' to %d listeners"), 
        *EventType, ListenersCopy.Num());
}

TArray<FTAEventRecord> UTAEventManager::GetEventsOfType(const FString& EventType) const
{
    TArray<FTAEventRecord> FilteredEvents;
    
    for (const FTAEventRecord& Event : EventHistory)
    {
        if (Event.EventType == EventType)
        {
            FilteredEvents.Add(Event);
        }
    }
    
    return FilteredEvents;
}

void UTAEventManager::ClearEventHistory()
{
    EventHistory.Empty();
}

void UTAEventManager::AddEventToHistory(const FString& EventType, const TMap<FString, FVariant>& EventData)
{
    // Create event record
    FTAEventRecord EventRecord;
    EventRecord.EventType = EventType;
    EventRecord.EventData = EventData;
    EventRecord.Timestamp = FDateTime::Now();
    
    // Add to history
    EventHistory.Add(EventRecord);
    
    // Trim history if needed
    if (EventHistory.Num() > MaxEventHistoryLength)
    {
        EventHistory.RemoveAt(0, EventHistory.Num() - MaxEventHistoryLength);
    }
}