// Copyright 2022 Guganana. All Rights Reserved.
#include "ExpressiveText/Public/Parameters/ExpressiveTextParameterLookup.h"

void FExpressiveTextParameterLookup::SetNext( const TSharedPtr<FExpressiveTextParameterLookup>& InNext )
{
	if( Next )
	{
		Next->SetNext(InNext);
	}
	else
	{
		Next = InNext;
	}
}