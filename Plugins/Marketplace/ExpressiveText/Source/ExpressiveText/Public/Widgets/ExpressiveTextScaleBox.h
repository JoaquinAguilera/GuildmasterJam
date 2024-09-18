// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <Slate/WidgetTransform.h>
#include <Widgets/Layout/SScaleBox.h>

class SExpressiveTextScaleBox : public SScaleBox
{
public:

	SExpressiveTextScaleBox();

	virtual void Tick( const FGeometry & Geom, const double InCurrentTime, const float InDeltaTime ) override;

	void RefreshCustomTransform();
	void SetFontSizeWhileCreatingAnimation(int32 NewFontSize);
	void SetCurrentFontSize(int32 NewFontSize);
	void SetGlyphTransform(FWidgetTransform NewGlyphTransform);
	void SetClipAmount(FVector2D InClipAmount);

	virtual int32 OnPaint
	(
	const FPaintArgs & Args,
	const FGeometry & AllottedGeometry,
	const FSlateRect & MyCullingRect,
	FSlateWindowElementList & OutDrawElements,
	int32 LayerId,
	const FWidgetStyle & InWidgetStyle,
	bool bParentEnabled
	) const override;

private:
	 int32 FontSizeWhileCreatingAnimation;
	 int32 CurrentFontSize;
	 FVector2D ClipAmount;
	 FWidgetTransform GlyphTransform;
};