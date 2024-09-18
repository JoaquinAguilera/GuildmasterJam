// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>

#include "CompiledExpressiveCharacter.h"
#include "Layout/ExpressiveTextAlignment.h"
#include "Layout/ExpressiveTextWrapSettings.h"
#include "ExpressiveText/Public/Extractions/TagsExtraction.h"

#include "CompiledExpressiveText.generated.h"

USTRUCT( BlueprintType )
struct EXPRESSIVETEXT_API FCompiledExpressiveText
{
    GENERATED_BODY()

    FCompiledExpressiveText()
        : Characters()
        , NewLines()
        , LastStartTimeStamp( 0.f )
        , TotalDuration( 0.f )
        , DrawSize( FVector2D::ZeroVector )
        , Alignment()
        , WrapSettings()
        , UseAutoSize( false )
        , HarvestedResources()
    {
    }

    UPROPERTY( VisibleAnywhere, BlueprintReadWrite, Category = ExpressiveText )
    TArray<FCompiledExpressiveCharacter> Characters;
    
    UPROPERTY( VisibleAnywhere, BlueprintReadWrite, Category = ExpressiveText )
    TArray<int32> NewLines;

    UPROPERTY( VisibleAnywhere, Category = ExpressiveText )
    float LastStartTimeStamp;

    UPROPERTY( VisibleAnywhere, BlueprintReadWrite, Category = ExpressiveText )
    float TotalDuration;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = ExpressiveText )
    FVector2D DrawSize;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = ExpressiveText )
    FExpressiveTextAlignment Alignment;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = ExpressiveText )
    FExpressiveTextWrapSettings WrapSettings;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = ExpressiveText)
    bool UseAutoSize;

    UPROPERTY(VisibleAnywhere, Category = ExpressiveText )
    TArray< UObject* > HarvestedResources;
};


