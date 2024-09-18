// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>

#include "Actions/ExpressiveTextAssetActions.h"
#include "Asset/ExpressiveTextFields.h"
#include "Compiled/CompiledExpressiveText.h"
#include "Extractions/TagsExtraction.h"
#include "Layout/ExpressiveTextAlignment.h"
#include "Handles/ExpressiveTextSelector.h"
#include "Handles/ExpressiveText.h"
#include "Layout/ExpressiveTextPositioning.h"
#include "Layout/ExpressiveTextWrapSettings.h"

#if !UE_VERSION_OLDER_THAN( 5, 0, 0 )
#include <UObject/ObjectSaveContext.h>
#endif

#include "ExpressiveTextAsset.generated.h"

UCLASS( BlueprintType, EditInlineNew, Category = "ExpressiveText")
class EXPRESSIVETEXT_API UExpressiveTextAsset 
    : public UObject
{
    GENERATED_BODY()

public:

    UExpressiveTextAsset() 
        : Super()
		, Fields()
    {
		if( Fields.Actions == nullptr )
		{
			Fields.Actions = CreateDefaultSubobject<UExpressiveTextAssetActions>(TEXT("FieldsActions"));
		}
    }
    
    UFUNCTION( BlueprintCallable, Category = ExpressiveText )
    int64 CalcChecksum() const
    {
        int64 Result = 0;
		Result = HashCombine(Result, Fields.CalcChecksum());
        return Result;
    }

    UFUNCTION( BlueprintCallable, Category = ExpressiveText )
    FExpressiveTextContext CreateContext()
    {
        return Fields.CreateContext();
    }

	UFUNCTION(BlueprintCallable, Category = ExpressiveText)
	void ProcessPositioning(FExpressiveTextAlignment& OutAlignment, FExpressiveTextWrapSettings& OutWidth)
	{
#if WITH_EDITORONLY_DATA	
		if (!IsValid(Positioning))
		{
			OutAlignment.VerticalAlignment = EExpressiveTextVerticalAlignment::Center;
			OutAlignment.HorizontalAlignment = EExpressiveTextHorizontalAlignment::Center;
			OutWidth.ValueType = EExpressiveTextWrapMode::WrapAtPercentageOfParentSize;
			OutWidth.Value = 1.f;
		}
		else
		{
			Positioning->FetchSettings(OutAlignment, OutWidth);
		}
#endif
	}
	
#if UE_VERSION_OLDER_THAN( 5, 0, 0 )
    virtual void PreSave( const class ITargetPlatform * TargetPlatform ) override
	{
		Super::PreSave(TargetPlatform);
		OnPreSave();
	}
#else
	virtual void PreSave(FObjectPreSaveContext SaveContext)
	{
		Super::PreSave(SaveContext);
		OnPreSave();
	}
#endif

	void OnPreSave();

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = ExpressiveText )
    FExpressiveTextFields Fields;
	
#if WITH_EDITORONLY_DATA
    UPROPERTY( EditAnywhere, BlueprintReadOnly, Instanced, Category = ExpressiveText )
    UExpressiveTextPositioningMethod* Positioning;
#endif

};

UCLASS()
class UExpressiveTextFunctions : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public: 

	UFUNCTION( BlueprintPure, Category = "ExpressiveText", meta = ( WorldContext = "WorldContextObject" ) )
	static FExpressiveText GetExpressiveTextFromSelector( UObject* WorldContextObject, const FExpressiveTextSelector& Selector )
	{
		FExpressiveText GeneratedExpressiveText = Selector.GenerateExpressiveText();
		GeneratedExpressiveText.SetWorldContext(WorldContextObject);
		return GeneratedExpressiveText;
	}

	UFUNCTION(BlueprintPure, Category = "ExpressiveText", meta = (WorldContext = "WorldContextObject"))
	static FExpressiveText MakeExpressiveTextFromFields(UObject* WorldContextObject, const FExpressiveTextFields& Fields)
	{
		FExpressiveText Result;
		Result.SetWorldContext(WorldContextObject);
		Result.SetFields(Fields);
		return Result;
	}

	UFUNCTION(BlueprintPure, Category = "ExpressiveText", meta = (WorldContext = "WorldContextObject"))
	static FExpressiveText MakeExpressiveTextFromAsset(UObject* WorldContextObject, UExpressiveTextAsset* Asset)
	{
		FExpressiveText Result;
		Result.SetWorldContext(WorldContextObject);
		if (Asset)
		{
			Result.SetAsset(*Asset);
		}
		return Result;
	}

	UFUNCTION(BlueprintPure, Category = "ExpressiveText", meta = (WorldContext = "WorldContextObject"))
	static FExpressiveText NewExpressiveText(UObject* WorldContextObject, FText StartingText)
	{
		FExpressiveText NewText;
		NewText.SetWorldContext(WorldContextObject);
		NewText.SetText(StartingText);
		return NewText;
	}

	UFUNCTION(BlueprintPure, Category = "ExpressiveText" )
	static FExpressiveText& AddTemplateValue( UPARAM(ref) FExpressiveText& Text, const FName& Key, const FString& Value )
	{
		Text.AddTemplateValue(Key, Value);
		return Text;
	}
	
	UFUNCTION(BlueprintPure, Category = "ExpressiveText")
	static const FExpressiveTextFields& GetFields(UPARAM(ref) FExpressiveText& Text)
	{
		return Text.GetFields();
	}

	UFUNCTION(BlueprintPure, Category = "ExpressiveText")
	static void GetLineInformation(UPARAM(ref) FExpressiveText& Text, TArray<FExpressiveTextLineInformation>& OutLineInformation)
	{
		Text.GetLineInformation(OutLineInformation);
	}

	UFUNCTION(BlueprintPure, Category = "ExpressiveText")
	static FExpressiveTextLayoutInformation GetLayoutInformation(UPARAM(ref) FExpressiveText& Text)
	{
		return Text.GetLayoutInformation();
	}

	UFUNCTION(BlueprintPure, Category = "ExpressiveText")
	static FString GetWordAtIndex(UPARAM(ref) FExpressiveText& Text, int LineIndex, int CharacterIndex)
	{
		return Text.GetWordAtIndex(LineIndex, CharacterIndex);
	}

	UFUNCTION(BlueprintPure, Category = "ExpressiveText")
	static FString GetChararacterAtLocation(UPARAM(ref) FExpressiveText& Text, FVector2D Location, float Scaling = 1.f)
	{
		return Text.GetChararacterAtLocation(Location, Scaling);
	}

	UFUNCTION(BlueprintPure, Category = "ExpressiveText")
	static FString GetWordAtLocation(UPARAM(ref) FExpressiveText& Text, FVector2D Location, float Scaling = 1.f)
	{
		return Text.GetWordAtLocation(Location, Scaling);
	}

	UFUNCTION(BlueprintPure, Category = "ExpressiveText")
	static const FVector2D GetBoxSize(UPARAM(ref) FExpressiveText& Text)
	{
		return Text.GetSize();
	}

	UFUNCTION(BlueprintPure, Category = "ExpressiveText")
	static int32 GetCharacterCount(UPARAM(ref) FExpressiveText& Text)
	{
		return Text.GetCharacterCount();
	}

	UFUNCTION(BlueprintPure, Category = "ExpressiveText")
	static int32 GetNumRevealedCharacters(UPARAM(ref) FExpressiveText& Text)
	{
		return Text.GetNumRevealedCharacters();
	}

	UFUNCTION(BlueprintPure, Category = "ExpressiveText")
	static float GetRevealProgress(UPARAM(ref) FExpressiveText& Text)
	{
		return Text.GetRevealProgress();
	}


	UFUNCTION(BlueprintPure, Category = "ExpressiveText")
	static int64 CalcSelectorChecksum(UPARAM(ref) FExpressiveTextSelector& Selector)
	{
		return Selector.CalcChecksum();
	}
	UFUNCTION(BlueprintCallable, Category = "ExpressiveText")
	static void SetInjectedText(UPARAM(ref) FExpressiveTextSelector& Selector, UPARAM( ref) FExpressiveText& Text)
	{
		Selector.InjectedText = Text;
	}

	// Setters
#define EXPOSE_SETTER( Function ) { ExpressiveText.Function( Value ); }

	UFUNCTION(BlueprintCallable, Category = "ExpressiveText")
	static void SetFields(UPARAM(ref) FExpressiveText& ExpressiveText, FExpressiveTextFields Value)
	EXPOSE_SETTER( SetFields )

	UFUNCTION(BlueprintCallable, Category = "ExpressiveText")
	static void SetAsset(UPARAM(ref) FExpressiveText& ExpressiveText, UExpressiveTextAsset* Value)
	EXPOSE_SETTER( SetAsset )

	UFUNCTION(BlueprintCallable, Category = "ExpressiveText")
	static void SetDefaultStyle(UPARAM(ref) FExpressiveText& ExpressiveText, UExpressiveTextStyleBase* Value)
	EXPOSE_SETTER( SetDefaultStyle )

	UFUNCTION(BlueprintCallable, Category = "ExpressiveText")
	static void SetText(UPARAM(ref) FExpressiveText& ExpressiveText, const FText& Value)
	EXPOSE_SETTER( SetText )

	UFUNCTION(BlueprintCallable, Category = "ExpressiveText")
	static void SetAlignment(UPARAM(ref) FExpressiveText& ExpressiveText, const FExpressiveTextAlignment& Value)
	EXPOSE_SETTER( SetAlignment )

	UFUNCTION(BlueprintCallable, Category = "ExpressiveText")
	static void SetJustification(UPARAM(ref) FExpressiveText& ExpressiveText, TEnumAsByte<ETextJustify::Type> Value)
	EXPOSE_SETTER( SetJustification )

	UFUNCTION(BlueprintCallable, Category = "ExpressiveText")
	static void SetDefaultFontSize(UPARAM(ref) FExpressiveText& ExpressiveText, int32 Value)
	EXPOSE_SETTER( SetDefaultFontSize )

	UFUNCTION(BlueprintCallable, Category = "ExpressiveText")
	static void SetWrapSettings(UPARAM(ref) FExpressiveText& ExpressiveText, const FExpressiveTextWrapSettings& Value)
	EXPOSE_SETTER( SetWrapSettings )

	UFUNCTION(BlueprintCallable, Category = "ExpressiveText")
	static void DisableDefaultFontSize(UPARAM(ref) FExpressiveText& ExpressiveText)
	{
		ExpressiveText.DisableDefaultFontSize();
	}

#undef EXPOSE_SETTER

	// Builder function
#define EXPOSE_BUILDER( FunctionName ) { return ExpressiveText.FunctionName( Value ); }

	UFUNCTION(BlueprintPure, Category = "ExpressiveText" )
	static FExpressiveText& DefineDefaultStyle( UPARAM(ref) FExpressiveText& ExpressiveText, UExpressiveTextStyleBase* Value)
	EXPOSE_BUILDER( DefineDefaultStyle )

	UFUNCTION(BlueprintPure, Category = "ExpressiveText" )
	static FExpressiveText& DefineWorldContext(UPARAM(ref) FExpressiveText& ExpressiveText, UObject* Value )
	EXPOSE_BUILDER( DefineWorldContext )

	UFUNCTION(BlueprintPure, Category = "ExpressiveText")
	static FExpressiveText& DefineFields(UPARAM(ref) FExpressiveText& ExpressiveText, const FExpressiveTextFields& Value)
	EXPOSE_BUILDER( DefineFields )

	UFUNCTION(BlueprintPure, Category = "ExpressiveText")
	static FExpressiveText& DefineAsset(UPARAM(ref) FExpressiveText& ExpressiveText, UExpressiveTextAsset* Value)
	EXPOSE_BUILDER( DefineAsset )

	UFUNCTION(BlueprintPure, Category = "ExpressiveText")
	static FExpressiveText& DefineText(UPARAM(ref) FExpressiveText& ExpressiveText, FText Value)
	EXPOSE_BUILDER( DefineText )

	UFUNCTION(BlueprintPure, Category = "ExpressiveText")
	static FExpressiveText& DefineAlignment(UPARAM(ref) FExpressiveText& ExpressiveText, const FExpressiveTextAlignment& Value)
	EXPOSE_BUILDER( DefineAlignment )

	UFUNCTION(BlueprintPure, Category = "ExpressiveText")
	static FExpressiveText& DefineJustification(UPARAM(ref) FExpressiveText& ExpressiveText, TEnumAsByte<ETextJustify::Type> Value)
	EXPOSE_BUILDER( DefineJustification )

	UFUNCTION(BlueprintPure, Category = "ExpressiveText")
	static FExpressiveText& DefineDefaultFontSize(UPARAM(ref) FExpressiveText& ExpressiveText, int32 Value)
	EXPOSE_BUILDER( DefineDefaultFontSize )

	UFUNCTION(BlueprintPure, Category = "ExpressiveText")
	static FExpressiveText& DefineWrapSettings(UPARAM(ref) FExpressiveText& ExpressiveText, const FExpressiveTextWrapSettings Value)
	EXPOSE_BUILDER( DefineWrapSettings )

	UFUNCTION(BlueprintPure, Category = "ExpressiveText")
	static FExpressiveText Clone(const FExpressiveText& ExpressiveText)
	{
		return ExpressiveText.Clone();
	}

#undef EXPOSE_BUILDER
};
