// Copyright 2022 Guganana. All Rights Reserved.

#include "ExpressiveText/Public/Materials/MaterialGeneration.h"

#include "ExpressiveText/Public/ExpressiveTextSettings.h"

#include <Guganana/Engine.h>
#include <Materials/MaterialInstanceConstant.h>
#include <UObject/Package.h>

#if WITH_EDITOR
#include <MaterialUtilities.h>
#endif

FMaterialLayersFunctions MaterialGeneration::GetLayers(const UMaterialInstance* Material, const FStaticParameterSet& StaticParameters)
{
    FMaterialLayersFunctions Result;

#if UE_VERSION_OLDER_THAN( 5, 0, 0 )
    check(StaticParameters.MaterialLayersParameters.Num() > 0);
    if (StaticParameters.MaterialLayersParameters.Num() > 0)
    {
        auto& MaterialLayerWrapper = StaticParameters.MaterialLayersParameters[0];
        Result = MaterialLayerWrapper.Value;
    }
#else
    Material->GetMaterialLayers(Result);
#endif

    return Result;
}

void MaterialGeneration::SetLayers(UMaterialInstance* Material, FStaticParameterSet& StaticParameters, const FMaterialLayersFunctions& Layers)
{
#if WITH_EDITOR
#if UE_VERSION_OLDER_THAN( 5, 0, 0 )
    check(StaticParameters.MaterialLayersParameters.Num() > 0);
    StaticParameters.MaterialLayersParameters[0].bOverride = true;
    StaticParameters.MaterialLayersParameters[0].Value = Layers;
#else
    StaticParameters.bHasMaterialLayers = true;
    StaticParameters.MaterialLayers = Guganana::Engine::GetRuntime(Layers);
#if UE_VERSION_NEWER_THAN( 5, 0, 3 )
    StaticParameters.EditorOnly.MaterialLayers = Guganana::Engine::GetEditorOnlyData(Layers);
#endif
#endif
#endif
}

void MaterialGeneration::ReconstructCombinedMaterial(UObject* Outer, UMaterialInstanceConstant*& ExistingMaterial, const TArray<UExpressiveTextMaterial*>& ExTextMats, bool MarkAsDirty)
{
#if WITH_EDITOR
    auto* Settings = GetDefault<UExpressiveTextSettings>();
    if (!Settings)
    {
        return;
    }

    if (Outer == nullptr)
    {
        Outer = GetTransientPackage();
        check(Outer);
    }

    if (ExistingMaterial == nullptr)
    {
        auto* BaseMaterial = Settings->BaseTextMaterial.LoadSynchronous();
        FGuid RandomGuid = FGuid::NewGuid();
        ExistingMaterial = NewObject<UMaterialInstanceConstant>(Outer, FName(*RandomGuid.ToString()), RF_Public);
        check(ExistingMaterial);
        ExistingMaterial->Parent = BaseMaterial;
    }

    auto* BaseBlend = Settings->BaseTextLayerBlend.LoadSynchronous();
    auto* BaseLayer = Settings->BaseTextLayer.LoadSynchronous();

    FStaticParameterSet SourceStaticParameters;
    ExistingMaterial->GetStaticParameterValues(SourceStaticParameters);
    
    FMaterialLayersFunctions LayerFunctions = GetLayers(ExistingMaterial, SourceStaticParameters);    

    // Update base layer
    if (LayerFunctions.Layers.Num() > 0)
    {
        if (LayerFunctions.Layers[0] != BaseLayer)
        {
            LayerFunctions.Layers[0] = BaseLayer;
        }
    }

    // Build MaterialsToLayer to help with some operations below
    TArray< UMaterialFunctionInterface* > MaterialsToLayer;
    for (const auto* ExTextMat : ExTextMats)
    {
        if (ExTextMat && ExTextMat->MaterialLayer )
        {
            MaterialsToLayer.Add(ExTextMat->MaterialLayer);
        }
    }

    // Remove old layers
    for (int32 i = LayerFunctions.Layers.Num() - 1; i > 0; i--)
    {
        const auto Layer = LayerFunctions.Layers[i];

        const bool NewSpecContainsMat = MaterialsToLayer.Contains(Layer);

        if (!NewSpecContainsMat)
        {
            LayerFunctions.RemoveBlendedLayerAt(i);
        }
    }

    // Add new layers
    for (const auto* ExTextMat : ExTextMats)
    {
        if (ExTextMat && ExTextMat->MaterialLayer)
        {
            if (!LayerFunctions.Layers.Contains(ExTextMat->MaterialLayer))
            {
                int32 Index = LayerFunctions.AppendBlendedLayer();
                LayerFunctions.Layers[Index] = ExTextMat->MaterialLayer;
                LayerFunctions.Blends[Index - 1] = BaseBlend;
                auto& EditorOnly = Guganana::Engine::GetEditorOnlyData(LayerFunctions);
                EditorOnly.LayerNames[Index] = FText::FromString(GetNameSafe(ExTextMat));
            }
        }
    }

    // Sort all layers to match the spec (inefficiently but it's not runtime code)
    for (int i = 1; i < LayerFunctions.Layers.Num(); i++)
    {
        for (int j = 0; j < LayerFunctions.Layers.Num() - i; j++)
        {
            const auto A = LayerFunctions.Layers[j];
            const auto B = LayerFunctions.Layers[j + 1];
            const int32 IndexA = MaterialsToLayer.IndexOfByKey(A);
            const int32 IndexB = MaterialsToLayer.IndexOfByKey(B);

            if (IndexA > IndexB)
            {
                LayerFunctions.MoveBlendedLayer(j, j + 1);
            }
        }
    }

    if (MarkAsDirty)
    {
        ExistingMaterial->MarkPackageDirty();
    }

    ExistingMaterial->PreEditChange(nullptr);
    SetLayers(ExistingMaterial, SourceStaticParameters, LayerFunctions);
    ExistingMaterial->UpdateStaticPermutation(SourceStaticParameters, ExistingMaterial->BasePropertyOverrides, true);
    ExistingMaterial->InitStaticPermutation();
    ExistingMaterial->PostEditChange();
#endif
}
