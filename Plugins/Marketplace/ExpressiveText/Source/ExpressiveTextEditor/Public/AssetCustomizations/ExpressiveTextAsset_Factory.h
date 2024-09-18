// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>

#include <Factories/Factory.h>

#include "Asset/ExpressiveTextAsset.h"

#include "ExpressiveTextAsset_Factory.generated.h"

UCLASS()
class UExpressiveTextAsset_Factory : public UFactory
{
    GENERATED_BODY()
    
    UExpressiveTextAsset_Factory()
        : Super()
    {
        SupportedClass = UExpressiveTextAsset::StaticClass();
        bCreateNew = true;
        bEditAfterNew = true;
    }

    virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override
    {
        check(Class->IsChildOf(UExpressiveTextAsset::StaticClass()));
        return NewObject<UExpressiveTextAsset>(InParent,Class,Name,Flags|RF_Transactional,Context);
    }
};