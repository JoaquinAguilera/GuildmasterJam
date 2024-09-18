// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>

#include "ExpressiveTextEditor.h"

#include <ExpressiveText/Public/Animation/ExpressiveTextAnimation.h>
#include <Factories/Factory.h>

#include "ExpressiveTextAnimation_Factory.generated.h"

UCLASS()
class UExpressiveTextAnimation_Factory : public UFactory
{
    GENERATED_BODY()
    
    UExpressiveTextAnimation_Factory()
        : Super()
    {
        SupportedClass = UExpressiveTextAnimation::StaticClass();
        bCreateNew = true;
        bEditAfterNew = true;
    }

    virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override
    {
        check(Class->IsChildOf(UExpressiveTextAnimation::StaticClass()));
        return NewObject<UExpressiveTextAnimation>(InParent,Class,Name,Flags|RF_Transactional,Context);
    }

    virtual uint32 GetMenuCategories() const override
    {
        return FExpressiveTextEditorModule::ExpressiveTextCategory;
    }
};