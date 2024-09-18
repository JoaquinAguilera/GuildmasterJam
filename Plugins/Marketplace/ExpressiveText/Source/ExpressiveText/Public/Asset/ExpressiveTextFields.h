// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"

#include "Actions/ExpressiveTextAssetActions.h"
#include "Actions/ExpressiveTextActionBase.h"
#include "Compiled/ExTextContext.h"
#include "Layout/ExpressiveTextAlignment.h"
#include "Layout/ExpressiveTextWrapSettings.h"
#include <Misc/EngineVersionComparison.h>

#include <Framework/Text/TextLayout.h>

#include "ExpressiveTextFields.generated.h"

class UExpressiveTextStyleBase;
class UExpressiveTextAssetActions;
class IExText_ActionInterface;

USTRUCT( BlueprintType )
struct FExpressiveTextFields
{
	GENERATED_BODY()

	FExpressiveTextFields() 
        : Text()
        , DefaultFontSize( 12 )
        , UseDefaultFontSize( false )
		, WrapSettings()
        , UseAutoSize( false )
		, Alignment()
		, Justification()
        , DefaultStyle()
        , Actions()
        , ReferencedResources()
		, AliveHandle ( MakeShareable( new int32(1) ) )
    {
    }

	virtual ~FExpressiveTextFields()
	{
		AliveHandle.Reset();
	}

	int64 CalcChecksum() const
	{
		int64 Result = 0;
		Result = HashCombine(Result, WrapSettings.CalcChecksum());
		Result = HashCombine(Result, Alignment.CalcChecksum());
		Result = HashCombine(Result, GetTypeHash(Text.ToString()));
		Result = HashCombine(Result, GetTypeHash(Justification));
		Result = HashCombine(Result, GetTypeHash(DefaultStyle));
		Result = HashCombine(Result, GetTypeHash(DefaultFontSize));
		Result = HashCombine(Result, GetTypeHash(UseDefaultFontSize));
		return Result;
	}

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = ExpressiveText, meta = (MultiLine = true))
    FText Text;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = ExpressiveText)
	int32 DefaultFontSize;
    
    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = ExpressiveText )
	bool UseDefaultFontSize;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = ExpressiveText )
    FExpressiveTextWrapSettings WrapSettings;
    
    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = ExpressiveText, meta = (Tooltip = "Activate to automatically resize text to fill the parent's size"))
    bool UseAutoSize;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = ExpressiveText )
    FExpressiveTextAlignment Alignment;
    
    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = ExpressiveText )
    TEnumAsByte<ETextJustify::Type> Justification;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ExpressiveText)
    UExpressiveTextStyleBase* DefaultStyle;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = ExpressiveText, Instanced )
    UExpressiveTextAssetActions* Actions;
	
    UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = ExpressiveText )
    TArray< UObject* > ReferencedResources;

	TWeakPtr<int32> GetAliveHandle() const
	{
		return AliveHandle;
	}

    FExpressiveTextContext CreateContext() const
    {
        FExpressiveTextContext Context;

        TWeakPtr<int32> Handle = GetAliveHandle();

        Context.SetSearchOuter([Handle, this](const FName& Name) -> IExText_ActionInterface* 
        {
            if (Handle.IsValid())
            {
                if (Actions)
                {
                    if (UExText_ActionBase** Result = Actions->Actions.Find(Name))
                    {
                        return *Result;
                    }
                }
            }

            return nullptr;
        });

        return Context;
    }

private:
	TSharedPtr<int32> AliveHandle;
};
