// Copyright 2022 Guganana. All Rights Reserved.

#pragma once

#include <CoreMinimal.h>

#include "ExpressiveTextSettings.h"
#include "Compiled/CompiledExpressiveText.h"
#include "ExpressiveTextProcessor.h"
#include "Handles/ExpressiveText.h"
#include "Layout/ExpressiveTextLayout.h"
#include "Layout/ExTextChronos.h"

#include <Templates/SharedPointer.h>

class EXPRESSIVETEXT_API SExpressiveTextRendererWidget 
	: public SWidget
{
public:


	SLATE_BEGIN_ARGS(SExpressiveTextRendererWidget)
		: _UsedInEditor( false )
	{
	}

	SLATE_ATTRIBUTE(bool, UsedInEditor)
	SLATE_END_ARGS()

	void Construct(const FArguments& Args)
	{
		TextLayout->SetParentWidget(AsShared());
		UsedInEditor = Args._UsedInEditor;
	}

	SExpressiveTextRendererWidget()
		: SWidget()
#if UE_VERSION_OLDER_THAN( 5, 0, 0 )
		, NoChildrenInstance()
#else
		, NoChildrenInstance(this)
#endif
		, CompiledText()
		, TextLayout( MakeShareable( new FExpressiveTextSlateLayout ) )
		, UsedInEditor(false)
	{
	}

	float CalcDesiredWrappingWidth( const FVector2D& ParentDrawSize ) const
	{
		const auto& WrapSettings = CompiledText.GetValue().WrapSettings;

		if (WrapSettings.ValueType == EExpressiveTextWrapMode::NoWrap)
		{
			return 0.f;
		}

		return WrapSettings.ValueType == EExpressiveTextWrapMode::WrapAtPercentageOfParentSize ? WrapSettings.Value * ParentDrawSize.X : WrapSettings.Value;
	}

	float CalcAutoSizeWidth(const FVector2D& ParentDrawSize) const
	{
		const auto& WrapSettings = CompiledText.GetValue().WrapSettings;

		if (WrapSettings.ValueType == EExpressiveTextWrapMode::WrapAtPercentageOfParentSize)
		{
			return WrapSettings.Value * ParentDrawSize.X;
		}

		return ParentDrawSize.X;
	}

	virtual int32 OnPaint( const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled ) const override
	{
		if( !HasText() )
		{
			return 0;
		}		

		FVector2D DrawSize = AllottedGeometry.GetLocalSize();
		if (DrawSize.IsNearlyZero())
		{
			return 0;
		}

		float DesiredWrapingWidth = CalcDesiredWrappingWidth(DrawSize);
		CommitWrappingWidth( DesiredWrapingWidth );

		TextLayout->GetSharedData()->Chronos.UpdateCurrentTime();
		
		if(TextLayout->GetWrappingWidth() >= 0.f)
		{
			TextLayout->UpdateIfNeeded();

			TextLayout->GetSharedData()->AlignmentOffset = CompiledText.GetValue().Alignment.CalculateDesiredPosition(DrawSize, TextLayout->GetSize());

			if (CompiledText->UseAutoSize)
			{
				FVector2D AutoSizeArea = FVector2D(CalcAutoSizeWidth(DrawSize), DrawSize.Y);
				TextLayout->ApplyAutoSize(AutoSizeArea);
			}

			// AllottedGeometry is the entire canvas
			return TextLayout->OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
		}
			
		return 0;
	}
	
	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const
	{
		if (HasText())
		{
			TextLayout->SetScale(LayoutScaleMultiplier);
			TextLayout->UpdateIfNeeded();
			return TextLayout->GetSize();
		}

		return FVector2D::ZeroVector;
	}

	bool HasText() const
	{
		return CompiledText.IsSet();
	}

	virtual FChildren* GetChildren() override
	{
		if (HasText())
		{
			return TextLayout->GetChildren();
		}

		return &NoChildrenInstance;
	}

	virtual void OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const override
	{
		if (HasText())
		{
			const auto& WrapSettings = CompiledText.GetValue().WrapSettings;
			FVector2D DrawSize = AllottedGeometry.GetDrawSize();

			CommitWrappingWidth( CalcDesiredWrappingWidth(DrawSize) );

			FVector2D StartPos = CompiledText.GetValue().Alignment.CalculateDesiredPosition(DrawSize, TextLayout->GetSize());
			TextLayout->ArrangeChildren(AllottedGeometry.MakeChild(FSlateRenderTransform(StartPos)), ArrangedChildren);
		}
	}

	void SetExpressiveText( FExpressiveText& Text )
	{
		TextLayout->ClearLines();
		Text.SetTextLayout( TextLayout );

		TWeakPtr<SExpressiveTextRendererWidget> WeakThisPtr( StaticCastSharedRef<SExpressiveTextRendererWidget>(AsShared()) );
		UExpressiveTextProcessor::CompileText(Text).Next(
			[WeakThisPtr](const FCompiledExpressiveText& InCompiledText)
			{
				if ( auto* RawThis = WeakThisPtr.Pin().Get() )
				{
					RawThis->CompiledText = InCompiledText;
					RawThis->TextLayout->AggregateChildren();
					RawThis->TextLayout->GetSharedData()->Chronos.UpdateStartTime();
					RawThis->TextLayout->GetSharedData()->Chronos.UpdateCurrentTime();
					RawThis->TextLayout->ResetAutoSizeCache();
				}
			}
		);
	}

	void Reset()
	{
		TextLayout->GetSharedData()->Chronos.UpdateStartTime();
		TextLayout->GetSharedData()->Chronos.UpdateCurrentTime();
	}

	void SkipReveal()
	{
		TextLayout->GetSharedData()->Chronos.SkipToRevealEnd();		
	}

	void GetChronos(FExpressiveTextChronos& OutChronos) const
	{
		OutChronos = TextLayout->GetSharedData()->Chronos;
	}


private:

	void CommitWrappingWidth( float value ) const
	{
		// Value is 0 when no wrapping is set
		value = FMath::Max(0.f, value);
		TextLayout->SetWrappingWidth(value);
	}

	FNoChildren NoChildrenInstance;
	TOptional<FCompiledExpressiveText> CompiledText;
	TSharedRef<FExpressiveTextSlateLayout> TextLayout;
	TAttribute<bool> UsedInEditor;
};
