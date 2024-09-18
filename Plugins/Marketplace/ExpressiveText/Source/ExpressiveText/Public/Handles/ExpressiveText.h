// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>

#include "Asset/ExpressiveTextFields.h"
#include "Compiled/CompiledExpressiveText.h"
#include "ExpressiveTextLineInformation.h"
#include "Layout/ExpressiveTextLayout.h"
#include "Framework/Text/TextHitPoint.h"

#include "ExpressiveText.generated.h"

class UExpressiveTextStyleBase;
class UExpressiveTextAsset;

struct FExpressiveTextInternal : public FGCObject
{
	FExpressiveTextInternal()
		: WorldContext()
		, TemplateDictionary()
		, CompiledText()
		, Context()
		, TextLayout( MakeShareable(new FExpressiveTextSlateLayout) )
		, Fields()
#if !UE_VERSION_OLDER_THAN( 5, 4, 0 )
		, KeepAliveReferences()
#endif
	{}
	
	//! Make sure to AddReferencedObjects on Field parameters	
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual FString GetReferencerName() const override
	{
		return TEXT("ExpressiveText::FExpressiveTextInternal");
	}

	void CloneTo(FExpressiveTextInternal& Target)
	{
		// !Important - always update to reflect all the member variables
		Target.WorldContext = WorldContext;
		Target.TemplateDictionary = TemplateDictionary;
		Target.Fields = Fields;
		Target.CompiledText = CompiledText;
		Target.Context = Context;
		Target.TextLayout = TextLayout;

		RecollectReferences();
	}


	int64 CalcChecksum() const;
	UExpressiveTextStyleBase* GetDefaultStyle() const;
	const FExpressiveTextFields& GetFields() const { return Fields;  }

	void SetDefaultStyle(UExpressiveTextStyleBase* InStyle);
	void SetText(const FText& Text) { Fields.Text = Text;  }
	void SetDefaultFontSize(int FontSize) { Fields.DefaultFontSize = FontSize; }
	void SetUseDefaultFontSize(bool UseDefaultFontSize) { Fields.UseDefaultFontSize = UseDefaultFontSize; }
	void SetWrapSettings(const FExpressiveTextWrapSettings& WrapSettings) { Fields.WrapSettings = WrapSettings; }
	void SetJustification(ETextJustify::Type Justification) { Fields.Justification = Justification; }
	void SetAlignment(const FExpressiveTextAlignment& Alignment) { Fields.Alignment = Alignment; }
	void SetActions(UExpressiveTextAssetActions* Actions);

	void SetFields(const FExpressiveTextFields& InFields) 
	{ 
		Fields = InFields; 
		RecollectReferences(); 
	}

	void RecollectReferences();

	TWeakObjectPtr<UObject> WorldContext;
	TMap<FName, FString> TemplateDictionary;
	TOptional<FCompiledExpressiveText> CompiledText;
	FExpressiveTextContext Context;
	TSharedRef<FExpressiveTextSlateLayout> TextLayout;

private:
	FExpressiveTextFields Fields;


#if !UE_VERSION_OLDER_THAN( 5, 4, 0 )
	TArray<TObjectPtr<UObject>> KeepAliveReferences;
#endif
};

USTRUCT( BlueprintType )
struct EXPRESSIVETEXT_API FExpressiveText
{
	GENERATED_BODY()

	FExpressiveText();

	// Setters
	void SetWorldContext( UObject* WorldContext );
	void SetFields( const FExpressiveTextFields& Fields );
	void SetAsset(UExpressiveTextAsset* Asset);
	void SetAsset(UExpressiveTextAsset& Asset);
	void AddTemplateValue( const FName& Key, const FString& Value );
	void SetDefaultStyle( UExpressiveTextStyleBase* Style );
	void SetCompiledText(const FCompiledExpressiveText& CompiledText);
	void SetContext(const FExpressiveTextContext& InContext);
	void SetText(const FText& Text);
	void SetAlignment(const FExpressiveTextAlignment& Alignment);
	void SetJustification(TEnumAsByte<ETextJustify::Type> Justification);
	void SetDefaultFontSize(int32 DefaultFontSize);
	void SetWrapSettings(const FExpressiveTextWrapSettings& WrapSettings);
	void DisableDefaultFontSize();

	FExpressiveText Clone() const;

	// Builders
	FExpressiveText& DefineText(const FText& Value);
	FExpressiveText& DefineFields(const FExpressiveTextFields& Value);
	FExpressiveText& DefineAsset(UExpressiveTextAsset* Value);
	FExpressiveText& DefineWorldContext(UObject* Value);
	FExpressiveText& DefineDefaultStyle(UExpressiveTextStyleBase* Value);
	FExpressiveText& DefineAlignment(const FExpressiveTextAlignment& Value);
	FExpressiveText& DefineJustification(TEnumAsByte<ETextJustify::Type> Value);
	FExpressiveText& DefineDefaultFontSize(int32 Value);
	FExpressiveText& DefineWrapSettings(const FExpressiveTextWrapSettings& Value);

	// Getters
	UObject* GetWorldContext() const;
	const FExpressiveTextFields& GetFields() const;
	UExpressiveTextStyleBase* GetDefaultStyle() const;
	TSharedRef<FExpressiveTextSlateLayout> GetTextLayout() const;
	const FExpressiveTextContext& GetContext() const;
	TOptional<int32> GetDefaultFontSize() const;
	FVector2D GetSize() const;

	int GetCharacterCount() const;
	int GetNumRevealedCharacters() const;
	float GetRevealProgress() const;

	void GetLineSizes( TArray<FVector2D>& OutLineSizes ) const;
	int GetLineCount()
	{
		return Internal->TextLayout->GetLineCount();
	}
	
	void GetLineInformation( TArray<FExpressiveTextLineInformation>& Lines) const
	{
		const TArray<FTextLayout::FLineView>& LineViews = Internal->TextLayout->GetLineViews();

		Lines.Empty( LineViews.Num() );

		for (int i = 0; i < LineViews.Num(); i++)
		{
			const auto& LineView = LineViews[i];
			auto& LineInfo = Lines.AddDefaulted_GetRef();
			LineInfo.JustificationWidth = LineView.JustificationWidth;
			LineInfo.Offset = LineView.Offset;
			LineInfo.Range = LineView.Range;
			LineInfo.Size = LineView.Size;
			LineInfo.TextHeight = LineView.TextHeight;
		}
	}
	
	FExpressiveTextLayoutInformation GetLayoutInformation() const
	{
		FExpressiveTextLayoutInformation Information;
		Information.DrawSize = Internal->TextLayout->GetSize();
		Information.WrappedDrawSize = Internal->TextLayout->GetWrappedDrawSize();
		return Information;
	}

	FString GetWordAtIndex(int LineIndex, int CharacterIndex )
	{
		FString Result;
		FTextLocation Location(LineIndex, CharacterIndex);
		FTextSelection Selection = Internal->TextLayout->GetWordAt(Location);

		if (Selection.GetEnd().IsValid() && Selection.GetBeginning().IsValid())
		{
			Internal->TextLayout->GetSelectionAsText(Result, Selection);
		}

		return Result;
	}

	FString GetChararacterAtLocation(FVector2D Position, float Scaling)
	{
		FVector2D Offset = Internal->TextLayout->GetSharedData()->AlignmentOffset;
		Position -= Offset;
		Position *= Scaling * Internal->TextLayout->GetScale();

		FString Result;
		ETextHitPoint HitPoint = ETextHitPoint::LeftGutter;
		FTextLocation Location = Internal->TextLayout->GetTextLocationAt(Position, &HitPoint);

		if (HitPoint == ETextHitPoint::WithinText)
		{
			FTextSelection Selection = Internal->TextLayout->GetGraphemeAt(Location);
			if (Selection.GetEnd().IsValid() && Selection.GetBeginning().IsValid())
			{			
				Internal->TextLayout->GetSelectionAsText(Result, Selection);
			}
		}

		return Result;
	}

	FString GetWordAtLocation(FVector2D Position, float Scaling)
	{
		FVector2D Offset = Internal->TextLayout->GetSharedData()->AlignmentOffset;
		Position -= Offset;
		Position *= Scaling * Internal->TextLayout->GetScale();

		FString Result;
		ETextHitPoint HitPoint = ETextHitPoint::LeftGutter;
		FTextLocation Location = Internal->TextLayout->GetTextLocationAt(Position, &HitPoint);

		if (HitPoint == ETextHitPoint::WithinText)
		{
			FTextSelection Selection = Internal->TextLayout->GetWordAt(Location);
			if (Selection.GetEnd().IsValid() && Selection.GetBeginning().IsValid())
			{
				Internal->TextLayout->GetSelectionAsText(Result, Selection);
			}
		}

		return Result;
	}

	int64 CalcChecksum() const;
	void SetTextLayout(TSharedRef<FExpressiveTextSlateLayout> InTextLayout);
	
private:
	TSharedRef<FExpressiveTextInternal> Internal;
};