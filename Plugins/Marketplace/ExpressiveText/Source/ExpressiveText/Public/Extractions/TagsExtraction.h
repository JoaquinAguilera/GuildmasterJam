// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>

#include "Actions/ExpressiveTextActionInterface.h"
#include "ExpressiveText/Public/Actions/ExpressiveTextActionBase.h"
#include "ExText_ParsedInterjection.h"
#include "Misc/EngineVersionComparison.h"
#include "Parameters/ExpressiveTextParameterLookup.h"
#include "TreeExtraction.h"

#include <Kismet/BlueprintFunctionLibrary.h>
#include <UObject/WeakInterfacePtr.h>


#include "TagsExtraction.generated.h"

struct FExTextTokenPosition
{
    int32 Position = -1;
};

struct FExpressiveTextExtraction 
{
    TSharedPtr<FTreeExtraction> ExtractionTree;
    TArray<FExText_ParsedInterjection> Interjections;
};

UCLASS()
class UExpressiveTextExtractionsFunctions : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

    UFUNCTION( BlueprintCallable, Category = ExpressiveText )
    static TArray<FExpressiveTextExtractionSection> GetAllSections( const FExpressiveLineExtractionsInfo& Info )
    {
        TArray<FExpressiveTextExtractionSection> Result;

		if (auto* Extraction = Info.Extraction.Get())
		{
			if (auto* ExtractionTree = Extraction->ExtractionTree.Get())
			{
				ExtractionTree->TrawlOriginalTextSections([&Result](const FExpressiveTextExtractionSection& Section) {
					Result.Add(Section);
				});
			}
		}
        return Result;
    }

};