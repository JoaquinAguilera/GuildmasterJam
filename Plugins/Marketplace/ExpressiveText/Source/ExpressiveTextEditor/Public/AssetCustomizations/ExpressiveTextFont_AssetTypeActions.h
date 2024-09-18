// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>

#include "AssetTypeActions_Base.h"
#include "ExpressiveTextEditorFunctions.h"

#include <EditorStyleSet.h>
#include <ExpressiveText/Public/Asset/ExpressiveTextFont.h>
#include <ExpressiveText/Public/Widgets/ExpressiveTextWidget.h>
#include <Guganana/Editor.h>

#include "ExpressiveTextEditor.h"

class FExpressiveTextFont_AssetTypeActions : public FAssetTypeActions_Base
{
public:

	virtual FText GetName() const override { return NSLOCTEXT("ExpressiveTextFont", "FExpressiveTextFont_AssetTypeActions", "Expressive Text Font"); }
	virtual FColor GetTypeColor() const override { return FColor(47, 72, 88); }
	virtual UClass* GetSupportedClass() const override{ return UExpressiveTextFont::StaticClass(); }
	virtual uint32 GetCategories() override { return FExpressiveTextEditorModule::ExpressiveTextCategory; }

    virtual TSharedPtr<SWidget> GetThumbnailOverlay(const FAssetData& AssetData) const override
    {
        UExpressiveTextFont* FontAsset = TSoftObjectPtr<UExpressiveTextFont>(AssetData.ToSoftObjectPath()).LoadSynchronous();

        if (!FontAsset)
        {
            return nullptr;
        }

        FExpressiveTextFields Fields;
        Fields.Text = FText::FromString(FString::Printf(TEXT("[20pt,&%s](Tt)"), *GetNameSafe(FontAsset)));
        Fields.Alignment.VerticalAlignment = EExpressiveTextVerticalAlignment::Center;
        Fields.Alignment.HorizontalAlignment = EExpressiveTextHorizontalAlignment::Center;

        FExpressiveText Text;
        Text.SetFields(Fields);
        Text.SetWorldContext(GWorld.GetReference());

        auto TextWidget = SNew(SExpressiveTextRendererWidget).UsedInEditor(true);
        TextWidget->SetExpressiveText(Text);

        auto Wrapper = SNew(SBorder)
            .BorderImage(Guganana::Editor::GetBrush("Menu.Background"))
            .BorderBackgroundColor(FLinearColor::Transparent)
            .Visibility(EVisibility::HitTestInvisible)
            .Clipping(EWidgetClipping::ClipToBounds)
            .HAlign(HAlign_Center)
            .VAlign(VAlign_Center)
            [TextWidget]
            .Padding(FMargin(0.f));

        auto OnMouseButtonDown = [TextSharedRef = TSharedRef<SExpressiveTextRendererWidget>(TextWidget)](const FGeometry&, const FPointerEvent&)
        {
            TextSharedRef->Reset();
        };
        Wrapper->SetOnMouseEnter(FNoReplyPointerEventHandler::CreateLambda(OnMouseButtonDown));

        return Wrapper;
    }
};