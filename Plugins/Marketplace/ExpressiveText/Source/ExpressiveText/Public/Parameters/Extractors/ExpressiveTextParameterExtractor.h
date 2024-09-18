// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>

class IExpressiveTextParameterExtractor
{
public:
	
	virtual ~IExpressiveTextParameterExtractor() {}

	virtual const UExpressiveTextParameterValue* GetValueObjectImpl( TSubclassOf<UExpressiveTextParameterValue> Class ) const = 0;
};