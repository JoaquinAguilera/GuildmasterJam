// Copyright 2022 Guganana. All Rights Reserved.

#pragma once

#include <CoreMinimal.h>

#include "Handles/ExpressiveText.h"
#include "Layout/ExTextChronos.h"
#include "Widgets/ExpressiveTextRendererWidget.h"

#include <Blueprint/UserWidget.h>

#include "ExpressiveTextWidget.generated.h"

UCLASS()
class EXPRESSIVETEXT_API UExpressiveTextWidget : public UUserWidget
{
	GENERATED_BODY()

public:	

	UFUNCTION( BlueprintCallable, Category = ExpressiveText, meta=( DisplayName = "Display Text (Internal use only!)") )
	void DisplayText( UPARAM(ref) FExpressiveText& Text );
	
	UFUNCTION( BlueprintCallable, Category = ExpressiveText )
	void SkipReveal()
	{
		if( Renderer )
		{
			Renderer->SkipReveal();
		}
	}

	UFUNCTION( BlueprintCallable, Category = ExpressiveText )
	void GetChronos( FExpressiveTextChronos& OutChronos ) const
	{
		check(Renderer);
		Renderer->GetChronos(OutChronos);
	}

	void Clear();

protected:

	virtual void NativePreConstruct() override;

	UPROPERTY(BlueprintReadOnly, Category = ExpressiveText, meta = (BindWidget))
	UExpressiveTextRendererWidget* Renderer;
private:

};
