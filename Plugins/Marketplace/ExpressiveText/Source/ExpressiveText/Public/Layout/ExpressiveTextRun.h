// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>

#include "Layout/ExTextSharedLayoutData.h"
#include "Framework/Text/IRun.h"
#include "Framework/Text/TextLayout.h"
#include "Framework/Text/ILayoutBlock.h"
#include "Framework/Text/SlateTextRun.h"
#include <Materials/MaterialInstanceDynamic.h>
#include "Extractions/TagsExtraction.h"
#include "Layout/ExpressiveTextAlignment.h"
#include "Layout/ExTextMIDCache.h"

class FExpressiveTextRun : public FSlateTextRun
{
	struct FInterjectionTracker
	{
		FInterjectionTracker(const TSharedRef< FExText_Interjection >& InInterjection, int32 InIndex, float InRelevantTime, float InRelevantTimeWithoutPauses, float InAccumulatedPauseTime )
			: Index( InIndex )
			, RelevantTime( InRelevantTime )
			, RelevantTimeWithoutPauses(InRelevantTimeWithoutPauses)
			, AccumulatedPauseTime( InAccumulatedPauseTime )
			, Interjection( InInterjection )
			, Fired( false )
		{}

		int32 Index;
		float RelevantTime;
		float RelevantTimeWithoutPauses;
		float AccumulatedPauseTime;
		TSharedRef< FExText_Interjection > Interjection;
		bool Fired;
	};

public:

	static TSharedRef< FExpressiveTextRun > Create( const FRunInfo& InRunInfo, const TSharedRef< const FString >& InText, const FTextBlockStyle& Style, const FTextRange& InRange, TSharedRef<FExTextSharedLayoutData> SharedData, float InRevealStartTime );

	void SetParameterLookup(TSharedPtr<FExpressiveTextParameterLookup> InLookup) { Lookup = InLookup; }
	void SetClearStartTime(float InTime) { ClearStartTime = InTime; }
	TSharedPtr<FExpressiveTextParameterLookup> GetLookup() const { return Lookup; }

	void SetOwnerExtraction(TSharedPtr<FExpressiveTextExtraction> InOwnerExtraction ){ OwnerExtraction = InOwnerExtraction; }
	TSharedPtr<FExpressiveTextExtraction> GetOwnerExtraction() const { return OwnerExtraction; }

	float CalculateDurationToFullyReveal() const;
	float CalculateDurationToFullyClear() const;

	int GetNumGlyphsRevealedInThisRun()
	{
		return NumGlyphsRevealedInThisRun;
	}
	
	void SetInterjections(const TArray<FExText_ParsedInterjection>& InInterjections);

	void SetWorld(UWorld* InWorld)
	{
		World = InWorld;
	}

	void FireInterjections(int32 SpecificIndex = -1) const;
	void ProcessInterjectionModifiers(struct FInterjectionOutput& Out, int32 SpecificIndex = -1) const;
	void FillMaterialRequest(const class UExpressiveTextMaterial& Material, int32 LayerIndex, struct FExTextMIDRequest& Request, const FGeometry& BlockGeom, int32 LineIndex, float FontSizeCompensation, FVector4 RevealAndClearInformation) const;


#if UE_VERSION_OLDER_THAN( 5, 0, 0 )
	virtual int32 OnPaint(const FPaintArgs& Args, const FTextLayout::FLineView& Line, const TSharedRef< ILayoutBlock >& Block, const FTextBlockStyle& DefaultStyle, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
#else
	virtual int32 OnPaint(const FPaintArgs& Args, const FTextArgs& TextArgs, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
#endif	

	static void ComputeEffectiveKernelSize(float Strength, int32& OutKernelSize, int32& OutDownsampleAmount, bool bDownsampleForBlur = false)
	{
		// If the radius isn't set, auto-compute it based on the strength
		if (OutKernelSize <= 0)
		{
			OutKernelSize = FMath::RoundToInt(Strength * 3.f);//BlurRadius.Get().Get(FMath::RoundToInt(Strength * 3.f));
		}

		// Downsample if needed
		if (bDownsampleForBlur && OutKernelSize > 9)
		{
			OutDownsampleAmount = OutKernelSize >= 64 ? 4 : 2;
			OutKernelSize /= OutDownsampleAmount;
		}

		// Kernel sizes must be odd
		if (OutKernelSize % 2 == 0)
		{
			++OutKernelSize;
		}

		OutKernelSize = FMath::Clamp(OutKernelSize, 3, 255);
	}

	static void BlurLayer( FSlateWindowElementList& OutDrawElements, int32& LayerId, const FGeometry& Geometry, float Strength );

	virtual ~FExpressiveTextRun() {}

	void SetAutoFontSizeScale(float NewFontSize)
	{
		AutoFontSize = NewFontSize;
		Style.SetFontSize(NewFontSize);
	}
private:
	TSharedPtr<FExpressiveTextParameterLookup> Lookup;
	TSharedPtr<FExpressiveTextExtraction> OwnerExtraction;
	TSharedPtr<FExTextSharedLayoutData> SharedData;
	TWeakObjectPtr<UWorld> World;
	float RevealStartTime;
	float ClearStartTime;
	float FontSize;
	float AutoFontSize;
	mutable TSharedPtr<FExTextMID> CachedMID;
	mutable TSharedPtr<FExTextMID> CachedOutlineMID;
	mutable TArray< FInterjectionTracker > Interjections;
	mutable int32 LastRevealCharacterIndex;
	mutable int NumGlyphsRevealedInThisRun;
public:

	const FTextBlockStyle& GetStyle() const { return Style;  }
protected:
	FExpressiveTextRun(const FRunInfo& InRunInfo, const TSharedRef< const FString >& InText, const FTextBlockStyle& InStyle, const FTextRange& InRange, TSharedRef<FExTextSharedLayoutData> InSharedData, float InRevealStartTime )
		: FSlateTextRun(InRunInfo, InText, InStyle, InRange)
		, SharedData( InSharedData )
		, World()
		, RevealStartTime( InRevealStartTime )
		, ClearStartTime( -1.f )
		, FontSize( InStyle.Font.Size )
		, AutoFontSize( 1.f )
		, Interjections()
		, LastRevealCharacterIndex( -1000 )
		, NumGlyphsRevealedInThisRun( 0 )
	{
	}

	FExpressiveTextRun(const FSlateTextRun& Run)
		: FSlateTextRun(Run)
	{}


};
