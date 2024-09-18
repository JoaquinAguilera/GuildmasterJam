// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>

#include <Factories/Factory.h>

#include <ExpressiveText/Public/Actions/ExpressiveTextActionBase.h>
#include <Kismet2/KismetEditorUtilities.h>

#include "Asset/ExpressiveTextAsset.h"
#include "ExpressiveTextEditor.h"

#include "ExpressiveTextAction_Factory.generated.h"

UCLASS()
class UExpressiveTextAction_Factory : public UFactory
{
    GENERATED_BODY()
    
    UExpressiveTextAction_Factory()
        : Super()
    {
        SupportedClass = UExText_ActionBase::StaticClass();
        bCreateNew = true;
        bEditAfterNew = true;
    }

    virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override
    {
        //check(Class->IsChildOf(UExText_ActionBase::StaticClass()));
	    return FKismetEditorUtilities::CreateBlueprint(UExText_ActionBase::StaticClass(), InParent, Name, BPTYPE_Normal, UBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass());
    }

    virtual uint32 GetMenuCategories() const override
    {
        return FExpressiveTextEditorModule::ExpressiveTextCategory;
    }

};