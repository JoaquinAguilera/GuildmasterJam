// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>

#include <Kismet/BlueprintFunctionLibrary.h>

#include "ExpressiveTextAlignment.generated.h"

UENUM(BlueprintType)
enum class EExpressiveTextHorizontalAlignment : uint8
{
    Left = 0,
    Center = 1,
    Right = 2	
};

UENUM(BlueprintType)
enum class EExpressiveTextVerticalAlignment : uint8
{
    Top = 0,
    Center = 1,
    Bottom = 2
};

UENUM(BlueprintType)
enum class EExpressiveTextAlignmentOffsetType : uint8
{
    PercentageOfParentSize,
    Pixels
};

USTRUCT( BlueprintType )
struct FExpressiveTextAlignment {

	GENERATED_BODY() 

    FExpressiveTextAlignment( EExpressiveTextVerticalAlignment InVerticalAlignment, EExpressiveTextHorizontalAlignment InHorizontalAlignment )
        : VerticalAlignment( InVerticalAlignment )
        , HorizontalAlignment( InHorizontalAlignment )
        , VerticalOffset( 0.f )
        , HorizontalOffset( 0.f )
        , VerticalOffsetType( EExpressiveTextAlignmentOffsetType::PercentageOfParentSize )
        , HorizontalOffsetType( EExpressiveTextAlignmentOffsetType::PercentageOfParentSize )
    {}

    FExpressiveTextAlignment()
        : FExpressiveTextAlignment( 
            EExpressiveTextVerticalAlignment::Top, 
            EExpressiveTextHorizontalAlignment::Left 
        )
    {

    }

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = ExpressiveText )
    EExpressiveTextVerticalAlignment VerticalAlignment;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = ExpressiveText )
    EExpressiveTextHorizontalAlignment HorizontalAlignment;
    
    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = ExpressiveText )
    float VerticalOffset;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = ExpressiveText )
    float HorizontalOffset;    

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = ExpressiveText )
    EExpressiveTextAlignmentOffsetType VerticalOffsetType;
    
    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = ExpressiveText )
    EExpressiveTextAlignmentOffsetType HorizontalOffsetType;


    int64 CalcChecksum() const
    {
        return  GetTypeHash(VerticalAlignment) +
            GetTypeHash(HorizontalAlignment) * 10 +
            GetTypeHash(VerticalOffsetType) * 100 +
            GetTypeHash(HorizontalOffsetType) * 1000 +
            static_cast<int32>(VerticalOffset * 100.f) +
            static_cast<int32>(HorizontalOffset * 1000.f);
    }

    FVector2D CalculateAnchorPoint( const FVector2D& DisplaySize ) const
    {
        return CalculateAnchorWeigth() * DisplaySize;
    }

    FVector2D CalculateTextStartOffset( const FVector2D& TextSize ) const 
    {
        return CalculateAnchorWeigth() * TextSize * -1.f;
    }

    FVector2D CalculateAnchorWeigth() const
    {
        return FVector2D( CalculateAnchorWeigth1D( HorizontalAlignment ), CalculateAnchorWeigth1D( VerticalAlignment ) );
    }

    FVector2D CalculateDesiredPosition( const FVector2D& DisplaySize, const FVector2D& TextSize ) const
    {
		const FVector2D AnchorPoint = CalculateAnchorPoint( DisplaySize );
        const FVector2D TextOffsetBasedOnAlignment = CalculateTextStartOffset( TextSize );

        FVector2D CalculatedTextStartPos = AnchorPoint + TextOffsetBasedOnAlignment;

        ApplyOffset( CalculatedTextStartPos.X, HorizontalOffset, HorizontalOffsetType, DisplaySize.X );
        ApplyOffset( CalculatedTextStartPos.Y, VerticalOffset, VerticalOffsetType, DisplaySize.Y );

        return CalculatedTextStartPos;
    }

    template< typename TAlignmentType >
    static float CalculateAnchorWeigth1D( TAlignmentType AlignmentType )
    {
        return static_cast<double>( AlignmentType ) / 2.f;
    }

    template< typename Number >
    static void ApplyOffset( Number& Value, const double Offset, EExpressiveTextAlignmentOffsetType OffsetType, double DisplaySizeAtAxis )
    {
        if( OffsetType == EExpressiveTextAlignmentOffsetType::PercentageOfParentSize )
        {
            Value += DisplaySizeAtAxis * Offset;
        }
        else if( OffsetType == EExpressiveTextAlignmentOffsetType::Pixels )
        {
            Value += Offset;
        }
    }
};


UCLASS()
class UExpressiveTextAlignmentFunctions : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintCallable, Category = ExpressiveText )
    static FVector2D PositionToAlignmentOffset( FVector2D Position, FVector2D DisplaySize, FVector2D TextSize , const FExpressiveTextAlignment& Alignment )
    {
        const FVector2D AnchorPoint = Alignment.CalculateAnchorWeigth() * DisplaySize;
        const FVector2D AnchorBasedTextStartOffset = Alignment.CalculateTextStartOffset( TextSize );

        const FVector2D StartPointWithZeroOffset = AnchorPoint + AnchorBasedTextStartOffset;

        FVector2D Offset = Position - StartPointWithZeroOffset;

        const auto ConvertToCorrectSpace = [](auto& Value, EExpressiveTextAlignmentOffsetType OffsetType, float TextSizeAtAxis, float DisplaySizeAtAxis) {
            if (OffsetType == EExpressiveTextAlignmentOffsetType::PercentageOfParentSize)
            {
                Value /= DisplaySizeAtAxis;
            }
        };

        ConvertToCorrectSpace(Offset.X, Alignment.HorizontalOffsetType, TextSize.X, DisplaySize.X);
        ConvertToCorrectSpace(Offset.Y, Alignment.VerticalOffsetType, TextSize.Y, DisplaySize.Y);

        return Offset;
    }

    UFUNCTION( BlueprintCallable, Category = ExpressiveText )
    static FVector2D AlignmentToPosition( const FExpressiveTextAlignment& Alignment, const FVector2D& DisplaySize, const FVector2D& TextSize )
    {
        return Alignment.CalculateDesiredPosition( DisplaySize, TextSize );
    }

    UFUNCTION( BlueprintPure, Category = ExpressiveText )
    static FExpressiveTextAlignment& DefineHorizontalOffset( UPARAM(ref) FExpressiveTextAlignment& Alignment, float Amount, EExpressiveTextAlignmentOffsetType OffsetType )
    {
        Alignment.HorizontalOffset = Amount;
        Alignment.HorizontalOffsetType = OffsetType;
        return Alignment;
    }
    
    UFUNCTION( BlueprintPure, Category = ExpressiveText )
    static FExpressiveTextAlignment& DefineVerticalOffset( UPARAM(ref) FExpressiveTextAlignment& Alignment, float Amount, EExpressiveTextAlignmentOffsetType OffsetType )
    {
        Alignment.VerticalOffset = Amount;
        Alignment.VerticalOffsetType = OffsetType;
        return Alignment;
    }

#define ALIGNMENT_FUNCTION( Vert, Horiz ) { return FExpressiveTextAlignment( EExpressiveTextVerticalAlignment::Vert, EExpressiveTextHorizontalAlignment::Horiz );  }
    
    // Common alignments
    UFUNCTION( BlueprintPure, Category = ExpressiveText )
    static FExpressiveTextAlignment TopLeft()
    ALIGNMENT_FUNCTION( Top, Left )
        
    UFUNCTION( BlueprintPure, Category = ExpressiveText )
    static FExpressiveTextAlignment TopCenter()
    ALIGNMENT_FUNCTION( Top, Center )
        
    UFUNCTION( BlueprintPure, Category = ExpressiveText )
    static FExpressiveTextAlignment TopRight()
    ALIGNMENT_FUNCTION( Top, Right )

    UFUNCTION( BlueprintPure, Category = ExpressiveText )
    static FExpressiveTextAlignment CenterLeft()
    ALIGNMENT_FUNCTION( Center, Left )
        
    UFUNCTION( BlueprintPure, Category = ExpressiveText )
    static FExpressiveTextAlignment Center()
    ALIGNMENT_FUNCTION( Center, Center )
        
    UFUNCTION( BlueprintPure, Category = ExpressiveText )
    static FExpressiveTextAlignment CenterRight()
    ALIGNMENT_FUNCTION( Center, Right )

    UFUNCTION( BlueprintPure, Category = ExpressiveText )
    static FExpressiveTextAlignment BottomLeft()
    ALIGNMENT_FUNCTION( Bottom, Left )
        
    UFUNCTION( BlueprintPure, Category = ExpressiveText )
    static FExpressiveTextAlignment BottomCenter()
    ALIGNMENT_FUNCTION( Bottom, Center )
        
    UFUNCTION( BlueprintPure, Category = ExpressiveText )
    static FExpressiveTextAlignment BottomRight()
    ALIGNMENT_FUNCTION( Bottom, Right )

#undef ALIGNMENT_FUNCTION
};