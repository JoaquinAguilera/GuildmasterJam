// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>

#include <Engine/DataAsset.h>

#include "ExpressiveTextFont.generated.h"

class UFont;

UCLASS(BlueprintType)
class EXPRESSIVETEXT_API UExpressiveTextFont : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:

    UFont* GetObject() const
    {
        return Font;
    }
    
    UPROPERTY( EditAnywhere, Category = "Expressive Text Font" )
    UFont* Font;
};