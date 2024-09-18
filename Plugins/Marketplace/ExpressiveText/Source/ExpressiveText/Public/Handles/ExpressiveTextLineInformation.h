// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>

#include "ExpressiveTextLineInformation.generated.h"

USTRUCT(BlueprintType)
struct FExpressiveTextLineInformation
{
	GENERATED_BODY()

public:

	FExpressiveTextLineInformation()
		: Offset( FVector2D::ZeroVector )
		, Size(FVector2D::ZeroVector)
		, TextHeight(-1.f)
		, JustificationWidth(-1.f)
		, Range()
	{
	}


	UPROPERTY(Category = ExpressiveText, BlueprintReadOnly, VisibleAnywhere )
	FVector2D Offset;

	UPROPERTY(Category = ExpressiveText, BlueprintReadOnly, VisibleAnywhere)
	FVector2D Size;

	UPROPERTY(Category = ExpressiveText, BlueprintReadOnly, VisibleAnywhere)
	float TextHeight;

	UPROPERTY(Category = ExpressiveText, BlueprintReadOnly, VisibleAnywhere)
	float JustificationWidth;

	FTextRange Range;

	float GetCharacterCount()
	{
		return Range.Len();
	}
};

USTRUCT(BlueprintType)
struct FExpressiveTextLayoutInformation
{
	GENERATED_BODY()

public:
	FExpressiveTextLayoutInformation()
		: DrawSize(FVector2D::ZeroVector)
		, WrappedDrawSize(FVector2D::ZeroVector)
	{}


	UPROPERTY(Category = ExpressiveText, BlueprintReadOnly)
	FVector2D DrawSize;

	UPROPERTY(Category = ExpressiveText, BlueprintReadOnly)
	FVector2D WrappedDrawSize;
};
