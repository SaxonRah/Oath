// ResourceDataAsset.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ResourceData.h"
#include "ResourceDataAsset.generated.h"

UCLASS(BlueprintType)
class OATH_API UResourceDataAsset : public UObject
{
    GENERATED_BODY()
    
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FResourceData ResourceData;
};