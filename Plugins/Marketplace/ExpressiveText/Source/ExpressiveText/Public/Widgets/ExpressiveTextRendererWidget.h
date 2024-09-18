// Copyright 2022 Guganana. All Rights Reserved.

#pragma once

#include <CoreMinimal.h>

#include "Handles/ExpressiveText.h"
#include "Widgets/SExpressiveTextRendererWidget.h"

#include <Components/Widget.h>
#include <Templates/SharedPointer.h>

#include "ExpressiveTextRendererWidget.generated.h"

UCLASS()
class EXPRESSIVETEXT_API UExpressiveTextRendererWidget : public UWidget
{
	GENERATED_BODY()
public:
	virtual TSharedRef<SWidget> RebuildWidget() override
	{
		Renderer = SNew(SExpressiveTextRendererWidget);
		return Renderer.ToSharedRef();
	}

	virtual void SynchronizeProperties() override
	{
		Super::SynchronizeProperties();
	}

	virtual void ReleaseSlateResources(bool bReleaseChildren) override
	{
		Renderer.Reset();
		Super::ReleaseSlateResources(bReleaseChildren);
	}

	void SetExpressiveText( FExpressiveText& Text)
	{
		if (Renderer)
		{
			Renderer->SetExpressiveText(Text);
		}
	}

	void SkipReveal()
	{
		if(Renderer)
		{
			Renderer->SkipReveal();
		}
	}

	void GetChronos( FExpressiveTextChronos& OutChronos ) const 
	{
		check(Renderer);
		Renderer->GetChronos(OutChronos);
	}

	TSharedPtr<SExpressiveTextRendererWidget> Renderer;
};
