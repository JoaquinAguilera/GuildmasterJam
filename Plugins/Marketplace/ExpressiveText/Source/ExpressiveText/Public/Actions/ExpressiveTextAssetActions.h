// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"

#include "ExpressiveTextAssetActions.generated.h"

class UExText_ActionBase;

UCLASS( BlueprintType, EditInlineNew )
class EXPRESSIVETEXT_API UExpressiveTextAssetActions
    : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY( EditAnywhere, Instanced, Category = ExpressiveText )
    TMap<FName, UExText_ActionBase*> Actions;
};
