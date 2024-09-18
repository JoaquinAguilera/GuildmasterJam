// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>

#include "AssetTypeActions_Base.h"
#include "ExpressiveTextEditorFunctions.h"

#include <EditorStyleSet.h>
#include <ExpressiveText/Public/ExpressiveTextProcessor.h>
#include <ExpressiveText/Public/Styles/ExpressiveTextStyle.h>
#include <ExpressiveText/Public/Widgets/ExpressiveTextWidget.h>
#include <Guganana/Editor.h>
#include <Misc/EngineVersionComparison.h>

#include "ExpressiveTextEditor.h"

class FExpressiveTextStyle_AssetTypeActions : public FAssetTypeActions_Base
{
public:

	virtual FText GetName() const override { return NSLOCTEXT("ExpressiveTextStyle", "FExpressiveTextStyle_AssetTypeActions", "Expressive Text Style"); }
	virtual FColor GetTypeColor() const override { return FColor(213, 41, 65); }
	virtual UClass* GetSupportedClass() const override{ return UExpressiveTextStyle::StaticClass(); }
	virtual uint32 GetCategories() override { return FExpressiveTextEditorModule::ExpressiveTextCategory; }

	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) override 
	{
        if (InObjects.Num() > 0)
        {
            if (auto* Asset = Cast<UExpressiveTextStyle>(InObjects[0]))
            {
                UExpressiveTextEditorFunctions::StartCustomEditor(*Asset, FSoftObjectPath(TEXT("/ExpressiveText/Core/Editor/CustomEditors/StyleEditor.StyleEditor") ) );
            }
        }
	}
        
    virtual TSharedPtr<SWidget> GetThumbnailOverlay(const FAssetData& AssetData) const override
    {
        UExpressiveTextStyleBase* AssetStyle = TSoftObjectPtr<UExpressiveTextStyle>(AssetData.ToSoftObjectPath()).LoadSynchronous();

		if (!AssetStyle)
		{
			return nullptr;
		}

        FExpressiveTextFields Fields;
        Fields.Text = FText::FromString(TEXT("[20pt](Aa)"));
        Fields.Alignment.VerticalAlignment = EExpressiveTextVerticalAlignment::Center;
        Fields.Alignment.HorizontalAlignment = EExpressiveTextHorizontalAlignment::Center;
		Fields.DefaultStyle = AssetStyle;

        FExpressiveText Text;
        Text.SetFields(Fields);
		Text.SetWorldContext(GWorld.GetReference());

        auto TextWidget = SNew(SExpressiveTextRendererWidget).UsedInEditor(true);
        TextWidget->SetExpressiveText(Text);

        auto Wrapper = SNew(SBorder)
            [
                SNew(SBorder)
                .BorderImage(Guganana::Editor::GetBrush("Menu.Background"))
                .BorderBackgroundColor(FLinearColor::Black)
                .Visibility(EVisibility::HitTestInvisible)
                .Clipping(EWidgetClipping::ClipToBounds)
                .HAlign(HAlign_Center)
                .VAlign(VAlign_Center)
                [
                    TextWidget
                ]
            ].Padding(FMargin(0.f, 0.f, 0.f, 5.f));        

        auto OnMouseButtonDown = [TextSharedRef = TSharedRef<SExpressiveTextRendererWidget>(TextWidget)](const FGeometry&, const FPointerEvent&)
        {
            TextSharedRef->Reset();
        };
        Wrapper->SetOnMouseEnter(FNoReplyPointerEventHandler::CreateLambda(OnMouseButtonDown));
        
        return Wrapper;
    }
};