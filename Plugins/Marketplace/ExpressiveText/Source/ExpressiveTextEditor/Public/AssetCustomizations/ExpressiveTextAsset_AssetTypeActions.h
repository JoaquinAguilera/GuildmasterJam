// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>

#include "AssetTypeActions_Base.h"
#include "ExpressiveTextEditorFunctions.h"

#include <LevelEditor.h>
#include <WidgetBlueprint.h>
#include <ExpressiveText/Public/Asset/ExpressiveTextAsset.h>
#include <Framework/MultiBox/MultiBoxBuilder.h>
#include <Toolkits/IToolkitHost.h>

#include "ExpressiveTextEditor.h"

class FExpressiveTextAsset_AssetTypeActions : public FAssetTypeActions_Base
{
public:

	FExpressiveTextAsset_AssetTypeActions( )
	{
	}

	virtual FText GetName() const override { return NSLOCTEXT("ExpressiveTextEditor", "AssetTypeActions_ExpressiveText", "Expressive Text"); }
	virtual FColor GetTypeColor() const override { return FColor(89, 205, 144); }
	virtual UClass* GetSupportedClass() const override { return UExpressiveTextAsset::StaticClass(); }
	virtual bool IsImportedAsset() const override { return false; }
	virtual uint32 GetCategories() override{ return FExpressiveTextEditorModule::ExpressiveTextCategory; }
    virtual bool HasActions(const TArray<UObject*>& InObjects) const override { return true; }
    virtual void GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder) override
    {
        auto ObjectsWeakPtrs = GetTypedWeakObjectPtrs<UExpressiveTextAsset>(InObjects);

        MenuBuilder.AddMenuEntry(
            FText::FromString("Open using Data Asset view"),
            FText::FromString("Opens the expressive text asset in data form in order to manually edit it"),
            FSlateIcon(),
            FUIAction(
                FExecuteAction::CreateSP(this, &FExpressiveTextAsset_AssetTypeActions::OpenWithAssetView, ObjectsWeakPtrs),
                FCanExecuteAction()
            )
        );
    }

	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) override 
	{
        if (InObjects.Num() > 0)
        {
            if (auto* Asset = Cast<UExpressiveTextAsset>(InObjects[0]))
            {
                UExpressiveTextEditorFunctions::StartCustomEditor(*Asset, FSoftObjectPath(TEXT("/ExpressiveText/Core/Editor/CustomEditors/AssetEditor.AssetEditor") ) );
            }
        }
	}

    FSlateIcon FindIcon(const FName& IconName) const
    {
        FSlateIcon Icon;

        FSlateStyleRegistry::IterateAllStyles(
            [&](const ISlateStyle& Style)
        {
            if (Style.GetOptionalBrush(IconName, nullptr, nullptr))
            {
                Icon = FSlateIcon(Style.GetStyleSetName(), IconName);
                // terminate iteration
                return false;
            }
            // continue iteration
            return true;
        }
        );

        return Icon;
    }

    void OpenWithAssetView( const TArray<TWeakObjectPtr<UExpressiveTextAsset>> Objects )
    {
        TArray<UObject*> ValidObjects;
        for (auto Obj : Objects)
        {
            if( auto* RawObject = Obj.Get() )
            {
                ValidObjects.Add(RawObject);
            }
        }

        if (ValidObjects.Num() > 0)
        {
            FSimpleAssetEditor::CreateEditor(EToolkitMode::Standalone, nullptr, ValidObjects);
        }
    }

    

    uint32 ExpressiveTextCategory;
};