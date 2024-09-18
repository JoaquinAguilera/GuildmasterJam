// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include "Widgets/ExpressiveTextScaleBox.h"

#include <Misc/EngineVersionComparison.h>

SExpressiveTextScaleBox::SExpressiveTextScaleBox() : SScaleBox()
, FontSizeWhileCreatingAnimation(24)
, CurrentFontSize(24)
, ClipAmount( FVector2D::ZeroVector)
, GlyphTransform()
{
	SetCanTick(true);
}

void SExpressiveTextScaleBox::Tick(const FGeometry & Geom, const double InCurrentTime, const float InDeltaTime)
{
	SScaleBox::Tick(Geom, InCurrentTime, InDeltaTime);

	RefreshCustomTransform();
}

void SExpressiveTextScaleBox::RefreshCustomTransform()
{
	TSharedPtr<SWidget> Parent = GetParentWidget();
	if (Parent)
	{
		FWidgetTransform TargetTransform = GlyphTransform;

		float FontSizeCompensation = 1.f;
#if UE_VERSION_OLDER_THAN( 4, 24, 0 )
		float SizeMultiplier = GetLayoutScale();
#else
		float SizeMultiplier = ComputeContentScale(FGeometry());
#endif
		
		if (CurrentFontSize > 0 && FontSizeWhileCreatingAnimation > 0)
		{
			FontSizeCompensation = static_cast<float>(CurrentFontSize) / static_cast<float>(FontSizeWhileCreatingAnimation);
		}

		FVector2D TopLeft = FVector2D::ZeroVector;
		auto ParentRenderTransform = Parent->GetRenderTransform();
		if (ParentRenderTransform.IsSet())
		{
			TopLeft = ParentRenderTransform.GetValue().GetTranslation();
		}

		const FVector2D& Size = Parent->GetDesiredSize();
		const FVector2D OriginalPivot = TopLeft + (Size / SizeMultiplier) * GetRenderTransformPivot();
		const FVector2D NewPivot = TopLeft + Size * GetRenderTransformPivot();
		const FVector2D DesiredOffset = OriginalPivot - NewPivot;

		TargetTransform.Translation = DesiredOffset + (GlyphTransform.Translation *FontSizeCompensation);
		SetRenderTransform(TargetTransform.ToSlateRenderTransform());
	}
}

void SExpressiveTextScaleBox::SetFontSizeWhileCreatingAnimation(int32 NewFontSize)
{
	FontSizeWhileCreatingAnimation = NewFontSize;
}

void SExpressiveTextScaleBox::SetCurrentFontSize(int32 NewFontSize)
{
	CurrentFontSize = NewFontSize;
}

void SExpressiveTextScaleBox::SetGlyphTransform(FWidgetTransform NewGlyphTransform)
{
	GlyphTransform = NewGlyphTransform;
	RefreshCustomTransform();
}

void SExpressiveTextScaleBox::SetClipAmount(FVector2D InClipAmount)
{
	ClipAmount = InClipAmount;
}

int32 SExpressiveTextScaleBox::OnPaint
(
    const FPaintArgs & Args,
    const FGeometry & AllottedGeometry,
    const FSlateRect & MyCullingRect,
    FSlateWindowElementList & OutDrawElements,
    int32 LayerId,
    const FWidgetStyle & InWidgetStyle,
    bool bParentEnabled
) const
{
	if (ClipAmount.IsNearlyZero())
	{
		return SScaleBox::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
	}

	FVector2D Size = AllottedGeometry.GetLocalSize();
	FVector2D Translation = ClipAmount * Size;
	FGeometry ClipGeometry = AllottedGeometry.MakeChild(Size, FSlateLayoutTransform(Translation));
	OutDrawElements.PushClip(FSlateClippingZone(ClipGeometry));
	int32 Result = SScaleBox::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
	OutDrawElements.PopClip();
	return Result;
}