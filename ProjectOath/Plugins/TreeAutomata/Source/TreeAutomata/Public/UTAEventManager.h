// UTAEventManager.h
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
//#include "UObject/NoExportTypes.h"
#include "TATypes.h"
#include "UTAEventManager.generated.h"

//DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTAEvent, const FString&, EventType, const TMap<FString, FTAVariant>&, EventData);
// Use a simplified delegate that doesn't try to pass a TMap directly
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTAEvent, const FString&, EventType);

/**
 * Event manager to handle system-wide events
 */
UCLASS(BlueprintType)
//class TREEAUTOMATA_API UTAEventManager : public UObject
class TREEAUTOMATA_API UTAEventManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()
    
public:
    UTAEventManager();

    // Initialize the subsystem
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    // Deinitialize the subsystem
    virtual void Deinitialize() override;

    // Singleton instance
    UFUNCTION(BlueprintCallable, Category = "Tree Automata|Events")
    static UTAEventManager* Get(UWorld* World);
    
    // Register for event notifications
    UFUNCTION(BlueprintCallable, Category = "Tree Automata|Events")
    void RegisterEventListener(const FString& EventType, UObject* Listener, FName FunctionName);
    
    // Unregister from event notifications
    UFUNCTION(BlueprintCallable, Category = "Tree Automata|Events")
    void UnregisterEventListener(const FString& EventType, UObject* Listener);
    
    // Broadcast event to all listeners
    UFUNCTION(BlueprintCallable, Category = "Tree Automata|Events")
    void BroadcastEvent(const FString& EventType, const TMap<FString, FTAVariant>& EventData);
    
    // Event history for debugging
    UPROPERTY(BlueprintReadOnly, Category = "Tree Automata|Events")
    TArray<FTAEventRecord> EventHistory;
    
    // Max event history length
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tree Automata|Events")
    int32 MaxEventHistoryLength;
    
    // Get all events of a specific type
    UFUNCTION(BlueprintCallable, Category = "Tree Automata|Events")
    TArray<FTAEventRecord> GetEventsOfType(const FString& EventType) const;
    
    // Clear event history
    UFUNCTION(BlueprintCallable, Category = "Tree Automata|Events")
    void ClearEventHistory();
    
    // Dynamic delegate for Blueprint event handling
    UPROPERTY(BlueprintAssignable, Category = "Tree Automata|Events")
    FOnTAEvent OnTAEvent;

    // Get the last event data for a specific event type
    UFUNCTION(BlueprintCallable, Category = "Tree Automata|Events")
    TMap<FString, FTAVariant> GetLastEventData(const FString& EventType) const;
    
private:
    // Map of event types to listeners
    TMap<FString, TArray<FTAEventListener>> EventListeners;
    
    // Add event to history
    void AddEventToHistory(const FString& EventType, const TMap<FString, FTAVariant>& EventData);
};