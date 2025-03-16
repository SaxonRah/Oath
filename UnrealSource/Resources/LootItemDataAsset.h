// LootItemDataAsset.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ResourceData.h"
#include "LootItemDataAsset.generated.h"

UCLASS(BlueprintType)
class OATH_API ULootItemDataAsset : public UObject
{
    GENERATED_BODY()
    
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLootItem LootItem;
};