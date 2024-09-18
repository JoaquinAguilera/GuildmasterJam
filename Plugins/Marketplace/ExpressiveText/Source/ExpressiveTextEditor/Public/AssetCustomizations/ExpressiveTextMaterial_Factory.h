// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>

#include "ExpressiveTextEditor.h"

#include <ExpressiveText/Public/Asset/ExpressiveTextMaterial.h>
#include <Factories/Factory.h>

#include "ExpressiveTextMaterial_Factory.generated.h"
UCLASS()
class UExpressiveTextMaterial_Factory : public UFactory
{
    GENERATED_BODY()
    
    UExpressiveTextMaterial_Factory()
        : Super()
    {
        SupportedClass = UExpressiveTextMaterial::StaticClass();
        bCreateNew = true;
        bEditAfterNew = true;
    }

    virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override
    {
        check(Class->IsChildOf(UExpressiveTextMaterial::StaticClass()));
        return NewObject<UExpressiveTextMaterial>(InParent,Class,Name,Flags|RF_Transactional,Context);
    }

    virtual uint32 GetMenuCategories() const override
    {
        return FExpressiveTextEditorModule::ExpressiveTextCategory;
    }
};