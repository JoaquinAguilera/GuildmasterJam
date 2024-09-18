// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>

#include <Components/Widget.h>
#include <ISequencerModule.h>
#include <SequencerSettings.h>
#include <Widgets/SCompoundWidget.h>
#include <SCurveEditor.h>
#include <ExpressiveText/Public/Animation/ExpressiveTextAnimation.h>
#include "ExpressiveTextAnimationEditor.generated.h"


class SExpressiveTextAnimationEditor : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SExpressiveTextAnimationEditor)
		: _Dummy(false)
	{}
		SLATE_ATTRIBUTE(bool, Dummy)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
    {
         
        ChildSlot[
             SNew(SOverlay)
            +SOverlay::Slot()
            .VAlign(VAlign_Fill)
            .HAlign(HAlign_Fill)
            [
                SNew(SCurveEditor)
                 .Visibility(EVisibility::Visible)
                 .ViewMinInput(0.f)
                 .ViewMaxInput(1.f)
                 .ViewMinOutput(0.f)
                 .ViewMaxOutput(1.f)
                 .TimelineLength(1.f)
                 .DesiredSize(FVector2D(300, 200))
                 .HideUI(false)
                 .DrawCurve(true)
            ]
        ];
    }

 
private:
	TAttribute<bool> Dummy;    
    TStrongObjectPtr<UMovieSceneSequence> Sequence;
};


UCLASS()
class UExpressiveTextAnimationEditor : public UWidget
{
    GENERATED_BODY()

public:

    UExpressiveTextAnimationEditor(const FObjectInitializer& ObjectInitializer)
        : Super(ObjectInitializer)
    {
    }

    #if WITH_EDITOR
	virtual const FText GetPaletteCategory() override
    {
        return FText::FromString(TEXT("Expressive Text Editor"));
    }
    #endif
    
public:
	
	//~ Begin UWidget Interface
	virtual void SynchronizeProperties() override
    {
        Super::SynchronizeProperties();
    }
	//~ End UWidget Interface

	//~ Begin UVisual Interface
	virtual void ReleaseSlateResources(bool bReleaseChildren) override
    {
        Super::ReleaseSlateResources(bReleaseChildren);
        AnimationEditor.Reset();
    }
	//~ End UVisual Interface

protected:
	//~ Begin UWidget Interface
	virtual TSharedRef<SWidget> RebuildWidget() override
    {	
        AnimationEditor = SNew(SExpressiveTextAnimationEditor);
        
        return AnimationEditor.ToSharedRef();
    }
	// End of UWidget
    
	TSharedPtr<SExpressiveTextAnimationEditor> AnimationEditor;
};
