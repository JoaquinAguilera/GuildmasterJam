// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>
#include <Blueprint/UserWidget.h>

#include "ExpressiveTextParameterExtraWidget.generated.h"

UCLASS()
class UExpressiveTextParameterExtraWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION( BlueprintCallable, BlueprintImplementableEvent, Category = "ExpressiveTextWidget" )
	void Display( class UExpressiveTextParameterValue* Parameter );
};