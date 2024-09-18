// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>
#include <Kismet/BlueprintFunctionLibrary.h>

#include "Compiled/ExTextContext.h"

#include "ExTextContextFunctions.generated.h"

UCLASS()
class UExpressiveTextContextFunctions : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

    UFUNCTION( BlueprintCallable, Category = ExpressiveText )
    static void AddAction( FExpressiveTextContext& Context, FName ActionName, TScriptInterface<IExText_ActionInterface> Action )
    {
        Context.AddAction( ActionName, Action );
    }

};