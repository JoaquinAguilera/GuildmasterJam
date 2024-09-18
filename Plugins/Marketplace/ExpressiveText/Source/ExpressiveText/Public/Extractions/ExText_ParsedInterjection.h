// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>

class FExText_Interjection;

struct FExText_ParsedInterjection 
{
    FExText_ParsedInterjection()
        : Index( -1 )
        , Interjection()
    {}

    int32 Index;
    TSharedPtr< FExText_Interjection > Interjection;
};