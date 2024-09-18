// Copyright 2022 Guganana. All Rights Reserved.
#include "Handles/ExpressiveText.h"

#include "Asset/ExpressiveTextAsset.h"
#include "Layout/ExpressiveTextLayout.h"
#include "Styles/ExpressiveTextDefaultStyle.h"

void FExpressiveTextInternal::AddReferencedObjects(FReferenceCollector& Collector)
{
#if UE_VERSION_OLDER_THAN( 5, 4, 0 )
	Collector.AddReferencedObject(Fields.Actions);
	Collector.AddReferencedObject(Fields.DefaultStyle);
	Collector.AddReferencedObjects(Fields.ReferencedResources);
#else
	// Use TObjectPtr tracking so it is supported by incremental GC
	Collector.AddReferencedObjects(KeepAliveReferences);
#endif
}

void FExpressiveTextInternal::RecollectReferences()
{
#if !UE_VERSION_OLDER_THAN( 5, 4, 0 )
	KeepAliveReferences.Empty();
	KeepAliveReferences.Append(Fields.ReferencedResources);
	KeepAliveReferences.Add(Fields.Actions);
	KeepAliveReferences.Add(Fields.DefaultStyle);
#endif
}

int64 FExpressiveTextInternal::CalcChecksum() const
{
	int64 Result = 0;
	Result = HashCombine(Result, Fields.CalcChecksum());
	return Result;
}

UExpressiveTextStyleBase* FExpressiveTextInternal::GetDefaultStyle() const
{
	return Fields.DefaultStyle;
}

void FExpressiveTextInternal::SetDefaultStyle(UExpressiveTextStyleBase* InStyle)
{
	Fields.DefaultStyle = InStyle;
}

void FExpressiveTextInternal::SetActions(UExpressiveTextAssetActions* Actions)
{
	Fields.Actions = Actions;
	RecollectReferences();
}


FExpressiveText::FExpressiveText() 
	: Internal(MakeShareable(new FExpressiveTextInternal))
{

}

void FExpressiveText::SetWorldContext(UObject* WorldContext)
{
	Internal->WorldContext = WorldContext;
}

void FExpressiveText::SetFields(const FExpressiveTextFields& Fields)
{
	Internal->SetFields(Fields);
	SetContext(Fields.CreateContext());
}

void FExpressiveText::SetAsset(UExpressiveTextAsset* Asset)
{
	if (Asset)
	{
		SetAsset(*Asset);
	}
}


void FExpressiveText::SetAsset(UExpressiveTextAsset& Asset)
{
	SetFields(Asset.Fields);
}

void FExpressiveText::AddTemplateValue(const FName& Key, const FString& Value)
{
	Internal->TemplateDictionary.Add(Key, Value);
}

void FExpressiveText::SetDefaultStyle(UExpressiveTextStyleBase* Style)
{
	Internal->SetDefaultStyle(Style);
}

UObject* FExpressiveText::GetWorldContext() const
{
	return Internal->WorldContext.Get();
}

const FExpressiveTextFields& FExpressiveText::GetFields() const
{
	return Internal->GetFields();
}

int FExpressiveText::GetCharacterCount() const
{
	return Internal->TextLayout->GetTextTotalLength();
}

int FExpressiveText::GetNumRevealedCharacters() const
{
	return Internal->TextLayout->GetTotalGlyphsRevealed();
}

float FExpressiveText::GetRevealProgress() const
{
	int CharacterCount = GetCharacterCount();
	return CharacterCount > 0 ? ((float)GetNumRevealedCharacters()) / CharacterCount : 0;
}


UExpressiveTextStyleBase* FExpressiveText::GetDefaultStyle() const
{
	return Internal->GetDefaultStyle();
}

TSharedRef<FExpressiveTextSlateLayout> FExpressiveText::GetTextLayout() const
{
	return Internal->TextLayout;
}

int64 FExpressiveText::CalcChecksum() const
{
	return Internal->CalcChecksum();
}

void FExpressiveText::SetCompiledText(const FCompiledExpressiveText& CompiledText)
{
	Internal->CompiledText = CompiledText;
}

void FExpressiveText::SetContext(const FExpressiveTextContext& InContext)
{
	Internal->Context = InContext;
}

void FExpressiveText::SetText(const FText& Text)
{
	Internal->SetText(Text);
}

void FExpressiveText::SetAlignment(const FExpressiveTextAlignment& Alignment)
{
	Internal->SetAlignment(Alignment);
}

void FExpressiveText::SetJustification(TEnumAsByte<ETextJustify::Type> Justification)
{
	Internal->SetJustification(Justification);
}

void FExpressiveText::SetDefaultFontSize(int32 DefaultFontSize)
{
	Internal->SetDefaultFontSize(DefaultFontSize);
	Internal->SetUseDefaultFontSize(true);
}

void FExpressiveText::SetWrapSettings(const FExpressiveTextWrapSettings& WrapSettings)
{
	Internal->SetWrapSettings(WrapSettings);
}

void FExpressiveText::DisableDefaultFontSize()
{
	Internal->SetDefaultFontSize(false);
}

FExpressiveText FExpressiveText::Clone() const
{
	FExpressiveText NewClone;
	Internal->CloneTo(NewClone.Internal.Get());
	return NewClone;
}


void FExpressiveText::SetTextLayout(TSharedRef<FExpressiveTextSlateLayout> InTextLayout)
{
	Internal->TextLayout = InTextLayout;
}

const FExpressiveTextContext& FExpressiveText::GetContext() const
{
	return Internal->Context;
}

TOptional<int32> FExpressiveText::GetDefaultFontSize() const
{
	const auto& Fields = Internal->GetFields();
	if( Fields.UseDefaultFontSize )
	{
		return TOptional<int32>(Fields.DefaultFontSize);
	}
	
	return TOptional<int32>();
}

FVector2D FExpressiveText::GetSize() const
{
	if (Internal->CompiledText.IsSet())
	{
		return Internal->CompiledText.GetValue().DrawSize;
	}

	return FVector2D::ZeroVector;
}

void FExpressiveText::GetLineSizes( TArray<FVector2D>& OutLineSizes ) const
{
	for ( const FTextLayout::FLineView& Line : Internal->TextLayout->GetLineViews())
	{
		OutLineSizes.Add(Line.Size);
	}
}

#define DEFINE_BUILDER(Type, Member) \
FExpressiveText& FExpressiveText::Define##Member( Type Value ) \
{ \
	Set##Member( Value ); \
	return *this; \
}

DEFINE_BUILDER(const FText&, Text)
DEFINE_BUILDER(const FExpressiveTextFields&, Fields)
DEFINE_BUILDER(UExpressiveTextAsset*, Asset)
DEFINE_BUILDER(UObject*, WorldContext)
DEFINE_BUILDER(UExpressiveTextStyleBase*, DefaultStyle)
DEFINE_BUILDER(const FExpressiveTextAlignment&, Alignment)
DEFINE_BUILDER(TEnumAsByte<ETextJustify::Type>, Justification)
DEFINE_BUILDER(int32, DefaultFontSize)
DEFINE_BUILDER(const FExpressiveTextWrapSettings&, WrapSettings)

#undef DEFINE_BUILDER