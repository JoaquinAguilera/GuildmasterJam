// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>

#include <Materials/MaterialFunctionMaterialLayer.h>

class UMaterialInstanceConstant;
class UExpressiveTextMaterial;

class EXPRESSIVETEXT_API MaterialGeneration
{
public:
    static void ReconstructCombinedMaterial(UObject* Outer, UMaterialInstanceConstant*& ExistingMaterial, const TArray<UExpressiveTextMaterial*>& ExTextMats, bool MarkAsDirty = true);
    static FMaterialLayersFunctions GetLayers(const UMaterialInstance* Material, const FStaticParameterSet& StaticParameters);
    static void SetLayers(UMaterialInstance* Material, FStaticParameterSet& StaticParameters, const FMaterialLayersFunctions& Layers);
};