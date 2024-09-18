// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>

#include "ExpressiveTextWrapSettings.generated.h"

UENUM(BlueprintType)
enum class EExpressiveTextWrapMode : uint8
{
    WrapAtPixelCount,
    WrapAtPercentageOfParentSize,
    NoWrap
};

USTRUCT( BlueprintType )
struct FExpressiveTextWrapSettings {

	GENERATED_BODY() 

    FExpressiveTextWrapSettings()
        : ValueType( EExpressiveTextWrapMode::WrapAtPercentageOfParentSize)
        , Value( 1.f )
    {}

    int64 CalcChecksum() const
    {
        return  GetTypeHash( ValueType ) + static_cast<int32>( Value * 100.f );
    }

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = ExpressiveText )
    EExpressiveTextWrapMode ValueType;
    
    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = ExpressiveText, meta = ( UIMin = 0, ClampMin = 0, EditCondition = "ValueType != EExpressiveTextWrapMode::NoWrap") )
    float Value;
};