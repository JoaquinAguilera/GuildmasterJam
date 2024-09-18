// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>

#include "AssetTypeActions_Base.h"

#include <ExpressiveText/Public/Asset/ExpressiveTextMaterial.h>
#include <Materials/MaterialFunctionMaterialLayer.h>
#include "ExpressiveTextEditorFunctions.h"

#include "ExpressiveTextEditor.h"

class FExpressiveTextMaterial_AssetTypeActions : public FAssetTypeActions_Base
{
public:

	virtual FText GetName() const override { return NSLOCTEXT("ExpressiveTextMaterial", "FExpressiveTextMaterial_AssetTypeActions", "Expressive Text Material"); }
	virtual FColor GetTypeColor() const override { return FColor(157, 255, 249); }
	virtual UClass* GetSupportedClass() const override{ return UExpressiveTextMaterial::StaticClass(); }
	virtual uint32 GetCategories() override { return FExpressiveTextEditorModule::ExpressiveTextCategory; }

    virtual TSharedPtr<SWidget> GetThumbnailOverlay(const FAssetData& AssetData) const override
    {
        UExpressiveTextMaterial* Material = TSoftObjectPtr<UExpressiveTextMaterial>(AssetData.ToSoftObjectPath()).LoadSynchronous();
        if (!Material)
        {
            return nullptr;
        }

        Material->RefreshPreview();
        auto* Brush = Material->Preview.Get();
        if (!Brush)
        {
            return nullptr;
        }

        auto Wrapper =
            SNew(SBorder)
            [
                SNew(SBorder)
                .BorderImage(Guganana::Editor::GetBrush("Menu.Background"))
                .BorderBackgroundColor(FLinearColor::Black)
                [
                    SNew(SBorder)
                    .BorderImage(Brush)
                    .BorderBackgroundColor(FLinearColor::White)
                    .Visibility(EVisibility::HitTestInvisible)
                    .Clipping(EWidgetClipping::ClipToBounds)
                    .HAlign(HAlign_Center)
                    .VAlign(VAlign_Center)
                ].Padding(FMargin(0.f))
            ]
            .Padding(FMargin(0.f, 0.f, 0.f, 5.f));

        return Wrapper;
    }
};