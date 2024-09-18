// Copyright 2022 Guganana. All Rights Reserved.
#pragma once

#include <CoreMinimal.h>
#include "ExText_Interjection.h"

class FExText_PauseInterjection : public FExText_Interjection
{
public:
    FExText_PauseInterjection(const FString& Parameter)
        : Duration( 0.f )
    {
        float ParsedValue = FCString::Atof(*Parameter);

        if( ParsedValue > 0.f )
        {
            Duration = ParsedValue;
            IsCorrectlyParsed = true;
        }
    }
protected:
    virtual void OnProcessModifiers( FInterjectionOutput& Out ) const override
    {
        Out.PauseDuration += Duration;
    }
private:
    float Duration;
};