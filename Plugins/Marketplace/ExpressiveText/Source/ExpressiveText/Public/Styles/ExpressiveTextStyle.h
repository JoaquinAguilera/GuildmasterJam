// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>

#include "ExpressiveTextStyleBase.h"
#include "ExpressiveTextStyle.generated.h"

class UExpressiveTextParameterValue;


UCLASS()
class EXPRESSIVETEXT_API UExpressiveTextStyle : public UExpressiveTextStyleBase
{
	GENERATED_BODY()

public:

	UExpressiveTextStyle()
		: Super()
		, Disabled( false )
	{}

	virtual bool NeedsLoadForClient() const override
	{
		return !IsDistabled();
	}

	virtual bool NeedsLoadForServer() const override
	{
		return false;
	}

	virtual FName GetDescriptor() const override
	{
		return GetFName();
	}

	bool IsDistabled() const
	{
		return Disabled;
	}

private:

	UPROPERTY( EditAnywhere, Category = StyleSettings, meta = ( ToolTip = "Disables this style from being used. Also ensures it isn't cooked in builds" ) )
	bool Disabled;
};
