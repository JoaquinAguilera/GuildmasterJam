// Copyright 2022 Guganana. All Rights Reserved.

#include "Compiled/ExpressiveTextCompiler.h"

bool ExpressiveTextCompilerFlag::SpecialMode = false;

#define INIT_INTERJECTION(_Interjection_) \
	const FName InterjectionTypes::_Interjection_ = TEXT(#_Interjection_);

INIT_INTERJECTION( Pause )
INIT_INTERJECTION( P )
INIT_INTERJECTION( Action )
INIT_INTERJECTION( A )

#undef INIT_INTERJECTION