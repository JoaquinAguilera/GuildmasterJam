// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>

namespace Guganana
{
    class Async
    {
    public:

		template <typename T>
		static TFuture<T> MakeFullfiledFuture(const T& Value)
		{
			return MakeFulfilledPromise<T>(Value).GetFuture();
		}

		static TFuture<void> MakeFullfiledFuture()
		{
			return MakeFulfilledPromise<void>().GetFuture();
		}

		template <typename T>
		static TFuture<TArray<T>> WhenAllFutures(TArray<TFuture<T>>& Futures)
		{
			if (Futures.Num() == 0)
			{
				return MakeFullfiledFuture(TArray<T>());
			}

			TSharedRef<int32> Counter = MakeShareable(new int32(Futures.Num()));
			TSharedRef<TArray<T>> Values = MakeShareable(new TArray<T>());
			TSharedRef<TPromise<TArray<T>>> FinalPromise = MakeShareable(new TPromise<TArray<T>>());

			for (TFuture<T>& Future : Futures)
			{
				Future.Next(
					[Counter, Values, FinalPromise](const T& Value)
					{
						int32& CounterRaw = Counter.Get();
				TArray<T>& ValuesRaw = Values.Get();

				check(CounterRaw >= 0);

				ValuesRaw.Add(Value);

				if (--CounterRaw == 0)
				{
					FinalPromise->SetValue(Values.Get());
				}
					}
				);
			}

			return FinalPromise->GetFuture();
		}

		static TFuture<void> WhenAllFutures(TArray<TFuture<void>>& Futures)
		{
			if (Futures.Num() == 0)
			{
				return MakeFulfilledPromise<void>().GetFuture();
			}

			TSharedRef<int32> Counter = MakeShareable(new int32(Futures.Num()));
			TSharedRef<TPromise<void>> FinalPromise = MakeShareable(new TPromise<void>());

			for (TFuture<void>& Future : Futures)
			{
				Future.Then(
					[Counter, FinalPromise](auto)
					{
						int32& CounterRaw = Counter.Get();
				check(CounterRaw >= 0);
				if (--CounterRaw == 0)
				{
					FinalPromise->SetValue();
				}
					}
				);
			}

			return FinalPromise->GetFuture();
		}




    };
}
