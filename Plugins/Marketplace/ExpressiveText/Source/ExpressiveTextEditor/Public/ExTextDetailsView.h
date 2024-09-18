// Copyright 2022 Guganana. All Rights Reserved.
#pragma once 

#include <CoreMinimal.h>

#include <Components/DetailsView.h>

#include "ExTextDetailsView.generated.h"

UCLASS()
class UExTextDetailsView : public UDetailsView
{
    GENERATED_BODY()

    #if WITH_EDITOR
	virtual const FText GetPaletteCategory() override
    {
        return FText::FromString(TEXT("Expressive Text Editor"));
    }
    #endif

    UFUNCTION(BlueprintCallable, Category = "Expressive Text Details View")
    void SetPropertiesToShow(const TArray<FName>& Props)
    {
        PropertiesToShow = Props;
    }

};
