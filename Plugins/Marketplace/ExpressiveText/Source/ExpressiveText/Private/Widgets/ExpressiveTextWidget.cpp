// Copyright 2022 Guganana. All Rights Reserved.

#include "Widgets/ExpressiveTextWidget.h"
#include "Debug/ExpressiveTextDebugger.h"

void UExpressiveTextWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
}

void UExpressiveTextWidget::DisplayText(FExpressiveText& Text)
{
	Clear();

	const auto* Settings = GetDefault<UExpressiveTextSettings>();
	if (!Settings)
	{
		EXTEXT_LOG(Error, TEXT("Failed to retrieve UExpressiveTextSettings"));
		return;
	}

	if (!Renderer)
	{
		EXTEXT_LOG(Error, TEXT("Failed to fetch ExpressiveTextRendererWidget"));
		return;
	}

	float BlockRand = FMath::RandRange(0.f, 1.f);
	float LineRand = FMath::RandRange(0.f, 1.f); // currently unsuported, need to do this per line
	Renderer->SetExpressiveText(Text);
}

void UExpressiveTextWidget::Clear()
{
}
