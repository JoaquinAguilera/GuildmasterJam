// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>

#include "Parameters/ExpressiveTextParameterLookup.h"
#include "Parameters/ExpressiveTextParams.h"
#include "Extractions/TagsExtraction.h"

#include "CompiledExpressiveCharacter.generated.h"

USTRUCT( BlueprintType )
struct  FCompiledExpressiveCharacter
{
    GENERATED_BODY()

    FCompiledExpressiveCharacter()
        : Glyph()
        , StartTimeStamp( 0.f )
        , EndTimeStamp()
        , Animation()
        , AnimationRate( 1.f )
        , Position( FVector2D::ZeroVector )
        , Bounds( FVector2D::ZeroVector )
        , MaterialAnimation( nullptr )
        , Color( FLinearColor::White )
    {}
      
    UPROPERTY( VisibleAnywhere, BlueprintReadWrite, Category = ExpressiveText )
    FString Glyph;

    UPROPERTY( VisibleAnywhere, BlueprintReadWrite, Category = ExpressiveText )
    float StartTimeStamp;

    TOptional<float> EndTimeStamp;

    UPROPERTY( BlueprintReadWrite, Category = ExpressiveText )
    FName Animation;

    UPROPERTY( BlueprintReadWrite, Category = ExpressiveText )
    float AnimationRate;
    
    UPROPERTY( BlueprintReadWrite, Category = ExpressiveText )
    FVector2D Position;
    
    UPROPERTY( BlueprintReadWrite, Category = ExpressiveText )
    FVector2D Bounds;
    
    UPROPERTY( BlueprintReadWrite, Category = ExpressiveText )
    class UMaterialInterface* MaterialAnimation;
    
    UPROPERTY( BlueprintReadWrite, Category = ExpressiveText )
    FLinearColor Color;

    TSharedPtr<FExpressiveTextParameterLookup> ParameterLookup;
    TArray< TSharedPtr<FExText_Interjection> > Interjections;

    template< typename ValueObjectType, typename ValueType = typename ValueObjectType::ValueType >
    const ValueType& GetValue() const
    {
        return ParameterLookup->GetValue<ValueObjectType, ValueType>();
    } 

    template< typename ValueObjectType >
    const ValueObjectType& GetValueObject() const
    {        
        return ParameterLookup->GetValueObject<ValueObjectType>();
    } 

    const UExpressiveTextParameterValue& GetValueObject( TSubclassOf<UExpressiveTextParameterValue> Type ) const
    {
        return ParameterLookup->GetValueObject( Type );
    }
};

