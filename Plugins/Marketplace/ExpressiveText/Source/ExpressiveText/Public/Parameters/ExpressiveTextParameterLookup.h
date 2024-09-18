// Copyright 2022 Guganana. All Rights Reserved.

#pragma once

#include <CoreMinimal.h>

#include <UObject/StrongObjectPtr.h>

#include "ExpressiveTextParameterCache.h"
#include "Extractors/ExpressiveTextParameterExtractor.h"
#include "Parameters/ExpressiveTextParameterValue.h"


struct FExpressiveTextParameterLookup
{
	FExpressiveTextParameterLookup() 
		: Descriptor()
		, Extractor( nullptr )
		, Next( nullptr ) 
	{}
	
	FExpressiveTextParameterLookup( const FName& Name, TSharedPtr<IExpressiveTextParameterExtractor> InExtractor )
		: Descriptor( Name )
		, Extractor( InExtractor )
		, Next( nullptr )
	{}

	void SetNext( const TSharedPtr<FExpressiveTextParameterLookup>& InNext );

    template< typename ValueObjectType, typename ValueType = typename ValueObjectType::ValueType >
    const ValueType& GetValue() const
    {
        const ValueObjectType& ValueObject = GetValueObject< ValueObjectType >();
        return ValueObject.Value;
    } 

    template< typename ValueObjectType >
    const ValueObjectType& GetValueObject() const
    {        
		return *CastChecked<ValueObjectType>( &GetValueObject( ValueObjectType::StaticClass() ) );
    }

	const UExpressiveTextParameterValue& GetValueObject( const TSubclassOf< UExpressiveTextParameterValue >& Class ) const
	{
		if (auto* ExtractorRaw = Extractor.Get())
		{
			if (auto* Result = ExtractorRaw->GetValueObjectImpl(Class))
			{
#if WITH_EDITOR
				Result->ConfirmUsage();
#endif
				return *Result;
			}
		}

		if (auto* NextRaw = Next.Get())
		{
			return Next->GetValueObject(Class);
		}

		//Reached the chain end - use CDO to derive value
		auto* CDO = Class.GetDefaultObject();
		check(CDO);
		return *CDO;
	}
	
private:
	FName Descriptor;
	TSharedPtr<IExpressiveTextParameterExtractor> Extractor;
	TSharedPtr<FExpressiveTextParameterLookup> Next;
};

