// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"

#include "ExpressiveTextStyleBase.h"

#include "ExpressiveTextDefaultStyle.generated.h"

UCLASS()
class EXPRESSIVETEXT_API UExpressiveTextDefaultStyle : public UExpressiveTextStyleBase
{
	GENERATED_BODY()

public:

	UExpressiveTextDefaultStyle()
		: Super()
	{}

	virtual FName GetDescriptor() const override
	{
		return FName("Default");
	}
};