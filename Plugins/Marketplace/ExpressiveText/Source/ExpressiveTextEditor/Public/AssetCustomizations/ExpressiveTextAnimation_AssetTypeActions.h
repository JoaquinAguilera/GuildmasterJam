// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>

#include "AssetTypeActions_Base.h"

#include <ExpressiveText/Public/Animation/ExpressiveTextAnimation.h>
#include "ExpressiveTextEditorFunctions.h"

#include "ExpressiveTextEditor.h"

class FExpressiveTextAnimation_AssetTypeActions : public FAssetTypeActions_Base
{
public:

	virtual FText GetName() const override { return NSLOCTEXT("ExpressiveTextAnimation", "FExpressiveTextAnimation_AssetTypeActions", "Expressive Text Animation"); }
	virtual FColor GetTypeColor() const override { return FColor(230, 194, 41); }
	virtual UClass* GetSupportedClass() const override{ return UExpressiveTextAnimation::StaticClass(); }
	virtual uint32 GetCategories() override { return FExpressiveTextEditorModule::ExpressiveTextCategory; }

	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) override 
	{
        if (InObjects.Num() > 0)
        {
            if (auto* Asset = Cast<UExpressiveTextAnimation>(InObjects[0]))
            {
                UExpressiveTextEditorFunctions::StartCustomEditor(*Asset, FSoftObjectPath(TEXT("/ExpressiveText/Core/Editor/CustomEditors/AnimationEditor.AnimationEditor") ) );
            }
        }
	}
};