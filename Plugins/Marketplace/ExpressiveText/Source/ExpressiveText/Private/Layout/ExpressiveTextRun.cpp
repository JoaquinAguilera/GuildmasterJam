// Copyright 2022 Guganana. All Rights Reserved.
#include "Layout/ExpressiveTextRun.h"

#include "Animation/ExpressiveTextAnimation.h"
#include "Fonts/FontMeasure.h"
#include "Fonts/ShapedTextFwd.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Text/DefaultLayoutBlock.h"
#include "Framework/Text/RunUtils.h"
#include "Framework/Text/ShapedTextCache.h"
#include "Interjections/ExText_Interjection.h"
#include "Parameters/ExpressiveTextParams.h"
#include "Subsystems/ExpressiveTextSubsystem.h"

#include <Fonts/CompositeFont.h>
#include <Fonts/FontCache.h>
#include <Materials/MaterialInstanceConstant.h>
#include <Misc/EngineVersionComparison.h>
#include <Slate/WidgetTransform.h>

TSharedRef< FExpressiveTextRun > FExpressiveTextRun::Create( const FRunInfo& InRunInfo, const TSharedRef< const FString >& InText, const FTextBlockStyle& Style, const FTextRange& InRange, TSharedRef<FExTextSharedLayoutData> SharedData, float InRevealStartTime )
{
	TSharedRef<FExpressiveTextRun> Run = MakeShareable( new FExpressiveTextRun( InRunInfo, InText, Style, InRange, SharedData, InRevealStartTime) );
	return Run;
}

#if UE_VERSION_OLDER_THAN( 5, 0, 0 )	
int32 FExpressiveTextRun::OnPaint(const FPaintArgs& Args, const FTextLayout::FLineView& Line, const TSharedRef< ILayoutBlock >& Block, const FTextBlockStyle& DefaultStyle, const FGeometry& InitialAllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
#else
int32 FExpressiveTextRun::OnPaint(const FPaintArgs& Args, const FTextArgs& TextArgs, const FGeometry& InitialAllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
#endif
{
		
#if !UE_VERSION_OLDER_THAN( 5, 0, 0 )
	const FTextLayout::FLineView& Line = TextArgs.Line;
	const TSharedRef< ILayoutBlock >& Block = TextArgs.Block;
#endif

	const float OriginalInverseScale = Inverse(InitialAllottedGeometry.Scale);

	static const float TypicalFontSize = 100.f;
	const float FontSizeCompensation = static_cast<float>(Lookup->GetValue<UExTextValue_FontSize>()) / TypicalFontSize;
	check(SharedData);

	FVector2D BlockSize = Block->GetSize();
	FVector2D BlockOffset = Block->GetLocationOffset() / InitialAllottedGeometry.GetAccumulatedLayoutTransform().GetScale();
	
	const FLayoutBlockTextContext BlockTextContext = Block->GetTextContext();

	float UnmodifiedTimePassed =  SharedData->Chronos.GetTimePassed() - RevealStartTime;
	if (UnmodifiedTimePassed < 0.f)
	{
		return LayerId;
	}
	
	UnmodifiedTimePassed = FMath::Max(0.f, UnmodifiedTimePassed);

	// process pauses from interjections
	FInterjectionTracker* FoundInterjectionTracker = nullptr;
	for (int i = Interjections.Num() - 1; i >= 0; i--)
	{
		if (Interjections[i].RelevantTime <= UnmodifiedTimePassed)
		{
			FoundInterjectionTracker = &Interjections[i];
			break;
		}
		
	}

	const float TimePassed = FoundInterjectionTracker ? 
		FMath::Max(FoundInterjectionTracker->RelevantTimeWithoutPauses, UnmodifiedTimePassed - FoundInterjectionTracker->AccumulatedPauseTime) :
		UnmodifiedTimePassed;

	const float RevealRate = Lookup->GetValue<UExTextValue_RevealRate>();

	const float ClearTimer = Lookup->GetValue<UExTextValue_ClearTimer>();
	const float ClearTimePassed = ClearTimer >= 0.f ? SharedData->Chronos.GetTimePassed() - ClearStartTime : -1.f;

	const bool IsClearing = ClearTimePassed >= 0.f;

	const ESlateDrawEffect DrawEffects = bParentEnabled ? 
		static_cast<ESlateDrawEffect>(Lookup->GetValue< UExTextValue_DrawBlendEffect >()) : 
		ESlateDrawEffect::DisabledEffect;

	UExpressiveTextFont* Font = Lookup->GetValue<UExTextValue_Font>();
	FSlateFontInfo FontInfo = Style.Font;
	FontInfo.FontObject = Font ? Font->GetObject() : nullptr;
	FontInfo.OutlineSettings.OutlineColor = Lookup->GetValue<UExTextValue_OutlineColor>();
	FontInfo.OutlineSettings.OutlineSize = FMath::CeilToInt(Lookup->GetValue<UExTextValue_OutlineSize>() * FontSizeCompensation);
	FontInfo.OutlineSettings.bApplyOutlineToDropShadows = Lookup->GetValue<UExTextValue_OutlineOnDropShadows>();

	FName TypefaceToUse = Lookup->GetValue<UExTextValue_Typeface>();
	// Use default typeface if none is specified
	if (TypefaceToUse.IsNone())
	{
		if (auto* FontObj = Cast<UFont>(FontInfo.FontObject))
		{
			const auto& Fonts = FontObj->CompositeFont.DefaultTypeface.Fonts;
			if (Fonts.Num() > 0)
			{
				TypefaceToUse = Fonts[0].Name;
			}
		}
	}

	FontInfo.TypefaceFontName = TypefaceToUse;

	// Sanitize font info to not have absurd values
	FontInfo.Size = FMath::Min( FontInfo.Size, 2000);

	// Animation
	FExTextAnimationPayload Payload;
	Payload.FontColor = Lookup->GetValue<UExTextValue_FontColor>();
	Payload.DropShadowColor = Lookup->GetValue<UExTextValue_ShadowColor>();
	Payload.DropShadowOffset = Lookup->GetValue<UExTextValue_ShadowOffset>() * FontSizeCompensation;
	Payload.BackgroundBlurAmount = Lookup->GetValue<UExTextValue_OutlineBlurAmount>() * FontSizeCompensation;

	const auto& GlyphAnim = Lookup->GetValue<UExTextValue_RevealAnimation>();
	const auto& ClearGlyphAnim = Lookup->GetValue<UExTextValue_ClearAnimation>();
	const float ClearAnimDuration = Lookup->GetValue<UExTextValue_ClearAnimationDuration>();
	const float AnimLoopPeriod = Lookup->GetValue<UExTextValue_AnimationLoopPeriod>();

	if (IsClearing)
	{
		if (ClearGlyphAnim.Animation)
		{
			ClearGlyphAnim.Animation->Evaluate( BlockSize, ClearTimePassed, ClearAnimDuration, Payload, FontInfo, FontSizeCompensation, ClearGlyphAnim.Reverse);
		}

	}
	else
	{
		if (GlyphAnim.Animation)
		{
			const float AnimationDuration = Lookup->GetValue<UExTextValue_AnimationDuration>();
			
			// only start animation when character is actually revealed
			float AnimationTime = RevealRate > 0.f ? TimePassed - (1.f / RevealRate) : TimePassed;
			AnimationTime = AnimLoopPeriod > 0.f ? FMath::Fmod(AnimationTime, AnimLoopPeriod) : AnimationTime;
			GlyphAnim.Animation->Evaluate(BlockSize, AnimationTime, AnimationDuration, Payload, FontInfo, FontSizeCompensation, GlyphAnim.Reverse);
		}
	}


	BlockSize *= Payload.Scale;

	float FontAtlasScale = Payload.MaxDisplayedScale * InitialAllottedGeometry.GetAccumulatedLayoutTransform().GetScale();


	// Reveal logic
	FTextRange BlockRange = Block->GetTextRange();
	const float ClearRate = Lookup->GetValue< UExTextValue_ClearRate >();


	const int32 NumGlyphsToReveal = FMath::CeilToInt(TimePassed * RevealRate); // Ceil to immediately show first character
	const int32 RunRelativeStart = BlockRange.BeginIndex - Range.BeginIndex;
	const int32 NumGlyphsToRevealInThisBlock = FMath::Min(NumGlyphsToReveal - RunRelativeStart, BlockRange.Len());
	
	NumGlyphsRevealedInThisRun = FMath::Min(NumGlyphsToReveal, Range.Len());

	const auto ModulateReveal = [this,&RevealRate, &BlockRange, &NumGlyphsToRevealInThisBlock, &NumGlyphsToReveal](bool& ShouldReturn) {
		if (RevealRate > 0.f)
		{

			if (NumGlyphsToRevealInThisBlock <= 0)
			{
				ShouldReturn = true;
				return;
			}

			FireInterjections(BlockRange.BeginIndex + NumGlyphsToRevealInThisBlock);
			BlockRange.EndIndex = BlockRange.BeginIndex + NumGlyphsToRevealInThisBlock;


			if (UWorld* RawWorld = World.Get())
			{
				if (RawWorld->IsGameWorld())
				{

					if (NumGlyphsRevealedInThisRun > 0 && NumGlyphsRevealedInThisRun > LastRevealCharacterIndex )
					{
						if (auto* PerCharAction = Lookup->GetValue<UExTextValue_PerCharacterAction>())
						{
							PerCharAction->SetWorld(RawWorld);
							IExText_ActionInterface::Execute_Run(PerCharAction);
						}
						LastRevealCharacterIndex = NumGlyphsRevealedInThisRun;
					}
				}
			}
		}
	};

	const auto ModulateClear = [&](bool& ShouldReturn) {
		if (ClearTimer >= 0.f)
		{
			const float ClearRate = Lookup->GetValue<UExTextValue_ClearRate>();
			const auto ClearDirection = Lookup->GetValue<UExTextValue_ClearDirection>();

			const int32 NumGlyphsToHide = FMath::FloorToInt((ClearTimePassed - ClearAnimDuration) * ClearRate);
			const int32 Bias = ClearDirection == EExText_ClearDirection::Forwards ?
				Range.BeginIndex - BlockRange.BeginIndex :
				BlockRange.EndIndex - Range.EndIndex;
			const int32 NumGlyphsToHideInThisBlock = FMath::Max(NumGlyphsToHide + Bias, 0);

			if (NumGlyphsToHideInThisBlock > 0)
			{
				if (NumGlyphsToHideInThisBlock >= BlockRange.Len())
				{
					ShouldReturn = true;
					return;
				}


				if (ClearDirection == EExText_ClearDirection::Forwards)
				{
					FTextRange HiddenRange(BlockRange.BeginIndex, BlockRange.BeginIndex + NumGlyphsToHideInThisBlock);

					auto HiddenShapedText = ShapedTextCacheUtil::GetShapedTextSubSequence(
						BlockTextContext.ShapedTextCache,
						FCachedShapedTextKey(Line.Range, FontAtlasScale, BlockTextContext, FontInfo),
						HiddenRange,
						**Text,
						BlockTextContext.TextDirection
					);
					BlockOffset.X += HiddenShapedText->GetMeasuredWidth() / InitialAllottedGeometry.GetAccumulatedLayoutTransform().GetScale();
					BlockRange.BeginIndex = BlockRange.BeginIndex + NumGlyphsToHideInThisBlock;
				}
				else
				{
					BlockRange.EndIndex = BlockRange.EndIndex - NumGlyphsToHideInThisBlock;
				}
			}
		}
	};

	if (RevealRate <= 0.f)
	{
		if (FoundInterjectionTracker)
		{
			if (TimePassed <= FoundInterjectionTracker->RelevantTimeWithoutPauses)
			{
				BlockRange.EndIndex = FMath::Max(BlockRange.BeginIndex, FoundInterjectionTracker->Index);
			}
		}
		FireInterjections();
	}

	bool ShouldReturn = false;
	if (IsClearing)
	{
		ModulateClear(ShouldReturn);
	}
	else
	{
		ModulateReveal(ShouldReturn);
	}

	if (ShouldReturn)
	{
		return LayerId;
	}

	const bool ShouldDropShadow = Payload.DropShadowColor.A > 0.f && Payload.DropShadowOffset.SizeSquared() > 0.f;
	const FVector2D BlockLocationOffset = Block->GetLocationOffset();


	const float InverseScale = 1.f; //Inverse(InitialAllottedGeometry.Scale);
	const float DividedScale = (Payload.Scale) / Payload.MaxDisplayedScale;
	const float InverseDividedScale = 1.f / DividedScale;

	// A negative shadow offset should be applied as a positive offset to the text to avoid clipping issues
	const FVector2D DrawShadowOffset(
		(Payload.DropShadowOffset.X > 0.0f) ? Payload.DropShadowOffset.X * Payload.Scale * InverseDividedScale : 0.0f,
		(Payload.DropShadowOffset.Y > 0.0f) ? Payload.DropShadowOffset.Y * Payload.Scale * InverseDividedScale : 0.0f
	);
	const FVector2D DrawTextOffset(
		(Payload.DropShadowOffset.X < 0.0f) ? -Payload.DropShadowOffset.X * Payload.Scale * InverseDividedScale : 0.0f,
		(Payload.DropShadowOffset.Y < 0.0f) ? -Payload.DropShadowOffset.Y * Payload.Scale * InverseDividedScale : 0.0f
	);

	const FVector2D Size = Block->GetSize() * OriginalInverseScale;
	const FVector2D OriginalPivot = Size * 0.5f; 
	const FVector2D NewPivot = Size * Payload.Scale * 0.5f;
	const FVector2D ScalingOffset = (OriginalPivot - NewPivot);
	const FVector2D DesiredOffset = (SharedData->AlignmentOffset) + ScalingOffset;

	FSlateLayoutTransform Transform(1.f, DesiredOffset);
	FSlateRenderTransform ScaledDownTransform(DividedScale);

	FGeometry AllottedGeometry = InitialAllottedGeometry.MakeChild(InitialAllottedGeometry.GetLocalSize(), Transform, ScaledDownTransform, FVector2D(0.f, 0.f));

	FVector2D PercentageOffset = Lookup->GetValue<UExTextValue_PercentageOffset>();
	if( PercentageOffset != FVector2D::ZeroVector )
	{
		AllottedGeometry = AllottedGeometry.MakeChild( BlockSize * PercentageOffset );
	}


	if (Payload.LetterSpacing != 0)
	{
		// Because text shapping has already happened, we need to offset the block position so it is centered
		FontInfo.LetterSpacing += Payload.LetterSpacing;
		Payload.WidgetTransform.Translation.X -= (Payload.LetterSpacing * (FontInfo.Size/1000.f) ) * (Range.Len()-1) * 0.5f;
	}

	FSlateRenderTransform RenderTransform = Payload.WidgetTransform.ToSlateRenderTransform();

	FGeometry BaseGeom = AllottedGeometry.MakeChild( 
		TransformVector(Payload.MaxDisplayedScale * InverseScale, BlockSize),
		FSlateLayoutTransform(TransformPoint(Payload.MaxDisplayedScale * InverseScale, BlockOffset)),
		RenderTransform, 
		Payload.Pivot * OriginalInverseScale
	);

	FGeometry TextGeom = BaseGeom.MakeChild(DrawTextOffset - FVector2D( FontInfo.OutlineSettings.OutlineSize, 0.f ) );

	if (Payload.ShouldClip)
	{
		const FVector2D TextSize = TextGeom.GetLocalSize() * OriginalInverseScale;
		const FVector2D Translation = Payload.ClipAmount * TextSize;
		const FGeometry ClipGeometry = BaseGeom.MakeChild(TextSize, FSlateLayoutTransform(Translation - RenderTransform.GetTranslation()));
		OutDrawElements.PushClip(FSlateClippingZone(ClipGeometry));
	}
	 
	FVector4 RevealAndClearInformation(
		RevealRate <= 0.f ? MAX_FLT : RevealRate,
		SharedData->Chronos.GetStartTime() + RevealStartTime,
		ClearRate <= 0.f ? MAX_FLT : ClearRate,
		SharedData->Chronos.GetStartTime() + RevealStartTime + ClearTimer
	);

	const auto FetchCachedMaterial = [this,&RevealAndClearInformation, &TextGeom,&Line,&FontSizeCompensation](const UExTextValue_MaterialBase& ExTextMaterialParam)
	{
		if (UMaterialInstanceConstant* CombinedMaterial = ExTextMaterialParam.CombinedMaterial)
		{
			FExTextMIDRequest Request(*CombinedMaterial);

			TArray<UExpressiveTextMaterial*> Materials;
			ExTextMaterialParam.GetMaterials(Materials);

			for (int i = 0; i < Materials.Num(); i++)
			{
				const auto* Material = Materials[i];
				const int32 LayerIndex = i + 1; // There's always a base layers that we should not account for here
				if (Material)
				{
					FillMaterialRequest(*Material, LayerIndex, Request, TextGeom, Line.ModelIndex, FontSizeCompensation, RevealAndClearInformation);
				}
			}

			return GEngine->GetEngineSubsystem<UExpressiveTextSubsystem>()->RequestMID(Request);
		}

		return TSharedPtr<FExTextMID>();
	};

	if (!CachedMID)
	{
		CachedMID = FetchCachedMaterial(Lookup->GetValueObject<UExTextValue_Material>());
	}

	if (!CachedOutlineMID)
	{
		CachedOutlineMID = FetchCachedMaterial(Lookup->GetValueObject<UExTextValue_OutlineMaterial>());
	}

	if (CachedMID && CachedMID->MID)
	{
		FontInfo.FontMaterial = CachedMID->GetMIDAndConfirmUsage();
	}

	if (CachedOutlineMID && CachedOutlineMID->MID)
	{
		FontInfo.OutlineSettings.OutlineMaterial = CachedOutlineMID->GetMIDAndConfirmUsage();
	}

#if false
	static uint32 DebugLayer = 20000;

	FSlateDrawElement::MakeBox(
		OutDrawElements,
		DebugLayer,
		InitialAllottedGeometry.ToPaintGeometry(FVector2D(40.f, 40.f), FSlateLayoutTransform(OriginalPivot)),
		FCoreStyle::Get().GetBrush(TEXT("Icons.Cross")),
		DrawEffects,
		FLinearColor::White);

	FSlateDrawElement::MakeBox(
		OutDrawElements,
		DebugLayer,
		AllottedGeometry.ToPaintGeometry(),
		FCoreStyle::Get().GetBrush(TEXT("Border")),
		DrawEffects,
		FLinearColor::Blue);

	FSlateDrawElement::MakeBox(
		OutDrawElements,
		DebugLayer,
		InitialAllottedGeometry.ToPaintGeometry(),
		FCoreStyle::Get().GetBrush(TEXT("Border")),
		DrawEffects,
		FLinearColor::Red);
#endif

	// Make sure we have up-to-date shaped text to work with
	// We use the full line view range (rather than the run range) so that text that spans runs will still be shaped correctly
	FShapedGlyphSequenceRef ShapedText = ShapedTextCacheUtil::GetShapedTextSubSequence(
		BlockTextContext.ShapedTextCache,
		FCachedShapedTextKey(Line.Range, FontAtlasScale, BlockTextContext, FontInfo),
		BlockRange,
		**Text,
		BlockTextContext.TextDirection
	);

	// Draw the optional shadow
	if (ShouldDropShadow)
	{
		FShapedGlyphSequenceRef ShadowShapedText = ShapedText;
		if (Payload.DropShadowColor != FontInfo.OutlineSettings.OutlineColor)
		{
			// Copy font info for shadow to replace the outline color
			FSlateFontInfo ShadowFontInfo = FontInfo;
			ShadowFontInfo.OutlineSettings.OutlineColor = Payload.DropShadowColor;
			ShadowFontInfo.OutlineSettings.OutlineMaterial = nullptr;
			if (!ShadowFontInfo.OutlineSettings.bApplyOutlineToDropShadows)
			{
				ShadowFontInfo.OutlineSettings.OutlineSize = 0;
			}

			// Create new shaped text for drop shadow
			ShadowShapedText = ShapedTextCacheUtil::GetShapedTextSubSequence(
				BlockTextContext.ShapedTextCache,
				FCachedShapedTextKey(Line.Range, FontAtlasScale, BlockTextContext, ShadowFontInfo),
				BlockRange,
				**Text,
				BlockTextContext.TextDirection
			);
		}

		FSlateDrawElement::MakeShapedText(
			OutDrawElements,
			++LayerId,
			BaseGeom.ToPaintGeometry(FSlateLayoutTransform( DrawShadowOffset )),
			ShadowShapedText,
			DrawEffects,
			InWidgetStyle.GetColorAndOpacityTint() * Payload.DropShadowColor,
			InWidgetStyle.GetColorAndOpacityTint() * FontInfo.OutlineSettings.OutlineColor
		);
	}


	const bool RenderTextInTwoPasses = Payload.BackgroundBlurAmount > 0.f || Lookup->GetValue<UExTextValue_DrawOutlineAsSeparateLayer>() || Payload.OutlineRenderSize != FVector2D(1.f,1.f);
	if (!RenderTextInTwoPasses)
	{
		FSlateDrawElement::MakeShapedText(
			OutDrawElements,
			++LayerId,
			TextGeom.ToPaintGeometry(),
			ShapedText,
			DrawEffects,
			InWidgetStyle.GetColorAndOpacityTint() * Payload.FontColor,
			InWidgetStyle.GetColorAndOpacityTint() * FontInfo.OutlineSettings.OutlineColor
		);
	}
	else
	{
		const FGeometry OutlineGeom = TextGeom.MakeChild( FSlateRenderTransform(FScale2D(Payload.OutlineRenderSize)) );
		FSlateDrawElement::MakeShapedText(
			OutDrawElements,
			++LayerId,
			OutlineGeom.ToPaintGeometry(),
			ShapedText,
			DrawEffects,
			InWidgetStyle.GetColorAndOpacityTint() * FontInfo.OutlineSettings.OutlineColor,
			InWidgetStyle.GetColorAndOpacityTint() * FontInfo.OutlineSettings.OutlineColor
		);

		BlurLayer( OutDrawElements, LayerId, OutlineGeom, Payload.BackgroundBlurAmount);

		FSlateDrawElement::MakeShapedText(
			OutDrawElements,
			++LayerId,
			TextGeom.ToPaintGeometry(),
			ShapedText,
			DrawEffects,
			InWidgetStyle.GetColorAndOpacityTint() * Payload.FontColor,
			FLinearColor::Transparent
		);
	}

	if (Payload.ShouldClip)
	{
		OutDrawElements.PopClip();
	}

	if (Payload.ShouldBlur)
	{
		const float Strength = Payload.BlurAmount; // Could multiply by opacity to soften the effect
		BlurLayer( OutDrawElements, LayerId, TextGeom, Strength);
	}

	return LayerId;
}

float FExpressiveTextRun::CalculateDurationToFullyReveal() const
{
	const float RevealRate = Lookup->GetValue<UExTextValue_RevealRate>();

	FInterjectionOutput Output;
	ProcessInterjectionModifiers(Output);

	if (RevealRate <= 0.f)
	{
		return Output.PauseDuration;
	}


	float RatedReveal = Range.Len() / RevealRate;

	return RatedReveal + Output.PauseDuration;
}


float FExpressiveTextRun::CalculateDurationToFullyClear() const
{
	const float ClearRate = Lookup->GetValue<UExTextValue_ClearRate>();

	if (ClearRate <= 0.f)
	{
		return 0.f;
	}

	return Range.Len() / ClearRate;
}

void FExpressiveTextRun::SetInterjections(const TArray<FExText_ParsedInterjection>& InInterjections)
{
	float AccumulatedRelevantTime = 0.f;
	float AccumulatedPauseTime = 0.f;
	float LastPauseDuration = 0.f;
	int32 LastInterjectionSize = 0;

	const float RevealRate = Lookup->GetValue<UExTextValue_RevealRate>();

	for (const auto& InInterjection : InInterjections)
	{
		FInterjectionOutput Modifiers;
		InInterjection.Interjection->ProcessModifiers(Modifiers);

		const float RunSectionSize = InInterjection.Index - Range.BeginIndex;
		const float InterjectionSectionSize = static_cast<float>(RunSectionSize - LastInterjectionSize) -1;
		const float TimeToRevealInterjectionSection = RevealRate > 0.f ? InterjectionSectionSize / RevealRate : 0.f;
		const float RelevantTimeWithoutPauses = RevealRate > 0.f ? RunSectionSize / RevealRate : 0.f;

		AccumulatedRelevantTime += LastPauseDuration + TimeToRevealInterjectionSection;
		AccumulatedPauseTime += Modifiers.PauseDuration;

		Interjections.Emplace(InInterjection.Interjection.ToSharedRef(), InInterjection.Index, AccumulatedRelevantTime, RelevantTimeWithoutPauses, AccumulatedPauseTime);

		LastInterjectionSize = InterjectionSectionSize;
		LastPauseDuration = Modifiers.PauseDuration;
	}
}


void FExpressiveTextRun::FireInterjections( int32 SpecificIndex ) const
{
	if (UWorld* RawWorld = World.Get())
	{
		if (RawWorld->IsGameWorld())
		{
			for (FInterjectionTracker& Interjection : Interjections)
			{
				if (!Interjection.Fired)
				{
					if (SpecificIndex == -1 || Interjection.Index <= SpecificIndex )
					{
						Interjection.Interjection->DoExecute(*RawWorld);
						Interjection.Fired = true;
					}
				}
			}
		}
	}
}

void FExpressiveTextRun::ProcessInterjectionModifiers(FInterjectionOutput& Out, int32 SpecificIndex) const
{
	for (FInterjectionTracker& Interjection : Interjections)
	{
		if (SpecificIndex == -1 || Interjection.Index <= SpecificIndex)
		{
			Interjection.Interjection->ProcessModifiers(Out);
		}
	}
}

void FExpressiveTextRun::FillMaterialRequest(const UExpressiveTextMaterial& Material, int32 LayerIndex, FExTextMIDRequest& Request, const FGeometry& BlockGeom, int32 LineIndex, float FontSizeCompensation, FVector4 RevealAndClearInformation) const
{
	const auto MakeInfo =[LayerIndex]( const FName& Name ){
		FMaterialParameterInfo Info;
		Info.Name = Name;
		Info.Association = EMaterialParameterAssociation::LayerParameter;
		Info.Index = LayerIndex;
		return Info;
	};

	Request.AddScalar(MakeInfo("FontSizeCompensation"), FontSizeCompensation);

	if (Material.RequiresDynamicParameter(EExTextDynamicMaterialParameters::BlockSizeAndTopLeftPosition) )
	{
		FLinearColor SizeAndPos(FVector4(BlockGeom.GetAbsoluteSize(), BlockGeom.GetAbsolutePosition()));
		Request.AddVector(MakeInfo("BlockSizeAndTopLeftPosition"), SizeAndPos);
	}

	if (Material.RequiresDynamicParameter(EExTextDynamicMaterialParameters::RevealAndClearInformation))
	{		
		Request.AddVector(MakeInfo("RevealAndClearInformation"), FLinearColor(RevealAndClearInformation));
	}


	if (Material.RequiresDynamicParameter(EExTextDynamicMaterialParameters::LineIndex))
	{
		Request.AddScalar(MakeInfo("LineIndex"), LineIndex);
	}
}


void FExpressiveTextRun::BlurLayer( FSlateWindowElementList& OutDrawElements, int32& LayerId, const FGeometry& Geometry, float Strength )
{
	static int32 BlurPadding = 32; //avoid having hard clipping edges when blur expands beyond the text rect

	FVector2D GeometrySize = Geometry.GetLocalSize() / Geometry.Scale;
	FVector2D ExtraPadding(BlurPadding, BlurPadding );
#if UE_VERSION_OLDER_THAN( 5, 2, 0 )
	FGeometry PaddedGeom = Geometry.MakeChild(-ExtraPadding / 2.f, GeometrySize + ExtraPadding);
#else
	FGeometry PaddedGeom = Geometry.MakeChild(GeometrySize + ExtraPadding, FSlateLayoutTransform(-ExtraPadding / 2.f) );

#endif
	if (Strength > 0.f)
	{
		int32 RenderTargetWidth = FMath::RoundToInt(PaddedGeom.GetLocalSize().X);
		int32 RenderTargetHeight = FMath::RoundToInt(PaddedGeom.GetLocalSize().Y);

		int32 KernelSize = FMath::RoundToInt(Strength * 3.f);
		int32 DownsampleAmount = 0;
		ComputeEffectiveKernelSize(Strength, KernelSize, DownsampleAmount);

		float ComputedStrength = FMath::Max(0.6f, Strength);

		if (DownsampleAmount > 0)
		{
			RenderTargetWidth = FMath::DivideAndRoundUp(RenderTargetWidth, DownsampleAmount);
			RenderTargetHeight = FMath::DivideAndRoundUp(RenderTargetHeight, DownsampleAmount);
			ComputedStrength /= DownsampleAmount;
		}

		if (RenderTargetWidth > 0 && RenderTargetHeight > 0)
		{

#if UE_VERSION_OLDER_THAN( 5, 0, 0 )
			FVector4 BlurData = FVector4(KernelSize, ComputedStrength, RenderTargetWidth, RenderTargetHeight);
#else
			FVector4f BlurData = FVector4f(KernelSize, ComputedStrength, RenderTargetWidth, RenderTargetHeight);
#endif


#if UE_VERSION_OLDER_THAN( 5, 4, 0 )
			FSlateDrawElement::MakePostProcessPass(OutDrawElements, ++LayerId, PaddedGeom.ToPaintGeometry(), BlurData, DownsampleAmount);
#else
			FSlateDrawElement::MakePostProcessBlur(OutDrawElements, ++LayerId, PaddedGeom.ToPaintGeometry(), BlurData, DownsampleAmount);
#endif		
		}

	}
}