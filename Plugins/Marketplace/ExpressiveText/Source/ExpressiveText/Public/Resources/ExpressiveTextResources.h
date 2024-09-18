// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>

class UExpressiveTextFont;
class UExpressiveTextStyle;
class UExpressiveTextAnimation;

template<typename InClass>
struct TExpressiveTextResource
{
	using Class = InClass;
	static UClass* GetClass()
	{
		return Class::StaticClass();
	}
	static FPrimaryAssetType AssetType() 
	{
		return FPrimaryAssetType(GetClass()->GetFName());
	}
};

namespace ExpressiveTextResource
{
	using Font = TExpressiveTextResource<UExpressiveTextFont>;
	using Style = TExpressiveTextResource<UExpressiveTextStyle>;
	using Animation = TExpressiveTextResource<UExpressiveTextAnimation>;
}