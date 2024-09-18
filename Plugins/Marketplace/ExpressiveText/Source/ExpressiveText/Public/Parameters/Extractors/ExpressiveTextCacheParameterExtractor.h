// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>
#include "ExpressiveTextParameterExtractor.h"

class FExpressiveTextCacheParameterExtractor : public IExpressiveTextParameterExtractor
{
public:
	
	FExpressiveTextCacheParameterExtractor( const TWeakPtr<FExpressiveTextParameterCache>& InCache )
		: Cache( InCache )
	{}

	virtual const UExpressiveTextParameterValue* GetValueObjectImpl( TSubclassOf<UExpressiveTextParameterValue> Class ) const override
	{
		if (auto* CacheRaw = Cache.Pin().Get())
		{
			return CacheRaw->GetParameter( Class );
		}

		return nullptr;
	}

private:
	TWeakPtr<FExpressiveTextParameterCache> Cache;
};