// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>

#include "Handles/ExpressiveText.h"

#include "ExpressiveTextSelector.generated.h"


USTRUCT( BlueprintType )
struct FExpressiveTextSelector
{
	GENERATED_BODY()
	
	FExpressiveTextSelector()
		: Asset( nullptr )
		, InlinedExpressiveText()
		, InjectedText()
	{}

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = ExpressiveText )
	class UExpressiveTextAsset* Asset;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = ExpressiveText)
	FExpressiveTextFields InlinedExpressiveText;

	TOptional<FExpressiveText> InjectedText;
	
	int64 CalcChecksum() const;
	FExpressiveText GenerateExpressiveText() const;
};