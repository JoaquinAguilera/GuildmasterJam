// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>

#include <Factories/Factory.h>

#include "ExpressiveTextEditor.h"

#include <ExpressiveText/Public/Asset/ExpressiveTextFont.h>

#include "ExpressiveTextFont_Factory.generated.h"
UCLASS()
class UExpressiveTextFont_Factory : public UFactory
{
    GENERATED_BODY()
    
    UExpressiveTextFont_Factory()
        : Super()
    {
        SupportedClass = UExpressiveTextFont::StaticClass();
        bCreateNew = true;
        bEditAfterNew = true;
    }

    virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override
    {
        check(Class->IsChildOf(UExpressiveTextFont::StaticClass()));
        return NewObject<UExpressiveTextFont>(InParent,Class,Name,Flags|RF_Transactional,Context);
    }

    virtual uint32 GetMenuCategories() const override
    {
        return FExpressiveTextEditorModule::ExpressiveTextCategory;
    }
};