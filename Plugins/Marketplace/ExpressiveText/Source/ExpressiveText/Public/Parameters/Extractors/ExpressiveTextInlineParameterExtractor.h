// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>
#include "ExpressiveTextParameterExtractor.h"

class FExpressiveTextInlineParameterExtractor : public IExpressiveTextParameterExtractor
{
public:

    template< typename ValueObjectType, typename ValueType = typename ValueObjectType::ValueType >
	static TSharedPtr<FExpressiveTextInlineParameterExtractor> Create( const ValueType& Value )
	{
		ValueObjectType* ValueObject = NewObject< ValueObjectType >();
		ValueObject->Value = Value;
		return MakeShareable( new FExpressiveTextInlineParameterExtractor( ValueObject ) );
	}

	FExpressiveTextInlineParameterExtractor( UExpressiveTextParameterValue* InValueObject )
		: ValueObject( InValueObject )
	{}

	virtual const UExpressiveTextParameterValue* GetValueObjectImpl( TSubclassOf<UExpressiveTextParameterValue> Class ) const override
	{
		if( auto* RawValueObject = ValueObject.Get() )
		{
			if( RawValueObject->GetClass() == Class )
			{
				return RawValueObject;
			}
		}

		return nullptr;
	}

private:

	TStrongObjectPtr<UExpressiveTextParameterValue> ValueObject;
};