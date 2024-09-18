// Copyright 2022 Guganana. All Rights Reserved.
#include "Components/ExpressiveTextComponent.h"

UExpressiveTextWidgetComponent::UExpressiveTextWidgetComponent() 
	: Super()
	, UserWidget()
{
	PrimaryComponentTick.bTickEvenWhenPaused = true;
	SetEditTimeUsable(true);
}

void UExpressiveTextWidgetComponent::BeginPlay()
{
	Super::BeginPlay();
	EnsureWidgetCreated();
}

void UExpressiveTextWidgetComponent::EnsureWidgetCreated()
{
	if (!UserWidget)
	{
		UserWidget = CreateWidget<UExpressiveTextWidget>(GetWorld(), GetDefault<UExpressiveTextSettings>()->ExpressiveTextWidgetClass.LoadSynchronous());
		check(UserWidget != nullptr);

		SetWidget(UserWidget);
	}
}

void UExpressiveTextWidgetComponent::OnRegister()
{
	Super::OnRegister();
}

void UExpressiveTextWidgetComponent::OnUnregister()
{
	UserWidget = nullptr;
	SetWidget(nullptr);
	Super::OnUnregister();
}
