// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>

#include "ExpressiveTextAlignment.h"
#include "ExpressiveTextWrapSettings.h"

#include "ExpressiveTextPositioning.generated.h"

UCLASS( Abstract, BlueprintType, EditInlineNew )
class EXPRESSIVETEXT_API UExpressiveTextPositioningMethod
    : public UObject
{
    GENERATED_BODY()

public:

#if WITH_EDITORONLY_DATA
	virtual void FetchSettings(FExpressiveTextAlignment& OutAlignment, FExpressiveTextWrapSettings& OutWidth) const
	{
	}

	virtual int64 CalcChecksum() const
	{
		// must implement
		check(false);
		return  0;
	}
#endif
};

UCLASS( )
class EXPRESSIVETEXT_API UExpressiveTextParameterizedPositioning
    : public UExpressiveTextPositioningMethod
{
    GENERATED_BODY()
public:

#if WITH_EDITORONLY_DATA
	virtual void FetchSettings(FExpressiveTextAlignment& OutAlignment, FExpressiveTextWrapSettings& OutWidth) const override
	{
		OutAlignment = Alignment;
		OutWidth = WrapSettings;
	}

	virtual int64 CalcChecksum() const override
	{
		int64 Result = 0;
		Result = HashCombine(Result, WrapSettings.CalcChecksum());
		Result = HashCombine(Result, Alignment.CalcChecksum());
		return Result;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ExpressiveText)
	FExpressiveTextAlignment Alignment;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ExpressiveText)
	FExpressiveTextWrapSettings WrapSettings;
#endif
};

USTRUCT(BlueprintType)
struct FExpressiveTextOnscreenPositioningBasis {

	GENERATED_BODY()

    FExpressiveTextOnscreenPositioningBasis()
        : VerticalAlignment(EExpressiveTextVerticalAlignment::Center)
        , HorizontalAlignment(EExpressiveTextHorizontalAlignment::Center)
        , VerticalOffsetType(EExpressiveTextAlignmentOffsetType::PercentageOfParentSize)
        , HorizontalOffsetType(EExpressiveTextAlignmentOffsetType::PercentageOfParentSize)
        , WidthType( EExpressiveTextWrapMode::WrapAtPercentageOfParentSize )
    {
    }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ExpressiveText)
	EExpressiveTextVerticalAlignment VerticalAlignment;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ExpressiveText)
	EExpressiveTextHorizontalAlignment HorizontalAlignment;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ExpressiveText)
	EExpressiveTextAlignmentOffsetType VerticalOffsetType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ExpressiveText)
	EExpressiveTextAlignmentOffsetType HorizontalOffsetType;
	
    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = ExpressiveText )
    EExpressiveTextWrapMode WidthType;
};

UCLASS( )
class EXPRESSIVETEXT_API UExpressiveTextOnscreenPositioning
    : public UExpressiveTextPositioningMethod
{
    GENERATED_BODY()

public:

	UExpressiveTextOnscreenPositioning()
		: Super()
#if WITH_EDITORONLY_DATA
		, PositioningBasis()
		, Alignment()
		, WrapSettings()
		, PreviewBoxSizeScalar( FVector2D(0.35f, 0.35f) )
		, PreviewBoxTopLeftCornerScalar( FVector2D::ZeroVector )
#endif
	{}

	UFUNCTION( BlueprintCallable, Category = ExpressiveText )
	void Populate( FVector2D EditorViewportSize, FVector2D InPreviewBoxTopLeftCorner, FVector2D InPreviewBoxSize )
	{
#if WITH_EDITORONLY_DATA
		// store values in 0-to-1 so it is resolution independant
		PreviewBoxSizeScalar = InPreviewBoxSize / EditorViewportSize;
		PreviewBoxTopLeftCornerScalar = InPreviewBoxTopLeftCorner / EditorViewportSize;

		Alignment.VerticalAlignment = PositioningBasis.VerticalAlignment;
		Alignment.HorizontalAlignment = PositioningBasis.HorizontalAlignment;
		Alignment.VerticalOffsetType = PositioningBasis.VerticalOffsetType;
		Alignment.HorizontalOffsetType = PositioningBasis.HorizontalOffsetType;

		const FVector2D Offset = UExpressiveTextAlignmentFunctions::PositionToAlignmentOffset(InPreviewBoxTopLeftCorner, EditorViewportSize, InPreviewBoxSize, Alignment);
		Alignment.HorizontalOffset = Offset.X;
		Alignment.VerticalOffset = Offset.Y;
		WrapSettings.ValueType = PositioningBasis.WidthType;
		
		if (WrapSettings.ValueType == EExpressiveTextWrapMode::WrapAtPixelCount)
		{
			WrapSettings.Value = InPreviewBoxSize.X;
		}
		else if (WrapSettings.ValueType == EExpressiveTextWrapMode::WrapAtPercentageOfParentSize)
		{
			WrapSettings.Value = InPreviewBoxSize.X / EditorViewportSize.X;
		}
		else
		{
			//missing support for WithValueType
			check(false);
		}
#endif
	}

#if WITH_EDITORONLY_DATA
	virtual int64 CalcChecksum() const override
	{
		int64 Result = 0;
		Result = HashCombine(Result, WrapSettings.CalcChecksum());
		Result = HashCombine(Result, Alignment.CalcChecksum());
		return Result;
	}

	virtual void FetchSettings(FExpressiveTextAlignment& OutAlignment, FExpressiveTextWrapSettings& OutWidth) const override
	{
		OutAlignment = Alignment;
		OutWidth = WrapSettings;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ExpressiveText)
	FExpressiveTextOnscreenPositioningBasis PositioningBasis;

	UPROPERTY()
	FExpressiveTextAlignment Alignment;
	
	UPROPERTY()
	FExpressiveTextWrapSettings WrapSettings;

	// Parameters used to re-set the viewport overlay
	UPROPERTY(BlueprintReadWrite, Category = ExpressiveText)
	FVector2D PreviewBoxSizeScalar;

	UPROPERTY(BlueprintReadWrite, Category = ExpressiveText)
	FVector2D PreviewBoxTopLeftCornerScalar;
#endif
};