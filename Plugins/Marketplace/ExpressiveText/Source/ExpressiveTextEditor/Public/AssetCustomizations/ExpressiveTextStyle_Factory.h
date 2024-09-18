// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>

#include "ExpressiveTextEditor.h"
#include "Asset/ExpressiveTextAsset.h"

#include <ExpressiveText/Public/Styles/ExpressiveTextStyle.h>
#include <Factories/Factory.h>

#include "ExpressiveTextStyle_Factory.generated.h"
UCLASS()
class UExpressiveTextStyle_Factory : public UFactory
{
    GENERATED_BODY()
    
    UExpressiveTextStyle_Factory()
        : Super()
    {
        SupportedClass = UExpressiveTextStyle::StaticClass();
        bCreateNew = true;
        bEditAfterNew = true;
    }

    virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override
    {
        check(Class->IsChildOf(UExpressiveTextStyle::StaticClass()));
        return NewObject<UExpressiveTextStyle>(InParent,Class,Name,Flags|RF_Transactional,Context);
    }

    virtual uint32 GetMenuCategories() const override
    {
        return FExpressiveTextEditorModule::ExpressiveTextCategory;
    }
};