// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>

#include <Engine/Font.h>

#include "ExpressiveTextParamsCategories.h"

#include "ExpressiveTextParameterValue.generated.h"

class UExpressiveTextParameterExtraWidget;

UCLASS( CollapseCategories, BlueprintType )
class EXPRESSIVETEXT_API UExpressiveTextParameterValue : public UObject
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	UFUNCTION(BlueprintCallable, Category = "Expressive Text")
	virtual FString MoreInfoTooltip() const 
	{
		return TEXT("");
	}

	UFUNCTION(BlueprintCallable, Category = "Expressive Text")
	virtual TSubclassOf<UUserWidget> DisplayExtraWidget() const
	{
		return nullptr;
	}

	UFUNCTION(BlueprintCallable, Category = "Expressive Text")
	virtual TSubclassOf<UExTextParameterCategory> GetParameterCategory() const
	{
		return UExTextParameterCategory_Other::StaticClass();
	}

	virtual void ConfirmUsage() const {}
#endif

};