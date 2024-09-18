// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>
#include <Templates/SubclassOf.h>

#include "Parameters/ExpressiveTextParameterValue.h"

struct FExpressiveTextParameterCache
{
public:

	FExpressiveTextParameterCache()
		: LookupTable()
	{}

	const UExpressiveTextParameterValue* GetParameter( const TSubclassOf<UExpressiveTextParameterValue>& Type ) const
	{
		const TStrongObjectPtr<UExpressiveTextParameterValue>* Result = LookupTable.Find( Type );
		
		return Result ? Result->Get() : nullptr;
	}

	void Empty()
	{
		LookupTable.Empty();
	}

	void Add( UExpressiveTextParameterValue& Value )
	{
		LookupTable.Emplace( Value.GetClass(), &Value );
	}

private:
	TMap< TSubclassOf<UExpressiveTextParameterValue>, TStrongObjectPtr<UExpressiveTextParameterValue> > LookupTable;
};