// Copyright 2022 Guganana. All Rights Reserved.

#include "Parameters/ExpressiveTextParams.h"

#include "Subsystems/ExpressiveTextSubsystem.h"

#include <Engine/Engine.h>

#if WITH_EDITORONLY_DATA
void UExTextValue_Font::PostLoad()
{
	Super::PostLoad();
	EnsureFontIsSet();
}
#endif

#if WITH_EDITOR
void UExTextValue_Font::EnsureFontIsSet() const 
{
	if (Value == nullptr && !StoredFontName.IsNone())
	{
		if (GEngine && GEngine->IsInitialized())
		{
			if (auto* Subsystem = GEngine->GetEngineSubsystem<UExpressiveTextSubsystem>())
			{
				FString Message = FString::Printf(TEXT("'%s' is missing it's font"), *GetNameSafe(GetOuter()));
				auto FetchFutureResult = Subsystem->FetchFont(StoredFontName, Message);
				if (FetchFutureResult.IsSet())
				{
					auto* MutableThis = const_cast<UExTextValue_Font*>(this);
					const auto SetFontValue =
					[WeakThis = TWeakObjectPtr<UExTextValue_Font>(MutableThis)](UExpressiveTextFont* Font) 
					{
						if (WeakThis.IsValid())
						{
							WeakThis->Value = Font;
						}
					};

					FetchFutureResult->Next(SetFontValue);
				}
			}
		}
	}
}
#endif
