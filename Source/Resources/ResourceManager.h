// ResourceManager.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ResourceData.h"
#include "ResourceManager.generated.h"

UCLASS(BlueprintType, Blueprintable)
class OATH_API UResourceManager : public UObject
{
    GENERATED_BODY()

public:
    UResourceManager();
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
    TArray<FResourceData> AvailableResources;
    
    UFUNCTION(BlueprintCallable, Category = "Resources")
    FResourceData GetResourceByName(const FString& ResourceName, bool& bFound) const;
    
    UFUNCTION(BlueprintCallable, Category = "Resources")
    TArray<FResourceData> GetResourcesByRarity(EResourceRarity Rarity) const;
    
    UFUNCTION(BlueprintCallable, Category = "Resources")
    int32 GetResourceBaseValue(const FString& ResourceName) const;
    
    UFUNCTION(BlueprintCallable, Category = "Resources")
    void AddResource(const FResourceData& NewResource);
    
    UFUNCTION(BlueprintCallable, Category = "Resources")
    void LoadResourcesFromDataTable(UDataTable* ResourceTable);
    
private:
    // Helper lookup map
    TMap<FString, int32> ResourceNameToIndex;
    
    void InitializeDefaultResources();
};