// Copyright 2022 Guganana. All Rights Reserved.
#pragma once 

#include <CoreMinimal.h>

#include <Components/TextWidgetTypes.h>
#include <Engine/Font.h>
#include <Fonts/FontMeasure.h>
#include <Fonts/SlateFontInfo.h>
#include <Framework/Application/SlateApplication.h>
#include <UObject/ConstructorHelpers.h>
#include <Widgets/Text/SlateEditableTextLayout.h>
#include <Widgets/Text/SMultiLineEditableText.h>

#include "ExTextMultiLineEditableText.generated.h"

class SExTextMultiLineEditableText : public SMultiLineEditableText
{
public:

    bool HasScrolled()
    {
		FVector2D NewOffset = EditableTextLayout->GetScrollOffset();

        if( NewOffset != OldOffset )
        {
            OldOffset = NewOffset;
            return true;
        }

        return false;
    }

	void GetTextBounds( const uint32 InBeginIndex, const uint32 InLength, FVector2D& OutPosition, FVector2D& OutSize ) const
    {
		const TSharedRef< FSlateFontMeasure > FontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
		const float FontMaxCharHeight = FontMeasure->GetMaxCharacterHeight(EditableTextLayout->GetTextStyle().Font );

		FVector2D ScreenPos;
		FVector2D ScreenSize;
		
        EditableTextLayout->GetTextInputMethodContext()->GetTextBounds( InBeginIndex, InLength, OutPosition, OutSize );
		ScreenSize = EditableTextLayout->GetSize();

		OutPosition = GetCachedGeometry().AbsoluteToLocal( OutPosition );
		OutPosition.Y -= FontMaxCharHeight;
		OutSize.Y = FontMaxCharHeight;

		if (ScreenSize.X - OutSize.X <= 0.001f )
		{
			if (InBeginIndex > 0)
			{
				FVector2D PrevPos;
				FVector2D PrevSize;
				GetTextBounds(InBeginIndex - 1, 1, PrevPos, PrevSize);
				OutPosition.X = PrevPos.X + PrevSize.X;
				OutSize.X = ScreenSize.X - OutPosition.X;
			}
		}
    }

private:
    FVector2D OldOffset;
};


UCLASS()
class UExTextMultiLineEditableText : public UTextLayoutWidget
{
    GENERATED_BODY()

public:

    UExTextMultiLineEditableText(const FObjectInitializer& ObjectInitializer)
        : Super(ObjectInitializer)
    {
        WidgetStyle = FTextBlockStyle(FCoreStyle::Get().GetWidgetStyle<FTextBlockStyle>("NormalText"));
        WidgetStyle.UnlinkColors();

        SetClipping(EWidgetClipping::ClipToBounds);
        VirtualKeyboardDismissAction = EVirtualKeyboardDismissAction::TextChangeOnDismiss;
        AutoWrapText = true;

        if (!IsRunningDedicatedServer())
        {
            static ConstructorHelpers::FObjectFinder<UFont> RobotoFontObj(*UWidget::GetDefaultFontName());
            WidgetStyle.SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 12));
    	}
    }

    #if WITH_EDITOR
	virtual const FText GetPaletteCategory() override
    {
        return FText::FromString(TEXT("Expressive Text Editor"));
    }
    #endif

	UPROPERTY(EditAnywhere, Category=Content, meta=(MultiLine="true"))
	FText Text;
    
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintSetter=SetWidgetStyle, Category="Style", meta=(ShowOnlyInnerProperties))
	FTextBlockStyle WidgetStyle;
	
	UPROPERTY(EditAnywhere, Category=Behavior, AdvancedDisplay)
	FVirtualKeyboardOptions VirtualKeyboardOptions;

	UPROPERTY(EditAnywhere, Category=Behavior, AdvancedDisplay)
	EVirtualKeyboardDismissAction VirtualKeyboardDismissAction;

    UFUNCTION(BlueprintCallable, Category = ExpressiveTextEditor)
    bool HasBeenScrolled()
    {
        if( SlateMultilineEditableText.IsValid() )
        {
            return SlateMultilineEditableText->HasScrolled();
        }
        return false;
    }

	UFUNCTION(BlueprintCallable, Category = ExpressiveTextEditor, meta=(DisplayName="GetText (Multi-Line Editable Text)"))
	FText GetText() const
    {
        if ( SlateMultilineEditableText.IsValid() )
        {
            return SlateMultilineEditableText->GetText();
        }

        return Text;
    }

	UFUNCTION(BlueprintCallable, Category = ExpressiveTextEditor)
    void GetTextBounds( int32 InBeginIndex, int32 InLength, FVector2D& OutPosition, FVector2D& OutSize ) const
    {
        check( InBeginIndex >= 0 );
        check( InLength >= 1 );

        SlateMultilineEditableText->GetTextBounds( static_cast<uint32>(InBeginIndex), static_cast<uint32>(InLength), OutPosition, OutSize );
    }

	UFUNCTION(BlueprintCallable, Category = ExpressiveTextEditor, meta=(DisplayName="SetText (Multi-Line Editable Text)"))
	void SetText(FText InText)
    {
        Text = InText;
        if ( SlateMultilineEditableText.IsValid() )
        {
            SlateMultilineEditableText->SetText(Text);
        }
    }


	UFUNCTION(BlueprintCallable, Category = ExpressiveTextEditor, meta=(DisplayName="SetIsReadOnly (Multi-Line Editable Text"))
	void SetIsReadOnly(bool bReadOnly)
    {
        if ( SlateMultilineEditableText.IsValid() )
        {
            SlateMultilineEditableText->SetIsReadOnly(bReadOnly);
        }
    }

	UFUNCTION(BlueprintSetter)
	void SetWidgetStyle(const FTextBlockStyle& InWidgetStyle)
    {
        WidgetStyle = InWidgetStyle;

        if (SlateMultilineEditableText.IsValid())
        {
            SlateMultilineEditableText->SetTextStyle(&WidgetStyle);
        }
    }
	
	//~ Begin UWidget Interface
	virtual void SynchronizeProperties() override
    {
        Super::SynchronizeProperties();

        SlateMultilineEditableText->SetTextStyle(&WidgetStyle);
        SlateMultilineEditableText->SetText(Text);
        SlateMultilineEditableText->SetAllowContextMenu(true);
        SlateMultilineEditableText->SetIsReadOnly(false);
        SlateMultilineEditableText->SetVirtualKeyboardDismissAction(VirtualKeyboardDismissAction);
        SlateMultilineEditableText->SetSelectAllTextWhenFocused(false);
        SlateMultilineEditableText->SetClearTextSelectionOnFocusLoss(true);
        SlateMultilineEditableText->SetRevertTextOnEscape(false);
        SlateMultilineEditableText->SetClearKeyboardFocusOnCommit(true);

        Super::SynchronizeTextLayoutProperties(*SlateMultilineEditableText);
    }
	//~ End UWidget Interface

	//~ Begin UVisual Interface
	virtual void ReleaseSlateResources(bool bReleaseChildren) override
    {
        Super::ReleaseSlateResources(bReleaseChildren);
        SlateMultilineEditableText.Reset();
    }
	//~ End UVisual Interface

protected:
	//~ Begin UWidget Interface
	virtual TSharedRef<SWidget> RebuildWidget() override
    {	
        SlateMultilineEditableText = SNew(SExTextMultiLineEditableText)
        .TextStyle(&WidgetStyle)
        .AllowContextMenu(true)
        .IsReadOnly(false)
        .SelectAllTextWhenFocused(false)
        .ClearTextSelectionOnFocusLoss(true)
        .RevertTextOnEscape(false)
        .ClearKeyboardFocusOnCommit(true)
        .VirtualKeyboardOptions(VirtualKeyboardOptions)
        .VirtualKeyboardDismissAction(VirtualKeyboardDismissAction);
    

        return SlateMultilineEditableText.ToSharedRef();
    }
	// End of UWidget
    
	TSharedPtr<SExTextMultiLineEditableText> SlateMultilineEditableText;
};
