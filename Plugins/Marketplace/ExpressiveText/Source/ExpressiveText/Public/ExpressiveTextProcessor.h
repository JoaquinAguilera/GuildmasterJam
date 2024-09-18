// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>
#include <Kismet/BlueprintFunctionLibrary.h>

#include "Compiled/CompiledExpressiveText.h"
#include "Handles/ExpressiveText.h"

#include "ExpressiveTextProcessor.generated.h"

class UExpressiveTextParameterValue;

UCLASS()
class EXPRESSIVETEXT_API UExpressiveTextProcessor : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public: 

#if WITH_EDITOR
    UFUNCTION(BlueprintCallable, Category = ExpressiveText, meta=(WorldContext="WorldContext"))
    static void GetExtractions(UObject* WorldContext, FExpressiveText ExpressiveText, TArray<FExpressiveLineExtractionsInfo>& OutLinesExtractionsInfo);
#endif

    UFUNCTION( BlueprintPure, Category = ExpressiveText,  meta = (DeterminesOutputType = "Type"))
    static UExpressiveTextParameterValue* GetParameter( const FCompiledExpressiveCharacter& Character, TSubclassOf<UExpressiveTextParameterValue> Type );

    UFUNCTION( BlueprintCallable, Category = ExpressiveText )
    static void StringIntoLinesArray( const FString& String, TArray<FString>& OutLines );

    static TFuture<FCompiledExpressiveText> CompileText(FExpressiveText ExpressiveText);
	
};
