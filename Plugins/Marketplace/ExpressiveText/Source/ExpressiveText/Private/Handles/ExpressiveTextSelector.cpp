// Copyright 2022 Guganana. All Rights Reserved.
#include "Handles/ExpressiveTextSelector.h"
#include "Asset/ExpressiveTextAsset.h"

int64 FExpressiveTextSelector::CalcChecksum() const
{
	int64 Result = 0;
	Result = HashCombine(Result, GetTypeHash(Asset) );

	if (Asset)
	{
		Result = HashCombine(Result, Asset->CalcChecksum());
	}

	Result += InlinedExpressiveText.CalcChecksum();

	if (InjectedText)
	{
		Result = HashCombine(Result, InjectedText.GetValue().CalcChecksum());
	}

	return Result;
}

FExpressiveText FExpressiveTextSelector::GenerateExpressiveText() const
{
	if (InjectedText.IsSet())
	{
		return InjectedText.GetValue();
	}

	FExpressiveText Result;

	if (Asset)
	{
		Result.SetFields(Asset->Fields);
	}
	else
	{
		Result.SetFields(InlinedExpressiveText);
	}

	return Result;
}
