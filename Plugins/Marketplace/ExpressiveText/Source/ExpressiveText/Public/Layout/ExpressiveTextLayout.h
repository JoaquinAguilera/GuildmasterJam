// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>

#include "Layout/ExpressiveTextRun.h"
#include "Layout/ExTextSharedLayoutData.h"

#include <Framework/Text/ISlateLineHighlighter.h>
#include <Framework/Text/ISlateRunRenderer.h>
#include <Framework/Text/TextLayout.h>
#include <Layout/Children.h>
#include <Styling/SlateTypes.h>

#include "Misc/Attribute.h"
#include "Widgets/SWidget.h"

//class FExpressiveTextLayout : public FTextLayout
//{
//protected:
//	virtual TSharedRef<IRun> CreateDefaultTextRun(const TSharedRef<FString>& NewText, const FTextRange& NewRange) const override
//    {
//	    FTextBlockStyle DefaultTextStyle;
//	    return FExpressiveTextRun::Create(FRunInfo(), NewText, DefaultTextStyle, NewRange); /
//    }
//public:
//    static TSharedRef<FExpressiveTextRun> CreateRun(const TSharedRef<FString>& NewText, FSlateFontInfo FontInfo, const FTextRange& Range)
//    {
//	    FTextBlockStyle Style;
//        Style.Font = FontInfo;
//	    return FExpressiveTextRun::Create(FRunInfo(), NewText, Style, Range); // should probably create my own type of TextRun
//    }
//};

class FExpressiveTextSlateLayout : public FTextLayout
{
	struct FSlotAndParentWrapper
	{
		TSlotlessChildren< SWidget > Children;
		TWeakPtr<SWidget> ParentWidget;

		FSlotAndParentWrapper( const TSharedPtr<SWidget>& InParentWidget )
			: Children( InParentWidget.Get() )
			, ParentWidget( TWeakPtr<SWidget>(InParentWidget) )
		{}
	};

public:
	FExpressiveTextSlateLayout()
		: FTextLayout()
		, SharedData( MakeShareable(new FExTextSharedLayoutData))
		, TextTotalLength( 0 )
	{
	}

	TSharedRef<FExTextSharedLayoutData> GetSharedData() const { return SharedData; }

	void SetParentWidget( TSharedPtr<SWidget> InWidget)
	{
		if (SlotAndParent && SlotAndParent->ParentWidget.IsValid())
		{
			ensure(false);
			return;
		}

		if (InWidget.IsValid())
		{
			SlotAndParent = MakeShareable( new FSlotAndParentWrapper(InWidget) );
			AggregateChildren();
		}
		else
		{
			SlotAndParent.Reset();
		}
	}

	virtual void AggregateChildren()
	{
		check(SlotAndParent->ParentWidget.IsValid());

		TextTotalLength = 0;
		SlotAndParent->Children.Empty();

		const TArray< FLineModel >& LayoutLineModels = GetLineModels();
		for (int32 LineModelIndex = 0; LineModelIndex < LayoutLineModels.Num(); LineModelIndex++)
		{
			const FLineModel& LineModel = LayoutLineModels[LineModelIndex];
			for (int32 RunIndex = 0; RunIndex < LineModel.Runs.Num(); RunIndex++)
			{
				const FRunModel& LineRun = LineModel.Runs[RunIndex];
				const TSharedRef< ISlateRun > SlateRun = StaticCastSharedRef< ISlateRun >(LineRun.GetRun());

				TextTotalLength += SlateRun->GetTextRange().Len();

				const TArray< TSharedRef<SWidget> >& RunChildren = SlateRun->GetChildren();
				for (int32 ChildIndex = 0; ChildIndex < RunChildren.Num(); ChildIndex++)
				{
					const TSharedRef< SWidget >& Child = RunChildren[ChildIndex];
					SlotAndParent->Children.Add(Child);
				}
			}
		}
	}

	int GetTextTotalLength() const
	{
		return TextTotalLength;
	}

	FChildren* GetChildren()
	{
		check(SlotAndParent && SlotAndParent->ParentWidget.IsValid());
		return &(SlotAndParent->Children);
	}


	virtual void ArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const
	{
		for (int32 LineIndex = 0; LineIndex < LineViews.Num(); LineIndex++)
		{
			const FTextLayout::FLineView& LineView = LineViews[LineIndex];

			for (int32 BlockIndex = 0; BlockIndex < LineView.Blocks.Num(); BlockIndex++)
			{
				const TSharedRef< ILayoutBlock > Block = LineView.Blocks[BlockIndex];
				const TSharedRef< ISlateRun > Run = StaticCastSharedRef< ISlateRun >(Block->GetRun());
				Run->ArrangeChildren(Block, AllottedGeometry, ArrangedChildren);
			}
		}
	}

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
	{
		const ESlateDrawEffect DrawEffects = bParentEnabled ? ESlateDrawEffect::None : ESlateDrawEffect::DisabledEffect;

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
		FLinearColor BlockDebugHue(0, 1.0f, 1.0f, 0.5);
#endif

		// The block size and offset values are pre-scaled, so we need to account for that when converting the block offsets into paint geometry
		const float InverseScale = Inverse(AllottedGeometry.Scale);

		int32 HighestLayerId = LayerId;

		for (const FTextLayout::FLineView& LineView : LineViews)
		{
			// Is this line visible?  This checks if the culling rect, which represents the AABB around the last clipping rect, intersects the 
			// line of text, this requires that we get the text line into render space.
			// TODO perhaps save off this line view rect during text layout?
			const FVector2D LocalLineOffset = LineView.Offset * InverseScale;
			const FSlateRect LineViewRect(AllottedGeometry.GetRenderBoundingRect(FSlateRect(LocalLineOffset, LocalLineOffset + (LineView.Size * InverseScale))));
			if (!FSlateRect::DoRectanglesIntersect(LineViewRect, MyCullingRect))
			{
				continue;
			}

			// Render any underlays for this line
			const int32 HighestUnderlayLayerId = OnPaintHighlights(Args, LineView, LineView.UnderlayHighlights, DefaultTextStyle, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

			const int32 BlockDebugLayer = HighestUnderlayLayerId;
			const int32 TextLayer = BlockDebugLayer + 1;
			int32 HighestBlockLayerId = TextLayer;


			bool bForceEllipsisDueToClippedLine = false;

			if (false)//OverflowPolicy == ETextOverflowPolicy::Ellipsis && LineViews.Num() > 1)
			{
				// Force the ellipsis to be on when the next line in a multi line text layout is clipped. This forces an ellipsis on this line even when its not clipped to indicate that an entire line or more is clipped
				//bForceEllipsisDueToClippedLine = LineViews.IsValidIndex(LineIndex + 1) ? !IsLineViewVisible(LineViews[LineIndex + 1], true) : false;
			}

			// Render every block for this line
			for (const TSharedRef< ILayoutBlock >& Block : LineView.Blocks)
			{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
				static TAutoConsoleVariable<float> CVarExpressiveTextBlockDebug(TEXT("ExpressiveText.DebugTextBlocks"), 0, TEXT("Renders debug boxes around expressive text"));
				if (CVarExpressiveTextBlockDebug.GetValueOnGameThread())
				{
					static uint32 DebugLayer = 20000;
					BlockDebugHue.R += 50.0f;

					FSlateDrawElement::MakeBox(
						OutDrawElements,
						DebugLayer,
						AllottedGeometry.ToPaintGeometry(TransformVector(InverseScale, Block->GetSize()), FSlateLayoutTransform(TransformPoint(InverseScale, Block->GetLocationOffset()))),
						&DefaultTextStyle.HighlightShape,
						DrawEffects,
						InWidgetStyle.GetColorAndOpacityTint() * BlockDebugHue.HSVToLinearRGB()
					);
				}
#endif
				const TSharedRef< ISlateRun > Run = StaticCastSharedRef< ISlateRun >(Block->GetRun());

				int32 HighestRunLayerId = TextLayer;
				const TSharedPtr< ISlateRunRenderer > RunRenderer = StaticCastSharedPtr< ISlateRunRenderer >(Block->GetRenderer());
				if (RunRenderer.IsValid())
				{
					HighestRunLayerId = RunRenderer->OnPaint(Args, LineView, Run, Block, DefaultTextStyle, AllottedGeometry, MyCullingRect, OutDrawElements, TextLayer, InWidgetStyle, bParentEnabled);
				}
				else
				{

#if UE_VERSION_OLDER_THAN( 5, 0, 0 )
					HighestRunLayerId = Run->OnPaint(Args, LineView, Block, DefaultTextStyle, AllottedGeometry, MyCullingRect, OutDrawElements, TextLayer, InWidgetStyle, bParentEnabled);
#else

					ETextOverflowPolicy OverflowPolicy = ETextOverflowPolicy::Clip;
					const ETextJustify::Type VisualJustification = CalculateLineViewVisualJustification(LineView);
					ETextOverflowDirection OverflowDirection = VisualJustification == ETextJustify::Left ? ETextOverflowDirection::LeftToRight : (VisualJustification == ETextJustify::Right ? ETextOverflowDirection::RightToLeft : ETextOverflowDirection::NoOverflow);
					FTextArgs TextArgs(LineView, Block, DefaultTextStyle, OverflowPolicy, OverflowDirection);
#if UE_VERSION_OLDER_THAN( 5, 3, 0 )
					TextArgs.bForceEllipsisDueToClippedLine = bForceEllipsisDueToClippedLine;
#endif
					HighestRunLayerId = Run->OnPaint(Args, TextArgs, AllottedGeometry, MyCullingRect, OutDrawElements, TextLayer, InWidgetStyle, bParentEnabled);
#endif
				}

				HighestBlockLayerId = FMath::Max(HighestBlockLayerId, HighestRunLayerId);
			}

			// Render any overlays for this line
			const int32 HighestOverlayLayerId = OnPaintHighlights(Args, LineView, LineView.OverlayHighlights, DefaultTextStyle, AllottedGeometry, MyCullingRect, OutDrawElements, HighestBlockLayerId, InWidgetStyle, bParentEnabled);
			HighestLayerId = FMath::Max(HighestLayerId, HighestOverlayLayerId);
		}

		return HighestLayerId;
	}

	virtual void EndLayout() override
	{
		FTextLayout::EndLayout();
		if (SlotAndParent && SlotAndParent->ParentWidget.IsValid())
		{
			AggregateChildren();
		}
	}

	virtual void UpdateIfNeeded() override
	{
		//const uint16 CurrentLocalizedFallbackFontRevision = FSlateApplication::Get().GetRenderer()->GetFontCache()->GetLocalizedFallbackFontRevision();
		//if (CurrentLocalizedFallbackFontRevision != LocalizedFallbackFontRevision)
		//{
		//	if (LocalizedFallbackFontRevision != 0)
		//	{
		//		// If the localized fallback font has changed, we need to purge the current layout data as things may need to be re-measured
		//		DirtyFlags |= ETextLayoutDirtyState::Layout;
		//		DirtyAllLineModels(ELineModelDirtyState::WrappingInformation | ELineModelDirtyState::ShapingCache);
		//	}

		//	LocalizedFallbackFontRevision = CurrentLocalizedFallbackFontRevision;
		//}

		FTextLayout::UpdateIfNeeded();
	}

	void SetDefaultTextStyle(FTextBlockStyle InDefaultTextStyle)
	{
		DefaultTextStyle = MoveTemp(InDefaultTextStyle);
	}

	const FTextBlockStyle& GetDefaultTextStyle() const
	{
		return DefaultTextStyle;
	}


	virtual TSharedRef<IRun> CreateDefaultTextRun(const TSharedRef<FString>& NewText, const FTextRange& NewRange) const override
	{
		check(false); // This should not run;
		return MakeShareable<IRun>(nullptr);
	}


	static TSharedRef<FExpressiveTextRun> CreateRun(const TSharedRef<FString>& NewText, FSlateFontInfo FontInfo, const FTextRange& Range, TSharedRef<FExTextSharedLayoutData> SharedData, float InRevealStartTime )
	{
		FTextBlockStyle Style;
		Style.Font = FontInfo;
		return FExpressiveTextRun::Create(FRunInfo(), NewText, Style, Range, SharedData, InRevealStartTime);
	}

	void ApplyFontHeightScaleToAllRuns(float AutoFontSize)
	{
		const TArray< FLineModel >& LayoutLineModels = GetLineModels();
		for (int32 LineModelIndex = 0; LineModelIndex < LayoutLineModels.Num(); LineModelIndex++)
		{
			const FLineModel& LineModel = LayoutLineModels[LineModelIndex];
			for (int32 RunIndex = 0; RunIndex < LineModel.Runs.Num(); RunIndex++)
			{
				const FRunModel& LineRun = LineModel.Runs[RunIndex];
				const TSharedRef< FExpressiveTextRun > SlateRun = StaticCastSharedRef< FExpressiveTextRun >(LineRun.GetRun());

				SlateRun->SetAutoFontSizeScale(AutoFontSize);
			}
		}
	}


	int GetTotalGlyphsRevealed()
	{
		float result = 0;
		const TArray< FLineModel >& LayoutLineModels = GetLineModels();
		for (int32 LineModelIndex = 0; LineModelIndex < LayoutLineModels.Num(); LineModelIndex++)
		{
			const FLineModel& LineModel = LayoutLineModels[LineModelIndex];
			for (int32 RunIndex = 0; RunIndex < LineModel.Runs.Num(); RunIndex++)
			{
				const FRunModel& LineRun = LineModel.Runs[RunIndex];
				const TSharedRef< FExpressiveTextRun > SlateRun = StaticCastSharedRef< FExpressiveTextRun >(LineRun.GetRun());

				result += SlateRun->GetNumGlyphsRevealedInThisRun();
			}
		}

		return result;
	}

	void ResetAutoSizeCache()
	{
		LastProcessedAreaForAutoSize = FVector2D::ZeroVector;
		LastProcessedScaleForAutoSize = 0;
	}

	void ApplyAutoSize(const FVector2D& DrawArea)
	{
		if (DrawArea == LastProcessedAreaForAutoSize && LastProcessedScaleForAutoSize == Scale)
		{
			return;
		}

		LastProcessedAreaForAutoSize = DrawArea;
		LastProcessedScaleForAutoSize = Scale;

		FVector2D CurrentDifference = FVector2D::ZeroVector;

		float MagnitudeA = 1.f;
		float MagnitudeB = 1000.f;
		float Mid = 80.f;
		float LastMid = -1.f;
		float BestFontCandidateHeightDifference = BIG_NUMBER;
		float BestFontCandidate = -1.f;

		int Iterations = 100;		
		while(Iterations-- > 0)
		{
			ApplyFontHeightScaleToAllRuns(Mid);

			DirtyLayout();
			UpdateLayout();

			FVector2D CurrentSize = GetSize();

			CurrentDifference = DrawArea - CurrentSize;

			float Compensation = 4.f;

			bool VerticalOverflow = CurrentDifference.Y < 0.f;

			bool Overflow = CurrentSize.X / DrawArea.X > 1.05f || VerticalOverflow;

			if (CurrentDifference.Y >= 0.f) // Only record best cases which haven't overflowed
			{
				float score = FMath::Abs(CurrentDifference.Y) + FMath::Abs(CurrentDifference.X);
				if (BestFontCandidateHeightDifference > score)
				{
					BestFontCandidate = Mid;
					BestFontCandidateHeightDifference = score;
				}
			}

			if (FMath::Abs(Mid - LastMid) <= 0.01f)
			{
				break;
			}

			LastMid = Mid;

			if (Overflow)
			{
				MagnitudeB = Mid;
			}
			else
			{
				MagnitudeA = Mid;
			}

			Mid = (MagnitudeA + MagnitudeB) / 2.f;
		}

		ApplyFontHeightScaleToAllRuns( BestFontCandidate );
		DirtyLayout();
		UpdateLayout();
	}

protected:

	virtual int32 OnPaintHighlights(const FPaintArgs& Args, const FTextLayout::FLineView& LineView, const TArray<FLineViewHighlight>& Highlights, const FTextBlockStyle& InDefaultTextStyle, const FGeometry& AllottedGeometry, const FSlateRect& ClippingRect, FSlateWindowElementList& OutDrawElements, const int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
	{
		int32 CurrentLayerId = LayerId;

		for (const FLineViewHighlight& Highlight : Highlights)
		{
			const TSharedPtr< ISlateLineHighlighter > LineHighlighter = StaticCastSharedPtr< ISlateLineHighlighter >(Highlight.Highlighter);
			if (LineHighlighter.IsValid())
			{
				CurrentLayerId = LineHighlighter->OnPaint(Args, LineView, Highlight.OffsetX, Highlight.Width, InDefaultTextStyle, AllottedGeometry, ClippingRect, OutDrawElements, CurrentLayerId, InWidgetStyle, bParentEnabled);
			}
		}

		return CurrentLayerId;
	}

protected:
	/** Default style used by the TextLayout */
	FTextBlockStyle DefaultTextStyle;

private:
	TSharedPtr<FSlotAndParentWrapper> SlotAndParent;
	TSharedRef<FExTextSharedLayoutData> SharedData;

	/** The localized fallback font revision the last time the text layout was updated. Used to force a flush if the font changes. */
	uint16 LocalizedFallbackFontRevision;
	FVector2D LastProcessedAreaForAutoSize = FVector2D:: ZeroVector;
	float LastProcessedScaleForAutoSize = -1.f;
	int TextTotalLength;
};