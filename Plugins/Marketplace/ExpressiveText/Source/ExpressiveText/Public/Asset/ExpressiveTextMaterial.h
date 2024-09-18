// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>

#include <Engine/DataAsset.h>
#include <Materials/MaterialInstanceConstant.h>
#include <Materials/MaterialFunctionMaterialLayer.h>
#include <Styling/SlateBrush.h>
#include <UObject/StrongObjectPtr.h>

#include "Materials/MaterialGeneration.h"

#include "ExpressiveTextMaterial.generated.h"


UENUM(meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EExTextDynamicMaterialParameters : int32
{
    PerInstanceRandom = 1,
    RevealAndClearInformation = 2,
    LineIndex = 4,
    BlockSizeAndTopLeftPosition = 8,
};

UCLASS()
class EXPRESSIVETEXT_API UExpressiveTextMaterial : public UDataAsset
{
    GENERATED_BODY()
public:

    FORCEINLINE bool RequiresDynamicParameter(EExTextDynamicMaterialParameters Parameters) const
    {
        return (DynamicParameters & (int32)Parameters) != 0;
    }
    
    UPROPERTY( EditAnywhere, Category = "Expressive Text Material" )
    UMaterialFunctionMaterialLayer* MaterialLayer;

    UPROPERTY( EditAnywhere, Category = "Expressive Text Material", Meta = (Bitmask, BitmaskEnum = "/Script/ExpressiveText.EExTextDynamicMaterialParameters") )
    int32 DynamicParameters;

#if WITH_EDITOR
    TSharedPtr<FSlateBrush> Preview;
    TStrongObjectPtr<UMaterialInstanceConstant> PreviewMaterial;

    void RefreshPreview()
    {
        if (!Preview.IsValid())
        {
            Preview = MakeShareable(new FSlateBrush);
        }

        if (MaterialLayer)
        {
            UMaterialInstanceConstant* MaterialPtr = nullptr;
            MaterialGeneration::ReconstructCombinedMaterial(nullptr, MaterialPtr, { this }, false);            
            Preview->SetResourceObject(MaterialPtr);
            PreviewMaterial = TStrongObjectPtr<UMaterialInstanceConstant>(MaterialPtr);
        }
        else
        {
            Preview->SetResourceObject(nullptr);
            PreviewMaterial = nullptr;
        }
    }

    virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override
    {
        Super::PostEditChangeProperty(PropertyChangedEvent);
        RefreshPreview();
    }
#endif

};