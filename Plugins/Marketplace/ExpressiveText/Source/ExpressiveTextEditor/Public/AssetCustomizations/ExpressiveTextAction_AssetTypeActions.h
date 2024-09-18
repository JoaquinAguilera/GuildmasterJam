// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>

#include "AssetTypeActions_Base.h"

#include <ExpressiveText/Public/Actions/ExpressiveTextActionBase.h>

#include "ExpressiveTextEditor.h"

class FExpressiveTextAction_AssetTypeActions : public FAssetTypeActions_Base
{
public:

	virtual FText GetName() const override { return NSLOCTEXT("ExpressiveTextActions", "FExpressiveTextAction_AssetTypeActions", "Expressive Text Action"); }
	virtual FColor GetTypeColor() const override { return FColor(63, 126, 255); }
	virtual UClass* GetSupportedClass() const override{ return UExText_ActionBase::StaticClass(); }
	virtual uint32 GetCategories() override { return FExpressiveTextEditorModule::ExpressiveTextCategory; }
};