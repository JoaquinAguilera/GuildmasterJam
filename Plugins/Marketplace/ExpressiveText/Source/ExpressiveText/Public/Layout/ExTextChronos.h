// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>

#include <Kismet/BlueprintFunctionLibrary.h>
#include <Misc/App.h>

#include "ExTextChronos.generated.h"

DECLARE_DYNAMIC_DELEGATE(FOnTextFullyRevealedDelegate);
	
USTRUCT( BlueprintType )
struct FExpressiveTextChronos
{
	GENERATED_BODY()
	
private:

	struct FInternal
	{
		FInternal()
			: CurrentTime(-1.0)
			, StartTime(0.0)
			, RevealDuration(-1.0)
			, PausedTime( 0.0 )
			, IsPaused( false )
			, OnTextFullyRevealed()
		{}

		double CurrentTime;
		double StartTime;
		double RevealDuration;
		double PausedTime;

		bool IsPaused;

		TArray<FOnTextFullyRevealedDelegate> OnTextFullyRevealed;
	};

	TSharedRef<FInternal> Internal;

public:
	FExpressiveTextChronos()
		: Internal( MakeShareable(new FInternal) )
	{}

	void SetRevealDuration(double RevealDuration)
	{
		Internal->RevealDuration = RevealDuration;
	}

	float GetRevealDuration() const
	{
		return Internal->RevealDuration;
	}

	float GetStartTime() const
	{
		return Internal->StartTime;
	}

	float GetTimePassed() const
	{
		return Internal->CurrentTime - Internal->StartTime;
	}

	float GetTimeSinceRevealEnd() const
	{
		return GetTimePassed() - Internal->RevealDuration;
	}

	void UpdateStartTime()
	{
		Internal->StartTime = FApp::GetCurrentTime() - GStartTime;
		Internal->PausedTime = 0.0;
	}
	
	void UpdateCurrentTime()
	{
		auto& Raw = Internal.Get();

		if (Raw.IsPaused)
		{
			Raw.PausedTime += FApp::GetDeltaTime();
		}

		Raw.CurrentTime = FApp::GetCurrentTime() - GStartTime - Raw.PausedTime;

		// Execute delegates if just finished revealing
		if (Raw.OnTextFullyRevealed.Num() > 0 && !IsRevealing())
		{
			for (const auto& Delegate : Raw.OnTextFullyRevealed)
			{
				Delegate.ExecuteIfBound();
			}

			Raw.OnTextFullyRevealed.Empty();
		}
	}

	void SkipToRevealEnd()
	{
		auto& Raw = Internal.Get();
		// To skip to end, we actually mutate the start time so the current time always holds true
		Raw.StartTime = FMath::Min( Raw.StartTime , Raw.CurrentTime - Raw.RevealDuration );
	}

	void SetChronosPaused(bool newValue)
	{
		auto& Raw = Internal.Get();
		Raw.IsPaused = newValue;
	}

	bool IsRevealing() const
	{
		auto& Raw = Internal.Get();
		return Raw.CurrentTime - Raw.StartTime < Raw.RevealDuration;
	}

	void BindOnTextFullyRevealed(FOnTextFullyRevealedDelegate NewFullyRevealedDelegate)
	{
		Internal->OnTextFullyRevealed.Add( NewFullyRevealedDelegate );
	}
};

UCLASS()
class UExpressiveTextChronosFunctions : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public: 

    UFUNCTION(BlueprintPure, Category = ExpressiveText)
    static float GetTimePassed( UPARAM(ref) const FExpressiveTextChronos& Chronos )
    {
		return Chronos.GetTimePassed();
	}

    UFUNCTION(BlueprintPure, Category = ExpressiveText)
	static float GetTimeSinceFullyRevealed( UPARAM(ref) const FExpressiveTextChronos& Chronos )
	{
		return Chronos.GetTimeSinceRevealEnd();
	}
	
    UFUNCTION(BlueprintPure, Category = ExpressiveText)
	static bool IsRevealing( UPARAM(ref) const FExpressiveTextChronos& Chronos )
	{
		return Chronos.IsRevealing();
	}

	UFUNCTION(BlueprintPure, Category = ExpressiveText)
	static float GetRevealDuration( UPARAM(ref) const FExpressiveTextChronos& Chronos )
	{
		return Chronos.GetRevealDuration();
	}

	UFUNCTION(BlueprintCallable, Category = ExpressiveText)
	static void SetRevealPaused(UPARAM(ref) FExpressiveTextChronos& Chronos, bool newValue)
	{
		Chronos.SetChronosPaused(newValue);
	}

	UFUNCTION(BlueprintCallable, Category = ExpressiveText)
	static void SkipToRevealEnd( UPARAM(ref) FExpressiveTextChronos& Chronos )
	{
		Chronos.SkipToRevealEnd();
	}

	UFUNCTION(BlueprintCallable, Category = ExpressiveText)
	static void BindOnTextFullyRevealed( UPARAM(ref) FExpressiveTextChronos& Chronos, FOnTextFullyRevealedDelegate Delegate )
	{
		Chronos.BindOnTextFullyRevealed(Delegate);
	}
	
	

	UFUNCTION(BlueprintCallable, Category = ExpressiveText)
	static void Reset( UPARAM(ref) FExpressiveTextChronos& Chronos)
	{
		Chronos.UpdateStartTime();
	}
};
