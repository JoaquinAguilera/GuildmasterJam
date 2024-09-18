// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>

#include <Engine/DataAsset.h>

#include "ExpressiveTextLightMode.generated.h"

class UMaterialInterface;

UENUM(BlueprintType)
enum class EExpressiveTextLightMode : uint8
{
    Lit,
    Emissive,
    Additive UMETA( Tooltip="Use this if your text is showing a black bounding box due to bluring" )
};

UCLASS(BlueprintType)
class UExpressiveTextLightModeMaterialsAsset : public UDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ExpressiveText)
    TMap<EExpressiveTextLightMode, UMaterialInterface*> LightModeMaterials;
};
