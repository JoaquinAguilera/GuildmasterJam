// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>

#include "Actions/ExpressiveTextActionInterface.h"

#include <Engine/World.h>

#include "ExpressiveTextActionBase.generated.h"

UCLASS(Abstract, Blueprintable, EditInlineNew, CollapseCategories, Category = "ExpressiveText" )
class EXPRESSIVETEXT_API UExText_ActionBase : public UObject, public IExText_ActionInterface
{
    GENERATED_BODY()

public:
    virtual UWorld* GetWorld() const override
    {
        return World.Get();
    }
    
    virtual void SetWorld( UWorld* InWorld ) override
    {
        World = InWorld;
    }

protected:
    TWeakObjectPtr<UWorld> World;
};
